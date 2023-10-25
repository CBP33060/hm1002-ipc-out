#pragma once
#include "ING_Rtsp_Session.h"
class ING_Rtsp_Handler
{
public:
	ING_Rtsp_Handler(void);
	virtual ~ING_Rtsp_Handler(void);
	// 关于会话的几个接口
	// 会话正在创建中
	virtual int handle_session_create(ING_Rtsp_Session *session, unsigned int user_data) = 0;
	// 会话已经建立
	virtual int handle_session_established(ING_Rtsp_Session *session, unsigned int user_data) = 0;
	// 会话已经关闭
	virtual int handle_session_closing(ING_Rtsp_Session *session, unsigned int user_data) = 0;

	// 会话控制
	virtual int handle_session_control(ING_Rtsp_Session *session, unsigned int user_data) = 0;
private:

};
