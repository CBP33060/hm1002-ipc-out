#include "ING_Rtsp_Session.h"

ING_Rtsp_Session::ING_Rtsp_Session(SESSION_HANDLE handle)
: handle_(handle)
, url_("")
, video_media_port_(0)
, video_client_ip_(0)
, video_client_port_(0)
, video_payload_(0)
, ctrl_(ctrl_none)
{
	trans_mode_ = RTP_TRANSMODE_UDP;	//默认使用UDP传输方式
	printf("ING_Rtsp_Session constructor.\n");
}

ING_Rtsp_Session::~ING_Rtsp_Session(void)
{
	printf("ING_Rtsp_Session destructor.\n");
}
