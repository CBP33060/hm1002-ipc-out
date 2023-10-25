#include "ING_Rtsp_Server.h"
#include "ING_Rtsp_Session.h"


ING_Rtsp_Server::ING_Rtsp_Server(void)
	:rtsp_handle_(NULL)
	, handler_(NULL)
{
// 	printf("ING_Rtsp_Server constructor.\n");
}

ING_Rtsp_Server::~ING_Rtsp_Server(void)
{
// 	printf("ING_Rtsp_Server destructor.\n");
}

int ING_Rtsp_Server::open(unsigned short listen_port, unsigned short medio_port_min, unsigned short medio_port_max)
{
	rtsp_handle_ = RTSP_Create(listen_port, medio_port_min, medio_port_max, 30 * 1000,
		30 * 1000);
	if (rtsp_handle_ == NULL)
	{
		printf("RTSP_Create failed\n");
		return -1;
	}
// 	medio_port_min_ = medio_port_min;
// 	medio_port_max_ = medio_port_max;

	listen_port_ = listen_port;
	return 0;
}

int ING_Rtsp_Server::close()
{
	if (rtsp_handle_ != NULL)
	{
		RTSP_Destroy(rtsp_handle_);
		rtsp_handle_ = NULL;
	}
	return 0;
}

int ING_Rtsp_Server::run()
{
	int ret = -1;
// 	printf("ING_Rtsp_Server::run start\n");
	ret = RTSP_Run(rtsp_handle_);
	if (ret != RTSP_ERROR_OK)
	{
		printf("RTSP_Run error\n");
		return ret;
	}
	// 
	{
		SESSION_HANDLE session_handle = RTSP_AcceptSession(rtsp_handle_);
		if (session_handle != NULL)
		{
			RTSP_SetLocalIP(session_handle, local_ip_.c_str());
			// 创建新的会话
			ING_Rtsp_Session *new_seesion = new ING_Rtsp_Session(session_handle);
			// 放到正在创建会话队列中
			ING_Guard guard(mutex_);
			establishing_seesion_arr_.push_back(new_seesion);
		}
	}
	{
		ING_Guard guard(mutex_);
		RTSP_SESSION_ARR::iterator iter = establishing_seesion_arr_.begin();
		for (; iter != establishing_seesion_arr_.end(); )
		{
			ING_Rtsp_Session *this_session = *iter;
			// 获取URL的状态
			ret = RTSP_GetSessionStatus(this_session->handle());
			if (ret != RTSP_ERROR_OK)
			{
				switch (ret)
				{
				case RTSP_ERROR_WOULDBLOCK:
					iter++;
					break;
				default:
					if (handler_ != NULL)
					{
						handler_->handle_session_closing(this_session, handler_user_data_);
					}
					iter = establishing_seesion_arr_.erase(iter);
					RTSP_DeleteSession(rtsp_handle_, this_session->handle());
					delete this_session;
				}
				continue;
			}
			if (this_session->url().size() == 0)
			{
				// 获取会话的URL信息
				unsigned char url[256] = "";
				int url_len = sizeof(url);
				ret = RTSP_GetURL(this_session->handle(), url, &url_len);
				if (ret == 0)
				{
					this_session->url((char *)url);
					ret = handler_->handle_session_create(this_session, handler_user_data_);
					if (ret != 0)
					{
						RTSP_DeleteSession(rtsp_handle_, this_session->handle());
						delete this_session;
						iter = establishing_seesion_arr_.erase(iter);
					}
					else
					{
						// 只支持视频
						ret = RTSP_SetTrackNum(this_session->handle(), 1);
						//关键点 端口分配
						unsigned short video_media_port = RTSP_GetMediaPort(rtsp_handle_);
						printf("ING_RtspServer::run() RTSP_AddVideoTrack\n");
						ret = RTSP_AddVideoTrack(this_session->handle(), 0, NULL, 0, 25, video_media_port);
						// 						ret = RTSP_AddVideoTrack(this_session->handle(), 0, NULL, 0, 100, 25, this_session->end_time(), this_session->begin_time(), video_media_port);
						if (ret != RTSP_ERROR_OK)
						{
							if (handler_ != NULL)
							{
								handler_->handle_session_closing(this_session, handler_user_data_);
							}
							RTSP_DeleteSession(rtsp_handle_, this_session->handle());
							delete this_session;
							iter = establishing_seesion_arr_.erase(iter);
						}
						else
						{
							iter++;
						}
					}
				}
				else
				{
					iter++;
				}
			}
			else
			{
				int video_track_valid_flg = 0;
				unsigned short video_media_port = 0;
				unsigned int video_client_ip = 0;
				unsigned short video_client_port = 0;
				int video_pay_load = 0;
				unsigned int video_ssrc = 0;
				int audio_track_valid_flg = 0;
				unsigned int audio_client_ip = 0;
				unsigned short audio_client_port = 0;
				int audio_payoad = 0;
				unsigned int audio_ssrc = 0;
				bool IsTCP = false;
				ret = RTSP_GetPlayTrackInfo(this_session->handle(), &video_track_valid_flg, &video_media_port, &video_client_ip,
											&video_client_port, &video_pay_load, &video_ssrc,
											&audio_track_valid_flg, &audio_client_ip, &audio_client_port,
											&audio_payoad, &audio_ssrc, &IsTCP);
				if (ret == RTSP_ERROR_OK)
				{
					// 设置参数
					this_session->video_media_port(video_media_port);
					this_session->video_client_ip(video_client_ip);
					this_session->video_client_port(video_client_port);
					this_session->video_payload(video_pay_load);
					this_session->trans_mode(IsTCP);
					if (handler_ != NULL)
					{
						ret = handler_->handle_session_established(this_session, handler_user_data_);
						if (ret != 0)
						{
							RTSP_DeleteSession(rtsp_handle_, this_session->handle());
							delete this_session;
						}
						else
						{
							established_seesion_arr_.push_back(this_session);
						}
						iter = establishing_seesion_arr_.erase(iter);
					}
					else
					{
						iter++;
					}
				}
				else
				{
					iter++;
				}
			}
		}
	}
	{
		ING_Guard guard(mutex_);
		RTSP_SESSION_ARR::iterator iter = established_seesion_arr_.begin();
		for (; iter != established_seesion_arr_.end(); )
		{
			ING_Rtsp_Session *this_session = *iter;
			ret = RTSP_GetSessionStatus(this_session->handle());
			if (ret != RTSP_ERROR_OK)
			{
				switch (ret)
				{
				case RTSP_ERROR_WOULDBLOCK:
					iter++;
					break;
				default:
					printf("ING_Rtsp_Server::run  RTSP_GetSessionStatus err!\n");
					// 关闭连接
					if (handler_ != NULL)
					{
						handler_->handle_session_closing(this_session, handler_user_data_);
					}
					iter = established_seesion_arr_.erase(iter);
					RTSP_DeleteSession(rtsp_handle_, this_session->handle());
					delete this_session;
					break;
				}
			}
			else
			{
				bool bPlayFlag = false;
				ret = RTSP_FetchPlayFlag(this_session->handle(), bPlayFlag);
				if (ret == 0)
				{
					this_session->ctrl(ING_Rtsp_Session::ctrl_play);
					handler_->handle_session_control(this_session, handler_user_data_);
				}
				bool bPauseFlag = false;
				ret = RTSP_FetchPauseFlag(this_session->handle(), bPauseFlag);
				if (ret == 0)
				{
					this_session->ctrl(ING_Rtsp_Session::ctrl_pause);
					handler_->handle_session_control(this_session, handler_user_data_);
				}
				iter++;
			}
		}
	}
// 	printf("ING_Rtsp_Server::run end\n");

	return 0;
}
