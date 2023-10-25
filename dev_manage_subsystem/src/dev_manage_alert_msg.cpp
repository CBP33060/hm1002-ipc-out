#include "dev_manage_alert_msg.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

namespace maix {

	CDevAlertMsg::CDevAlertMsg(CDevManageModule *objDevModule)
				:m_objDevModule(objDevModule)
	{

	}

	CDevAlertMsg::~CDevAlertMsg()
	{
		
	}

	mxbool CDevAlertMsg::handleSetAutoAlarmSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();

		int iValue = atoi(strObjValue.c_str());
		if(iValue == 1)
		{
			m_iAutoAlarmValue = iValue;
		}
		else
		{
			m_iAutoAlarmValue = iValue;
			iValue = 4; //关闭声光警告白光灯
			strObjValue = std::to_string(iValue);
			m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);

			cJSON *jsonParam = cJSON_CreateObject(); //关闭喇叭报警
			cJSON_AddStringToObject(jsonParam, "fileId", "warming_alarm");
			char *out = cJSON_Print(jsonParam);
			std::string strJsonParam = std::string(out);
			cJSON_Delete(jsonParam);
			if (out)
			{
				free(out);
				out = NULL;
			}
			m_objDevModule->stopWithFileId(strJsonParam);
		}

		return mxtrue;
	}

	mxbool CDevAlertMsg::handleSetManualAlarmSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();

		int iValue = atoi(strObjValue.c_str());
		if(iValue == 1)
		{
			iValue = 5; //打开声光警告白光灯
			strObjValue = std::to_string(iValue);
			m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);
			
			cJSON *jsonParam = cJSON_CreateObject(); //打开喇叭报警
			cJSON_AddStringToObject(jsonParam, "fileId", "warming_alarm");
			cJSON_AddNumberToObject(jsonParam,"level", 1);
			cJSON_AddNumberToObject(jsonParam,"playTime",30);
			char *out = cJSON_Print(jsonParam);
			std::string strJsonParam = std::string(out);
			cJSON_Delete(jsonParam);
			if (out)
			{
				free(out);
				out = NULL;
			}

			m_objDevModule->playWithFileId(strJsonParam);

		}
		else
		{
			iValue = 4; //关闭声光警告白光灯
			strObjValue = std::to_string(iValue);
			m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);

			cJSON *jsonParam = cJSON_CreateObject(); //关闭喇叭报警
			cJSON_AddStringToObject(jsonParam, "fileId", "warming_alarm");
			char *out = cJSON_Print(jsonParam);
			std::string strJsonParam = std::string(out);
			cJSON_Delete(jsonParam);
			if (out)
			{
				free(out);
				out = NULL;
			}
			m_objDevModule->stopWithFileId(strJsonParam);
		}


		return mxtrue;
	}
}