#ifndef __DEV_MANAGE_MODULE_H__
#define __DEV_MANAGE_MODULE_H__
#include "module.h"
#include "dev_play_voice.h"
#include "dev_manage_spec_handle.h"

namespace maix {
	class CDevSpecHandle;
	
	class MAIX_EXPORT CDevManageModule : public CModule
	{
	public:
		CDevManageModule(std::string strGUID, std::string strName);
		~CDevManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
		mxbool initPalyVoice();
		mxbool initVideoSourceRemoteEventServer();
		mxbool initMcuSerialPortRemoteEventServer();
		mxbool initAiRemoteEventServer();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

		std::string getVideoModuleGuid();
		std::string getVideoModuleRmoteServerName();
		std::string outputMsg(std::string strGUID, std::string strModuleName,
			std::string strEvent, std::string strValue);
		std::string outputConfigVideo(std::string strGUID, std::string strModuleName,
			std::string strEvent, std::string strValue);
		std::string enterLowPower(std::string strParam);

        std::string playWithFileId(std::string strParam);
        std::string playWithFilePath(std::string strParam);
        std::string stopWithFileId(std::string strParam);

		std::string getMcuSerialPortGUID();
		std::string getMcuSerialPortServer();

		std::string getLowPowerGUID();
		std::string getLowPowerServer();

		std::string getAiGUID();
		std::string getAiServer();

		std::string handleDevConfig(std::string strParam);

		std::string getAiEvent(std::string strParam);

	private:
		std::shared_ptr<CDevPlayVoice> m_objDevPlayVoice;
		std::shared_ptr<CDevSpecHandle> m_objDevSpecHandle;

		std::string m_strVideoModuleGUID;
		std::string m_strVideoModuleRemoteServer;
		
		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

		std::string m_strMcuSerialPortGUID;
		std::string m_strMcuSerialPortServer;	

		std::string m_strAiGUID;
		std::string m_strAiServer;	
	};
}
#endif //__DEV_MANAGE_MODULE_H__
