#ifndef EYEPUPILLOCALIZATION_H
#define EYEPUPILLOCALIZATION_H

#include <QtWidgets/QMainWindow>
#include "ui_eyepupillocalization.h"
#include <qfiledialog.h>
#include "Mat2QImage.h"
#include "imgprocess.h" 
#include "MatMerge.h"
#include <conio.h>

class EyePupilLocalization : public QMainWindow
{
	Q_OBJECT

public:
	EyePupilLocalization(QWidget *parent = 0);
	~EyePupilLocalization();

public slots:
	void on_pushButton_openvideo_clicked();//�򿪱�����Ƶ
	void on_pushButton_closecamera_clicked();//�ر�����ͷ
	void on_pushButton_opencamera_clicked();//������ͷ
	void on_pushButton_print_clicked();//��ӡ
	void printPreviewSlot(QPrinter *printerPixmap);//��ӡԤ��

private slots:
	void readFarme();

private:
	Ui::EyePupilLocalizationClass ui;
	//std::string videoStreamAddress = "http://192.168.1.233:8080/?action=stream?dummy=param.mjpg";//��������ͷ�����ַ,��Ϊ�˶�ȡ�����ļ�
	std::string videoStreamAddressLeft;
	std::string videoStreamAddressRight;
	QString fileName;//�򿪱�����Ƶ���Ե�ַ
	cv::Mat Leye;//����Mat����
	cv::Mat Reye;//����Mat����
	QImage Limg;//����Image����
	QImage Rimg;//����Image����

	//�����ϴ���Ϣ
	int OldFrameNum = 0;//���β���֡��
	QVector<double> OldFrameL;//�������۲��κ�����
	QVector<double> OldFrameR;//�������۲��κ�����
	QVector<double> OldReyeX;//�������۲���X����
	QVector<double> OldReyeY;//�������۲���Y����
	QVector<double> OldLeyeX;//�������۲���X����
	QVector<double> OldLeyeY;//�������۲���Y����
	QDateTime TESTtime;//����ʱ��
	QString str_TESTtime;//����ʱ���ַ�����׼��ʽ

	cv::Mat NoVedio;
	cv::VideoCapture vcapLeft;//����������ͷ�򿪶���
	cv::VideoCapture vcapRight;//����������ͷ�򿪶���
	cv::VideoCapture capture;//���屾����Ƶ�򿪶���
	cv::Mat frameL;//����
	cv::Mat frameR;//����
	cv::Mat frameAll;//������Ƶ֡,˫��
	int FrameNum;//֡��
	bool IsReyeCenter;//�Ƿ����۳�����һ����λ����
	bool IsLeyeCenter;//�Ƿ����۳�����һ����λ����
	CvPoint ReyeCenter;//��������
	CvPoint LeyeCenter;//��������
	QVector<double> TimeR, TimeL, Rx, Ry, Lx, Ly;//��ͼ������
	QTimer *timer;//��ʱ��
	int EyeNum;//�۾���Ŀ

	QImage NoVideoImage;
	int LabelWidth;
	int LabelHeight;

	cv::Mat Pic_Leye;
	cv::Mat Pic_Reye;

	QImage PicImgLeye;
	QImage PicImgReye;
	double plx;
	double ply;
	double prx;
	double pry;
	cv::Point Lcenter;
	cv::Point Rcenter;

	void plotWight(bool IsLevel);//���Ʋ���
	
};

#endif 
