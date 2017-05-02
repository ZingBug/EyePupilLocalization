#ifndef IMGPROCESS_H  
#define IMGPROCESS_H  
#include <opencv2/opencv.hpp>  
#include <iostream>  
#include <stdio.h>  
#include <vector>
#include <opencv2/core/core.hpp> 

//��Ƶ������  
class ImgProcess
{
private:
	double EyeRatio;
	cv::Mat inimg;//����ͼ��  
	cv::Mat outimg;//������  
	cv::Mat Leye;//����
	cv::Mat Reye;//����
	std::vector<cv::Vec3f> Lcircles;//���ۻ���Բ
	std::vector<cv::Vec3f> Rcircles;//���ۻ���Բ
	std::vector<std::vector<cv::Point>> Lcontours;//�����������
	std::vector<std::vector<cv::Point>> Rcontours;//�����������
	std::vector<cv::Vec4i> Lhierarchy;//����������ϵ
	std::vector<cv::Vec4i> Rhierarchy;//����������ϵ
	double Lmaxarea = 0;//�����������
	int LmaxAreaIndex = 0;//������������±�
	double Rmaxarea = 0;//�����������
	int RmaxAreaIndex = 0;//������������±�
	cv::Rect Rrect;//���������߽�
	cv::Rect Lrect;//���������߽�
	//static RadiusRange Radiusrange;//�۾��뾶��Χ 

	void DivideEye(const cv::Mat divideimg);//��������  
	cv::Mat GrayDetect(cv::Mat grayimg);//�Ҷȴ���
	cv::Mat Binary(const cv::Mat binaryimg, int value);//��ֵ������
	void EdgeDetect(cv::Mat &edgeimg);//��Ե���  
	std::vector<cv::Vec3f> Hough(const cv::Mat midImage, int minradius, int maxradius);//hough�任   
	cv::Mat PlotC(std::vector<cv::Vec3f> circles, cv::Mat &midImage);//��HOUGH�任�ļ����  
	void RemoveSmallRegion(cv::Mat& Src, cv::Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode);//ȥ��С���

public:
	std::vector<cv::Vec3f> circles;//�������Բ
	//ImgProcess(cv::Mat image) :inimg(image) {}
	ImgProcess(cv::Mat image, double ratio = 1.3) :inimg(image), EyeRatio(ratio) {}
	//void Process(int & LminRadius, int & LmaxRadius, int & RminRadius, int & RmaxRadius);//������
	void Process(int* transmit);
	cv::Mat Outputimg();//������  
	cv::Mat OutLeye();//�������  
	cv::Mat OutReye();//�������
};


#endif // IMGPROCESS_H  