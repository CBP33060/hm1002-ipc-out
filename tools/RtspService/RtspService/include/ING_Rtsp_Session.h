#pragma once
#include "INGCommon.h"
#include "RTSPComponent/RTSPObj.h"
#include "RTSPComponent/RTSPObj.h"

#define RTP_TRANSMODE_UDP 0
#define RTP_TRANSMODE_TCP 1

class ING_Rtsp_Session
{
public:
	enum
	{
		ctrl_none = 0,
		ctrl_pause,				// 暂停
		ctrl_play,				// 恢复播放
		ctrl_setpos,			// 定位拖放
		ctrl_setspeed,			// 调速
	};
	typedef struct _RANGE
	{
		double begin;
		double end;
	}RANGE;
	ING_Rtsp_Session(SESSION_HANDLE handle);
	~ING_Rtsp_Session(void);
	// 返回句柄
	SESSION_HANDLE handle(){
		return handle_;
	}
	// 返回URL信息
	void url(const std::string & url){
		url_ = url;
	}
	const std::string &url(){
		return url_;
	}
	unsigned int video_media_port(){
		return video_media_port_;
	}
	void video_media_port(unsigned short video_media_port){
		video_media_port_ = video_media_port;
	}
	// 远程IP
	unsigned int video_client_ip(){
		return video_client_ip_;
	}
	void video_client_ip(unsigned int video_client_ip){
		video_client_ip_ = video_client_ip;
	}
	// 端口
	unsigned int video_client_port(){
		return video_client_port_;
	}
	void video_client_port(unsigned short video_client_port){
		video_client_port_ = video_client_port;
	}
	unsigned short video_payload(){
		return video_payload_;
	}
	void video_payload(unsigned short video_payload){
		video_payload_ = video_payload;
	}
	void trans_mode(bool IsTCP) {
		if(IsTCP)
			trans_mode_ = RTP_TRANSMODE_TCP;
		else
			trans_mode_ = RTP_TRANSMODE_UDP;
	}
	unsigned short trans_mode() {
		return trans_mode_;
	}

	unsigned int ctrl(){
		return ctrl_;
	}
	void ctrl(unsigned int ctrl){
		ctrl_ = ctrl;
	}
private:
	SESSION_HANDLE handle_;
	std::string url_;
	bool video_track_valid_flg_;
	unsigned short video_media_port_;
	unsigned int video_client_ip_;
	unsigned short video_client_port_;
	unsigned short video_payload_;
	unsigned short trans_mode_;
	unsigned int video_ssrc_;
	bool audio_track_valid_flg_;
    unsigned int audio_client_ip_;
	unsigned short unaudio_client_port_;
	unsigned short audio_payoad_;
	unsigned int audio_ssrc_;
	unsigned int ctrl_;
};

typedef std::vector<ING_Rtsp_Session *> RTSP_SESSION_ARR;
