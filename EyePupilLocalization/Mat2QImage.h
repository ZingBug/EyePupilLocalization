#ifndef MAT2QIMAGE_H  
#define MAT2QIMAGE_H  

#include <QtGui>  
#include <QDebug>  
#include <iostream>  
#include <opencv/cv.h>  
#include <opencv/highgui.h>  
#include <opencv2/opencv.hpp>

QImage Mat2QImage(const cv::Mat&);//mat��ʽת��ΪQimage��ʽ  
cv::Mat QImage2Mat(const QImage);//Qimage��ʽת��Ϊmat��ʽ

#endif  