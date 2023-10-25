#ifndef __OTA_MANAGE_PROGRESS_REPORT_H__
#define __OTA_MANAGE_PROGRESS_REPORT_H__
#include "module.h"
#include "b_queue.h"

namespace maix {
	typedef enum
	{
		E_R_STATUS,
		E_R_PROGRESS
	}E_OTA_REPORT_TYPE;

	typedef enum
	{
		E_OTA_MCU,
		E_OTA_IPC,
		E_OTA_MCU_ERROR,
		E_OTA_MCU_SUCCESS,
		E_OTA_IPC_ERROR,
		E_OTA_IPC_SUCCESS,
	}E_OTA_STATUS_TYPE;

	typedef struct __OTAReportData
	{
		E_OTA_REPORT_TYPE eReportType;
		int64_t lTimeStamp;
		int iTryNum;
		std::string strData;
	}T_OTAReportData;

	class COTAManageProgressReport
	{
	public:
		COTAManageProgressReport(CModule *module);
		~COTAManageProgressReport();

		mxbool init();
		mxbool unInit();

		bool pushData(T_OTAReportData tReportData);
		void popData(T_OTAReportData &tReportData);

		void run();

		mxbool parseResult(std::string &strInput,
			std::string &code, std::string &strMsg, 
			std::string &strErr);

		int64_t getCurrentTime();
		E_OTA_STATUS_TYPE getStatus();
	private:
		CModule* m_module;
		mxbool m_bRun;
		E_OTA_STATUS_TYPE m_eStatus;
		CBQueue<T_OTAReportData> m_objMsgQueue;
		std::string m_strIPCGUID;
		std::string m_strIPCRemoteEventServer;
	};
}
#endif //__OTA_MANAGE_PROGRESS_REPORT_H__