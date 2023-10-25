#include "dev_manage_intelligent_det.h"

namespace maix {
//getFWParaConfig
	CDevIntelligentDet::CDevIntelligentDet(CDevManageModule* DevManageModule)
	: m_objDevModule(DevManageModule)
	{
		
	}

	CDevIntelligentDet::~CDevIntelligentDet()
	{

	}

	mxbool CDevIntelligentDet::handleSetPeopleDetection(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		if(m_objDevModule != NULL)
		{
			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();

			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_PEOPLE_DETECTION, strObjValue);
		}
		else
		{
			return mxfalse;
		}
		
        logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set People Det value:%s", strObjValue.c_str());
        return mxtrue;
	}
	mxbool CDevIntelligentDet::handleSetPackageDetection(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		if(m_objDevModule != NULL)
		{
			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();

			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_PACKAGE_DETECTION, strObjValue);
		}
		else
		{
			return mxfalse;
		}
		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Package Det value:%s", strObjValue.c_str());
		return mxtrue;
	}
	mxbool CDevIntelligentDet::handleSetVehicleDetection(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		if(m_objDevModule != NULL)
		{
			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();

			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_VEHICLE_DETECTION, strObjValue);
		}
		else
		{
			return mxfalse;
		}
		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Vehicle Det value:%s", strObjValue.c_str());
		return mxtrue;
	}
	mxbool CDevIntelligentDet::handleSetAnimalDetection(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		if(m_objDevModule != NULL)
		{
			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();

			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_ANIMAL_DETECTION, strObjValue);
		}
		else
		{
			return mxfalse;
		}
		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Animal Det value:%s", strObjValue.c_str());
		return mxtrue;		
	}
	mxbool CDevIntelligentDet::handleSetPeopleStayTime(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		int iArr[3] = {0, 3, 6}; ///< 0到2分别对应 0s、3s、6s

		int iValue = atoi(strObjValue.c_str());
		if (iValue >= int(sizeof(iArr) / sizeof(iArr[0])))
		{
			logPrint(MX_LOG_ERROR, "CDevIntelligentDet people stay time:%s is out of range", strObjValue.c_str());
			return mxfalse;
		}

		strObjValue = std::to_string(iArr[iValue]);

		if(m_objDevModule != NULL)
		{
			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();
			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_PEOPLE_STAY, strObjValue);
		}
		return mxtrue;			
	}

	mxbool CDevIntelligentDet::handleSetFaceRecognition(std::string &strValue)
	{
		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Face Recognition value:%s", strValue.c_str());
		return mxtrue;			
	}

	mxbool CDevIntelligentDet::handleSetAreaDetectCoord(std::string &strValue)
	{
		cJSON *pJsonStrValue = cJSON_Parse(strValue.c_str());
		cJSON *pJsonValue = cJSON_GetObjectItem(pJsonStrValue, "value");
		std::string strObjValue = std::string(pJsonValue->valuestring);	

		logPrint(MX_LOG_ERROR, "CDevIntelligentDet set Area m_objDevModule strValue is:%s", strObjValue.c_str());

		if(m_objDevModule != NULL)
		{
#if 0
			cJSON* jsonRoot = cJSON_CreateArray();
			cJSON* jsonVal = cJSON_Parse(strObjValue.c_str());
			if(jsonVal && cJSON_IsArray(jsonVal))
			{
				int iSize = cJSON_GetArraySize(jsonVal);
				for(int i = 0; i < iSize; i++)
				{
					cJSON* jsonValIndex = cJSON_GetArrayItem(jsonVal, i);
					cJSON* jsonIfUsing = cJSON_GetObjectItem(jsonValIndex, "ifUsing");
					int iVal = jsonIfUsing->valueint;
					if(iVal == 1)
					{
						cJSON* jsonLeftTop = cJSON_GetObjectItem(jsonValIndex, "leftTop");
						cJSON* jsonLeftBottom = cJSON_GetObjectItem(jsonValIndex, "leftBottom");
						cJSON* jsonRightTop = cJSON_GetObjectItem(jsonValIndex, "rightTop");
						double fUp = cJSON_GetObjectItem(jsonLeftTop, "y")->valuedouble;
						double fDown = cJSON_GetObjectItem(jsonLeftBottom, "y")->valuedouble;
						double fLeft = cJSON_GetObjectItem(jsonLeftTop, "x")->valuedouble;
						double fRight = cJSON_GetObjectItem(jsonRightTop, "x")->valuedouble;

						cJSON* jsonObject = cJSON_CreateObject();
						cJSON_AddNumberToObject(jsonObject, "up", fUp);
						cJSON_AddNumberToObject(jsonObject, "down", fDown);
						cJSON_AddNumberToObject(jsonObject, "left", fLeft);
						cJSON_AddNumberToObject(jsonObject, "right", fRight);

						cJSON_AddItemToArray(jsonRoot, jsonObject);
					}
				}
			}

			strObjValue = std::string(cJSON_Print(jsonRoot));
#endif

			std::string strGUID = m_objDevModule->getAiGUID();
			std::string strServerName = m_objDevModule->getAiServer();

			m_objDevModule->outputMsg(strGUID, strServerName, EVENT_SET_AREA_DETECT_COORD, strObjValue);
		}
		else
		{
			logPrint(MX_LOG_ERROR, "CDevIntelligentDet set Area m_objDevModule is NULL and strValue is:%s", strObjValue.c_str());
			return mxfalse;
		}

		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Area Det value:%s", strObjValue.c_str());
		return mxtrue;			
	}

	mxbool CDevIntelligentDet::handleSetTimingDetection(std::string &strValue)
	{
		logPrint(MX_LOG_DEBUG, "CDevIntelligentDet set Timing Det value:%s", strValue.c_str());
		return mxtrue;			
	}

}