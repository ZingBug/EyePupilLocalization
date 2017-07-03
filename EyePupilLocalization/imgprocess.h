#ifndef IMGPROCESS_H  
#define IMGPROCESS_H  
#include <opencv2/opencv.hpp>   
#include <stdio.h>  
#include <vector>
#include <opencv2/core/core.hpp> 
#include "eyepupillocalization.h"

#define ALL_EYE 5//����ͷ˫�۶���
#define NOT_LEYE 2//����ͷû������
#define NOT_REYE 0//����ͷû������
#define NOT_ALLEYE 8//����ͷ˫�۶�û��
#define VEDIO_EYE 1//������Ƶ˫�۶���
#define VEDIO_ONLY_EYE 6//������Ƶ����

typedef struct Box//����Բ
{
	double x;//x����
	double y;//x����
	double r;//�뾶
}Box;

//��Ƶ������  
class ImgProcess
{
private:
	double EyeRatio;
	int EyeNum;

	cv::Mat inimg;//����ͼ��  
	cv::Mat outimg;//������  
	cv::Mat Leye;//����
	cv::Mat Reye;//����
	
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
	std::vector<cv::Vec3f> circles;//�������ͫ��Բ

	void DivideEye(const cv::Mat divideimg);//��������  
	cv::Mat GrayDetect(cv::Mat grayimg);//�Ҷȴ���
	cv::Mat Binary(const cv::Mat binaryimg, int value);//��ֵ������
	void EdgeDetect(cv::Mat &edgeimg);//��Ե���  
	std::vector<cv::Vec3f> Hough(const cv::Mat midImage, int minradius, int maxradius);//hough�任   
	cv::Mat PlotC(std::vector<cv::Vec3f> circles, cv::Mat &midImage);//�������Բ
	void RemoveSmallRegion(cv::Mat& Src, cv::Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode);//ȥ��С���
	Box circleLeastFit(const std::vector<cv::Point> points);//���Բ���
	void contrastStretch(cv::Mat & image);

public:
	std::vector<cv::Vec3f> Lcircles;//�����������ͫ��Բ
	std::vector<cv::Vec3f> Rcircles;//�����������ͫ��Բ

	ImgProcess() {};//Ĭ�Ϲ��캯��
	ImgProcess(cv::Mat image, double ratio = 1.3) :inimg(image), EyeRatio(ratio) {}//���캯��
	ImgProcess(cv::Mat leye, cv::Mat reye, double ratio = 1.3, int eye_num = 2) :Leye(leye), Reye(reye), EyeRatio(ratio), EyeNum(eye_num) {}

	void Start(cv::Mat, double ratio = 1.3);
	void Start(cv::Mat, cv::Mat, double ratio = 1.3, int eye_num = 2);
	void Process();//˫����������ʶ��
	void ProcessSignal();//˫�۷ֱ�����ʶ��
	cv::Mat Outputimg();//������  
	cv::Mat OutLeye();//�������  
	cv::Mat OutReye();//�������
};


#endif // IMGPROCESS_H  