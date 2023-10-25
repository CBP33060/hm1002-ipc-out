#ifndef __IPC_MANAGE_MODULE_H__
#define __IPC_MANAGE_MODULE_H__
#include "module.h"
#include "timer_mx.h"
#include "ipc_manage_access.h"

namespace maix {

	#define PLAY_FILE_RESET_SUCCESS						"reset_success"		///< 重置音效

	class CIPCManageBindState;
	class CIPCManageAccess;
	
	class MAIX_EXPORT CIPCManageModule : public CModule
	{
	public:
		CIPCManageModule(std::string strGUID, std::string strName);
		~CIPCManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
		mxbool initAccess();
		mxbool initLocalStorage();
		mxbool initCenterSubsystem();
		mxbool initBindState();
		mxbool initOTA();
        mxbool initLogManage();
		

		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		mxbool initEventModuleServer();
		mxbool initDevModuleServer();
		mxbool initAudioModuleServer();
		mxbool initVideoModuleServer();
		mxbool initSpeakerModuleServer();
		mxbool initMcuModuleServer();
		void   sendMcuAesRun();
		std::string sendAesProc();
		mxbool setAES128Key(unsigned char *pcAESKey);
		mxbool getAES128Key(unsigned char *pcAESKey);
		mxbool setUpPersistentConnect(std::string strIp, int iPort);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
		std::string enterLowPower(std::string strParam);
		std::string sendEventToCenterSystem(std::string strParam);
		std::string sendEventToIPC(std::string strEvent,
			std::string strParam);
		std::string sendAllIPCDevInfo(std::string strEvent,
			std::string strParam);
		std::string sendOTAStatusToIPC(std::string strEvent,
			std::string strParam);
        std::string sendLogDataToIPC(std::string strEvent,
            std::string strParam);
		std::string sendEventToLocalStorage(std::string strEvent,
			std::string strParam);
		std::string sendSpecData(std::string strEvent,
			std::string strParam);
		std::string ipcAgentConfig(std::string strParam);
		std::string sendPirFalseWakeup();
        void sendToIpcGetbindState();
		
		mxbool unBind();
		mxbool playAudioFile(std::string strFileId);

        mxbool sendMikeEnterLowpower();
		mxbool getAllDevInfo();
		
	private:
		std::string m_strEventModuleGUID;
		std::string m_strEventModuleRemoteServer;
		std::string m_strDevModuleGUID;
		std::string m_strDevModuleRemoteServer;
		std::string m_strAudioModuleGUID;
		std::string m_strAudioModuleRemoteServer;
		std::string m_strVideoModuleGUID;
		std::string m_strVideoModuleRemoteServer;
		std::string m_strSpeakerModuleGUID;
		std::string m_strSpeakerModuleRemoteServer;
		std::string m_strMcuModuleGUID;
		std::string m_strMcuModuleRemoteServer;

		std::thread m_mapAccessSessionProc;
		std::thread m_mapBindStateProc;
		std::string m_strDID;
		unsigned char m_acAESKey[16];
		
		std::thread m_threadEnterLowPower;
		std::thread m_threadSendMcuAes;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;
		std::mutex m_mutexSendAes;
		std::condition_variable m_conditionSendAes;
	
	    std::string m_strIPCAgentGUID;
		std::string m_strIPCAgentRemoteEventServer;

		std::string m_strLocalStorageGUID;
		std::string m_strLocalStorageRemoteEventServer;

		std::string m_strCenterSubSystemGUID;
		std::string m_strCenterSubSystemServer;

		CIPCManageBindState *m_objIPCManageBindState;
		CIPCManageAccess *m_objIPCManageAccess;

		std::string m_strOTAGUID;
		std::string m_strOTARemoteEventServer;

        std::string m_strLOGGUID;
        std::string m_strLOGRemoteEventServer;
		
		unsigned int  m_iRelayIndex;
		unsigned int  m_iIPCIndex;		
	};
}
#endif //__IPC_MANAGE_MODULE_H__
