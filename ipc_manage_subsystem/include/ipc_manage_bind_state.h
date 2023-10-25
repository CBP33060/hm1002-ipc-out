#ifndef __IPC_MANAGE_BIND_STATE_H__
#define __IPC_MANAGE_BIND_STATE_H__
#include "ipc_manage_module.h"
#include <list>

namespace maix {

	#define ENV_BIND_STATUS								"bind_status"
	#define LOWPOWER_MODE							    "lowpower_mode"

	#define PLAY_WAITING_CONNECT						"waiting_connect"		//等待连接
	#define PLAY_WIFI_CONNECTING						"wifi_connecting"		//连接中，请稍后
	#define PLAY_WIFI_CONNECT_ERR 						"wifi_connect_err"      //连接失败
	#define PLAY_WIFI_CONNECT_SUCCESS					"wifi_connect_success"  //连接成功

	#define WIFI_AP_MODE 								"wifi_ap_mode"
	#define WIFI_CONNECTING 							"wifi_connecting"
	#define WIFI_FAILED 								"wifi_failed"

	typedef enum _LedInfo_Color {
		LedInfo_Color_RED = 0,
		LedInfo_Color_GREEN = 1,
		LedInfo_Color_BLUE = 2,
		LedInfo_Color_ORANGE = 3,
		LedInfo_Color_WHITE = 4
	} LedInfo_Color;

	typedef enum _LedInfo_State {
		LedInfo_State_OFF = 0,
		LedInfo_State_ON = 1,
		LedInfo_State_FLASHING = 2,
		LedInfo_State_BREATHING = 3
	} LedInfo_State;

	typedef enum
	{
		E_BIND_WAITING,
		E_BIND_TIMEOUT,
		E_BIND_CONNECTING,
		E_BIND_FAILED,
		E_BIND_SUCCESS,
		E_BIND_CONNECT,
		E_BIND_DISCONNECT,
		E_BIND_IDLE,
	}E_BIND_STATE;

	class CIPCManageBindState
	{
	public:
		CIPCManageBindState(CIPCManageModule *objIPCManageModule, 
			std::string strMcuModuleGUID, std::string strMcuModuleRemoteServer);
		~CIPCManageBindState();

		mxbool init();
		mxbool unInit();
		mxbool connectOT();
		mxbool disConnectOT();
		mxbool getBindStateFromOT(std::string &strState);

		mxbool sendLedStatus(int iColor, int iState, int iOnTime, int iOffTime);
		mxbool sendVoicePrompts(std::string strVoiceType);
		mxbool setBindState(E_BIND_STATE bindState);
		mxbool getBindState();

		void run();
		void runWaiting();

		E_BIND_STATE getState();
		mxbool setState(E_BIND_STATE eState);

	private:
		bool isHandleStep(E_BIND_STATE eState);

		CIPCManageModule* m_objIPCManageModule;
		std::string m_strMcuModuleGUID;
		std::string m_strMcuModuleRemoteServer;
		E_BIND_STATE m_eBindState;

#ifdef _WIN32
		SOCKET m_sockClient;
#else
		int m_sockClient;
#endif
		int m_connetErrno;
		std::string m_strBindState;
		std::thread m_threadWait;
		mxbool m_bWait;
		mxbool m_bRun;
	};
}
#endif //__IPC_MANAGE_BIND_STATE_H__
