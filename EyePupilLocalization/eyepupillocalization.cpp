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
	ui.customPlot_x->yAxis->setRange(-50, 50);//���������귶Χ
	ui.customPlot_x->addGraph();//���ӵ�һ������
	ui.customPlot_x->graph(0)->setPen(QPen(Qt::red));//����Ϊ��ɫ
	ui.customPlot_x->addGraph();//������һ������
	ui.customPlot_x->graph(1)->setPen(QPen(Qt::blue));//����Ϊ��ɫ

	ui.customPlot_y->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);
	ui.customPlot_y->xAxis->setLabel("TIME");
	ui.customPlot_y->yAxis->setLabel("VERTICAL");
	ui.customPlot_y->yAxis->setRange(-50, 50);//����������
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(0)->setPen(QPen(Qt::red));
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(1)->setPen(QPen(Qt::blue));

	label_name = new QLabel;
	label_name->setText("    DESIGNED BY HJY     ");
	ui.statusBar->addWidget(label_name);
	label_time = new QLabel;
	label_time->setText("     May , 2017  ");
	ui.statusBar->addWidget(label_time);

	ui.customPlot_print->hide();
	ui.customPlot_print->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);//�������߷�Χ
	ui.customPlot_print->xAxis->setLabel("TIME");//���ú�����
	ui.customPlot_print->yAxis->setRange(-50, 50);//���������귶Χ
}

EyePupilLocalization::~EyePupilLocalization()
{
}

//����Ƶ
void EyePupilLocalization::on_pushButton_openvideo_clicked()
{
	fileName = QFileDialog::getOpenFileName(
		this,
		"Open Video",
		QDir::currentPath(),
		"Video files(*.avi)"
	);
	cv::VideoCapture capture(fileName.toStdString());
	if (!capture.isOpened())
	{
		QMessageBox::warning(this, "Warn", "No video selected");
		return;
	}
	//double rate = capture.get(CV_CAP_PROP_FPS);
	double numFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	QVector<double> TimeR, TimeL, Rx, Ry, Lx, Ly;
	OldFrameNum = numFrames;
	OldFrameR.clear();
	OldFrameL.clear();
	OldReyeX.clear();
	OldReyeY.clear();
	OldLeyeX.clear();
	OldLeyeY.clear();
	bool stop(false);
	cv::Mat frame;
	cv::Mat EyeShow = cv::imread("C:\\Users\\LZH\\Documents\\Visual Studio 2015\\Projects\\EyePupilLocalization\\EyePupilLocalization\\Resources\\eye.png", -1);
	cv::resize(EyeShow, EyeShow, cv::Size(120, 60), 0, 0);//���ô�С
	cv::namedWindow("Project");
	cv::imshow("Project", EyeShow);
	//cv::destroyAllWindows();
	ui.customPlot_x->xAxis->setRange(0, numFrames);//���ú�����
	ui.customPlot_y->xAxis->setRange(0, numFrames);//���ú�����

	bool IsReyeCenter(false);//�Ƿ����۳�����һ����λ����
	bool IsLeyeCenter(false);//�Ƿ����۳�����һ����λ����
	int FrameNum = 0;//��Ƶ����֡�Ĵ���
	CvPoint ReyeCenter;//��������
	CvPoint LeyeCenter;//��������
	while (!stop)
	{
		if (!capture.read(frame))
		{
			break;
		}
		++FrameNum;
		if (FrameNum == numFrames)
		{
			stop = true;
		}
		ImgProcess pro(frame,1.7);
		pro.Process();

		Leye = pro.OutLeye();
		Reye = pro.OutReye();

		Limg = Mat2QImage(Leye);
		Rimg = Mat2QImage(Reye);
		ui.label_Leye->setPixmap(QPixmap::fromImage(Limg));//��ʾ����
		ui.label_Reye->setPixmap(QPixmap::fromImage(Rimg));

		//��������
		for (cv::Vec3f box : pro.circles)
		{
			if (box[0] < frame.cols / 2)//X����С��һ��λ��Ϊ����
			{
				//����
				ui.lcdNumber_Lx->display(floor(box[0]));
				ui.lcdNumber_Ly->display(floor(box[1]));
				ui.lcdNumber_Lr->display(floor(box[2]));
				if (!IsLeyeCenter)
				{
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
					ui.customPlot_x->graph(0)->addData(FrameNum, box[0] - LeyeCenter.x);
					ui.customPlot_y->graph(0)->addData(FrameNum, box[1] - LeyeCenter.y);
					OldLeyeX.push_back(box[0] - LeyeCenter.x);
					OldLeyeY.push_back(box[1] - LeyeCenter.y);
				}
				OldFrameL.push_back(FrameNum);
			}
			else
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
				}
				OldFrameR.push_back(FrameNum);
			}
		}
		ui.customPlot_x->replot();
		ui.customPlot_y->replot();
		cv::waitKey(1);
	}
	cv::destroyWindow("Project");
}

//������ͷ
void EyePupilLocalization::on_pushButton_opencamera_clicked()
{
	cv::VideoCapture vcap;
	QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Choose", "Do you want to open PC camera?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	if (rb == QMessageBox::Yes)
	{
		if (!vcap.open(0))
		{
			QMessageBox::warning(this, "Warn", "No Camera");
			return;
		}
	}
	else
	{
		if (!vcap.open(videoStreamAddress))
		{
			QMessageBox::warning(this, "Warn", "No Camera");
			return;
		}
	}
	cv::namedWindow("Camera Video");
	bool stop(false);
	cv::Mat frame;
	while (!stop)
	{
		if (!vcap.read(frame))
		{
			break;
		}
		cv::imshow("Camera Video", frame);
		if (cv::waitKey(5) == 27)
		{
			break;//����ESE�˳�
		}
	}
	cv::destroyWindow("Camera Video");
}

//��ӡ
void EyePupilLocalization::on_pushButton_print_clicked()
{
	if (OldFrameNum == 0)
	{
		QMessageBox::information(this, "Warn", "No Waveforms");
		return;
	}
	//QPrinter printer(QPrinter::HighResolution);
	//printer.setPageSize(QPrinter::A4);
	//printer.setPaperSize(QSizeF(ui.customPlot_print->height() * 3, ui.customPlot_print->width()), QPrinter::Point);

	QPainter painter(&printer);
	QRect rect = painter.viewport();
	QSize size = ui.customPlot_print->size();
	size.scale(rect.size(), Qt::KeepAspectRatio);
	painter.setViewport(rect.x(), rect.y(), size.width(), size.width());//�������һ������Ū��width
	plotWight(true);
	ui.customPlot_print->render(&painter);
	painter.setViewport(rect.x(), rect.y() + size.height(), size.width(), rect.y() + size.width());
	plotWight(false);
	ui.customPlot_print->render(&painter);
	painter.end();
	//QPrintPreviewDialog preview(&printer, this);//��ӡԤ��
	//QPrintDialog printdlg(&printer, this);//��ӡԤ��
	//preview.showNormal();
	//preview.setWindowTitle(tr("Print Waveform"));
	//connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printView(QPrinter*)));
	//preview.exec();
}

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
	ui.customPlot_print->replot();
}


