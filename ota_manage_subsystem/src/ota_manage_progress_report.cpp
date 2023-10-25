#include "ota_manage_progress_report.h"
#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include "cJSON.h"
#include "log_mx.h"

namespace maix {
	COTAManageProgressReport::COTAManageProgressReport(CModule * module)
		: m_module(module)
		, m_bRun(mxfalse)
	{
		m_objMsgQueue.init(5, 0);
	}

	COTAManageProgressReport::~COTAManageProgressReport()
	{
	}

	mxbool COTAManageProgressReport::init()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!m_module->getConfig("IPC_REMOTE_EVENT",
			"GUID", m_strIPCGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("IPC_REMOTE_EVENT",
			"SERVER", m_strIPCRemoteEventServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool COTAManageProgressReport::unInit()
	{
		return mxtrue;
	}

	bool COTAManageProgressReport::pushData(T_OTAReportData tReportData)
	{
		return  m_objMsgQueue.push(tReportData);
	}

	void COTAManageProgressReport::popData(T_OTAReportData &tReportData)
	{
		m_objMsgQueue.pop(tReportData);
	}

	void COTAManageProgressReport::run()
	{
		m_bRun = mxtrue;
		while (m_bRun)
		{
			if (m_objMsgQueue.isEmpty())
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(1000 * 500);
#endif
				continue;
			}

			T_OTAReportData tReportData;
			popData(tReportData);

			if (m_module)
			{
				std::string strResult = m_module->output(m_strIPCGUID,
					m_strIPCRemoteEventServer, 
					(unsigned char*)tReportData.strData.c_str(),
					tReportData.strData.length());

				tReportData.iTryNum++;
				
				std::string strCode;
				std::string strMsg;
				std::string strErr;

				if (!parseResult(strResult, strCode, strMsg, strErr))
				{
					logPrint(MX_LOG_ERROR, "ota report parse result error");
					if (tReportData.eReportType == E_R_STATUS)
					{
						if (tReportData.lTimeStamp > getCurrentTime())
						{
							pushData(tReportData);
#ifdef	WIN32
							Sleep(100);
#else
							usleep(1000 * 100);
#endif
							continue;
						}
					}
					
				}
				else
				{
					if (strCode.compare("200") == 0)
					{
						logPrint(MX_LOG_DEBUG, "ota report send success");
					}
					else
					{
						logPrint(MX_LOG_ERROR, "ota report send failed: %s %s %s",
							strCode.c_str(), strMsg.c_str(),
							strErr.c_str());
						logPrint(MX_LOG_ERROR, "ota report parse result error");
						if (tReportData.eReportType == E_R_STATUS)
						{
							if (tReportData.lTimeStamp > getCurrentTime())
							{
								pushData(tReportData);
#ifdef	WIN32
								Sleep(100);
#else
								usleep(1000 * 100);
#endif
								continue;
							}
						}
					}
				}
			}
		}
	}

	mxbool COTAManageProgressReport::parseResult(std::string & strInput, 
		std::string & code, std::string & strMsg, std::string & strErr)
	{
		if (strInput.empty())
			return mxfalse;

		cJSON *jsonRoot = cJSON_Parse(strInput.c_str());

		if (jsonRoot)
		{
			cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
			if (jsonCode)
			{
				code = std::string(jsonCode->valuestring);
			}
			else
			{
				return mxfalse;
			}

			cJSON *jsonMsg = cJSON_GetObjectItem(jsonRoot, "msg");
			if (jsonMsg)
			{
				char *pcMsg = cJSON_Print(jsonMsg);
				if (pcMsg)
				{
					strMsg = std::string(pcMsg);
					free(pcMsg);
				}
			}

			cJSON_Delete(jsonRoot);
		}

		return mxtrue;
	}

	int64_t COTAManageProgressReport::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

	E_OTA_STATUS_TYPE COTAManageProgressReport::getStatus()
	{
		return m_eStatus;
	}
}
