#include "ING_Controller.h"
#include "RTSPComponent/RTSPCommon.h"
#include <stdio.h>

ING_Controller::ING_Controller(void)
	: rtsp_server_(NULL)
// 	, get_net_ip_(GET_NET_STATUS_UNSTATRED)
{
	printf("ING_Controller constructor.\n");
}

ING_Controller::~ING_Controller(void)
{
	printf("ING_Controller destructor.\n");
}
int ING_Controller::run()
{
	// 	printf("ING_Controller::run()********************\n");
	static unsigned int time_now = 0;
	unsigned long ulTickCount = GetTickCount();
// 	printf("***************ING_Controller::run() TickCount=%lu time_now=%d Differ=%lu***************\n", ulTickCount, time_now, ulTickCount - time_now);
	if (ulTickCount - time_now > 1000 * 20)
	{
		printf("***************ING_Controller::run()***************\n");
		time_now = ulTickCount;
	}
	int ret = 0;
// 	ret |= stream_mgr_->run();
	ret |= rtsp_server_->run();
	return ret;
}
#define ETH_NAME       "eth0"
char* get_local_ip()
{
	// int sock;
	// struct sockaddr_in sin;
	// struct ifreq ifr;
	// sock = socket(AF_INET, SOCK_DGRAM, 0);
	// if (sock == -1) {
	// 	perror("socket");
	// 	return NULL;
	// }
	// strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
	// ifr.ifr_name[IFNAMSIZ - 1] = 0;
	// if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
	// 	perror("ioctl");
	// 	return NULL;
	// }
	// memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	// return inet_ntoa(sin.sin_addr);
	return "0.0.0.0";
}

int ISPRunMode_Switch()
{
	IMPVI_NUM vinum = IMPVI_MAIN;
	IMPISPRunningMode pmode;

	while (true)
	{
		if (!access("/tmp/dn", F_OK))//夜晚
		{
			FILE *fp = fopen("/tmp/dn", "r");
			char ch = fgetc(fp);

			if(ch == '0')
			{
				printf("### entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
				pmode = IMPISP_RUNNING_MODE_NIGHT;
			}
			else
			{
				printf("### entry night mode IMPISP_RUNNING_MODE_DAY###\n");
				pmode = IMPISP_RUNNING_MODE_DAY;
			}

			IMP_ISP_Tuning_SetISPRunningMode(vinum, &pmode);

			// IMPISPAEExprInfo info;
			// int ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
			// if(ret){
			// 	printf("IMP_ISP_Tuning_GetAeExprInfo error !\n");
			// 	return -1;
			// }

			// info.AeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
			// // info.AeIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
			// // info.AeAGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
			// // info.AeIspDGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
			// info.AeIntegrationTime = 3492;
			// info.AeAGain = 30720;
			// info.AeIspDGain = 1024;

			// ret = IMP_ISP_Tuning_SetAeExprInfo(IMPVI_MAIN, &info);
			// if(ret){
			// 	printf("IMP_ISP_Tuning_SetAeExprInfo error !\n");
			// 	return -1;
			// }

			// IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
			// if (pmode!=IMPISP_RUNNING_MODE_NIGHT) 
			// {
			// 	printf("### entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
			// 	IMPISPRunningMode mode = IMPISP_RUNNING_MODE_NIGHT;
			// 	IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
			// }
			// else
			// {
			// 	printf("### entry night mode IMPISP_RUNNING_MODE_DAY###\n");
			// 	IMPISPRunningMode mode = IMPISP_RUNNING_MODE_DAY;
			// 	IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
			// }
			remove("/tmp/dn");
		}

		sleep(1);
	}
	
	return 0;
}


int ING_Controller::open(const char *param)
{
	int ret = 0;

	// 解析参数
	printf("*********************controller::open*************************\n");
	stream_mgr_ = new ING_Stream_Mgr();

	// 创建并设置DLG工厂
	rtsp_server_ = new ING_Rtsp_Server();
	rtsp_server_->register_handler(this);
	//rtsp://193.168.41.108:5060
	ret = rtsp_server_->open(5060, 24400, 30000);
	char *pcIP = get_local_ip();
	if(pcIP==NULL)
	{
		return -1;
	}
	else
	{
		printf("*******controller::open get localip success*******\n");
		printf("*****************please use rtsp://%s:5060/ to play live video***********************\n", pcIP);
	}
	rtsp_server_->set_local_ip(pcIP); //调试需要，实际需要实时获取

	//创建线程,收发数据流
	ret = stream_mgr_->open();
	stream_mgr_thread_ = new std::thread(&ING_Stream_Mgr::run, stream_mgr_);
	ISPRunMode_switch_thread_ = new std::thread(ISPRunMode_Switch);
	printf("*********************get_net_ip_*************************\n");
	return ret;
}

// int ING_Controller::open_kernel()
// {
// 
// }

int ING_Controller::close()
{
	printf("*********************controller::close*************************\n");
	if (rtsp_server_ != NULL)
	{
		rtsp_server_->close();
	}
	if (rtsp_server_ != NULL)
	{
		delete rtsp_server_;
		rtsp_server_ = NULL;
	}
	//销毁线程,然后删除stream_mgr

	if (stream_mgr_ != NULL)
	{
		stream_mgr_->close();
		stream_mgr_thread_->join();
		delete stream_mgr_;
		stream_mgr_ = NULL;
	}
	if (stream_mgr_thread_ != NULL)
	{
		delete stream_mgr_thread_;
	}
	return 0;
}

int ING_Controller::handle_session_create(ING_Rtsp_Session *session, unsigned int user_data)
{
	printf("ING_Controller::handle_session_create..\n");
	std::string url = session->url();
	std::string record_file_name;

	std::string diskLetter = "";
	std::string res_id;

	return 0;
}

int ING_Controller::handle_session_established(ING_Rtsp_Session *session, unsigned int user_data)
{
	printf("ING_Controller::handle_session_established..\n");
	return 0;
}


int ING_Controller::handle_session_closing(ING_Rtsp_Session *session, unsigned int user_data)
{
	printf("ING_Controller::handle_session_closing..\n");
	// 收到会话关闭请求
	stream_mgr_->remove_stream((unsigned int)session->handle());
	return 0;
}

int ING_Controller::handle_session_control(ING_Rtsp_Session *session, unsigned int user_data)
{
	printf("ING_Controller::handle_session_control..opt = %d\n", session->ctrl());
	unsigned short usIsTCP = session->trans_mode();
	switch (session->ctrl())
	{
	case ING_Rtsp_Session::ctrl_pause:    //暂停
	{
		// 关闭已有的流
// 		stream_mgr_->pause_stream((unsigned int)session->handle());
		return 0;
	}
	break;
	case ING_Rtsp_Session::ctrl_play:
	{
		int ret = 0;
		// modified by slz rtsp播放命令处理
		ING_Stream_Vdo *pstINGStream = stream_mgr_->find_stream((unsigned int)session->handle());
		if (pstINGStream == NULL)         //没有流，创建
		{
			char video_client_ip[RTSP_MAX_IPSTR_LEN] = "";
			RTSP_U32ToAddr(session->video_client_ip(), video_client_ip);
			printf("ING_Controller::new ING_Stream_Vdo mode=%d! LocalPort=%d, clientIP=%s, clientPort=%d\n", usIsTCP,
												session->video_media_port(), video_client_ip, session->video_client_port());
			ING_Stream_Vdo *new_stream = NULL;
			if (usIsTCP)
			{
				SOCKET tcpFd = (SOCKET)RTSP_GetConnectFd(session->handle());
				new_stream = new ING_Stream_Vdo((unsigned int)session->handle(), ING_Stream_Vdo::stream_type_realtime, "video", 96, tcpFd);
			}
			else
			{
				new_stream = new ING_Stream_Vdo((unsigned int)session->handle(),
					session->video_media_port(), video_client_ip, session->video_client_port(), ING_Stream_Vdo::stream_type_realtime, "video", 96);
			}

			ret = new_stream->open();
			if (ret != 0)
			{
				printf("ING_Controller::handle_session_control ING_Stream_Vdo open err! ret=%d\n", ret);
				new_stream->close();
				return ret;
			}
			// 根据SDP信息创建流
			ret = stream_mgr_->add_stream(new_stream);
			if (ret != 0)
			{
				printf("ING_Controller::handle_session_established ING_Stream_Mgr::add_stream err! ret=%d\n", ret);
				new_stream->close();
				return ret;
			}
		}
		else  // 有流，继续发送已有的流
		{
			pstINGStream->resume();
// 			stream_mgr_->resume_stream((unsigned int)session->handle());
		}

		return 0;
	}
	case ING_Rtsp_Session::ctrl_setspeed:
		// 暂停
	{
		/*
		bool pause_flag = false;
					stream_mgr_->is_stream_pause((unsigned int)session->handle(), pause_flag);
					if (pause_flag)
					{
						stream_mgr_->resume_stream((unsigned int)session->handle());
						return 0;
					}*/

// 		double scale = session->scale();
// 		stream_mgr_->set_scale((unsigned int)session->handle(), scale);
// 		// 继续发送已有的流
// 		stream_mgr_->resume_stream((unsigned int)session->handle());
		return 0;
	}
	break;
	case ING_Rtsp_Session::ctrl_setpos:
		// 暂停
	{
		/*************************************************************************************************
			modified by slz 平台对接时拖拽rtsp命令处理，
			如果存在stream    不删除原有RTPSession(重新建立流对象保留原来的jrtphandle seqnum timestamp及暂停位标志)
			如果不存在        新建RTPSession
		***************************************************************************************************/
		ING_Stream_Vdo* pstINGStream = stream_mgr_->find_stream((unsigned int)session->handle());
		if (pstINGStream != NULL)
		{
			printf("ING_Controller::ING_Rtsp_Session::ctrl_setpos find_stream is not null\n");
			// 关闭已有的流
			pstINGStream->remove();
		}
		else
		{
			printf("ING_Controller::ING_Rtsp_Session::ctrl_setpos use setpos to play\n");
		}

		// 重新启动流
		char video_client_ip[RTSP_MAX_IPSTR_LEN] = "";
		RTSP_U32ToAddr(session->video_client_ip(), video_client_ip);
		int ret = -1;
		ING_Stream_Vdo *new_stream = NULL;
			new_stream = new ING_Stream_Vdo((unsigned int)session->handle(),
			session->video_media_port(), video_client_ip, session->video_client_port(), ING_Stream_Vdo::stream_type_vod, "video", 100);

		ret = new_stream->open();
		if (ret != 0)
		{
			printf("ING_Controller::handle_session_control ING_Stream_Vdo open err! ret=%d\n", ret);
			return ret;
		}
		// 根据SDP信息创建流
		ret = stream_mgr_->add_stream(new_stream);
		if (ret != 0)
		{
			printf("ING_Controller::handle_session_established ING_Stream_Mgr::add_stream err! ret=%d\n", ret);
			return ret;
		}

		std::string url = session->url();
		std::string record_file_name;
		std::string diskLetter = "";
		// 根据filename找到设备的id信息（获取摄像头索引索引）
		std::string res_id;
		std::string res_idx;

		// 查找资源以及文件成功
// 		u_int uiTimePos = (u_int)range.begin - begin_time;
		printf("ING_Controller::ING_Rtsp_Session::ctrl_setpos\n");
		return 0;
	}
	break;
	}
	return -1;
}
