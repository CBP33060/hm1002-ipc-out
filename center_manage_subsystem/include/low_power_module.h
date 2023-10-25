#ifndef __LOW_POWER_MODULE_H__
#define __LOW_POWER_MODULE_H__
#include "module.h"
#include "low_power_inform_flow.h"

namespace maix {
	typedef enum
	{
		E_EventDetectStart,
		E_EventNoOccur,
		E_EventOccur,
		E_EventIdle,
	}E_Event_Detect_Type;

	class MAIX_EXPORT CLowPowerModule : public CModule
	{
	public:
		CLowPowerModule(std::string strGUID, std::string strName);
		~CLowPowerModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initEventDetect();
		mxbool initLowPowerFlow();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

		E_Module_Type getModuleType();
		mxbool setModuleType(E_Module_Type eType);
		
		std::string eventOccurs(std::string strParam);
		std::string ipcManageExit(std::string strParam);
		std::string devManageExit(std::string strParam);
		std::string eventManageExit(std::string strParam);
		std::string videoManageExit(std::string strParam);
		std::string audioManageExit(std::string strParam);
		std::string speakerManageExit(std::string strParam);
        std::string logManageExit(std::string strParam);
        std::string VideoSourceExit(std::string strParam);
        std::string parseRecordTime(std::string strParam);
		std::string eventNoOccurs(std::string strParam);

		void eventDetectProc();
		E_Event_Detect_Type getLastEventDetectType();
		E_Event_Detect_Type getEventDetectType();
		mxbool setEventDetectType(E_Event_Detect_Type eType);
	private:
		std::mutex m_mutexEventDetectType;
		E_Event_Detect_Type m_eLastEventDetectType;
		E_Event_Detect_Type m_eEventDetectType;
		int m_iCumulativeTime;
		int m_iEventDetectTime;
		int m_iRecordLastTime;
		int m_iCumulativeRecordTime;
		int m_iLongLastTime;
		int m_iCumulativeLongLastTime;
        int m_iUserSetRecordTime;
		std::shared_ptr<CLowPowerInformFlow> m_lowPowerInformFlow;
		std::thread m_threadLowPowerFlow;
		std::thread m_threadEventDetect;
	};
}
#endif //__LOW_POWER_MODULE_H__
