#include "imgprocess.h"  

//RadiusRange ImgProcess::Radiusrange = { 5,30,5,30 };

//������
cv::Mat ImgProcess::Outputimg()
{
	return outimg;
}

//��������
void ImgProcess::DivideEye(const cv::Mat divideimg)
{
	if (divideimg.cols > 0 && divideimg.rows > 0)//cols������320��rows������120
	{
		CvRect leye_box;
		leye_box.x = 1;
		leye_box.y = 1;
		leye_box.height = divideimg.rows - 1;
		leye_box.width = divideimg.cols / 2 - 1;
		CvRect reye_box;
		reye_box.x = leye_box.x + leye_box.width;
		reye_box.y = 1;
		reye_box.height = divideimg.rows - 1;
		reye_box.width = divideimg.cols / 2 - 1;
		Leye = divideimg(leye_box);
		Reye = divideimg(reye_box);
	}

}

//�������
cv::Mat ImgProcess::OutLeye()//�������
{
	return Leye;
}

//�������
cv::Mat ImgProcess::OutReye()//�������
{
	return Reye;
}

//ͫ�׶�λ����
void ImgProcess::Process(int* transmit)
{
	cv::Mat grayimg;
	double temparea;
	grayimg = GrayDetect(inimg);//�õ��Ҷ�ͼ,��ʱinimgû�б��޸�
	EdgeDetect(grayimg);//��Ե���
	DivideEye(grayimg);//�����۷ָ�
	cv::findContours(Leye, Lcontours, Lhierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);//Ѱ����������
	cv::findContours(Reye, Rcontours, Rhierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);//Ѱ����������
	if (Lcontours.size() > 0)//����������
	{
		for (int i = 0; i < Lcontours.size(); ++i)
		{
			temparea = fabs(cv::contourArea(Lcontours[i]));
			if (temparea > Lmaxarea)
			{
				Lmaxarea = temparea;//��������������
				LmaxAreaIndex = i;//������������±�
				continue;
			}
		}
		//���ۼ��
		Lrect = cv::boundingRect(Lcontours[LmaxAreaIndex]);
		if ((Lrect.width / (float)Lrect.height) < EyeRatio && (Lrect.height / (float)Lrect.width) < EyeRatio && Lrect.width > 15 && Lrect.height > 15)//���۱��ۼ��
		{
			Lcircles = Hough(Leye, transmit[0], transmit[1]);
		}
	}
	if (Rcontours.size() > 0)//����������
	{
		for (int i = 0; i < Rcontours.size(); ++i)
		{
			temparea = fabs(cv::contourArea(Rcontours[i]));
			if (temparea > Rmaxarea)
			{
				Rmaxarea = temparea;
				RmaxAreaIndex = i;
				continue;
			}
		}
		//���ۼ��
		Rrect = cv::boundingRect(Rcontours[RmaxAreaIndex]);
		if ((Rrect.width / (float)Rrect.height) < EyeRatio && (Rrect.height / (float)Rrect.width) < EyeRatio && Rrect.width > 15 && Rrect.height > 15)//���۱��ۼ��
		{
			Rcircles = Hough(Reye, transmit[2], transmit[3]);
		}
	}
	
	int MaxRadius = 0;//���뾶
	size_t MaxNum = 0;
	if (Lcircles.size() > 0)
	{
		for (size_t i = 0; i < Lcircles.size(); ++i)
		{
			if (cvRound(Lcircles[i][2]) >= MaxRadius)
			{
				MaxNum = i;
			}
		}
		if (transmit[4] == 0 && transmit[5] == 0 && Lcircles[MaxNum][0] > 5)
		{
			//��һ�����
			transmit[4] = Lcircles[MaxNum][0];//Բ��x����
			transmit[5] = Lcircles[MaxNum][1];//Բ��y����
		}
		else
		{
			if (abs(transmit[4] - Lcircles[MaxNum][0]) < 30 && abs(transmit[5] - Lcircles[MaxNum][1]) < 30)
			{
				//Բ�İ뾶���μ���
				circles.push_back(Lcircles[MaxNum]);//����������Բ
				transmit[0] = Lcircles[MaxNum][2] - 8;
				transmit[1] = Lcircles[MaxNum][2] + 8;
			}
		}
	}
	if (Rcircles.size() > 0)
	{
		MaxRadius = 0;//���³�ʼ��
		MaxNum = 0;
		for (size_t i = 0; i < Rcircles.size(); ++i)
		{
			Rcircles[i][0] += inimg.cols / 2 - 1;
			if (cvRound(Rcircles[i][2]) >= MaxRadius)
			{
				MaxNum = i;
			}
		}
		if (transmit[6] == 0 && transmit[7] == 0 && Rcircles[MaxNum][0] > 5)
		{
			//��һ�����
			transmit[6] = Rcircles[MaxNum][0];//Բ��x����
			transmit[7] = Rcircles[MaxNum][1];//Բ��y����
		}
		else
		{
			if (abs(transmit[6] - Rcircles[MaxNum][0]) < 30 && abs(transmit[7] - Rcircles[MaxNum][1]) < 30)
			{
				//Բ�İ뾶���μ���
				circles.push_back(Rcircles[MaxNum]);//����������Բ
				transmit[2] = Rcircles[MaxNum][2] - 8;
				transmit[3] = Rcircles[MaxNum][2] + 8;
			}
		}
	}
	if (circles.size() > 0)
	{
		outimg = PlotC(circles, inimg);
	}
	else
	{
		//û�л���Բ
		outimg = inimg;
	}
	DivideEye(outimg);//�������ٴηָ�,��ʱ�Ѿ����˻���Բ
	if (Lcontours.size() > 0)
	{
		cv::drawContours(Leye, Lcontours, LmaxAreaIndex, cv::Scalar(0, 0, 255), 1);//����������ʾ
	}
	if (Rcontours.size() > 0)
	{
		cv::drawContours(Reye, Rcontours, RmaxAreaIndex, cv::Scalar(0, 0, 255), 1);//����������ʾ
	}
}

//�Ҷȴ���
cv::Mat ImgProcess::GrayDetect(cv::Mat grayimg)
{
	cv::Mat grayout;
	cvtColor(grayimg, grayimg, CV_BGR2GRAY);//�ҶȻ�����
	medianBlur(grayimg, grayimg, 9);//��ֵ�˲�
	grayout = Binary(grayimg, 43);//��ֵ������
	//RemoveSmallRegion(grayout, grayout, 50, 0, 0);//���Բ�ȥ��С���
	RemoveSmallRegion(grayout, grayout, 250, 1, 0);//ȥ��������
	return grayout;
}

//��ֵ������
cv::Mat ImgProcess::Binary(const cv::Mat binaryimg, int value)
{
	cv::Mat binaryout = binaryimg < value;
	return binaryout;
}

//��Ե���
void ImgProcess::EdgeDetect(cv::Mat &edgeimg)
{
	//Mat edgeout;
	Canny(edgeimg, edgeimg, 100, 250, 3);//����ͼ��,���ͼ��,����ֵ,����ֵ��opencv�����ǵ���ֵ��3��,�ڲ�sobel�˲�����С  
										 //return edgeimg;
}

//hough���
std::vector<cv::Vec3f> ImgProcess::Hough(const cv::Mat midImage, int minradius, int maxradius)
{
	std::vector<cv::Vec3f> circles;
	HoughCircles(midImage, circles, CV_HOUGH_GRADIENT, 3, 5, 100, 30, minradius, maxradius);
	return circles;
}

//��HOUGH�任�ļ����  
cv::Mat ImgProcess::PlotC(std::vector<cv::Vec3f> circles, cv::Mat &midImage)
{
	for (size_t i = 0; i < circles.size(); i++)
	{
		//��Բ��λ�úͰ뾶������������
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		//����Բ��
		circle(midImage, center, 1, cv::Scalar(0, 0, 255), -1, 8);
		//����Բ����  
		circle(midImage, center, radius, cv::Scalar(255, 0, 0), 1, 8);
	}
	return midImage;
}

//CheckMode: 0����ȥ��������1����ȥ��������; NeihborMode��0����4����1����8����;  
void ImgProcess::RemoveSmallRegion(cv::Mat& Src, cv::Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;       //��¼��ȥ�ĸ���  
							   //��¼ÿ�����ص����״̬�ı�ǩ��0����δ��飬1�������ڼ��,2�����鲻�ϸ���Ҫ��ת��ɫ����3������ϸ������  
	cv::Mat Pointlabel = cv::Mat::zeros(Src.size(), CV_8UC1);

	if (CheckMode == 1)
	{
		std::cout << "Mode: ȥ��С����. ";
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] < 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}
	else
	{
		std::cout << "Mode: ȥ���׶�. ";
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] > 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}

	std::vector<cv::Point2i> NeihborPos;  //��¼�����λ��  
	NeihborPos.push_back(cv::Point2i(-1, 0));
	NeihborPos.push_back(cv::Point2i(1, 0));
	NeihborPos.push_back(cv::Point2i(0, -1));
	NeihborPos.push_back(cv::Point2i(0, 1));
	if (NeihborMode == 1)
	{
		std::cout << "Neighbor mode: 8����." << std::endl;
		NeihborPos.push_back(cv::Point2i(-1, -1));
		NeihborPos.push_back(cv::Point2i(-1, 1));
		NeihborPos.push_back(cv::Point2i(1, -1));
		NeihborPos.push_back(cv::Point2i(1, 1));
	}
	else std::cout << "Neighbor mode: 4����." << std::endl;
	int NeihborCount = 4 + 4 * NeihborMode;
	int CurrX = 0, CurrY = 0;
	//��ʼ���  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 0)
			{
				//********��ʼ�õ㴦�ļ��**********  
				std::vector<cv::Point2i> GrowBuffer;                                      //��ջ�����ڴ洢������  
				GrowBuffer.push_back(cv::Point2i(j, i));
				Pointlabel.at<uchar>(i, j) = 1;
				int CheckResult = 0;                                               //�����жϽ�����Ƿ񳬳���С����0Ϊδ������1Ϊ����  

				for (int z = 0; z<GrowBuffer.size(); z++)
				{

					for (int q = 0; q<NeihborCount; q++)                                      //����ĸ������  
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows)  //��ֹԽ��  
						{
							if (Pointlabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(cv::Point2i(CurrX, CurrY));  //��������buffer  
								Pointlabel.at<uchar>(CurrY, CurrX) = 1;           //���������ļ���ǩ�������ظ����  
							}
						}
					}

				}
				if (GrowBuffer.size()>AreaLimit) CheckResult = 2;                 //�жϽ�����Ƿ񳬳��޶��Ĵ�С����1Ϊδ������2Ϊ����  
				else { CheckResult = 1;   RemoveCount++; }
				for (int z = 0; z<GrowBuffer.size(); z++)                         //����Label��¼  
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					Pointlabel.at<uchar>(CurrY, CurrX) += CheckResult;
				}
				//********�����õ㴦�ļ��**********  


			}
		}
	}

	CheckMode = 255 * (1 - CheckMode);
	//��ʼ��ת�����С������  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iData = Src.ptr<uchar>(i);
		uchar* iDstData = Dst.ptr<uchar>(i);
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 2)
			{
				iDstData[j] = CheckMode;
			}
			else if (iLabel[j] == 3)
			{
				iDstData[j] = iData[j];
			}
		}
	}

	std::cout << RemoveCount << " objects removed." << std::endl;
}