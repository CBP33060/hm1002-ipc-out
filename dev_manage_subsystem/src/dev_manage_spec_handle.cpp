#include "dev_manage_spec_handle.h"
#include <iostream>
#include <map>
#include <functional>


namespace maix {

	CDevSpecHandle::CDevSpecHandle(CDevManageModule *objDevModule)
				   :m_objDevModule(objDevModule)
	{

	}

	CDevSpecHandle::~CDevSpecHandle()
	{

	}

	mxbool CDevSpecHandle::init()
	{
		m_objAlertMsg = std::shared_ptr<CDevAlertMsg>(new CDevAlertMsg(m_objDevModule));
		m_objVideoSetting = std::shared_ptr<CDevVideoSetting>(new CDevVideoSetting(m_objDevModule));
		m_objDevSetting = std::shared_ptr<CDevSetting>(new CDevSetting(m_objDevModule));
 		m_objDevIntelligentDet = std::shared_ptr<CDevIntelligentDet>(new CDevIntelligentDet(m_objDevModule));
        m_objDevPirMotionDet = std::shared_ptr<CDevPirMotionDet>(new CDevPirMotionDet(m_objDevModule));
		if ((NULL == m_objAlertMsg) || (NULL == m_objVideoSetting) || (NULL == m_objVideoSetting) || (NULL == m_objDevIntelligentDet) || (NULL == m_objDevPirMotionDet))
		{
			logPrint(MX_LOG_ERROR, "CDevSpecBase new shared ptr failed");
			return mxfalse;
		}

		T_SPEC_HANDLE_CONFIG specHandleSetConfig[] = 
		{
			{std::string(EVENT_SET_AUTO_ALARM_SWITCH), PARA_AUTO_ALARM_SWITCH, BIND_FUNC(&CDevAlertMsg::handleSetAutoAlarmSwitch, m_objAlertMsg)},
			{std::string(EVENT_SET_MANUAL_ALARM_SWITCH), PARA_MANUAL_ALARM_SWITCH, BIND_FUNC(&CDevAlertMsg::handleSetManualAlarmSwitch, m_objAlertMsg)},
			{std::string(EVENT_SET_NIGHT_SHOT), PARA_SET_NIGHT_SHOTH, BIND_FUNC(&CDevVideoSetting::handleSetNightShot, m_objVideoSetting)},
			{std::string(EVENT_SET_WDR), PARA_SET_SET_WDR, BIND_FUNC(&CDevVideoSetting::handleSetWdrSwitch, m_objVideoSetting)},
			{std::string(EVENT_SET_WATER_MARK), PARA_SET_WATER_MARK, BIND_FUNC(&CDevVideoSetting::handleSetWaterMark, m_objVideoSetting)},
			{std::string(EVENT_SET_CAMERA), NULL, BIND_FUNC(&CDevSetting::handleSetCameraSwitch, m_objDevSetting)},
			{std::string(EVENT_SET_INDICATE_LIGHT), PARA_INDICATE_LIGHT_SWITCH, BIND_FUNC(&CDevSetting::handleSetIndicatorSwitch, m_objDevSetting)},
			{std::string(EVENT_SET_WHITE_LIGHT), PARA_WHITE_LIGHT_SWITCH, BIND_FUNC(&CDevSetting::handleSetWhiteLightSwitch, m_objDevSetting)},
			{std::string(EVENT_SET_WHITE_LIGHT_BRIGHTNESS), PARA_WHITE_LIGHT_BRIGHTNESS, BIND_FUNC(&CDevSetting::handleSetWhiteLightBrightness, m_objDevSetting)},
			{std::string(EVENT_SET_BATTERY_LEVEL), NULL, NULL},
			{std::string(EVENT_SET_BATTERY_STATE), NULL, NULL},
			{std::string(EVENT_SET_PEOPLE_DETECTION), PARA_PERSON_DETECTION, BIND_FUNC(&CDevIntelligentDet::handleSetPeopleDetection, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_ANIMAL_DETECTION), PARA_ANIMAL_DETECTION, BIND_FUNC(&CDevIntelligentDet::handleSetAnimalDetection, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_VEHICLE_DETECTION), PARA_VEHICLE_DETECTION, BIND_FUNC(&CDevIntelligentDet::handleSetVehicleDetection, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_PACKAGE_DETECTION), PARA_PACKAGE_DETECTION, BIND_FUNC(&CDevIntelligentDet::handleSetPackageDetection, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_AREA_DETECT_COORD), PARA_AREA_DETECT_COORD, BIND_FUNC(&CDevIntelligentDet::handleSetAreaDetectCoord, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_PEOPLE_STAY), PARA_PERSON_STAY, BIND_FUNC(&CDevIntelligentDet::handleSetPeopleStayTime, m_objDevIntelligentDet)},
            {std::string(EVENT_SET_PIR_SENSITIVITY), PARA_PIR_SENSITIVITY, BIND_FUNC(&CDevPirMotionDet::handleSetPirSensitivity, m_objDevPirMotionDet)},
            {std::string(EVENT_SET_PIR_DETECTION), PARA_PIR_DETECTION, BIND_FUNC(&CDevPirMotionDet::handleSetPirSwitch, m_objDevPirMotionDet)},
            {std::string(EVENT_SET_PIR_INTERVAL), PARA_PIR_INTERVAL, BIND_FUNC(&CDevPirMotionDet::handleSetPirInterval, m_objDevPirMotionDet)},
			{std::string(EVENT_SET_WIFI_SIGNAL),NULL, NULL},
			{std::string(EVENT_SET_TIMING_DECTION), PARA_TIMING_DECTION, BIND_FUNC(&CDevPirMotionDet::handleSetTimingDection, m_objDevPirMotionDet)},
		};

		T_SPEC_HANDLE_CONFIG specHandleGetConfig[] = 
		{
			{std::string(EVENT_GET_AUTO_ALARM_SWITCH), PARA_AUTO_ALARM_SWITCH, NULL},
			{std::string(EVENT_GET_MANUAL_ALARM_SWITCH), PARA_MANUAL_ALARM_SWITCH, NULL},
			{std::string(EVENT_GET_NIGHT_SHOT), NULL, BIND_FUNC(&CDevVideoSetting::handleGetNightShot, m_objVideoSetting)},
			{std::string(EVENT_GET_WDR), NULL, BIND_FUNC(&CDevVideoSetting::handleGetWdrSwitch, m_objVideoSetting)},
			{std::string(EVENT_GET_WATER_MARK), NULL, BIND_FUNC(&CDevVideoSetting::handleGetWaterMark, m_objVideoSetting)},
			{std::string(EVENT_GET_CAMERA), PARA_CAMERA_SWITCH, NULL},
			{std::string(EVENT_GET_INDICATE_LIGHT), PARA_INDICATE_LIGHT_SWITCH, NULL},
			{std::string(EVENT_GET_WHITE_LIGHT), PARA_WHITE_LIGHT_SWITCH, NULL},
			{std::string(EVENT_GET_WHITE_LIGHT_BRIGHTNESS), PARA_WHITE_LIGHT_BRIGHTNESS, NULL},
			{std::string(EVENT_GET_BATTERY_LEVEL), NULL, BIND_FUNC(&CDevSetting::handleGetBatteryLevel, m_objDevSetting)},
			{std::string(EVENT_GET_BATTERY_STATE), NULL, BIND_FUNC(&CDevSetting::handleGetChargingState, m_objDevSetting)},
            {std::string(EVENT_GET_PEOPLE_DETECTION), PARA_PERSON_DETECTION, NULL},
            {std::string(EVENT_GET_ANIMAL_DETECTION), PARA_ANIMAL_DETECTION, NULL},
            {std::string(EVENT_GET_VEHICLE_DETECTION), PARA_VEHICLE_DETECTION, NULL},
            {std::string(EVENT_GET_PACKAGE_DETECTION), PARA_PACKAGE_DETECTION, NULL},
            {std::string(EVENT_GET_AREA_DETECT_COORD), PARA_AREA_DETECT_COORD, NULL},
            {std::string(EVENT_GET_PEOPLE_STAY), PARA_PERSON_STAY, NULL},
            {std::string(EVENT_GET_PIR_SENSITIVITY), PARA_PIR_SENSITIVITY, NULL},
            {std::string(EVENT_GET_PIR_DETECTION), PARA_PIR_DETECTION, NULL},
            {std::string(EVENT_GET_PIR_INTERVAL), PARA_PIR_INTERVAL, NULL},
            {std::string(EVENT_GET_WIFI_SIGNAL), NULL, BIND_FUNC(&CDevSetting::handleGetWifiSignal, m_objDevSetting)},
			{std::string(EVENT_GET_TIMING_DECTION), PARA_TIMING_DECTION, NULL},
		};

		unsigned int i = 0;
		for (i = 0; i < sizeof(specHandleSetConfig) / sizeof(specHandleSetConfig[0]); i++)
		{
			m_specHandleSetMap.insert(std::make_pair<std::string, T_SPEC_HANDLE_CONFIG>(
									(std::string)specHandleSetConfig[i].m_strEvent, (T_SPEC_HANDLE_CONFIG)specHandleSetConfig[i]));
		}

		for (i = 0; i < sizeof(specHandleGetConfig) / sizeof(specHandleGetConfig[0]); i++)
		{
			m_specHandleGetMap.insert(std::make_pair<std::string, T_SPEC_HANDLE_CONFIG>(
									(std::string)specHandleGetConfig[i].m_strEvent, (T_SPEC_HANDLE_CONFIG)specHandleGetConfig[i]));
		}

		return mxtrue;
	}

    std::string CDevSpecHandle::inputRet(cJSON *pJsonParam)
    {
        std::string strRet;
        cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "devConfig");

		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

        char *pOut = cJSON_PrintUnformatted(pJsonRoot);
        strRet = std::string(pOut);
		logPrint(MX_LOG_DEBUG, "dev spec handle input ret:%s", strRet.c_str());

        if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (pOut)
        {
            free(pOut);
            pOut = NULL;
        }

        return strRet;
    }

	std::string CDevSpecHandle::handleSpec(const std::string &strParam)
	{
		cJSON *pJsonParam = cJSON_CreateObject();
		int iId = 0;
		std::string strDid;
		std::string strRet;
		if (strParam.empty())
		{
			logPrint(MX_LOG_ERROR, "handle spec failed, param is empty");
			return inputRet(pJsonParam);
		}

		logPrint(MX_LOG_ERROR, "handle spec param:%s", strParam.c_str());

		cJSON *pJsonRoot = cJSON_Parse(strParam.c_str());
		if (!pJsonRoot)
		{
			logPrint(MX_LOG_ERROR, "handle spec failed, json root is null");
			return inputRet(pJsonParam);
		}

		cJSON *pJsonId = cJSON_GetObjectItem(pJsonRoot, "id");
		if (pJsonId && (pJsonId->type != cJSON_Number))
		{
			iId = pJsonId->valueint;
		}
		cJSON *pJsonDid = cJSON_GetObjectItem(pJsonRoot, "did");
		if (pJsonDid && (pJsonDid->type == cJSON_String))
		{
			strDid = std::string(pJsonId->valuestring);
		}

		cJSON *pJsonCmd = cJSON_GetObjectItem(pJsonRoot, "cmd");
		if ((!pJsonCmd) || (pJsonCmd->type != cJSON_Array))
		{
			logPrint(MX_LOG_ERROR, "parse dev config msg:%s failed", strParam.c_str());
			return inputRet(pJsonParam);
		}

		char *pCmd = cJSON_PrintUnformatted(pJsonCmd);
		if (!pCmd)
		{
			logPrint(MX_LOG_ERROR, "handle spec failed, cmd printf failed, param:%s", strParam.c_str());
			return inputRet(pJsonParam);
		}
		std::string strCmd = std::string(pCmd);	
		cJSON_free(pCmd);
		pCmd = NULL;

		size_t iCnt = cJSON_GetArraySize(pJsonCmd);
		if (iCnt <= 0)
		{
			logPrint(MX_LOG_ERROR, "handle spec failed, cmd array size =%d", iCnt);
			return inputRet(pJsonParam);
		}

		cJSON *jsonReplyArray = cJSON_CreateArray();

		std::string strConfigName;
		std::string strConfigParam;
		for (size_t i = 0; i < iCnt; i++)
		{
			std::string strCode("200");
			std::string strValue("0");
			cJSON *pJsonArrayItem = cJSON_GetArrayItem(pJsonCmd, i);
			if (!pJsonArrayItem)
			{
				logPrint(MX_LOG_ERROR, "handle spec warn, Cmd i is not vaild, cmd:%s", strCmd.c_str());
				continue;
			}

			cJSON *pJsonConfigName = cJSON_GetObjectItem(pJsonArrayItem, "configName");
			if ((NULL == pJsonConfigName) || (false == cJSON_IsString(pJsonConfigName)))
			{
				logPrint(MX_LOG_ERROR, "handle spec warn, Cmd i config name is not vaild, cmd:%s", strCmd.c_str());
				continue;
			}
			strConfigName = std::string(pJsonConfigName->valuestring);

			cJSON *pJsonConfigParam = cJSON_GetObjectItem(pJsonArrayItem, "configParam");

			if (m_specHandleGetMap.count(strConfigName) > 0)
			{
				if (!handleGetSpec(strConfigName, pJsonConfigParam, strCode, strValue))
				{
					logPrint(MX_LOG_WARN, "handle spec failed, str config name:%s", strConfigName.c_str());
					strCode = "-1000";
					strValue = "-1000";
				}
			}
			else if (m_specHandleSetMap.count(strConfigName) > 0)
			{
				if (!handleSetSpec(strConfigName, pJsonConfigParam, strCode, strValue))
				{
					logPrint(MX_LOG_WARN, "handle spec failed, str config name:%s", strConfigName.c_str());
					strCode = "-1000";
					strValue = "-1000";
				}
			}
			else
			{
				logPrint(MX_LOG_ERROR, "handle spec failed, str config name:%s is not exist", strConfigName.c_str());
				continue;
			}

			cJSON *pJsonReplyItem = cJSON_CreateObject();
			cJSON_AddStringToObject(pJsonReplyItem, "configName", strConfigName.c_str());
			cJSON *pJsonReplyParam = cJSON_CreateObject();
			cJSON_AddStringToObject(pJsonReplyParam, "value", strValue.c_str());
			cJSON_AddItemToObject(pJsonReplyItem, "configParam", pJsonReplyParam);
			cJSON_AddStringToObject(pJsonReplyItem, "code", strCode.c_str());

			cJSON_AddItemToArray(jsonReplyArray, pJsonReplyItem);
		}

		cJSON_AddNumberToObject(pJsonParam, "id", iId);
		cJSON_AddStringToObject(pJsonParam, "did", strDid.c_str());
		cJSON_AddItemToObject(pJsonParam, "cmd", jsonReplyArray);

		if (pJsonRoot)
		{
			cJSON_Delete(pJsonRoot);
			pJsonRoot = NULL;
		}

		return inputRet(pJsonParam);
	}


	mxbool CDevSpecHandle::handleGetSpec(const std::string &strEvent, const cJSON *pJsonParam, std::string &strCode, std::string &strValue)
	{
		T_SPEC_HANDLE_CONFIG handleConfig = m_specHandleGetMap[strEvent];

		/// 若不使用para存储，且未自定义处理函数，认为是无效数据
		if ((NULL == handleConfig.m_pParaName) && (NULL == handleConfig.m_handleFunc))
		{
			logPrint(MX_LOG_ERROR, "CDevSpecHandle get spec, event:%s is not supported", strEvent.c_str());
			return mxfalse;
		}

		if (handleConfig.m_pParaName)	///< 使用para存储
		{
			char *pValuePara = getFWParaConfig(handleConfig.m_pParaName);
			if(NULL == pValuePara)
			{
				logPrint(MX_LOG_ERROR, "CDevSpecHandle get spec, handle para:%s is failed", handleConfig.m_pParaName);
				return mxfalse;
			}
			strValue = std::string(pValuePara);
		}

		if (handleConfig.m_handleFunc) ///< 使用自定义的get处理函数
		{
			if (handleConfig.m_handleFunc(strValue) == false)
			{	
				logPrint(MX_LOG_ERROR, "CDevSpecHandle get spec, handle event:%s exec get func failed", strEvent.c_str());
				return mxfalse;
			}
		}

		return mxtrue;
	}

	mxbool CDevSpecHandle::handleSetSpec(const std::string &strEvent, const cJSON *pJsonParam, std::string &strCode, std::string &strValue)
	{
		T_SPEC_HANDLE_CONFIG handleConfig = m_specHandleSetMap[strEvent];
		
		/// 若不使用para存储，且未自定义处理函数，认为是无效数据
		if ((NULL == handleConfig.m_pParaName) && (NULL == handleConfig.m_handleFunc))
		{
			logPrint(MX_LOG_ERROR, "CDevSpecHandle set spec, event:%s is not supported", strEvent.c_str());
			return mxfalse;
		}

		cJSON *pValue = cJSON_GetObjectItem(pJsonParam, "value");
		if (NULL == pValue) 
		{
			logPrint(MX_LOG_ERROR, "CDevSpecHandle set spec, handle event:%s parse value failed", strEvent.c_str());
			return mxfalse;
		}
		strValue = std::string(pValue->valuestring);

		cJSON *jsonValue = cJSON_CreateObject();
		cJSON_AddItemToObject(jsonValue, "value", pValue);
		std::string strValues = std::string(cJSON_PrintUnformatted(jsonValue));
		if (handleConfig.m_handleFunc)	///< 使用自定义的set处理函数
		{
			if (handleConfig.m_handleFunc(strValues) == false) 
			{
				logPrint(MX_LOG_ERROR, "CDevSpecHandle set spec, handle event:%s set func failed", strEvent.c_str());
				return mxfalse;
			}
		}

		if (handleConfig.m_pParaName)		///< 使用para存储
		{
			if (setFWParaConfig(handleConfig.m_pParaName, strValue.c_str(), 1) != 0) 
			{
				logPrint(MX_LOG_ERROR, "CDevSpecHandle set spec, set para:%s, value:%s is failed", handleConfig.m_pParaName, strValue.c_str());
				return mxfalse;
			}
			saveFWParaConfig();
		}

		return mxtrue;
	}
}