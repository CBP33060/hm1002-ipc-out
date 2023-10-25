#include "dev_manage_pir_det.h"

namespace maix {

	CDevPirMotionDet::CDevPirMotionDet(CDevManageModule *objDevModule)
				:m_objDevModule(objDevModule)
	{

	}

	CDevPirMotionDet::~CDevPirMotionDet()
	{

	}

	mxbool CDevPirMotionDet::handleSetPirSensitivity(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

        logPrint(MX_LOG_DEBUG, "CDevAlertMsg set Pir Sensitivity value:%s", strObjValue.c_str());
		std::string ret;
		cJSON *pJsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonRoot,"event","StrPirSensitivity");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "pir_Sensitivity", atoi(strObjValue.c_str()));
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
		std::string strGUID = m_objDevModule->getMcuSerialPortGUID();
		std::string strValues = m_objDevModule->getMcuSerialPortServer();
		m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
        return mxtrue;
	}

	mxbool CDevPirMotionDet::handleSetPirSwitch(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Pir Switch value:%s", strObjValue.c_str());
		std::string ret;
		cJSON *pJsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonRoot,"event","pir_Sensitivity");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "pir_Status", atoi(strObjValue.c_str()));
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
		std::string strGUID = m_objDevModule->getMcuSerialPortGUID();
		std::string strValues = m_objDevModule->getMcuSerialPortServer();
		m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
		return mxtrue;	
	}

	mxbool CDevPirMotionDet::handleSetPirInterval(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Pir Interval value:%s", strObjValue.c_str());
		std::string ret;
		cJSON *pJsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonRoot,"event","StrPirlnterva");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "pir_Interval", atoi(strObjValue.c_str()));
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
		std::string strGUID = m_objDevModule->getMcuSerialPortGUID();
		std::string strValues = m_objDevModule->getMcuSerialPortServer();
		m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());
		return mxtrue;	
	}
	
	mxbool CDevPirMotionDet::handleSetTimingDection(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Pir Switch value:%s", strObjValue.c_str());
		std::string ret;
		cJSON *pJsonRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonRoot,"event","pir_Sensitivity");

		cJSON *pJsonParam = cJSON_CreateObject();
		cJSON_AddNumberToObject(pJsonParam, "pir_Status", atoi(strObjValue.c_str()));
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
		std::string strGUID = m_objDevModule->getMcuSerialPortGUID();
		std::string strValues = m_objDevModule->getMcuSerialPortServer();
		m_objDevModule->output(strGUID, strValues, (unsigned char*)ret.c_str(), ret.length());		
		return mxtrue;
	}
}