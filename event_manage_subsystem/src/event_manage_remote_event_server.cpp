#include "event_manage_remote_event_server.h"
#include "cJSON.h"
namespace maix {
	CEventManageRemoteEventServer::CEventManageRemoteEventServer(
		CEventManageModule * objEventManageModule)
		: m_objEventManageModule(objEventManageModule)
	{
	}

	CEventManageRemoteEventServer::~CEventManageRemoteEventServer()
	{
	}

	mxbool CEventManageRemoteEventServer::init()
	{
		return mxbool();
	}

	mxbool CEventManageRemoteEventServer::unInit()
	{
		return mxbool();
	}

	std::string CEventManageRemoteEventServer::eventProc(std::string strMsg)
	{
		std::string strEvent;
		std::string strParam;
		cJSON *jsonRoot = cJSON_Parse(strMsg.c_str());

		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				char *pcParam = cJSON_Print(jsonParam);
				if (pcParam)
				{
					strParam = std::string(pcParam);
					free(pcParam);
				}

			}
			cJSON_Delete(jsonRoot);
		}

		if (m_objEventManageModule)
		{
			return m_objEventManageModule->remoteEventServerProc(
				strEvent, strParam);
		}
		else
		{
			return std::string("error");
		}
	}
}