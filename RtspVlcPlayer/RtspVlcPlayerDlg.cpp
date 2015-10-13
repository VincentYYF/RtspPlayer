
// RtspVlcPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RtspVlcPlayer.h"
#include "RtspVlcPlayerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRtspVlcPlayerDlg 对话框



CRtspVlcPlayerDlg::CRtspVlcPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRtspVlcPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRtspVlcPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRtspVlcPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PlLAY, &CRtspVlcPlayerDlg::OnBnClickedButtonPllay)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CRtspVlcPlayerDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_INVADE_HOG_MESSAGE, OnInvadeHogMessage)//自定义消息
END_MESSAGE_MAP()


// CRtspVlcPlayerDlg 消息处理程序

BOOL CRtspVlcPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	canSendData = true;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRtspVlcPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRtspVlcPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRtspVlcPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT ThreadRtspPlay(LPVOID pParam)
{
		CRtspVlcPlayerDlg *mDlg = (CRtspVlcPlayerDlg *)pParam;
		//CWinThread* pThread=AfxBeginThread(ThreadRtspPlay, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		//pThread->ResumeThread();
		int videoStream;
		int frameFinished;
		int numBytes;
		uint8_t * buffer;
		AVPacket * packet;
		AVFrame * pFrameRGB;
		AVFrame * pFrame;
		AVCodec * pCodec;
		AVCodecContext * pCodecCtx;
		AVFormatContext * pFormatCtx;
		char* filename = "rtsp://218.204.223.237:554/live/1/66251FC11353191F/e7ooqwcfbqjoo80j.sdp";
		//char* filename = "rtsp://115.29.239.238/000000001201.sdp";
		//char* filename = "rtsp://218.204.223.237:554/live/1/0547424F573B085C/gsfp90ef4k0a6iap.sdp";
		//char* filename = "D:\\motiondetect\\test.avi";
		av_register_all();//注册组件
		avformat_network_init();//支持网络流

		pFormatCtx = avformat_alloc_context(); //初始化AVFormatContext, 分配一块内存，保存视频属性信息

		if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
		{
			AfxMessageBox(L"open  input  file  faile !");
			return -1;
		}
		TRACE(L"\n open  input  file !");

		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)//查找流信息
		{
			TRACE("Couldn't find stream information.\n");
			return -1;
		}

		for (int i = 0; i < pFormatCtx->nb_streams; i++)//获取视频流ID
		{
			if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoStream = i;
				break;
			}
		}
		if (videoStream == -1)
		{
			AfxMessageBox(L"could  not find video stream !");
			return -1;
		}
		TRACE(L"find video stream !");

		pCodecCtx = pFormatCtx->streams[videoStream]->codec;
		pCodec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
		if (pCodec == NULL)
		{
			AfxMessageBox(L"could not find decoder");
			return -1;
		}
		TRACE(L"find decoder");

		if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)//打开解码器
		{
			AfxMessageBox(L"could not open decoder");
			return -1;
		}
		TRACE(L"open decoder");

		av_dump_format(pFormatCtx, 0, filename, 0);

		pFrame = av_frame_alloc();//avcodec_alloc_frame //存储解码后AVFrame
		pFrameRGB = av_frame_alloc(); //存储转换后AVFrame（为什么要转换？后文解释）
		packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		if (pFrameRGB == NULL)
		{
			AfxMessageBox(L"open mem faile");
			return -1;
		}

		numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);//分配AVFrame所需内存

		buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

		avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);////填充AVFrame
		
		IplImage* motion = 0;
		while (av_read_frame(pFormatCtx, packet) >= 0)
		{
			if (packet->stream_index == videoStream)
			{
				avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);

				if (frameFinished)
				{
					struct SwsContext * img_convert_ctx;

					img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
					if (img_convert_ctx == NULL)
					{
						AfxMessageBox(L"Cannot initialize the conversion context !");
						return -1;
					}

					static uchar *p = NULL;

					p = pFrame->data[1];
					pFrame->data[1] = pFrame->data[2];
					pFrame->data[2] = p;

					sws_scale(img_convert_ctx,
						pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrameRGB->data, pFrameRGB->linesize);

					if (img_convert_ctx)
					{
						sws_freeContext(img_convert_ctx);
						img_convert_ctx = NULL;
					}

					IplImage * Image;
					CvSize Size = cvSize(pCodecCtx->width, pCodecCtx->height);
					Image = cvCreateImage(Size, IPL_DEPTH_8U, 3);
					memcpy(Image->imageData, buffer, pCodecCtx->width*pCodecCtx->height * 24 / 8);
					Image->widthStep = pCodecCtx->width * 3;
					Image->origin = 0;
					//cvShowImage("video", Image);
					if (Image)//若取到则判断motion是否为空 
					{
						if (!motion)
						{
							motion = cvCreateImage(cvSize(Image->width, Image->height), 8, 1); //创建motion帧，八位，一通道 
							cvZero(motion); //零填充motion 
							motion->origin = Image->origin; //内存存储的顺序和取出的帧相同 
						}
					}
					mDlg->mInvade.update_mhi(Image, motion, 60);//更新历史图像 
					//mDlg->mHogInvade.HogImage(Image);
					mDlg->DrawPicToHDC(Image, IDC_PIC);
					//mDlg->UpdateData(FALSE);

					cvReleaseImage(&Image);
				}
			}
		}
		return 0;
}


void CRtspVlcPlayerDlg::DrawPicToHDC(IplImage *img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img, 1);

	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);
}


void CRtspVlcPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	//CDialogEx::OnTimer(nIDEvent);
	if (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoStream)
		{
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);

			if (frameFinished)
			{
				struct SwsContext * img_convert_ctx;

				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
				if (img_convert_ctx == NULL)
				{
					AfxMessageBox(L"Cannot initialize the conversion context !");
					return;
				}

				static uchar *p = NULL;

				p = pFrame->data[1];
				pFrame->data[1] = pFrame->data[2];
				pFrame->data[2] = p;

				sws_scale(img_convert_ctx,
					pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				if (img_convert_ctx)
				{
					sws_freeContext(img_convert_ctx);
					img_convert_ctx = NULL;
				}

				IplImage * Image;
				CvSize Size = cvSize(pCodecCtx->width, pCodecCtx->height);
				Image = cvCreateImage(Size, IPL_DEPTH_8U, 3);
				memcpy(Image->imageData, buffer, pCodecCtx->width*pCodecCtx->height * 24 / 8);
				Image->widthStep = pCodecCtx->width * 3;
				Image->origin = 0;
				//cvShowImage("video", Image);
				DrawPicToHDC(Image, IDC_PIC);
				UpdateData(FALSE);

				cvReleaseImage(&Image);
			}
		}
	}
}


void CRtspVlcPlayerDlg::OnBnClickedButtonStop()
{
	// TODO:  在此添加控件通知处理程序代码
	KillTimer(1);
}

UINT ThreadRtspBaiscPlay(LPVOID pParam)
{
	AVFormatContext	*pFormatCtx;
	int i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec	*pCodec;
	//char filepath[] = "nwn.mp4";
	//char rtspUrl[] = "rtsp://192.168.11.19/live0.264";
	//char rtspUrl[] = "rtsp://218.204.223.237:554/live/1/0547424F573B085C/gsfp90ef4k0a6iap.sdp";
	char rtspUrl[] = "rtsp://211.139.194.251:554/live/2/13E6330A31193128/5iLd2iNl5nQ2s8r8.sdp";
	av_register_all();//注册组件
	avformat_network_init();//支持网络流
	pFormatCtx = avformat_alloc_context();//初始化AVFormatContext
	if (avformat_open_input(&pFormatCtx,/*filepath*/rtspUrl, NULL, NULL) != 0){//打开文件
		AfxMessageBox(L"无法网络流\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx,NULL) < 0)//查找流信息
	{
		AfxMessageBox(L"不能够找到流信息\n");
		return -1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) //获取视频流ID
	if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		videoindex = i;
		break;
	}
	if (videoindex == -1)
	{
		TRACE("Didn't find a video stream.\n");
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
	if (pCodec == NULL)
	{
		TRACE("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)//打开解码器
	{
		TRACE("Could not open codec.\n");
		return -1;
	}
	AVFrame	*pFrame, *pFrameYUV;
	pFrame = av_frame_alloc();//存储解码后AVFrame
	pFrameYUV = av_frame_alloc();//存储转换后AVFrame（为什么要转换？后文解释）
	uint8_t *out_buffer;
	out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];//分配AVFrame所需内存
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//填充AVFrame

	
	int ret, got_picture;
	static struct SwsContext *img_convert_ctx;
	int y_size = pCodecCtx->width * pCodecCtx->height;

	
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));//存储解码前数据包AVPacket
	av_new_packet(packet, y_size);
	//输出一下信息-----------------------------
	printf("文件信息-----------------------------------------\n");
	av_dump_format(pFormatCtx, 0, rtspUrl, 0);
	printf("-------------------------------------------------\n");
	//------------------------------
	while (av_read_frame(pFormatCtx, packet) >= 0)//循环获取压缩数据包AVPacket
	{
		if (packet->stream_index == videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//解码。输入为AVPacket，输出为AVFrame
			if (ret < 0)
			{
				printf("解码错误\n");
				return -1;
			}
			if (got_picture)
			{
				//像素格式转换。pFrame转换为pFrameYUV。
				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				sws_freeContext(img_convert_ctx);

			}
		}
		av_free_packet(packet);

		
		
	}


	delete[] out_buffer;
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
/*
UINT ThreadOnePicDealPlay(LPVOID pParam)
{
	CRtspVlcPlayerDlg *mDlg = (CRtspVlcPlayerDlg *)pParam;

	IplImage * Image = mDlg->sendImage;

	//mDlg->mInvade.update_mhi(Image, motion, 60);//更新历史图像 
	mDlg->mHogInvade.HogImage(Image);
	mDlg->DrawPicToHDC(Image, IDC_PIC);
	//mDlg->UpdateData(FALSE);
	mDlg->canSendData = true;
	//cvReleaseImage(&Image);

	return 0;
}
*/
UINT ThreadRtspNoDealPlay(LPVOID pParam)
{
	CRtspVlcPlayerDlg *mDlg = (CRtspVlcPlayerDlg *)pParam;
	//CWinThread* pThread=AfxBeginThread(ThreadRtspPlay, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	//pThread->ResumeThread();
	int videoStream;
	int frameFinished;
	int numBytes;
	uint8_t * buffer;
	AVPacket * packet;
	AVFrame * pFrameRGB;
	AVFrame * pFrame;
	AVCodec * pCodec;
	AVCodecContext * pCodecCtx;
	AVFormatContext * pFormatCtx;
	char* filename = "rtsp://218.204.223.237:554/live/1/66251FC11353191F/e7ooqwcfbqjoo80j.sdp";
	//char* filename = "rtsp://115.29.239.238/000000001201.sdp";
	//char* filename = "rtsp://218.204.223.237:554/live/1/0547424F573B085C/gsfp90ef4k0a6iap.sdp";
	//char* filename = "D:\\motiondetect\\test.avi";
	av_register_all();//注册组件
	avformat_network_init();//支持网络流

	pFormatCtx = avformat_alloc_context(); //初始化AVFormatContext, 分配一块内存，保存视频属性信息

	if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
	{
		AfxMessageBox(L"open  input  file  faile !");
		return -1;
	}
	TRACE(L"\n open  input  file !");

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)//查找流信息
	{
		TRACE("Couldn't find stream information.\n");
		return -1;
	}

	for (int i = 0; i < pFormatCtx->nb_streams; i++)//获取视频流ID
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1)
	{
		AfxMessageBox(L"could  not find video stream !");
		return -1;
	}
	TRACE(L"find video stream !");

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);//查找解码器
	if (pCodec == NULL)
	{
		AfxMessageBox(L"could not find decoder");
		return -1;
	}
	TRACE(L"find decoder");

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)//打开解码器
	{
		AfxMessageBox(L"could not open decoder");
		return -1;
	}
	TRACE(L"open decoder");

	av_dump_format(pFormatCtx, 0, filename, 0);

	pFrame = av_frame_alloc();//avcodec_alloc_frame //存储解码后AVFrame
	pFrameRGB = av_frame_alloc(); //存储转换后AVFrame（为什么要转换？后文解释）
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	if (pFrameRGB == NULL)
	{
		AfxMessageBox(L"open mem faile");
		return -1;
	}

	numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);//分配AVFrame所需内存

	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);////填充AVFrame

	IplImage* motion = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoStream)
		{
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);

			if (frameFinished)
			{
				struct SwsContext * img_convert_ctx;

				img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
				if (img_convert_ctx == NULL)
				{
					AfxMessageBox(L"Cannot initialize the conversion context !");
					return -1;
				}

				static uchar *p = NULL;

				p = pFrame->data[1];
				pFrame->data[1] = pFrame->data[2];
				pFrame->data[2] = p;

				sws_scale(img_convert_ctx,
					pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				if (img_convert_ctx)
				{
					sws_freeContext(img_convert_ctx);
					img_convert_ctx = NULL;
				}

				IplImage * Image;
				CvSize Size = cvSize(pCodecCtx->width, pCodecCtx->height);
				Image = cvCreateImage(Size, IPL_DEPTH_8U, 3);
				memcpy(Image->imageData, buffer, pCodecCtx->width*pCodecCtx->height * 24 / 8);
				Image->widthStep = pCodecCtx->width * 3;
				Image->origin = 0;
				//cvShowImage("video", Image);
				if (Image)//若取到则判断motion是否为空 
				{
					if (!motion)
					{
						motion = cvCreateImage(cvSize(Image->width, Image->height), 8, 1); //创建motion帧，八位，一通道 
						cvZero(motion); //零填充motion 
						motion->origin = Image->origin; //内存存储的顺序和取出的帧相同 
					}
				}
				//mDlg->mInvade.update_mhi(Image, motion, 60);//更新历史图像 
				//mDlg->mHogInvade.HogImage(Image);
				//if (mDlg->canSendData)
				//{
					//mDlg->canSendData = false;
					//mDlg->sendImage = *Image;
					//mDlg->SendMessage(WM_INVADE_HOG_MESSAGE, 0, 0);
				//}
				
				mDlg->DrawPicToHDC(Image, IDC_RTSP);
				//mDlg->UpdateData(FALSE);

				cvReleaseImage(&Image);
			}
		}
	}
	return 0;
}



void CRtspVlcPlayerDlg::OnBnClickedButtonPllay()
{
	// TODO:  在此添加控件通知处理程序代码
	//CWinThread* pThread0 = AfxBeginThread(ThreadRtspPlay, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	//pThread0->ResumeThread();

	CWinThread* pThread1 = AfxBeginThread(ThreadRtspNoDealPlay, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	pThread1->ResumeThread();

}

LRESULT CRtspVlcPlayerDlg::OnInvadeHogMessage(WPARAM wParam, LPARAM lParam)
{
	//TODO  
	IplImage  Image = sendImage;

	//mDlg->mInvade.update_mhi(Image, motion, 60);//更新历史图像 
	mHogInvade.HogImage(&Image);
	DrawPicToHDC(&Image, IDC_PIC);
	//mDlg->UpdateData(FALSE);
	canSendData = true;
	//cvReleaseImage(&Image);
	return 0;
}