#include "event_mange_remote_event.h"
#include "cJSON.h"
#include "log_mx.h"
#include "fw_env_para.h"
#include "common.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <algorithm>

namespace maix
{
	CEventManageRemoteEvent::CEventManageRemoteEvent(CModule *module)
		: m_module(module)
	{
		m_bDetect = mxtrue;
	}

	CEventManageRemoteEvent::~CEventManageRemoteEvent()
	{
	}

	mxbool CEventManageRemoteEvent::init(
		std::string strGUID, std::string strServer)
	{
		if (!m_objEventQueue.init(10, 1000))
			return mxfalse;

		m_strGUID = strGUID;
		m_strServer = strServer;
		m_strDID = getDID();

		return mxtrue;
	}

	mxbool CEventManageRemoteEvent::unInit()
	{
		return mxbool();
	}

	void CEventManageRemoteEvent::run()
	{
		std::vector<std::string> vecEvent;

		while (1)
		{
			std::string strEvent;
			popFrameData(strEvent);

			if ((strEvent.length() == 0) || (m_bDetect == mxfalse))
			{
#ifdef WIN32
				Sleep(500);
#else
				usleep(1000 * 500);
#endif
				continue;
			}

			sendAlarmEvent(strEvent, "CenterEvent");

			auto it = find(vecEvent.begin(), vecEvent.end(), strEvent);
			if(it == vecEvent.end())
			{
				vecEvent.push_back(strEvent);
				sendAlarmEvent(strEvent, "AlarmEvent");
			}
		}
	}

	void CEventManageRemoteEvent::sendAlarmEvent(std::string strEventValue, std::string strEventType)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", strEventType.c_str());
		cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
		cJSON_AddStringToObject(jsonParam, "value", strEventValue.c_str());
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strAIEvent = std::string(out);
		logPrint(MX_LOG_DEBUG,"sendAlarmEvent: %s ", strAIEvent.c_str());
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		m_module->output(m_strGUID, m_strServer, (unsigned char *)strAIEvent.c_str(), strAIEvent.length());
	}

	mxbool CEventManageRemoteEvent::pushFrameData(std::string strEvent)
	{
		return m_objEventQueue.push(strEvent);
	}

	void CEventManageRemoteEvent::popFrameData(std::string &strEvent)
	{
		m_objEventQueue.pop(strEvent);
	}

	void CEventManageRemoteEvent::openAIEventPush()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bDetect = mxtrue;
	}

	void CEventManageRemoteEvent::closeAIEventPush()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_bDetect = mxfalse;
	}

}