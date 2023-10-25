#include "mcu_dev_spec_adapter.h"
#include <sstream>
#include "cJSON.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log_mx.h"

namespace maix
{
	CMcuDecSpecAdapter::CMcuDecSpecAdapter(CModule *module)
		: m_module(module)
	{
		m_bInit = mxfalse;
		memset(&m_ipcDevConfig, 0, sizeof(m_ipcDevConfig));
	}

	CMcuDecSpecAdapter::~CMcuDecSpecAdapter()
	{
		if (m_bInit)
		{
			unInit();
		}
	}

	mxbool CMcuDecSpecAdapter::init()
	{
		T_SPEC_ADAPTER specAdapter[] = 
		{
			{EVENT_SET_AUTO_ALARM_SWITCH, EVENT_GET_AUTO_ALARM_SWITCH, offsetof(DevConfig, auto_audible_visual_alarm), DATA_TYPE_UINT8},
			{EVENT_SET_MANUAL_ALARM_SWITCH, EVENT_GET_MANUAL_ALARM_SWITCH, offsetof(DevConfig, m_audible_visual_alarm), DATA_TYPE_UINT8},
			{EVENT_SET_NIGHT_SHOT, EVENT_GET_NIGHT_SHOT, offsetof(DevConfig, mai_dn), DATA_TYPE_UINT8},
			{EVENT_SET_WDR, EVENT_GET_WDR, offsetof(DevConfig, mai_hdr), DATA_TYPE_UINT8},
			{EVENT_SET_WATER_MARK, EVENT_GET_WATER_MARK, offsetof(DevConfig, mai_wm), DATA_TYPE_UINT8},
			{EVENT_SET_CAMERA, EVENT_GET_CAMERA, offsetof(DevConfig, camera_switch), DATA_TYPE_UINT8},
			{EVENT_SET_INDICATE_LIGHT, EVENT_GET_INDICATE_LIGHT, offsetof(DevConfig, indicate_light_switch), DATA_TYPE_UINT8},
			{EVENT_SET_WHITE_LIGHT, EVENT_GET_WHITE_LIGHT, offsetof(DevConfig, white_light_switch), DATA_TYPE_UINT8},
			// {EVENT_SET_WHITE_LIGHT_BRIGHTNESS, EVENT_GET_WHITE_LIGHT_BRIGHTNESS, offsetof(DevConfig, auto_audible_visual_alarm), DATA_TYPE_UINT8},
			// {EVENT_SET_BATTERY_LEVEL, EVENT_GET_BATTERY_LEVEL, offsetof(DevConfig, auto_audible_visual_alarm), DATA_TYPE_UINT8},
			// {EVENT_SET_BATTERY_STATE, EVENT_GET_BATTERY_STATE, offsetof(DevConfig, auto_audible_visual_alarm), DATA_TYPE_UINT8},
			{EVENT_SET_PEOPLE_DETECTION, EVENT_GET_PEOPLE_DETECTION, offsetof(DevConfig, person_detection), DATA_TYPE_UINT8},
            {EVENT_SET_ANIMAL_DETECTION, EVENT_GET_ANIMAL_DETECTION, offsetof(DevConfig, animal_detection), DATA_TYPE_UINT8},
            {EVENT_SET_VEHICLE_DETECTION, EVENT_GET_VEHICLE_DETECTION, offsetof(DevConfig, vehicle_detection), DATA_TYPE_UINT8},
            {EVENT_SET_PACKAGE_DETECTION, EVENT_GET_PACKAGE_DETECTION, offsetof(DevConfig, package_detection), DATA_TYPE_UINT8},
            {EVENT_SET_AREA_DETECT_COORD, EVENT_GET_AREA_DETECT_COORD, offsetof(DevConfig, area_detect_coord), DATA_TYPE_ARRAY},
            {EVENT_SET_PEOPLE_STAY, EVENT_GET_PEOPLE_STAY, offsetof(DevConfig, people_stay_time), DATA_TYPE_UINT8},
            {EVENT_SET_PIR_SENSITIVITY, EVENT_GET_PIR_SENSITIVITY, offsetof(DevConfig, pir_sensivitive), DATA_TYPE_UINT8},
            {EVENT_SET_PIR_DETECTION, EVENT_GET_PIR_DETECTION, offsetof(DevConfig, pir_detection), DATA_TYPE_UINT8},
            {EVENT_SET_PIR_INTERVAL, EVENT_GET_PIR_INTERVAL, offsetof(DevConfig, pir_interval), DATA_TYPE_UINT32},
			{EVENT_SET_WHITE_LIGHT_BRIGHTNESS, EVENT_GET_WHITE_LIGHT_BRIGHTNESS, offsetof(DevConfig, white_light_brightness), DATA_TYPE_UINT8},
			{EVENT_SET_TIMING_DECTION, EVENT_GET_TIMING_DECTION, offsetof(DevConfig, timing_detection), DATA_TYPE_UINT8},
			// {EVENT_SET_WIFI_SIGNAL, EVENT_GET_WIFI_SIGNAL, offsetof(DevConfig, auto_audible_visual_alarm), DATA_TYPE_UINT8},
		};


		for (unsigned int i = 0; i < sizeof(specAdapter) / sizeof(specAdapter[0]); i++)
		{
			m_vecSpecAdapter.push_back(specAdapter[i]);
		}

		m_ipcDevConfig.area_detect_coord = (pb_bytes_array_t *)malloc(sizeof(pb_bytes_array_t) + MAX_BUFFER_SIZE);
		if (!m_ipcDevConfig.area_detect_coord)
		{
			logPrint(MX_LOG_ERROR, "dev spec adapter area_detect_coord malloc failed");
			return mxfalse;
		}
		memset(m_ipcDevConfig.area_detect_coord, 0, sizeof(pb_bytes_array_t) + MAX_BUFFER_SIZE);
		m_ipcDevConfig.area_detect_coord->size = 0;

		m_bRun = mxtrue;
        m_syncThread = std::thread([this](){
            this->run();
        });

		m_bInit = mxtrue;

		return mxtrue;
	}

	mxbool CMcuDecSpecAdapter::unInit()
	{
		if (m_bInit)
		{
			if (m_ipcDevConfig.area_detect_coord)
			{
				free(m_ipcDevConfig.area_detect_coord);
				m_ipcDevConfig.area_detect_coord = NULL;
			}

			m_bRun = mxfalse;
			m_syncThread.join();
			m_bInit = mxfalse;
		}

		return mxtrue;
	}

	void CMcuDecSpecAdapter::run()
	{
		std::string strRet;
		std::string strJson;
		std::string strValue;

		while (m_bRun)
		{
			if (strJson.empty())
			{
				cJSON *pJsonRoot = cJSON_CreateObject();
				if (pJsonRoot)
				{
					cJSON_AddStringToObject(pJsonRoot, "event", "devConfig");
					cJSON *pjsonParam = cJSON_CreateObject();
					if (pjsonParam)
					{
						cJSON *pjsonArray = cJSON_CreateArray();
						if (pjsonArray)
						{
							for (unsigned int i = 0; i < m_vecSpecAdapter.size(); i++)
							{
								cJSON *pjsonArrayItem = cJSON_CreateObject();
								if(pjsonArrayItem)
								{
									cJSON_AddStringToObject(pjsonArrayItem, "configName", m_vecSpecAdapter[i].strEventGet.c_str());
									cJSON_AddItemToArray(pjsonArray, pjsonArrayItem);
								}
							}
						}

						cJSON_AddItemToObject(pjsonParam, "cmd", pjsonArray);
						cJSON_AddItemToObject(pJsonRoot, "param", pjsonParam);

						char *pOut = cJSON_PrintUnformatted(pJsonRoot);
						if (pOut)
						{
							strJson = std::string(pOut);
							cJSON_free(pOut);
							pOut = NULL;
						}
					}
					cJSON_Delete(pJsonRoot);
					pJsonRoot = NULL;
				}
			}
			else
			{
				CMCUSerialPortModule *module = dynamic_cast<CMCUSerialPortModule *>(m_module);
			 	strRet = module->sendDevMsg(strJson);
				logPrint(MX_LOG_DEBUG, "dev spec adapter sync all dev config ret:%s", strRet.c_str());
				if (strRet.length() > 0)
				{
					std::string strCode;
					std::string strErrMsg;
					cJSON *pJsonRoot = cJSON_Parse(strRet.c_str());

					if (pJsonRoot)
					{
						cJSON *pJsonParam = cJSON_GetObjectItem(pJsonRoot, "param");
						if (pJsonParam)
						{
							if (syncAllIpcDevConfig(pJsonParam))
							{
								m_bRun = mxfalse;

								logPrint(MX_LOG_TRACE, "mai_dn:%d, mai_wm:%d, mai_hdr:%d vehicle_detection:%d, person_detection:%d, animal_detection:%d, "
                                "package_detection:%d, people_stay_time:%d, auto_audible_visual_alarm:%d, m_audible_visual_alarm%d, camera_switch:%d, "
                                "indicate_light_switch:%d, white_light_switch:%d, pir_detection:%d, pir_sensivitive:%d, pir_interval:%d, white_light_brightness:%d ,timing_detection:%d",
                                m_ipcDevConfig.mai_dn, m_ipcDevConfig.mai_wm, m_ipcDevConfig.mai_hdr, m_ipcDevConfig.vehicle_detection, m_ipcDevConfig.person_detection, m_ipcDevConfig.animal_detection,
                                m_ipcDevConfig.package_detection, m_ipcDevConfig.people_stay_time, m_ipcDevConfig.auto_audible_visual_alarm, m_ipcDevConfig.m_audible_visual_alarm, m_ipcDevConfig.camera_switch,
                                m_ipcDevConfig.indicate_light_switch, m_ipcDevConfig.white_light_switch, m_ipcDevConfig.pir_detection, m_ipcDevConfig.pir_sensivitive, m_ipcDevConfig.pir_interval, m_ipcDevConfig.white_light_brightness, m_ipcDevConfig.timing_detection);
								ipc_dev_config_request();
							}
						}

						cJSON_Delete(pJsonRoot);
						pJsonRoot = NULL;
					}
				}
			}

#ifdef _WIN32
			Sleep(10000);
#else
			usleep(500 * 1000);
#endif
		}

		return ;
	}

	std::string CMcuDecSpecAdapter::getAllDevInfo()
	{
		std::string strRet;
		std::string strJson;
		std::string strValue;

		cJSON *pJsonRoot = cJSON_CreateObject();
		if (pJsonRoot)
		{
			cJSON_AddStringToObject(pJsonRoot, "event", "devConfig");
			cJSON *pjsonParam = cJSON_CreateObject();
			if (pjsonParam)
			{
				cJSON *pjsonArray = cJSON_CreateArray();
				if (pjsonArray)
				{
					for (unsigned int i = 0; i < m_vecSpecAdapter.size(); i++)
					{
						cJSON *pjsonArrayItem = cJSON_CreateObject();
						if(pjsonArrayItem)
						{
							cJSON_AddStringToObject(pjsonArrayItem, "configName", m_vecSpecAdapter[i].strEventGet.c_str());
							cJSON_AddItemToArray(pjsonArray, pjsonArrayItem);
						}
					}
				}

				cJSON_AddItemToObject(pjsonParam, "cmd", pjsonArray);
				cJSON_AddItemToObject(pJsonRoot, "param", pjsonParam);

				strJson = cJSON_PrintUnformatted(pJsonRoot);
			}
			cJSON_Delete(pJsonRoot);
			pJsonRoot = NULL;
		}

		CMCUSerialPortModule *module = dynamic_cast<CMCUSerialPortModule *>(m_module);
		strRet = module->sendDevMsg(strJson);
		return strRet;
	}

	mxbool CMcuDecSpecAdapter::syncAllIpcDevConfig(const cJSON *pJsonParam)
	{
		std::string strConfigName;
		std::string strValue;

		cJSON *pJsonCmd = cJSON_GetObjectItem(pJsonParam, "cmd");
		if (!pJsonCmd || pJsonCmd->type != cJSON_Array)
		{
			logPrint(MX_LOG_ERROR, "sync ipc dev config failed:%p", pJsonCmd);
			return mxfalse;
		}

		int iCnt = cJSON_GetArraySize(pJsonCmd);
		if (iCnt <= 0)
		{
			logPrint(MX_LOG_ERROR, "sync ipc dev config failed, array:%d", iCnt);
			return mxfalse;
		}

		for (int i = 0; i < iCnt; i++)
		{
			cJSON *pJsonItem = cJSON_GetArrayItem(pJsonCmd, i);
			if (pJsonItem)
			{
				cJSON *pJsonConfigName = cJSON_GetObjectItem(pJsonItem, "configName");
				if (pJsonConfigName && pJsonConfigName->type == cJSON_String)
				{
					cJSON *pJsonConfigParam = cJSON_GetObjectItem(pJsonItem, "configParam");
					if (pJsonConfigParam)
					{
						cJSON *pJsonValue = cJSON_GetObjectItem(pJsonConfigParam, "value");
						if (pJsonValue && pJsonValue->type == cJSON_String)
						{
							strConfigName = std::string(pJsonConfigName->valuestring);
							strValue = std::string(pJsonValue->valuestring);
							if (!syncSingleDevConfig(strConfigName, strValue))
							{
								logPrint(MX_LOG_WARN, "sync ipc dev config index:%d failed", i);
								continue;
							}
						}
					}
				}

				if (strConfigName.empty() || strValue.empty())
				{
					logPrint(MX_LOG_WARN, "sync ipc dev config index:%d failed", i);
					continue;
				} 
			}
		}

		logPrint(MX_LOG_DEBUG, "loadDevConfig success");

		return mxtrue;
	}

	mxbool CMcuDecSpecAdapter::setDevConfigValue(DevConfig *config, T_SPEC_ADAPTER *specAdapter, const std::string &strValue)
	{
		if (specAdapter->dataType == DATA_TYPE_UINT8)
		{
			uint8_t* ptr = (uint8_t*)((char*)config + specAdapter->offset);
			*ptr = atoi(strValue.c_str());
		}
		else if (specAdapter->dataType == DATA_TYPE_UINT32)
		{
			uint32_t* ptr = (uint32_t*)((char*)config + specAdapter->offset);
			*ptr = atoi(strValue.c_str());
		}
		else if (specAdapter->dataType == DATA_TYPE_ARRAY)
		{
			pb_bytes_array_s* ptr = *(pb_bytes_array_s **)((char*)config+specAdapter->offset);
			ptr->size = strValue.length() + 1;
			strncpy((char *)ptr->bytes, strValue.c_str(), MAX_BUFFER_SIZE);
		}
		else
		{
			logPrint(MX_LOG_ERROR, "sync single dev config failed, dataType:%d", specAdapter->dataType);
			return mxfalse;
		}

		return mxtrue;
	}
	
	mxbool CMcuDecSpecAdapter::getDevConfigValue(DevConfig *config, T_SPEC_ADAPTER *specAdapter, std::string &strValue)
	{
		if (specAdapter->dataType == DATA_TYPE_UINT8)
		{
			uint8_t* ptr = (uint8_t*)((char*)config + specAdapter->offset);
			strValue = std::to_string(*ptr);
		}
		else if (specAdapter->dataType == DATA_TYPE_UINT32)
		{
			uint32_t* ptr = (uint32_t*)((char*)config + specAdapter->offset);
			strValue = std::to_string(*ptr);
		}
		else if (specAdapter->dataType == DATA_TYPE_ARRAY)
		{
			pb_bytes_array_s* ptr = *(pb_bytes_array_s **)((char*)config + specAdapter->offset);
			if (ptr->size > 2)
			{
				strValue.assign(std::string((char *)ptr->bytes));
				strValue = strValue.substr(1, strValue.length() - 2);
				std::string strTmpValue;
				std::copy_if(strValue.begin(), strValue.end(), std::back_inserter(strTmpValue), [](char ch) {
					return ch != '\\';
				});

				strValue = strTmpValue;
			}
		}
		else
		{
			logPrint(MX_LOG_ERROR, "sync single dev config failed, dataType:%d", specAdapter->dataType);
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CMcuDecSpecAdapter::syncSingleDevConfig(std::string strName, std::string strValue)
	{
		mxbool bRet = mxfalse;
		if (strName.empty() || strValue.empty())
		{
			logPrint(MX_LOG_WARN, "sync single dev config failed");
			return bRet;
		}

		for (unsigned int i = 0; i < m_vecSpecAdapter.size(); i++)
		{
			if (0 == m_vecSpecAdapter[i].strEventGet.compare(strName))
			{
				std::lock_guard<std::mutex> lock(m_configMutex);
				setDevConfigValue(&m_ipcDevConfig, &m_vecSpecAdapter[i], strValue);
				bRet = mxtrue;
				break;
			}
		}
		
		return bRet;
	}

	void CMcuDecSpecAdapter::handleSingleConfig(int iOffset, DevConfig *pConfig, cJSON *pJsonArray)
	{
		std::string strValue;
		mxbool bRet = mxfalse;

		for (unsigned int i = 0; i < m_vecSpecAdapter.size(); i++)
		{
			if (m_vecSpecAdapter[i].offset == iOffset)
			{
				if (getDevConfigValue(pConfig, &m_vecSpecAdapter[i], strValue))
				{
					if (composeConfigMsg(m_vecSpecAdapter[i].strEventSet, strValue, pJsonArray))
					{
						if (setDevConfigValue(&m_ipcDevConfig, &m_vecSpecAdapter[i], strValue))
						{
							bRet = mxtrue;
							logPrint(MX_LOG_DEBUG, "handleSingleConfig success, event:%s strValue:%s", m_vecSpecAdapter[i].strEventSet.c_str(), strValue.c_str());
						}
					}
				}
				break;
			}
		}

		if (!bRet)
		{
			logPrint(MX_LOG_ERROR, "handleSingleConfig failed, offset:%d", iOffset);
		}
	}

	void CMcuDecSpecAdapter::handleDevConfig(Ipc *pIpc)
	{
		std::lock_guard<std::mutex> lock(m_configMutex);

		DevConfig *oldConfig = &m_ipcDevConfig;
		DevConfig *newConfig = &pIpc->payload.dev_config;

		cJSON *pJsonArray = cJSON_CreateArray();

		if (newConfig->mai_dn != oldConfig->mai_dn)
		{
			handleSingleConfig(offsetof(DevConfig, mai_dn), newConfig, pJsonArray);
		} 
		if (newConfig->mai_wm != oldConfig->mai_wm)
		{
			handleSingleConfig(offsetof(DevConfig, mai_wm), newConfig, pJsonArray);
		} 
		if (newConfig->mai_hdr != oldConfig->mai_hdr)
		{
			handleSingleConfig(offsetof(DevConfig, mai_hdr), newConfig, pJsonArray);
		} 
		if (newConfig->vehicle_detection != oldConfig->vehicle_detection)
		{
			handleSingleConfig(offsetof(DevConfig, vehicle_detection), newConfig, pJsonArray);
		} 
		if (newConfig->person_detection != oldConfig->person_detection)
		{
			handleSingleConfig(offsetof(DevConfig, person_detection), newConfig, pJsonArray);
		} 
		if (newConfig->animal_detection != oldConfig->animal_detection)
		{
			handleSingleConfig(offsetof(DevConfig, animal_detection), newConfig, pJsonArray);
		} 
		if (newConfig->package_detection != oldConfig->package_detection)
		{
			handleSingleConfig(offsetof(DevConfig, package_detection), newConfig, pJsonArray);
		} 
		if (newConfig->people_stay_time != oldConfig->people_stay_time)
		{
			handleSingleConfig(offsetof(DevConfig, people_stay_time), newConfig, pJsonArray);
		} 
		if (newConfig->auto_audible_visual_alarm != oldConfig->auto_audible_visual_alarm)
		{
			handleSingleConfig(offsetof(DevConfig, auto_audible_visual_alarm), newConfig, pJsonArray);
		} 
		if (newConfig->m_audible_visual_alarm != oldConfig->m_audible_visual_alarm)
		{
			handleSingleConfig(offsetof(DevConfig, m_audible_visual_alarm), newConfig, pJsonArray);
		} 
		if (newConfig->camera_switch != oldConfig->camera_switch)
		{
			handleSingleConfig(offsetof(DevConfig, camera_switch), newConfig, pJsonArray);
		} 
		if (newConfig->indicate_light_switch != oldConfig->indicate_light_switch)
		{
			handleSingleConfig(offsetof(DevConfig, indicate_light_switch), newConfig, pJsonArray);
		} 
		if (newConfig->white_light_switch != oldConfig->white_light_switch)
		{
			handleSingleConfig(offsetof(DevConfig, white_light_switch), newConfig, pJsonArray);
		} 
		if (newConfig->pir_detection != oldConfig->pir_detection)
		{
			handleSingleConfig(offsetof(DevConfig, pir_detection), newConfig, pJsonArray);
		} 
		if (newConfig->pir_sensivitive != oldConfig->pir_sensivitive)
		{
			handleSingleConfig(offsetof(DevConfig, pir_sensivitive), newConfig, pJsonArray);
		} 
		if (newConfig->pir_interval != oldConfig->pir_interval)
		{
			handleSingleConfig(offsetof(DevConfig, pir_interval), newConfig, pJsonArray);
		} 
		if (newConfig->area_detect_coord != oldConfig->area_detect_coord)
		{
			handleSingleConfig(offsetof(DevConfig, area_detect_coord), newConfig, pJsonArray);
		} 
		if (newConfig->white_light_brightness != oldConfig->white_light_brightness)
        {
            handleSingleConfig(offsetof(DevConfig, white_light_brightness), newConfig, pJsonArray);
        }
		if(newConfig->timing_detection != oldConfig->timing_detection)
		{
			handleSingleConfig(offsetof(DevConfig, timing_detection), newConfig, pJsonArray);
		}

		logPrint(MX_LOG_DEBUG, "mai_dn:%d, mai_wm:%d, mai_hdr:%d vehicle_detection:%d, person_detection:%d, animal_detection:%d, "
                                "package_detection:%d, people_stay_time:%d, auto_audible_visual_alarm:%d, m_audible_visual_alarm%d, camera_switch:%d, "
                                "indicate_light_switch:%d, white_light_switch:%d, pir_detection:%d, pir_sensivitive:%d, pir_interval:%d, white_light_brightness:%d, "
								"timing_detection:%d",
                                m_ipcDevConfig.mai_dn, m_ipcDevConfig.mai_wm, m_ipcDevConfig.mai_hdr, m_ipcDevConfig.vehicle_detection, m_ipcDevConfig.person_detection, m_ipcDevConfig.animal_detection,
                                m_ipcDevConfig.package_detection, m_ipcDevConfig.people_stay_time, m_ipcDevConfig.auto_audible_visual_alarm, m_ipcDevConfig.m_audible_visual_alarm, m_ipcDevConfig.camera_switch,
                                m_ipcDevConfig.indicate_light_switch, m_ipcDevConfig.white_light_switch, m_ipcDevConfig.pir_detection, m_ipcDevConfig.pir_sensivitive, m_ipcDevConfig.pir_interval, 
								m_ipcDevConfig.white_light_brightness, m_ipcDevConfig.timing_detection);

		if (cJSON_GetArraySize(pJsonArray) > 0)
		{
			sendSpecMsg(pJsonArray);
		}

		return ;
	}

	mxbool CMcuDecSpecAdapter::composeConfigMsg(const std::string &strName, const std::string &strValue, cJSON *pJsonArray)
	{
		cJSON *pJsonArrayItem = cJSON_CreateObject();
		cJSON_AddStringToObject(pJsonArrayItem, "configName", strName.c_str());

		cJSON *jsonConfigParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonConfigParam, "value", strValue.c_str());

		cJSON_AddItemToObject(pJsonArrayItem, "configParam", jsonConfigParam);
		cJSON_AddItemToArray(pJsonArray, pJsonArrayItem);

		return mxtrue;
	}

	mxbool CMcuDecSpecAdapter::sendSpecMsg(cJSON *pjsonArray)
	{
		std::string strJson;
		char *pOut = NULL;
		cJSON *pJsonRoot = cJSON_CreateObject();
		if (pJsonRoot)
		{
			cJSON_AddStringToObject(pJsonRoot, "event", "devConfig");
			cJSON *pjsonParam = cJSON_CreateObject();
			if (pjsonParam)
			{
				// cJSON_AddNumberToObject(pjsonParam, "id", id);
				// cJSON_AddStringToObject(pjsonParam, "did", strDid.c_str());
				cJSON_AddItemToObject(pjsonParam, "cmd", pjsonArray);
				cJSON_AddItemToObject(pJsonRoot, "param", pjsonParam);
				pOut = cJSON_PrintUnformatted(pJsonRoot);
				if (pOut)
				{
					strJson = std::string(pOut);
					cJSON_free(pOut);
					pOut = NULL;
				}
			}
			cJSON_Delete(pJsonRoot);
			pJsonRoot = NULL;
		}

		if (strJson.empty())
		{
			logPrint(MX_LOG_ERROR, "send spec msg, str json is empty, create failed");
			return mxfalse;
		}

		CMCUSerialPortModule *module = dynamic_cast<CMCUSerialPortModule *>(m_module);
		std::string strRet = module->sendDevMsg(strJson);

		logPrint(MX_LOG_DEBUG, "sendSpecMsg, json:%s, len:%d, ret:%s", 
										strJson.c_str(), strJson.length(), strRet.c_str());

		/// 失败或成功可通知mcu

		return mxtrue;
	}


}