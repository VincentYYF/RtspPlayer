#include "stdafx.h"
#include "Invade.h"


Invade::Invade()
{
}


Invade::~Invade()
{
}

// 参数： 
// img - 输入视频帧 // dst - 检测结果 
void Invade::update_mhi(IplImage* img, IplImage* dst, int diff_threshold)
{
	double timestamp = clock() / 100.; // get current time in seconds 时间戳 
	CvSize size = cvSize(img->width, img->height); // get current frame size，得到当前帧的尺寸 
	int i, idx1, idx2;
	IplImage* silh;
	IplImage* pyr = cvCreateImage(cvSize((size.width & -2) / 2, (size.height & -2) / 2), 8, 1);
	CvMemStorage *stor;
	CvSeq *cont;

	/*先进行数据的初始化*/
	if (!mhi || mhi->width != size.width || mhi->height != size.height)
	{
		if (buf == 0) //若尚没有初始化则分配内存给他 
		{
			buf = (IplImage**)malloc(N*sizeof(buf[0]));
			memset(buf, 0, N*sizeof(buf[0]));
		}

		for (i = 0; i < N; i++)
		{
			cvReleaseImage(&buf[i]);
			buf[i] = cvCreateImage(size, IPL_DEPTH_8U, 1);
			cvZero(buf[i]);// clear Buffer Frame at the beginning 
		}
		cvReleaseImage(&mhi);
		mhi = cvCreateImage(size, IPL_DEPTH_32F, 1);
		cvZero(mhi); // clear MHI at the beginning 
	} // end of if(mhi) 

	/*将当前要处理的帧转化为灰度放到buffer的最后一帧中*/
	cvCvtColor(img, buf[last], CV_BGR2GRAY); // convert frame to grayscale 

	/*设定帧的序号*/
	idx1 = last;
	idx2 = (last + 1) % N; // index of (last - (N-1))th frame 
	last = idx2;

	// 做帧差 
	silh = buf[idx2];//差值的指向idx2 
	cvAbsDiff(buf[idx1], buf[idx2], silh); // get difference between frames 

	// 对差图像做二值化 
	cvThreshold(silh, silh, 50, 255, CV_THRESH_BINARY); //threshold it,二值化 

	//去掉超时的影像以更新运动历史图像
	cvUpdateMotionHistory(silh, mhi, timestamp, MHI_DURATION); // update MHI 

	cvConvert(mhi, dst);//将mhi转化为dst,dst=mhi 

	// 中值滤波，消除小的噪声 
	cvSmooth(dst, dst, CV_MEDIAN, 3, 0, 0, 0);

	cvPyrDown(dst, pyr, CV_GAUSSIAN_5x5);// 向下采样，去掉噪声，图像是原图像的四分之一 
	cvDilate(pyr, pyr, 0, 1); // 做膨胀操作，消除目标的不连续空洞 
	cvPyrUp(pyr, dst, CV_GAUSSIAN_5x5);// 向上采样，恢复图像，图像是原图像的四倍 

	// 下面的程序段用来找到轮廓 
	// Create dynamic structure and sequence. 
	stor = cvCreateMemStorage(0);
	cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), stor);

	// 找到所有轮廓 
	cvFindContours(dst, stor, &cont, sizeof(CvContour),
		CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	// 直接使用CONTOUR中的矩形来画轮廓 
	for (; cont; cont = cont->h_next)
	{
		CvRect r = ((CvContour*)cont)->rect;
		if (r.height * r.width > CONTOUR_MAX_AERA) // 面积小的方形抛弃掉 
		{
			cvRectangle(img, cvPoint(r.x, r.y),
				cvPoint(r.x + r.width, r.y + r.height),
				CV_RGB(255, 0, 0), 1, CV_AA, 0);
		}
	}
	// free memory 
	cvReleaseMemStorage(&stor);
	cvReleaseImage(&pyr);
}

/*
void Invade::update_mhiEx(IplImage* img, IplImage* dst, int diff_threshold)
{
	int i, idx1, idx2;
	IplImage* silh;
	CvSeq *cont;
	CvCapture*pCapture = NULL;
	CvMemStorage *stor;
	//从文件USB摄像头捕获视频帧:
//	pCapture = cvCaptureFromAVI();
	//将当前要处理的帧转化为灰度放到buffer的最后一帧中
	cvCvtColor(img, buf[last], CV_BGR2GRAY); // convert frame to grayscale 
	//设定帧的序号
	idx1 = last;
	idx2 = (last + 1) % N; // index of (last - (N-1))th frame 
	last = idx2;
	//基于高斯统计估计背景模型
	CvGaussBGModel*bg_model = NULL;//定义高斯背景模型 : 
	bg_model = (CvGaussBGModel*)cvCreateGaussianBGModel(buf[idx1], 0);//使用第一帧buf[idx1]创建高斯背景模型:
	cvUpdateBGStatModel(buf[idx1], (CvBGStatModel*)bg_model);//使用当前帧更新高斯背景模型:
	//将高斯背景模型的背景图像复制到事先定义的背景图像pBkImg中：
	cvCopy(bg_model->background, dst, 0);
	//使用瞬时差分法获得不包含运动区域的当前帧，并更新背景。当前帧buf[idx1]和前一帧buf[idx2]做差分, 结果放在差分图像diff中，并二值化:
	silh = buf[idx2];
	cvAbsDiff(buf[idx1], buf[idx2], silh);
	//对差图像做二值化
	cvThreshold(silh, silh, 30, 255, CV_THRESH_BINARY);
	cvSmooth(dst, dst, CV_MEDIAN, 3, 0, 0, 0);
	//下面的程序段用来找到轮廓
	//Createdynamicstructureandsequence.

	stor=cvCreateMemStorage(0);
	cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), stor);
	//找到所有轮廓
	cvFindContours(dst, stor, &cont, sizeof(CvContour),
		CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	//直接使用CONTOUR中的矩形来画轮廓
	for (; cont; cont = cont->h_next)
	{
		CvRect r = ((CvContour*)cont)->rect;
		if (r.height*r.width > CONTOUR_MAX_AERA)//面积小的方形抛弃
		{
			cvRectangle(img, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height),
				CV_RGB(255, 255, 0), 1, CV_AA, 0);
		}
	}
}
*/