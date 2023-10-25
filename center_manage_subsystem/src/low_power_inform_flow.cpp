#include "low_power_inform_flow.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "cJSON.h"
#include "log_mx.h"
#include "fw_env_para.h"
#include "sys/prctl.h"

namespace maix {
	CLowPowerInformFlow::CLowPowerInformFlow(CModule * module)
		: m_module(module)
		, m_eModuleType(E_Module_IDLE)
	{
		m_iModuleExit = ALL_MODULE_EXIT;
        m_iEnterProcCount = 0;
	}

	CLowPowerInformFlow::~CLowPowerInformFlow()
	{
	}

	mxbool CLowPowerInformFlow::init()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!m_module->getConfig("EVENT_REMOTE_SERVER", "GUID",
			m_strEventManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("EVENT_REMOTE_SERVER", "SERVER",
			m_strEventManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("DEV_REMOTE_SERVER", "GUID",
			m_strDevManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("DEV_REMOTE_SERVER", "SERVER",
			m_strDevManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("AUDIO_REMOTE_SERVER", "GUID",
			m_strAudioManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("AUDIO_REMOTE_SERVER", "SERVER",
			m_strAudioManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("VIDEO_REMOTE_SERVER", "GUID",
			m_strVideoManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("VIDEO_REMOTE_SERVER", "SERVER",
			m_strVideoManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_REMOTE_SERVER", "GUID",
			m_strSpeakerManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_REMOTE_SERVER", "SERVER",
			m_strSpeakerManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("IPC_REMOTE_SERVER", "GUID",
			m_strIPCManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("IPC_REMOTE_SERVER", "SERVER",
			m_strIPCManageModuleServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("MCU_REMOTE_SERVER", "GUID",
			m_strMCUManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("MCU_REMOTE_SERVER", "SERVER",
			m_strMCUManageModuleServer))
		{
			return mxfalse;
		}

        if (!m_module->getConfig("LOG_REMOTE_SERVER", "GUID",
            m_strLogManageModuleGUID))
        {
            return mxfalse;
        }

        if (!m_module->getConfig("LOG_REMOTE_SERVER", "SERVER",
            m_strLogManageModuleServer))
        {
            return mxfalse;
        }

        if (!m_module->getConfig("MEDIA_SOURCE_REMOTE_SERVER", "GUID",
            m_strMediaSourceModuleGUID))
        {
            return mxfalse;
        }

        if (!m_module->getConfig("MEDIA_SOURCE_REMOTE_SERVER", "SERVER",
            m_strMediaSourceModuleServer))
        {
            return mxfalse;
        }

		return mxtrue;
	}

	mxbool CLowPowerInformFlow::unInit()
	{
		return mxbool();
	}

	void CLowPowerInformFlow::run()
	{
        printf("CLowPowerInformFlow run\n");
        logPrint(MX_LOG_INFOR, "CLowPowerInformFlow run");
        prctl(PR_SET_NAME,"flow_lowpower");
		while (1)
		{
			switch (m_eModuleType)
			{
				case E_Module_IDLE:
				{
					if (m_iModuleExit == 0x00000000)
					{
						m_eModuleType = E_EnterLowPower;
					}		
#ifdef _WIN32
					Sleep(500);
#else
					usleep(500*1000);
#endif
				}
				break;
				case E_VideoSourceModule:
				{
					if (sendMediaSourceModuleEnterLowPower())
					{
						m_eModuleType = E_AudioManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(500 * 1000);
#endif
					}
				}
                break;
				case E_VideoManageModule:
// 				{
// 					if (sendVideoManageModuleEnterLowPower())
// 					{
// 						m_eModuleType = E_AudioManageModule;
// 					}
// 					else
// 					{
// #ifdef _WIN32
// 						Sleep(100);
// #else
// 						usleep(100 * 1000);
// #endif
// 					}
// 				}
				break;
				case E_AudioManageModule:
				{
					if (sendAudioManageModuleEnterLowPower())
					{
						m_eModuleType = E_SpeakerManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_SpeakerManageModule:
				{
					if (sendSpeakerManageModuleEnterLowPower())
					{
						m_eModuleType = E_EventManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_EventManageModule:
				{
					if (sendEventManageModuleEnterLowPower())
					{
						m_eModuleType = E_DevManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_DevManageModule:
				{
					if (sendDevManageModuleEnterLowPower())
					{
						m_eModuleType = E_IPCManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_IPCManageModule:
				{
					if (sendIPCManageModuleEnterLowPower())
					{
						m_eModuleType = E_LogManageModule;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_LogManageModule:
				{
					if (sendLogManageModuleEnterLowPower())
					{
						m_eModuleType = E_Module_IDLE;
					}
					else
					{
#ifdef _WIN32
						Sleep(100);
#else
						usleep(100 * 1000);
#endif
					}
				}
				break;
				case E_EnterLowPower:
				{
					if (lowPowerProc())
					{
						m_eModuleType = E_Module_IDLE;
					}
					else
					{
#ifdef _WIN32
						Sleep(500);
#else
						usleep(500 * 1000);
#endif
					}
				}
				break;
			}
		}
	}

	E_Module_Type CLowPowerInformFlow::getModuleType()
	{
		return m_eModuleType;
	}

	mxbool CLowPowerInformFlow::setModuleType(E_Module_Type eType)
	{
		m_eModuleType = eType;
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendIPCManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strIPCManageModuleGUID,
				m_strIPCManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "ipc: %s",strResult.c_str());

			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~IPC_MANAGE_EXIT);
				return mxtrue;
			}
		}

		m_iModuleExit &= (~IPC_MANAGE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendDevManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strDevManageModuleGUID,
				m_strDevManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "dev: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~DEV_MANAGE_EXIT);
				return mxtrue;
			}
		}

		m_iModuleExit &= (~DEV_MANAGE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendEventManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strEventManageModuleGUID,
				m_strEventManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "event: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~EVENT_MANAGE_EXIT);
				return mxtrue;
			}
		}

		m_iModuleExit &= (~EVENT_MANAGE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::checkLowPowerMode()
	{
		char *lowpower_mode = getFWParaConfig("user", LOWPOWER_MODE);
		if(lowpower_mode)
		{
			if(atoi(lowpower_mode) == ENABLE_LOWPOWER)
			{
				return mxtrue;
			}
		}

		return mxfalse;
	}

	mxbool CLowPowerInformFlow::sendMediaSourceModuleEnterLowPower()
	{
		if(access("/tmp/_netWakeUp",F_OK) == 0)
		{
			logPrint(MX_LOG_INFOR, "tmp/_netWakeUp is ok sendMediaSourceModuleEnterLowPower is Blocked\n");
			return mxfalse;
		}

		if (m_module == NULL)
			return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

        if(m_iEnterProcCount > 5)
        {
            if(access("/tmp/_liveing",F_OK) != 0)
            {
                logPrint(MX_LOG_INFOR,"touch enter lowpower");
                system("touch /tmp/force_enter_lowpower");
            }
        }
        m_iEnterProcCount++;

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strMediaSourceModuleGUID,
				m_strMediaSourceModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());
			logPrint(MX_LOG_DEBUG, "video source: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
                    logPrint(MX_LOG_DEBUG, "video source err: %s ", strCode.c_str());
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~VIDEO_SOURCE_EXIT);
				return mxtrue;
			}
		}
		m_iModuleExit &= (~VIDEO_SOURCE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendVideoManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strVideoManageModuleGUID,
				m_strVideoManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "video: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~VIDEO_MANAGE_EXIT);
				return mxtrue;
			}
		}
		m_iModuleExit &= (~VIDEO_MANAGE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendAudioManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strAudioManageModuleGUID,
				m_strAudioManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "audio: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~AUDIO_MANAGE_EXIT);
				return mxtrue;
			}
		}

		m_iModuleExit &= (~AUDIO_MANAGE_EXIT);
		return mxtrue;
	}

	mxbool CLowPowerInformFlow::sendSpeakerManageModuleEnterLowPower()
	{
		if (m_module == NULL)
			return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strSpeakerManageModuleGUID,
				m_strSpeakerManageModuleServer,
				(unsigned char*)strLowPower.c_str(), 
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "speaker: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
				else
				{
					return mxfalse;
				}
			}
			else
			{
				m_iModuleExit &= (~SPEAKER_MANAGE_EXIT);
				return mxtrue;
			}
		}

		m_iModuleExit &= (~SPEAKER_MANAGE_EXIT);
		return mxtrue;
	}

    mxbool CLowPowerInformFlow::sendLogManageModuleEnterLowPower()
    {
        if (m_module == NULL)
            return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
        cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strLowPower = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        if (m_module != NULL)
        {
            std::string strResult = m_module->output(
                m_strLogManageModuleGUID,
                m_strLogManageModuleServer,
                (unsigned char*)strLowPower.c_str(), 
                strLowPower.length());

            logPrint(MX_LOG_INFOR, "log: %s", strResult.c_str());
            if (strResult.length() > 0)
            {
                std::string strCode;
                std::string strErrMsg;
                cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

                if (jsonRoot)
                {
                    cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
                    if (jsonCode)
                    {
                        strCode = std::string(jsonCode->valuestring);
                    }
                    cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
                    if (jsonErrMsg)
                    {
                        char *pcErrMsg = cJSON_Print(jsonErrMsg);
                        if (pcErrMsg)
                        {
                            strErrMsg = std::string(pcErrMsg);
                            free(pcErrMsg);
                        }

                    }
                    cJSON_Delete(jsonRoot);
                }

                if (strCode.compare("200") == 0)
                {
                    return mxtrue;
                }
                else
                {
                    return mxfalse;
                }
            }
            else
            {
                m_iModuleExit &= (~LOG_MANAGE_EXIT);
                return mxtrue;
            }
        }

        m_iModuleExit &= (~LOG_MANAGE_EXIT);
        return mxtrue;        
    }

	mxbool CLowPowerInformFlow::enterLowPower(E_Module_Type eType)
	{
		logPrint(MX_LOG_INFOR, "enterLowPower: %d", eType);
		switch (eType)
		{
		case E_IPCManageModule:
		{
			m_iModuleExit &= (~IPC_MANAGE_EXIT);
		}
		break;
		case E_DevManageModule:
		{
			m_iModuleExit &= (~DEV_MANAGE_EXIT);
		}
		break;
		case E_EventManageModule:
		{
			m_iModuleExit &= (~EVENT_MANAGE_EXIT);
		}
		break;
		// case E_VideoManageModule:
		// {
		// 	m_iModuleExit &= (~VIDEO_MANAGE_EXIT);
		// }
		// break;
		case E_AudioManageModule:
		{
			m_iModuleExit &= (~AUDIO_MANAGE_EXIT);
		}
		break;
		case E_SpeakerManageModule:
		{
			m_iModuleExit &= (~SPEAKER_MANAGE_EXIT);
		}
		break;
        case E_LogManageModule:
        {
            m_iModuleExit &= (~LOG_MANAGE_EXIT);
        }
        break;
        case E_VideoSourceModule:
        {
            m_iModuleExit &= (~VIDEO_SOURCE_EXIT);
        }
        break;
		default:
			break;
		}

		if (m_iModuleExit == 0x00000000)
		{
			setModuleType(E_EnterLowPower);
		}

		return mxtrue;
	}

	mxbool CLowPowerInformFlow::lowPowerProc()
	{
		m_iModuleExit = ALL_MODULE_EXIT;
		logPrint(MX_LOG_INFOR, "lowPowerProc");

		if (m_module == NULL)
			return mxfalse;

		if (!checkLowPowerMode())
		{
			return mxfalse;
		}

		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "EnterLowPower");
		cJSON_AddNumberToObject(jsonParam, "timeout", 123456);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLowPower = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(
				m_strMCUManageModuleGUID,
				m_strMCUManageModuleServer,
				(unsigned char*)strLowPower.c_str(),
				strLowPower.length());

			logPrint(MX_LOG_INFOR, "lowPowerProc: %s", strResult.c_str());
			if (strResult.length() > 0)
			{
				std::string strCode;
				std::string strErrMsg;
				cJSON *jsonRoot = cJSON_Parse(strResult.c_str());

				if (jsonRoot)
				{
					cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
					if (jsonCode)
					{
						strCode = std::string(jsonCode->valuestring);
					}
					cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "msg");
					if (jsonErrMsg)
					{
						char *pcErrMsg = cJSON_Print(jsonErrMsg);
						if (pcErrMsg)
						{
							strErrMsg = std::string(pcErrMsg);
							free(pcErrMsg);
						}

					}
					cJSON_Delete(jsonRoot);
				}

				if (strCode.compare("200") == 0)
				{
					return mxtrue;
				}
			}
		}

		return mxfalse;
	}
}
