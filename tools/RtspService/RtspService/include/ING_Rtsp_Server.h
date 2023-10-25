#pragma once
// #ifdef WIN32
// #include "../../../ThirdPartyPlatform/RTSP/RTSPComponent/RTSPObj.h"
// #else
// #include "../RTSP/RTSPComponent/RTSPObj.h"
// #endif
#include "RTSPComponent/RTSPObj.h"
#include "ING_Guard.h"
#include "ING_Rtsp_Handler.h"

class ING_Rtsp_Server
{
public:
	ING_Rtsp_Server(void);
	~ING_Rtsp_Server(void);
	// 注册命令处理回调
	void register_handler(ING_Rtsp_Handler *handler, unsigned int ud = 0){
		handler_ = handler;
		handler_user_data_ = ud;
	}

	int open(unsigned short listen_port, unsigned short medio_port_min, unsigned short medio_port_max);
	int close();
	int run();

// 	unsigned short medio_port(){
// 		return medio_port_min_;
// 	}
	unsigned short listen_port(){
		return listen_port_;
	}

	void set_local_ip(const char *pszLocalIP){
		local_ip_.assign(pszLocalIP);
	}

private:
	RTSP_HANDL rtsp_handle_;
	ING_Rtsp_Handler *handler_;   //ING_Cotroller  无所不能的对象！
	unsigned int handler_user_data_;

	ING_Mutex mutex_;
	RTSP_SESSION_ARR establishing_seesion_arr_;    //  正在创建的会话队列
	RTSP_SESSION_ARR established_seesion_arr_;     //  已经创建的会话队列
// 	unsigned short medio_port_min_;
// 	unsigned short medio_port_max_;

	unsigned short listen_port_;
	std::string local_ip_;
};
