#include "local_storage_remote_event_server.h"
#include "cJSON.h"

namespace maix {
	CLocalStorageRemoteEventServer::CLocalStorageRemoteEventServer(
		CLocalStorageModule *objLocalStorageModule)
		: m_objLocalStorageModule(objLocalStorageModule)
	{

	}

	CLocalStorageRemoteEventServer::~CLocalStorageRemoteEventServer()
	{
	}

	mxbool CLocalStorageRemoteEventServer::init()
	{
		return mxbool();
	}

	mxbool CLocalStorageRemoteEventServer::unInit()
	{
		return mxbool();
	}

	std::string CLocalStorageRemoteEventServer::eventProc(
		std::string strMsg)
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

		if (m_objLocalStorageModule)
		{
			return m_objLocalStorageModule->remoteEventServerProc(
				strEvent, strParam);
		}
		else
		{
			return procResult(std::string("500"), "",
				std::string("video manage module not exist"));
		}
	}

	std::string CLocalStorageRemoteEventServer::procResult(std::string code,
		std::string strMsg, std::string strErr)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
		cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
		char *pcResult = cJSON_Print(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}

}