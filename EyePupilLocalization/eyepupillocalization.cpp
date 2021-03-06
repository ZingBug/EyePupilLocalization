#include "eyepupillocalization.h"

EyePupilLocalization::EyePupilLocalization(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//初始化曲线
	ui.customPlot_x->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);//创建曲线范围
	ui.customPlot_x->xAxis->setLabel("TIME");//设置横坐标
	ui.customPlot_x->yAxis->setLabel("LEVEL");//设置纵坐标
	ui.customPlot_x->yAxis->setRange(-70, 70);//设置纵坐标范围
	ui.customPlot_x->addGraph();//增加第一条曲线
	ui.customPlot_x->graph(0)->setPen(QPen(Qt::red));//设置为红色
	ui.customPlot_x->addGraph();//在增加一条曲线
	ui.customPlot_x->graph(1)->setPen(QPen(Qt::blue));//设置为蓝色

	ui.customPlot_y->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);
	ui.customPlot_y->xAxis->setLabel("TIME");
	ui.customPlot_y->yAxis->setLabel("VERTICAL");
	ui.customPlot_y->yAxis->setRange(-70, 70);//设置纵坐标
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(0)->setPen(QPen(Qt::red));
	ui.customPlot_y->addGraph();
	ui.customPlot_y->graph(1)->setPen(QPen(Qt::blue));	

	ui.customPlot_print->hide();
	ui.customPlot_print->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);//创建曲线范围
	ui.customPlot_print->xAxis->setLabel("TIME");//设置横坐标
	ui.customPlot_print->yAxis->setRange(-70, 70);//设置纵坐标范围

	TESTtime = QDateTime::currentDateTime();//获取系统现在的时间
	str_TESTtime= TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //设置显示格式

	QSettings *configIniRead = new QSettings("config.ini", QSettings::IniFormat);//读取INI配置文件
	videoStreamAddressLeft = configIniRead->value("/WebCam/addressL").toString().toStdString();//获得配置文件中的网络摄像头地址,左眼
	videoStreamAddressRight = configIniRead->value("/WebCam/addressR").toString().toStdString();//右眼
	
	QImage image;
	image.load(":/EyePupilLocalization/NoVedio");
	NoVideoImage = image.scaled(ui.label_Leye->width(), ui.label_Leye->height());
	NoVedio = QImage2Mat(NoVideoImage);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(readFarme()));//建立信号槽

	ui.Button_closecamera->setEnabled(false);

	ui.statusBar->showMessage(tr("Ready"));
}
EyePupilLocalization::~EyePupilLocalization()
{
}
//打开视频
void EyePupilLocalization::on_pushButton_openvideo_clicked()
{
	ui.statusBar->showMessage(QString::fromLocal8Bit("正在打开本地视频"));
	fileName = QFileDialog::getOpenFileName(
		this,
		"Open Video",
		QDir::currentPath(),
		"Video files(*.avi)"
	);
	capture.open(fileName.toStdString());
	if (!capture.isOpened())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("没有选中本地视频"));
		ui.statusBar->showMessage(NULL);
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("正在处理本地视频"));
	TESTtime = QDateTime::currentDateTime();//获取系统现在的时间
	str_TESTtime = TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //设置显示格式
	double numFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	//OldFrameNum = numFrames;
	OldFrameNum = 0;
	OldFrameR.clear();//清空上一条记录
	OldFrameL.clear();
	OldReyeX.clear();
	OldReyeY.clear();
	OldLeyeX.clear();
	OldLeyeY.clear();
	
	ui.customPlot_x->xAxis->setRange(0, numFrames);//设置横坐标
	ui.customPlot_y->xAxis->setRange(0, numFrames);//设置横坐标
	ui.customPlot_x->yAxis->setRange(-70, 70);//设置纵坐标范围
	ui.customPlot_y->yAxis->setRange(-70, 70);//设置纵坐标范围

	IsReyeCenter = false;
	IsLeyeCenter = false;
	FrameNum = 0;//视频处理帧的次数

	TimeR.clear();//清空绘图点坐标
	TimeL.clear();
	Rx.clear();
	Ry.clear();
	Lx.clear();
	Ly.clear();

	ui.customPlot_x->graph(0)->setData(TimeL, Lx);
	ui.customPlot_y->graph(0)->setData(TimeL, Ly);
	ui.customPlot_x->graph(1)->setData(TimeR, Rx);
	ui.customPlot_y->graph(1)->setData(TimeR, Ry);

	//坐标初始化
	Pic_Leye = cv::imread(".\\Resources\\map.jpg", -1);
	Pic_Reye = cv::imread(".\\Resources\\map.jpg", -1);
	cv::resize(Pic_Leye, Pic_Leye, cv::Size(130, 130));
	cv::resize(Pic_Reye, Pic_Reye, cv::Size(130, 130));
	PicImgReye = Mat2QImage(Pic_Reye);
	ui.label_ReyePosition->setPixmap(QPixmap::fromImage(PicImgReye));
	PicImgLeye = Mat2QImage(Pic_Leye);
	ui.label_LeyePosition->setPixmap(QPixmap::fromImage(PicImgLeye));

	//EyeNum = VEDIO_EYE;//打开双眼本地视频
	EyeNum = VEDIO_ONLY_EYE;//打开单眼本地视频
	timer->start(30);
}
//打开摄像头
void EyePupilLocalization::on_pushButton_opencamera_clicked()
{
	ui.statusBar->showMessage(QString::fromLocal8Bit("正在打开摄像头"));
	
	QMessageBox::StandardButton rb = QMessageBox::question(NULL, QString::fromLocal8Bit("选择"), QString::fromLocal8Bit("是否打开本地摄像头？"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);//提示选择打开哪一个摄像头
	if (rb == QMessageBox::Yes)//打开本机PC上的摄像头
	{
		EyeNum = NOT_REYE;
		frameR = NoVedio;
		if (!vcapLeft.open(0))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("没有本地摄像头"));
			EyeNum = NOT_ALLEYE;
			ui.statusBar->showMessage(NULL);
			return;
		}
	}
	else if (rb == QMessageBox::No)//打开网络摄像头
	{
		EyeNum = ALL_EYE;
		if (!vcapLeft.open(videoStreamAddressLeft))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("左眼摄像头连接失败"));
			EyeNum = NOT_LEYE;
			frameL = NoVedio;
		}

		if (!vcapRight.open(videoStreamAddressRight))
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("右眼摄像头连接失败"));
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
			//摄像头打开都不成功
			ui.statusBar->showMessage(NULL);
			return;
		}
	}
	else//取消打开
	{
		ui.statusBar->showMessage(NULL);
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("正在处理实时视频"));
	TESTtime = QDateTime::currentDateTime();//获取系统现在的时间
	str_TESTtime = TESTtime.toString("yyyy-MM-dd hh:mm:ss ddd"); //设置显示格式

	OldFrameNum = 0;//清空上一条记录
	OldFrameR.clear();
	OldFrameL.clear();
	OldReyeX.clear();
	OldReyeY.clear();
	OldLeyeX.clear();
	OldLeyeY.clear();

	TimeR.clear();//清空绘图点坐标
	TimeL.clear();
	Rx.clear();
	Ry.clear();
	Lx.clear();
	Ly.clear();

	//EyeNum = ALL_EYE;//假定一开始两个眼睛都有
	IsReyeCenter = false;
	IsLeyeCenter = false;
	FrameNum = 0;//视频处理帧的次数

	ui.customPlot_x->graph(0)->setData(TimeL, Lx);
	ui.customPlot_y->graph(0)->setData(TimeL, Ly);
	ui.customPlot_x->graph(1)->setData(TimeR, Rx);
	ui.customPlot_y->graph(1)->setData(TimeR, Ry);

	//坐标初始化
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
//读取视频帧数
void EyePupilLocalization::readFarme()
{
	if (EyeNum == NOT_LEYE || EyeNum == ALL_EYE)
	{
		//此时有右眼
		vcapRight >> frameR;
	}
	if (EyeNum == NOT_REYE || EyeNum == ALL_EYE)
	{
		//此时有左眼
		vcapLeft >> frameL;
	}
	if (EyeNum == VEDIO_EYE)
	{
		//双眼本地视频
		if (!capture.read(frameAll))//读取视频每帧
		{
			timer->stop();
			ui.statusBar->showMessage(QString::fromLocal8Bit("视频处理结束"));
			return;//当没有帧数读取时就退出循环
		}
	}
	if (EyeNum == VEDIO_ONLY_EYE)
	{
		//单眼本地视频
		if (!capture.read(frameL))//读取视频每帧
		{
			timer->stop();
			ui.statusBar->showMessage(QString::fromLocal8Bit("视频处理结束"));
			return;//当没有帧数读取时就退出循环
		}
	}
	++FrameNum;
	++OldFrameNum;
	ImgProcess pro;
	if (EyeNum == VEDIO_EYE)
	{
		//打开本地视频
		pro.Start(frameAll, 1.5);
		pro.Process();
	}
	else
	{
		//实时显示
		pro.Start(frameL, frameR, 1.5, EyeNum);
		pro.ProcessSignal();
	}

	if (EyeNum == NOT_LEYE)
	{
		//如果没有左眼
		Limg = NoVideoImage;
	}
	else
	{
		Leye = pro.OutLeye();//输出左眼
		Limg = Mat2QImage(Leye);//将左眼MAT类型装为IMAGE类型
	}
	if (EyeNum == NOT_REYE || EyeNum == VEDIO_ONLY_EYE)
	{
		//如果没有右眼
		Rimg = NoVideoImage;
	}
	else
	{
		Reye = pro.OutReye();//输出右眼
		Rimg = Mat2QImage(Reye);//将右眼MAT类型装为IMAGE类型
	}

	ui.label_Leye->setPixmap(QPixmap::fromImage(Limg));//在程序界面将左眼显示出来
	ui.label_Reye->setPixmap(QPixmap::fromImage(Rimg));//在程序界面将右眼显示出来

	for (cv::Vec3f box : pro.Lcircles)
	{
		//左眼
		ui.lcdNumber_Lx->display(floor(box[0]));
		ui.lcdNumber_Ly->display(floor(box[1]));
		ui.lcdNumber_Lr->display(floor(box[2]));
		if (!IsLeyeCenter)
		{
			//第一帧作为中心原点
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
			//后续相对地址是基于第一帧位置的
			ui.customPlot_x->graph(0)->addData(FrameNum, box[0] - LeyeCenter.x);
			ui.customPlot_y->graph(0)->addData(FrameNum, box[1] - LeyeCenter.y);

			OldLeyeX.push_back(box[0] - LeyeCenter.x);
			OldLeyeY.push_back(box[1] - LeyeCenter.y);

			//左眼定位坐标
			plx = (box[0] - LeyeCenter.x) * 1.5 + 65;
			ply = (box[1] - LeyeCenter.y) * 1.5 + 65;
			Lcenter.x = plx;
			Lcenter.y = ply;
			cv::circle(Pic_Leye, Lcenter, 1, cv::Scalar(255, 0, 0), -1, 8);//画瞳孔中心 
			PicImgLeye = Mat2QImage(Pic_Leye);
			ui.label_LeyePosition->setPixmap(QPixmap::fromImage(PicImgLeye));
		}
		OldFrameL.push_back(FrameNum);
		
	}
	for (cv::Vec3f box : pro.Rcircles)
	{
		//右眼
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

			//右眼定位
			prx = (box[0] - ReyeCenter.x) * 1.5 + 65;
			pry = (box[1] - ReyeCenter.y) * 1.5 + 65;
			Rcenter.x = prx;
			Rcenter.y = pry;
			cv::circle(Pic_Reye, Rcenter, 1, cv::Scalar(255, 0, 0), -1, 8);//画瞳孔中心 
			PicImgReye = Mat2QImage(Pic_Reye);
			ui.label_ReyePosition->setPixmap(QPixmap::fromImage(PicImgReye));
		}
		OldFrameR.push_back(FrameNum);
	}
	if (EyeNum != VEDIO_EYE)
	{
		ui.customPlot_x->rescaleAxes();//自动调整坐标
		ui.customPlot_y->rescaleAxes();//自动调整坐标
	}

	ui.customPlot_x->replot();//重绘x坐标波形图
	ui.customPlot_y->replot();//重绘y坐标波形图
}
//关闭摄像头
void EyePupilLocalization::on_pushButton_closecamera_clicked()
{
	timer->stop();
	ui.statusBar->showMessage(QString::fromLocal8Bit("视频处理结束"));
	if (EyeNum == NOT_LEYE || EyeNum == ALL_EYE)
	{
		//此时只有右眼
		vcapRight.release();//释放资源
	}
	if (EyeNum == NOT_REYE || EyeNum == ALL_EYE)
	{
		//此时只有左眼
		vcapLeft.release();
	}
	ui.Button_closecamera->setEnabled(false);
	ui.Button_opencamera->setEnabled(true);
}
//打印
void EyePupilLocalization::on_pushButton_print_clicked()
{
	if (OldFrameNum == 0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("没有需要打印的波形"));
		return;
	}
	ui.statusBar->showMessage(QString::fromLocal8Bit("准备打印"));
	QPrinter printer;//新建打印机对象
	printer.setPageSize(QPrinter::A4);//设置打印为A4纸张
	QPrintPreviewDialog preview(&printer, this);
	connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreviewSlot(QPrinter*)));
	preview.exec();
}
//打印预览
void EyePupilLocalization::printPreviewSlot(QPrinter * printerPixmap)
{
	//文字备注位置
	QRect rect_level(0, 20, 800, 30);
	QRect rect_vertical(0, 520, 800, 30);
	//时间备注位置
	QRect rect_time(0, 1020, 700, 30);
	//获取界面照片
	plotWight(true);
	QPixmap pixmap_level = QPixmap::grabWidget(ui.customPlot_print, ui.customPlot_print->rect());//绘制水平波形图
	plotWight(false);
	QPixmap pixmap_vertical = QPixmap::grabWidget(ui.customPlot_print, ui.customPlot_print->rect());//绘制竖直波形图
	QPainter painterPixmap(this);
	painterPixmap.begin(printerPixmap);
	//绘制文字
	QFont font_lable("Arial", 20, QFont::Bold, false);//设置标签字体的类型，大小，加粗，非斜体
	QFont font_time("Arial", 15, QFont::Bold, false);//设置时间字体的类型，大小，加粗，非斜体
	painterPixmap.setFont(font_lable);
	painterPixmap.drawText(rect_level, Qt::AlignHCenter, tr("LEVEL"));
	painterPixmap.drawText(rect_vertical, Qt::AlignHCenter, tr("VERTICAL"));
	painterPixmap.setFont(font_time);
	painterPixmap.drawText(rect_time, Qt::AlignRight, str_TESTtime);
	//绘制波形
	painterPixmap.drawPixmap(30, 50, pixmap_level);
	painterPixmap.drawPixmap(30, 550, pixmap_vertical);
	painterPixmap.end();
	ui.statusBar->showMessage(NULL);
}
//绘制波形
void EyePupilLocalization::plotWight(bool IsLevel)
{
	ui.customPlot_print->addGraph();//增加第一条曲线
	ui.customPlot_print->xAxis->setRange(0, OldFrameNum);//设置横坐标
	ui.customPlot_print->graph(0)->setPen(QPen(Qt::red));//设置为红色
	ui.customPlot_print->addGraph();//在增加一条曲线
	ui.customPlot_print->graph(1)->setPen(QPen(Qt::blue));//设置为蓝色
	if (IsLevel)
	{
		//水平位置
		ui.customPlot_print->yAxis->setLabel("LEVEL");
		ui.customPlot_print->graph(0)->setData(OldFrameL, OldLeyeX);
		ui.customPlot_print->graph(1)->setData(OldFrameR, OldReyeX);
	}
	else
	{
		//竖直位置
		ui.customPlot_print->yAxis->setLabel("VERTICAL");
		ui.customPlot_print->graph(0)->setData(OldFrameL, OldLeyeY);
		ui.customPlot_print->graph(1)->setData(OldFrameR, OldReyeY);
	}
	ui.customPlot_print->rescaleAxes();//自动调整坐标
	ui.customPlot_print->rescaleAxes();//自动调整坐标
	ui.customPlot_print->replot();
}


