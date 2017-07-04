#include "eyepupillocalization.h"

EyePupilLocalization::EyePupilLocalization(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//��ʼ������
	ui.customPlot_x->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);//�������߷�Χ
	ui.customPlot_x->xAxis->setLabel("TIME");//���ú�����
	ui.customPlot_x->yAxis->setLabel("LEVEL");//����������
	ui.customPlot_x->yAxis->setRange(-70, 70);//���������귶Χ
	ui.customPlot_x->addGraph();//���ӵ�һ������
	ui.customPlot_x->graph(0)->setPen(QPen(Qt::red));//����Ϊ��ɫ
	ui.customPlot_x->addGraph();//������һ������
	ui.customPlot_x->graph(1)->setPen(QPen(Qt::blue));//����Ϊ��ɫ

	ui.customPlot_y->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);
	ui.customPlot_y->xAxis->setLabel("TIME");
	ui.customPlot_y->yAxis->setLabel("VERTICAL");
	ui.customPlot_y->yAxis->setRange(-70, 70);//����������
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(0)->setPen(QPen(Qt::red));
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(1)->setPen(QPen(Qt::blue));	

	ui.customPlot_print->hide();
	ui.customPlot_print->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);//�������߷�Χ
	ui.customPlot_print->xAxis->setLabel("TIME");//���ú�����
	ui.customPlot_print->yAxis->setRange(-70, 70);//���������귶Χ

	TESTtime = QDateTime::currentDateTime();//��ȡϵͳ���ڵ�ʱ��
	str_TESTtime= TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //������ʾ��ʽ

	QSettings *configIniRead = new QSettings("config.ini", QSettings::IniFormat);//��ȡINI�����ļ�
	videoStreamAddressLeft = configIniRead->value("/WebCam/addressL").toString().toStdString();//��������ļ��е���������ͷ��ַ,����
	videoStreamAddressRight = configIniRead->value("/WebCam/addressR").toString().toStdString();//����
	
	QImage image;
	image.load(":/EyePupilLocalization/NoVedio");
	NoVideoImage = image.scaled(ui.label_Leye->width(), ui.label_Leye->height());
	NoVedio = QImage2Mat(NoVideoImage);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(readFarme()));//�����źŲ�

	ui.Button_closecamera->setEnabled(false);

	ui.statusBar->showMessage(tr("Ready"));
}
EyePupilLocalization::~EyePupilLocalization()
{
}
//����Ƶ
void EyePupilLocalization::on_pushButton_openvideo_clicked()
{
	ui.statusBar->showMessage(QString::fromLocal8Bit("���ڴ򿪱�����Ƶ"));
	fileName = QFileDialog::getOpenFileName(
		this,
		"Open Video",
		QDir::currentPath(),
		"Video files(*.avi)"
	);
	capture.open(fileName.toStdString());
	if (!capture.isOpened())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("û��ѡ�б�����Ƶ"));
		ui.statusBar->showMessage(NULL);
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("���ڴ�������Ƶ"));
	TESTtime = QDateTime::currentDateTime();//��ȡϵͳ���ڵ�ʱ��
	str_TESTtime = TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //������ʾ��ʽ
	double numFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	//OldFrameNum = numFrames;
	OldFrameNum = 0;
	OldFrameR.clear();//�����һ����¼
	OldFrameL.clear();
	OldReyeX.clear();
	OldReyeY.clear();
	OldLeyeX.clear();
	OldLeyeY.clear();
	
	ui.customPlot_x->xAxis->setRange(0, numFrames);//���ú�����
	ui.customPlot_y->xAxis->setRange(0, numFrames);//���ú�����
	ui.customPlot_x->yAxis->setRange(-70, 70);//���������귶Χ
	ui.customPlot_y->yAxis->setRange(-70, 70);//���������귶Χ

	IsReyeCenter = false;
	IsLeyeCenter = false;
	FrameNum = 0;//��Ƶ����֡�Ĵ���

	TimeR.clear();//��ջ�ͼ������
	TimeL.clear();
	Rx.clear();
	Ry.clear();
	Lx.clear();
	Ly.clear();

	ui.customPlot_x->graph(0)->setData(TimeL, Lx);
	ui.customPlot_y->graph(0)->setData(TimeL, Ly);
	ui.customPlot_x->graph(1)->setData(TimeR, Rx);
	ui.customPlot_y->graph(1)->setData(TimeR, Ry);

	//�����ʼ��
	Pic_Leye = cv::imread(".\\Resources\\map.jpg", -1);
	Pic_Reye = cv::imread(".\\Resources\\map.jpg", -1);
	cv::resize(Pic_Leye, Pic_Leye, cv::Size(130, 130));
	cv::resize(Pic_Reye, Pic_Reye, cv::Size(130, 130));
	PicImgReye = Mat2QImage(Pic_Reye);
	ui.label_ReyePosition->setPixmap(QPixmap::fromImage(PicImgReye));
	PicImgLeye = Mat2QImage(Pic_Leye);
	ui.label_LeyePosition->setPixmap(QPixmap::fromImage(PicImgLeye));

	//EyeNum = VEDIO_EYE;//��˫�۱�����Ƶ
	EyeNum = VEDIO_ONLY_EYE;//�򿪵��۱�����Ƶ
	timer->start(30);
}
//������ͷ
void EyePupilLocalization::on_pushButton_opencamera_clicked()
{
	ui.statusBar->showMessage(QString::fromLocal8Bit("���ڴ�����ͷ"));
	
	QMessageBox::StandardButton rb = QMessageBox::question(NULL, QString::fromLocal8Bit("ѡ��"), QString::fromLocal8Bit("�Ƿ�򿪱�������ͷ��"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);//��ʾѡ�����һ������ͷ
	if (rb == QMessageBox::Yes)//�򿪱���PC�ϵ�����ͷ
	{
		EyeNum = NOT_REYE;
		frameR = NoVedio;
		if (!vcapLeft.open(0))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("û�б�������ͷ"));
			EyeNum = NOT_ALLEYE;
			ui.statusBar->showMessage(NULL);
			return;
		}
	}
	else if (rb == QMessageBox::No)//����������ͷ
	{
		EyeNum = ALL_EYE;
		if (!vcapLeft.open(videoStreamAddressLeft))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��������ͷ����ʧ��"));
			EyeNum = NOT_LEYE;
			frameL = NoVedio;
		}

		if (!vcapRight.open(videoStreamAddressRight))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��������ͷ����ʧ��"));
			if (EyeNum == ALL_EYE)
			{
				EyeNum = NOT_REYE;
			}
			else
			{
				EyeNum = NOT_ALLEYE;
			}
			frameR = NoVedio;
		}

		if (EyeNum == NOT_ALLEYE)
		{
			//����ͷ�򿪶����ɹ�
			ui.statusBar->showMessage(NULL);
			return;
		}
	}
	else//ȡ����
	{
		ui.statusBar->showMessage(NULL);
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("���ڴ���ʵʱ��Ƶ"));
	TESTtime = QDateTime::currentDateTime();//��ȡϵͳ���ڵ�ʱ��
	str_TESTtime = TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //������ʾ��ʽ

	OldFrameNum = 0;//�����һ����¼
	OldFrameR.clear();
	OldFrameL.clear();
	OldReyeX.clear();
	OldReyeY.clear();
	OldLeyeX.clear();
	OldLeyeY.clear();

	TimeR.clear();//��ջ�ͼ������
	TimeL.clear();
	Rx.clear();
	Ry.clear();
	Lx.clear();
	Ly.clear();

	//EyeNum = ALL_EYE;//�ٶ�һ��ʼ�����۾�����
	IsReyeCenter = false;
	IsLeyeCenter = false;
	FrameNum = 0;//��Ƶ����֡�Ĵ���

	ui.customPlot_x->graph(0)->setData(TimeL, Lx);
	ui.customPlot_y->graph(0)->setData(TimeL, Ly);
	ui.customPlot_x->graph(1)->setData(TimeR, Rx);
	ui.customPlot_y->graph(1)->setData(TimeR, Ry);

	//�����ʼ��
	Pic_Leye = cv::imread(".\\Resources\\map.jpg", -1);
	Pic_Reye = cv::imread(".\\Resources\\map.jpg", -1);
	cv::resize(Pic_Leye, Pic_Leye, cv::Size(130, 130));
	cv::resize(Pic_Reye, Pic_Reye, cv::Size(130, 130));
	PicImgReye = Mat2QImage(Pic_Reye);
	ui.label_ReyePosition->setPixmap(QPixmap::fromImage(PicImgReye));
	PicImgLeye = Mat2QImage(Pic_Leye);
	ui.label_LeyePosition->setPixmap(QPixmap::fromImage(PicImgLeye));


	ui.Button_closecamera->setEnabled(true);
	ui.Button_opencamera->setEnabled(false);
	timer->start(30);
}
//��ȡ��Ƶ֡��
void EyePupilLocalization::readFarme()
{
	if (EyeNum == NOT_LEYE || EyeNum == ALL_EYE)
	{
		//��ʱ������
		vcapRight >> frameR;
	}
	if (EyeNum == NOT_REYE || EyeNum == ALL_EYE)
	{
		//��ʱ������
		vcapLeft >> frameL;
	}
	if (EyeNum == VEDIO_EYE)
	{
		//˫�۱�����Ƶ
		if (!capture.read(frameAll))//��ȡ��Ƶÿ֡
		{
			timer->stop();
			ui.statusBar->showMessage(QString::fromLocal8Bit("��Ƶ�������"));
			return;//��û��֡����ȡʱ���˳�ѭ��
		}
	}
	if (EyeNum == VEDIO_ONLY_EYE)
	{
		//���۱�����Ƶ
		if (!capture.read(frameL))//��ȡ��Ƶÿ֡
		{
			timer->stop();
			ui.statusBar->showMessage(QString::fromLocal8Bit("��Ƶ�������"));
			return;//��û��֡����ȡʱ���˳�ѭ��
		}
	}
	++FrameNum;
	++OldFrameNum;
	ImgProcess pro;
	if (EyeNum == VEDIO_EYE)
	{
		//�򿪱�����Ƶ
		pro.Start(frameAll, 1.5);
		pro.Process();
	}
	else
	{
		//ʵʱ��ʾ
		pro.Start(frameL, frameR, 1.5, EyeNum);
		pro.ProcessSignal();
	}

	if (EyeNum == NOT_LEYE)
	{
		//���û������
		Limg = NoVideoImage;
	}
	else
	{
		Leye = pro.OutLeye();//�������
		Limg = Mat2QImage(Leye);//������MAT����װΪIMAGE����
	}
	if (EyeNum == NOT_REYE || EyeNum == VEDIO_ONLY_EYE)
	{
		//���û������
		Rimg = NoVideoImage;
	}
	else
	{
		Reye = pro.OutReye();//�������
		Rimg = Mat2QImage(Reye);//������MAT����װΪIMAGE����
	}

	ui.label_Leye->setPixmap(QPixmap::fromImage(Limg));//�ڳ�����潫������ʾ����
	ui.label_Reye->setPixmap(QPixmap::fromImage(Rimg));//�ڳ�����潫������ʾ����

	for (cv::Vec3f box : pro.Lcircles)
	{
		//����
		ui.lcdNumber_Lx->display(floor(box[0]));
		ui.lcdNumber_Ly->display(floor(box[1]));
		ui.lcdNumber_Lr->display(floor(box[2]));
		if (!IsLeyeCenter)
		{
			//��һ֡��Ϊ����ԭ��
			LeyeCenter.x = box[0];
			LeyeCenter.y = box[1];
			IsLeyeCenter = true;
			Lx.push_back(0);
			Ly.push_back(0);
			TimeL.push_back(FrameNum);
			ui.customPlot_x->graph(0)->setData(TimeL, Lx);
			ui.customPlot_y->graph(0)->setData(TimeL, Ly);
			OldLeyeX.push_back(0);
			OldLeyeY.push_back(0);
		}
		else
		{
			//������Ե�ַ�ǻ��ڵ�һ֡λ�õ�
			ui.customPlot_x->graph(0)->addData(FrameNum, box[0] - LeyeCenter.x);
			ui.customPlot_y->graph(0)->addData(FrameNum, box[1] - LeyeCenter.y);

			OldLeyeX.push_back(box[0] - LeyeCenter.x);
			OldLeyeY.push_back(box[1] - LeyeCenter.y);

			//���۶�λ����
			plx = (box[0] - LeyeCenter.x) * 1.5 + 65;
			ply = (box[1] - LeyeCenter.y) * 1.5 + 65;
			Lcenter.x = plx;
			Lcenter.y = ply;
			cv::circle(Pic_Leye, Lcenter, 1, cv::Scalar(255, 0, 0), -1, 8);//��ͫ������ 
			PicImgLeye = Mat2QImage(Pic_Leye);
			ui.label_LeyePosition->setPixmap(QPixmap::fromImage(PicImgLeye));
		}
		OldFrameL.push_back(FrameNum);
		
	}
	for (cv::Vec3f box : pro.Rcircles)
	{
		//����
		ui.lcdNumber_Rx->display(floor(box[0]));
		ui.lcdNumber_Ry->display(floor(box[1]));
		ui.lcdNumber_Rr->display(floor(box[2]));
		if (!IsReyeCenter)
		{
			ReyeCenter.x = box[0];
			ReyeCenter.y = box[1];
			IsReyeCenter = true;
			Rx.push_back(0);
			Ry.push_back(0);
			TimeR.push_back(FrameNum);
			ui.customPlot_x->graph(1)->setData(TimeR, Rx);
			ui.customPlot_y->graph(1)->setData(TimeR, Ry);
			OldReyeX.push_back(0);
			OldReyeY.push_back(0);
		}
		else
		{
			ui.customPlot_x->graph(1)->addData(FrameNum, box[0] - ReyeCenter.x);
			ui.customPlot_y->graph(1)->addData(FrameNum, box[1] - ReyeCenter.y);
			OldReyeX.push_back(box[0] - ReyeCenter.x);
			OldReyeY.push_back(box[1] - ReyeCenter.y);

			//���۶�λ
			prx = (box[0] - ReyeCenter.x) * 1.5 + 65;
			pry = (box[1] - ReyeCenter.y) * 1.5 + 65;
			Rcenter.x = prx;
			Rcenter.y = pry;
			cv::circle(Pic_Reye, Rcenter, 1, cv::Scalar(255, 0, 0), -1, 8);//��ͫ������ 
			PicImgReye = Mat2QImage(Pic_Reye);
			ui.label_ReyePosition->setPixmap(QPixmap::fromImage(PicImgReye));
		}
		OldFrameR.push_back(FrameNum);
	}
	if (EyeNum != VEDIO_EYE)
	{
		ui.customPlot_x->rescaleAxes();//�Զ���������
		ui.customPlot_y->rescaleAxes();//�Զ���������
	}

	ui.customPlot_x->replot();//�ػ�x���겨��ͼ
	ui.customPlot_y->replot();//�ػ�y���겨��ͼ
}
//�ر�����ͷ
void EyePupilLocalization::on_pushButton_closecamera_clicked()
{
	timer->stop();
	ui.statusBar->showMessage(QString::fromLocal8Bit("��Ƶ�������"));
	if (EyeNum == NOT_LEYE || EyeNum == ALL_EYE)
	{
		//��ʱֻ������
		vcapRight.release();//�ͷ���Դ
	}
	if (EyeNum == NOT_REYE || EyeNum == ALL_EYE)
	{
		//��ʱֻ������
		vcapLeft.release();
	}
	ui.Button_closecamera->setEnabled(false);
	ui.Button_opencamera->setEnabled(true);
}
//��ӡ
void EyePupilLocalization::on_pushButton_print_clicked()
{
	if (OldFrameNum == 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("û����Ҫ��ӡ�Ĳ���"));
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("׼����ӡ"));
	QPrinter printer;//�½���ӡ������
	printer.setPageSize(QPrinter::A4);//���ô�ӡΪA4ֽ��
	QPrintPreviewDialog preview(&printer, this);
	connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreviewSlot(QPrinter*)));
	preview.exec();
}
//��ӡԤ��
void EyePupilLocalization::printPreviewSlot(QPrinter * printerPixmap)
{
	//���ֱ�עλ��
	QRect rect_level(0, 20, 800, 30);
	QRect rect_vertical(0, 520, 800, 30);
	//ʱ�䱸עλ��
	QRect rect_time(0, 1020, 700, 30);
	//��ȡ������Ƭ
	plotWight(true);
	QPixmap pixmap_level = QPixmap::grabWidget(ui.customPlot_print, ui.customPlot_print->rect());//����ˮƽ����ͼ
	plotWight(false);
	QPixmap pixmap_vertical = QPixmap::grabWidget(ui.customPlot_print, ui.customPlot_print->rect());//������ֱ����ͼ
	QPainter painterPixmap(this);
	painterPixmap.begin(printerPixmap);
	//��������
	QFont font_lable("Arial", 20, QFont::Bold, false);//���ñ�ǩ��������ͣ���С���Ӵ֣���б��
	QFont font_time("Arial", 15, QFont::Bold, false);//����ʱ����������ͣ���С���Ӵ֣���б��
	painterPixmap.setFont(font_lable);
	painterPixmap.drawText(rect_level, Qt::AlignHCenter, tr("LEVEL"));
	painterPixmap.drawText(rect_vertical, Qt::AlignHCenter, tr("VERTICAL"));
	painterPixmap.setFont(font_time);
	painterPixmap.drawText(rect_time, Qt::AlignRight, str_TESTtime);
	//���Ʋ���
	painterPixmap.drawPixmap(30, 50, pixmap_level);
	painterPixmap.drawPixmap(30, 550, pixmap_vertical);
	painterPixmap.end();
	ui.statusBar->showMessage(NULL);
}
//���Ʋ���
void EyePupilLocalization::plotWight(bool IsLevel)
{
	ui.customPlot_print->addGraph();//���ӵ�һ������
	ui.customPlot_print->xAxis->setRange(0, OldFrameNum);//���ú�����
	ui.customPlot_print->graph(0)->setPen(QPen(Qt::red));//����Ϊ��ɫ
	ui.customPlot_print->addGraph();//������һ������
	ui.customPlot_print->graph(1)->setPen(QPen(Qt::blue));//����Ϊ��ɫ
	if (IsLevel)
	{
		//ˮƽλ��
		ui.customPlot_print->yAxis->setLabel("LEVEL");
		ui.customPlot_print->graph(0)->setData(OldFrameL, OldLeyeX);
		ui.customPlot_print->graph(1)->setData(OldFrameR, OldReyeX);
	}
	else
	{
		//��ֱλ��
		ui.customPlot_print->yAxis->setLabel("VERTICAL");
		ui.customPlot_print->graph(0)->setData(OldFrameL, OldLeyeY);
		ui.customPlot_print->graph(1)->setData(OldFrameR, OldReyeY);
	}
	ui.customPlot_print->rescaleAxes();//�Զ���������
	ui.customPlot_print->rescaleAxes();//�Զ���������
	ui.customPlot_print->replot();
}


