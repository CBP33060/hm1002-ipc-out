#ifndef __EVENT_MANAGE_MODULE_H__
#define __EVENT_MANAGE_MODULE_H__
#include <memory>
#include "module.h"
#include "event_manage_attempt_report.h"

namespace maix {
	class MAIX_EXPORT CEventManageModule : public CModule
	{
	public:
		CEventManageModule(std::string strGUID, std::string strName);
		~CEventManageModule();

		mxbool init();
		mxbool unInit();
		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		mxbool initAttemptReport();
        mxbool initMcuRemote();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
		std::string enterLowPower(std::string strParam);
		std::string sendAlarmEvent(std::string strEvent, std::string strParam);
		std::string sendAlertEvent(std::string strEvent, std::string strParam);
		std::string sendSpecData(std::string strEvent, std::string strParam);
		void sendToCenterEventNoOccurs();

		mxbool sendSpecDataVoiceLight(std::string strParam);
		mxbool sendSpecDataRecord();

		int64_t getCurrentTime();
		mxbool initIpcEventServer();
	private:

		void sendToCenterEventOccurs();
        std::string parseMCUEvemt(std::string param);

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

		std::shared_ptr<CEventManageAttemptReport> m_objEventManageAttemptReport;
		std::thread m_threadEventManageAttemptReport;

		std::string m_strIpcManageGUID;
		std::string m_strIpcManageServer;

		std::string m_strMcuManageGUID;
		std::string m_strMcuManageServer;
	};
}
#endif //__EVENT_MANAGE_MODULE_H__
