#pragma once
#include "INGCommon.h"
#include "ING_Rtsp_Server.h"
#include "ING_Mutex.h"
#include "ING_Stream_Mgr.h"


enum
{
	GET_NET_STATUS_UNSTATRED = 0,
	GET_NET_STATUS_REQURING,
	GET_NET_STATUS_SUCCEED,
	GET_NET_STATUS_FAILED
};

class ING_Controller
	:public ING_Rtsp_Handler
{
public:
// 	typedef std::map<ING_C7_Cmd*, SIP_Cmd*> C7_TO_SIP_CMD_MAP; // C7命令到SIP命令的映射
// 	typedef std::map<std::string, C7_RECORD_ARR> USER_TO_C7_RECORD_MAP;
// 	typedef struct _STREAM_REQUEST {
// 		ING_C7_Cmd* c7_cmd;
// 		ING_Stream *stream;
// 	}STREAM_REQUEST;
// 	typedef std::vector<SIP_Cmd *>  SIP_CMD_ARR;
// 	typedef std::list<STREAM_REQUEST *> STREAM_REQUEST_LIST;
// 	typedef std::vector<STREAM_REQUEST *> STREAM_REQUEST_ARR;
	ING_Controller(void);
	virtual ~ING_Controller(void);
	// 启动/停止模块
	int open(const char *param);
	int close();
	// 执行
	int run();

protected:
	// 对传入参数进行解析
	// rtsp_server模块、stream_mgr模块开启
// 	int open_kernel(void);

	// 会话正在创建中
	virtual int handle_session_create(ING_Rtsp_Session *session, unsigned int user_data);
	// 会话已经建立
	virtual int handle_session_established(ING_Rtsp_Session *session, unsigned int user_data);
	// 会话已经关闭
	virtual int handle_session_closing(ING_Rtsp_Session *session, unsigned int user_data);
	// 会话控制
	virtual int handle_session_control(ING_Rtsp_Session *session, unsigned int user_data);
private:
// 	std::string platform_id_;			    // 平台编码ID
// 	std::string platform_ip_;				// 前端系统IP地址
// 	unsigned short sip_remote_port_;		// 前端系统端口	
	std::string local_ip_;					// 本地的IP			
	unsigned short rtsp_accept_port_;		// rtsp服务绑定的本地TCP端口
	int get_net_ip_;						// 设备的地址
	std::string crypt_server_url_;
	std::string crypt_iv_no_;
	std::thread* stream_mgr_thread_;
	std::thread* ISPRunMode_switch_thread_;
private:
	ING_Mutex mutex_;
// 	STREAM_REQUEST_LIST to_process_stream_req_list_; // 要处理的流请求
// 	STREAM_REQUEST_ARR processing_stream_req_arr_;	// 正在处理的流请求
private:
	ING_Stream_Mgr *stream_mgr_;
	ING_Rtsp_Server *rtsp_server_;
};
