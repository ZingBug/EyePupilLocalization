#ifndef MAT2QIMAGE_H  
#define MAT2QIMAGE_H  

#include <QtGui>  
#include <QDebug>  
#include <iostream>  
#include <opencv/cv.h>  
#include <opencv/highgui.h>  

QImage Mat2QImage(const cv::Mat&);//mat��ʽת��ΪQimage��ʽ  

#endif  