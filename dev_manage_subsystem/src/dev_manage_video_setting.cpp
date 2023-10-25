#include "dev_manage_video_setting.h"
#include <string.h>
#include "common.h"

namespace maix {

	CDevVideoSetting::CDevVideoSetting(CDevManageModule *objDevModule)
				   	:m_objDevModule(objDevModule)
	{

	}

	CDevVideoSetting::~CDevVideoSetting()
	{

	}

	//value：0：白天模式；1：夜晚黑白模式；2：夜晚全彩模式；
	mxbool CDevVideoSetting::handleSetNightShot(std::string &strValue)
	{	
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);
		std::string strResult;

		int iSwitch= atoi(strObjValue.c_str());
		linuxPopenExecCmd(strResult, "tag_env_info --set HW 70mai_dn_sw %d", iSwitch);
		
		if(iSwitch == 0)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode off");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
		}
		else if(iSwitch == 1)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode off");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode auto");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW pwm_duty 1900:3100");
		}
		else if(iSwitch == 2)
		{
			linuxPopenExecCmd(strResult, "tag_env_info --get HW white_light_brightness");
			int iPos = strResult.find("Value=");
			int iValue = 50;
			if(-1 == iPos)
			{
				char *cValue = getFWParaConfig("white_light_brightness");
				if(cValue == NULL)
				{
					linuxPopenExecCmd(strResult, "tag_env_info --set HW white_light_brightness 50");
					setFWParaConfig("white_light_brightness","50",1);
				}
				else
				{
					iValue = atoi(cValue);
					linuxPopenExecCmd(strResult, "tag_env_info --set HW white_light_brightness %d",iValue);
				}
			}
			linuxPopenExecCmd(strResult, "tag_env_info --set HW wl_mode auto");
			linuxPopenExecCmd(strResult, "tag_env_info --set HW ir_mode off");
			// 0-100映射到0-50，灯亮度调整之后要修改
			 if(iValue != 1 )
				iValue = iValue/2;
			linuxPopenExecCmd(strResult, "tag_env_info --set HW pwm_duty %d:%d", (iValue * 50), (5000 - iValue * 50));
		}

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();
		m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_NIGHT_SHOT, strObjValue);

        logPrint(MX_LOG_DEBUG, "CDevVideoSetting set night shot value:%s", strObjValue.c_str());
        return mxtrue;
	}

	mxbool CDevVideoSetting::handleGetNightShot(std::string &strValue)
	{
		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_dn_sw");

		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_dn_sw");
			return mxfalse;
		}

		strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		if(strValue[strValue.length()-1] == '\n')
		{
			strValue[strValue.length() - 1] = '\0';
		}		

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting get night shot value:%s", strValue.c_str());

		return mxtrue;
	}

	mxbool CDevVideoSetting::handleSetWdrSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();
		m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_WDR, strObjValue);

		bool bSwitch = (bool)atoi(strObjValue.c_str());
		linuxPopenExecCmd(strObjValue, "tag_env_info --set HW 70mai_wdr %d", bSwitch);

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set wdr value:%s", strObjValue.c_str());
        return mxtrue;
	}

	mxbool CDevVideoSetting::handleGetWdrSwitch(std::string &strValue)
	{
		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_wdr");

		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_wdr");
			return mxfalse;
		}

		strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		
		if(strValue[strValue.length()-1] == '\n')
		{
			strValue[strValue.length() - 1] = '\0';
		}
		logPrint(MX_LOG_DEBUG, "CDevVideoSetting get wdr value:%s", strValue.c_str());
		return mxtrue;
	}

	mxbool CDevVideoSetting::handleSetWaterMark(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		std::string strGUID = m_objDevModule->getVideoModuleGuid();
		std::string strServerName = m_objDevModule->getVideoModuleRmoteServerName();
		m_objDevModule->outputConfigVideo(strGUID, strServerName, EVENT_SET_WATER_MARK, strObjValue);

		bool bSwitch = (bool)atoi(strObjValue.c_str());

		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW 70mai_wm %d", bSwitch);

		logPrint(MX_LOG_DEBUG, "CDevVideoSetting set watermark value:%s", strObjValue.c_str());
		return mxtrue;
	}

	mxbool CDevVideoSetting::handleGetWaterMark(std::string &strValue)
	{
		std::string strCmdValue;
		linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_wm");

		int iPos = strCmdValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_wm");
			return mxfalse;
		}

		strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		if(strValue[strValue.length()-1] == '\n')
		{
			strValue[strValue.length() - 1] = '\0';
		}
		logPrint(MX_LOG_DEBUG, "CDevVideoSetting get watermark value:%s", strValue.c_str());
		return mxtrue;
	}

}