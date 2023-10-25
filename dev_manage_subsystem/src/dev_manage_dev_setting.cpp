#include "dev_manage_dev_setting.h"
#include "common.h"
#include "string.h"
#include <string>
#include <unistd.h>

namespace maix {

	CDevSetting::CDevSetting(CDevManageModule *objDevModule)
				:m_objDevModule(objDevModule)
	{

	}

	CDevSetting::~CDevSetting()
	{

	}

	mxbool CDevSetting::handleSetCameraSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		int iValue = atoi(strObjValue.c_str());

		if (setFWParaConfig(PARA_CAMERA_SWITCH, strObjValue.c_str(), 1) != 0) 
		{
			logPrint(MX_LOG_ERROR, "CDevSetting set camera switch, set para:%s, value:%s is failed", PARA_CAMERA_SWITCH, strObjValue.c_str());
			return mxfalse;
		}
		saveFWParaConfig();

		std::string ret;
		cJSON *pJsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonRoot,"event","StrPirStatus");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "pir_Status", iValue);
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);

		char *out = cJSON_Print(pJsonRoot);
        ret = std::string(out);

        if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }

		std::string strGUID = m_objDevModule->getLowPowerGUID();
		std::string strValues = m_objDevModule->getLowPowerServer();

		if(iValue == 0)
		{
			cJSON *pJsonIndicatorSwitch = cJSON_CreateObject();
			cJSON_AddStringToObject(pJsonIndicatorSwitch,"value","0");
			std::string strIndicatorSwitch = std::string(cJSON_Print(pJsonIndicatorSwitch));
			if(pJsonIndicatorSwitch)
			{
				cJSON_free(pJsonIndicatorSwitch);
				pJsonIndicatorSwitch = NULL;
			}
			handleSetIndicatorSwitch(strIndicatorSwitch);

			m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
			cJSON *pJsonRootLow = cJSON_CreateObject();
			cJSON_AddStringToObject(pJsonRootLow,"event","EnterLowPower");
			char* outLow = cJSON_Print(pJsonRootLow);
			ret = std::string(outLow);
			if(pJsonRootLow)
			{
				cJSON_free(pJsonRootLow);
				pJsonRootLow = NULL;
			}
				
			m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
		}
		else if(iValue == 1)
		{
			char *pValuePara = getFWParaConfig(PARA_PIR_DETECTION);
			int iValuePir = atoi(pValuePara);
			if(iValuePir == 1)
			{
				m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
			}
		}

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set camera switch value:%s", ret.c_str());
		return mxtrue;
	}

	mxbool CDevSetting::handleSetIndicatorSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		int iValue = atoi(strObjValue.c_str());
		cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "SetLedSwitch");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "LedSwitch", iValue);
		cJSON_AddItemToObject(pJsonRoot, "param", pJsonParam);
		char *out = cJSON_Print(pJsonRoot);
		std::string ret = std::string(out);
		std::string strGuid =  m_objDevModule->getMcuSerialPortGUID();
		std::string strServer = m_objDevModule->getMcuSerialPortServer();
		m_objDevModule->output(strGuid, strServer, (unsigned char *)out, strlen(out) + 1);

		if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }	

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set indicator switch value:%s", ret.c_str());
		return mxtrue;
	}

	mxbool CDevSetting::handleSetWhiteLightSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();

		int bSwitch= atoi(strObjValue.c_str());

		// printf(" CDevSetting::handleSetWhiteLightSwitch get wled status-[%d]\n", bSwitch);

		if(bSwitch == 1)
		{
			bSwitch = 3; //打开白光灯
			strObjValue = std::to_string(bSwitch);
			m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);
		}
		else if(bSwitch == 0)
		{
			bSwitch = 4; //关闭白光灯
			strObjValue = std::to_string(bSwitch);
			std::string strResult = m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);
			// printf("m_objDevModule->outputConfigVideo strResult:-[%s]\n", strResult.c_str());
		}

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set whitelight switch value:%s", strObjValue.c_str());
		return mxtrue;
	}

	mxbool CDevSetting::handleGetBatteryLevel(std::string &strValue)
	{
		cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "GetBatteryLevel");
		cJSON_AddStringToObject(pJsonRoot, "param", "");
		char *out = cJSON_Print(pJsonRoot);
		std::string ret = std::string(out);

		std::string strGuid =  m_objDevModule->getMcuSerialPortGUID();
		std::string strServer = m_objDevModule->getMcuSerialPortServer();
		std::string strResult = m_objDevModule->output(strGuid, strServer, (unsigned char *)out, strlen(out) + 1);

		if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }

		pJsonRoot = cJSON_Parse(strResult.c_str());
		cJSON *pJsonParam = cJSON_GetObjectItem(pJsonRoot, "param");
		cJSON *pJsonCapacityValue = cJSON_GetObjectItem(pJsonParam, "capacityValue");

		int iValue = (int)pJsonCapacityValue->valuedouble;
		strValue = std::to_string(iValue);
		logPrint(MX_LOG_DEBUG, "CDevVideoSetting get battery level value:%s", strValue.c_str());
		return mxtrue;
	}

	mxbool CDevSetting::handleGetChargingState(std::string &strValue)
	{
		cJSON *pJsonRoot = cJSON_CreateObject();
        cJSON_AddStringToObject(pJsonRoot, "event", "GetChargingState");

		cJSON_AddStringToObject(pJsonRoot, "param", "");
		char *out = cJSON_Print(pJsonRoot);
		std::string ret = std::string(out);


		std::string strGuid =  m_objDevModule->getMcuSerialPortGUID();
		std::string strServer = m_objDevModule->getMcuSerialPortServer();
		std::string strResult = m_objDevModule->output(strGuid, strServer, (unsigned char *)out, strlen(out) + 1);
		if (pJsonRoot)
        {
            cJSON_Delete(pJsonRoot);
            pJsonRoot = NULL;
        }
        if (out)
        {
            free(out);
            out = NULL;
        }
		std::cout<<strResult<<std::endl;

		pJsonRoot = cJSON_Parse(strResult.c_str());
		cJSON *pJsonParam = cJSON_GetObjectItem(pJsonRoot, "param");
		cJSON *pJsonChargingState = cJSON_GetObjectItem(pJsonParam, "status");
		
		int iValue = (int)pJsonChargingState->valuedouble + 1;
		strValue = std::to_string(iValue);
		logPrint(MX_LOG_DEBUG, "CDevVideoSetting get charging state value:%s", strValue.c_str());
		return mxtrue;
	}

	mxbool CDevSetting::handleSetWhiteLightBrightness(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	
		int iValue = atoi(strObjValue.c_str());
		std::string strResult;

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();

		linuxPopenExecCmd(strResult, "tag_env_info --set HW white_light_brightness %d", iValue);

		char *cValue = getFWParaConfig("user","set_night_shot");
		int night_mode = 0;
		if(cValue != NULL)
		{
			night_mode = atoi(cValue);
		}

		if(night_mode == 2)
		{
			if(iValue == 0)
			{
				linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode off");
				linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
			}
			else
			{
				linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode auto");
				linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
				linuxPopenExecCmd(strResult, "tag_env_info --set HW pwm_duty %d:%d", (iValue * 50), (5000 - iValue * 50));
			}
			

			logPrint(MX_LOG_DEBUG, "CDevVideoSetting set white light brightness value:%s", strValue.c_str());

			int value = 6; //刷新白光灯强度
			strObjValue = std::to_string(value);
			m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);
		}
		
		return mxtrue;
	}
	
	mxbool CDevSetting::handleGetWifiSignal(std::string &strValue)
	{
		strValue = "-30";
		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set white light brightness value:%s", strValue.c_str());
		return mxtrue;
	}
}