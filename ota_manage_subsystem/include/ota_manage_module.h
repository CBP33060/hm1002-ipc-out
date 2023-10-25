#ifndef __OTA_MANAGE_MODULE_H__
#define __OTA_MANAGE_MODULE_H__
#include "module.h"
#include <stdio.h>
#include "ini_config.h"
#include "timer_mx.h"
#include "ota_manage_upgrade_process.h"
#include "ota_manage_progress_report.h"

namespace maix {
	#define LOWPOWER_MODE							    "lowpower_mode"
	#define ENABLE_LOWPOWER								1
	#define DISABLE_LOWPOWER							0
	#define FILE_OTA_START								"ota_upgrading"
	#define FILE_OTA_FAILED								"ota_upgrade_fail"
	#define FILE_OTA_SUCCESS							"ota_upgrade_success"
	#define OTA_LOW_BATTERY								10

	typedef enum
	{
		E_EVENT_NORMAL = 0,
		E_EVENT_LOW_POWER_5,
		E_EVENT_LOW_POWER_10,
		E_EVENT_HIGH_VOLTAGE,
		E_EVENT_LOW_TEMP,
		E_EVENT_HIGH_TEMP,
		E_EVENT_AI_VEHICLE,
		E_EVENT_AI_PET,
		E_EVENT_AI_PACKAGE,
		E_EVENT_AI_PERSON,
		E_EVENT_LOW_POWER_OTA,
		E_EVENT_PRI_FALSE_WAKEUP,
	} E_ALARM_EVENT;

	typedef enum
	{
		E_OTA_STA_IDLE = 0,
		E_OTA_STA_DOWNLOADING = 0,
		E_OTA_STA_INSTALLING,
		E_OTA_STA_COMPLETE,
	}T_E_OTA_STAGE;

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

	class MAIX_EXPORT COTAManageModule : public CModule
	{
	public:
		COTAManageModule(std::string strGUID, std::string strName);
		~COTAManageModule();

		mxbool init();
		mxbool unInit();
		mxbool initServer();
		mxbool initConnectModule();
		mxbool initUpgradeProcess();
		mxbool initProgressReport();
		mxbool initUpgradeStatus();
		mxbool initOtaManageRemoteEvent();
		mxbool initAIRemoteEvent();
		mxbool initEventManageRemoteEvent();

		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

		std::string startOTA(std::string strParam);
		std::string OTAData(std::string strParam);
		std::string sendOTAEnd(std::string strParam);
		std::string directStartOTA(std::string strParam);

		std::string lostDataResult();
		mxbool isInOTAProc();
		void  exitOTAProc();
		mxbool isDevIdle(std::string &strResult);
		T_E_OTA_STAGE getOTAStage();
		mxbool setOTAStage(T_E_OTA_STAGE eStage);

		mxbool upgradeCheck();
		mxbool upgradeKernel();
		mxbool upgradeRootfs();
		mxbool upgradeSystem();
		mxbool upgradeModel();
		mxbool upgradeMCU();
		mxbool upgradeComplete();
		mxbool upgradeFailed();
		

		std::string reportProcessData(std::string strItem,
			std::string strCode,
			int iProcess);
		void reportOTATimeout();
		void reportOTAFailed();

		int64_t getCurrentTime();
	private:
		std::string m_strDID;
		FILE *m_pFD;
		int m_iFileLen;
		int m_iFileOffset;
		int m_iIndex;
		int m_iTimeout;
		int m_iSystemPartition;
		mxbool m_bStart;
		mxbool m_bLoadConfig;
		
		CTimer m_timerOTATimeout;
		T_E_OTA_STAGE m_eOTAStage;

		std::string m_strMcuManageGUID;
		std::string m_strMcuManageServer;
		std::string m_strDevManageGUID;
		std::string m_strDevManageServer;
		std::string m_strAIGUID;
		std::string m_strAIServer;
        std::string m_strEventManageGUID;
        std::string m_strEventManageServer;
		
		std::string m_strOTAPackageFileName;
		std::string m_strOTAKernelFileName;
		std::string m_strOTARootfsFileName;
		std::string m_strOTASystemFileName;
		std::string m_strOTAModelFileName;
		std::string m_strOTAMCUFileName;

		std::string m_strOTAPackageVersion;
		std::string m_strOTAKernelVersion;
		std::string m_strOTARootfsVersion;
		std::string m_strOTASystemVersion;
		std::string m_strOTAModelVersion;
		std::string m_strOTAMCUVersion;

		std::string m_strOTAPackageNewVersion;
		std::string m_strOTAKernelNewVersion;
		std::string m_strOTARootfsNewVersion;
		std::string m_strOTASystemNewVersion;
		std::string m_strOTAModelNewVersion;
		std::string m_strOTAMCUNewVersion;

		COTAManageUpgradeProcess * m_objOTAManageUpgradeProcess;
		std::thread m_threadOTAManageUpgradeProcess;

		COTAManageProgressReport *m_objOTAManageProgressReport;
		std::thread m_threadOTAManageProgressReport;

		std::mutex m_mutexOTA;
		std::condition_variable m_conditionOTA;

		mxbool checkBatteryLevel();
		mxbool sendAlertEvent(const std::string &strValue);
		mxbool loadVersionConfig();
		mxbool playAudioFile(std::string strFileId);
		mxbool setLedStatus(int iColor, int iState, int iOnTimeMs, int iOffTimeMs);

		int SendToAtCmd(std::string strCommand);

		std::string closeAIDetect();
		std::string openAIDetect();
	};
}
#endif //__OTA_MANAGE_MODULE_H__
