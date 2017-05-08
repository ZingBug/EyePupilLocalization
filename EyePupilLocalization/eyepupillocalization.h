#ifndef EYEPUPILLOCALIZATION_H
#define EYEPUPILLOCALIZATION_H

#include <QtWidgets/QMainWindow>
#include "ui_eyepupillocalization.h"
#include <qfiledialog.h>
#include "Mat2QImage.h"
#include "imgprocess.h" 

class EyePupilLocalization : public QMainWindow
{
	Q_OBJECT

public:
	EyePupilLocalization(QWidget *parent = 0);
	~EyePupilLocalization();

public slots:
	void on_pushButton_openvideo_clicked();//�򿪱�����Ƶ
	void on_pushButton_opencamera_clicked();//������ͷ
	void on_pushButton_print_clicked();//��ӡ
private:
	Ui::EyePupilLocalizationClass ui;
	std::string videoStreamAddress = "http://192.168.1.233:8080/?action=stream?dummy=param.mjpg";//��������ͷ�����ַ
	QString fileName;//�򿪱�����Ƶ���Ե�ַ
	cv::Mat Leye;//����Mat����
	cv::Mat Reye;//����Mat����
	QImage Limg;//����Image����
	QImage Rimg;//����Image����
	QLabel *label_name;//״̬��������Ϣ��ע
	QLabel *label_time;//״̬��ʱ����Ϣ��ע

	//�����ϴ���Ϣ
	int OldFrameNum = 0;//���β���֡��
	QVector<double> OldFrameL;//�������۲��κ�����
	QVector<double> OldFrameR;//�������۲��κ�����
	QVector<double> OldReyeX;//�������۲���X����
	QVector<double> OldReyeY;//�������۲���Y����
	QVector<double> OldLeyeX;//�������۲���X����
	QVector<double> OldLeyeY;//�������۲���Y����

	void plotWight(bool IsLevel);//���Ʋ���
};

#endif // EYEPUPILLOCALIZATION_H
