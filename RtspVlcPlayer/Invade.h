#pragma once
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "opencv2/opencv.hpp"
//#include "opencv/cvaux.hpp"//必须引此头文件

class Invade
{
public:
	Invade();
	~Invade();
public:
	double TrianglArea(int x1, int y1, int x2, int y2, int x3, int y3);
	double SquArea(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
	void detect_face(IplImage* dst_img, int* count);

	// various tracking parameters (in seconds)跟踪的参数(单位为秒)
// 	const double MHI_DURATION = 0.5;
// 	const double MAX_TIME_DELTA = 0.5;
// 	const double MIN_TIME_DELTA = 0.05;
	const double MHI_DURATION = 1;//运动跟踪的最大持续时间 
	const double MAX_TIME_DELTA = 1;////最大时间增量
	const double MIN_TIME_DELTA = 0.05;//最小时间增量0.05s
	const int    N = 3;
	const int CONTOUR_MAX_AERA = 100;


	// ring image buffer 圈出图像缓冲
	IplImage **buf = 0;
	int last = 0;
	int nFrmnum = 1;
	// 临时图像参数
	IplImage *mhi = 0;							 // MHI: 运动历史图像

	//CvFilter filter = CV_GAUSSIAN_5x5;      
	int filter = CV_GAUSSIAN_5x5;          //采用5x5的高斯滤波模板
	CvConnectedComp *cur_comp, min_comp; //连接部件
	CvConnectedComp comp;
	CvMemStorage *storage;					//内存存储器
	CvPoint pt[4];									//二维坐标系下的点，类型为整型 ，通常以0点为原点，有x坐标和y坐标
	//检测出的感兴趣区域参数
	IplImage* dst_img1 = 0;                    //定义感兴趣区域图像指针
	//CvMemStorage *stor1;
	//CvSeq *cont1;
	IplImage *sub_buy = 0;
	//IplImage* sub_buy=0;
	//  参数：
	//  img – 输入视频帧
	//  dst – 检测结果
public:
	void update_mhi(IplImage* img, IplImage* dst, int diff_threshold);
	//void update_mhiEx(IplImage* img, IplImage* dst, int diff_threshold);
};

