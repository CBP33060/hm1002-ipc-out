#include "dev_play_voice.h"
#include "com_rpc_client_end_point.h"
#include "cJSON.h"
#include "log_mx.h"
#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "crypt_api_mx.h"
#include "fw_env_para.h"
#include "sstream"
#include "iostream"
#include "fstream"

namespace maix {
	CDevPlayVoice::CDevPlayVoice(CModule * module)
		: m_module(module)
		, m_bInit(mxfalse)
		, m_iLostFrameNum(0)
	{
	}

	CDevPlayVoice::~CDevPlayVoice()
	{
	}

	mxbool CDevPlayVoice::init()
	{
		if (!m_module)
			return mxfalse;

		if (!m_objPacketQueue.init(10, 0))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strDevModuleName))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("MODULE", "GUID", m_strDevModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_MANAGE_REMOTE_EVENT", 
			"GUID", m_strSpeakerManageModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_MANAGE_REMOTE_EVENT",
			"SERVER", m_strSpeakerManageRemoteEventServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_MANAGE_REMOTE_EVENT",
			"CHN", m_strSpeakerManageChannelServer))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("MCU_SERIAL_REMOTE_EVENT", 
			"GUID", m_strMCUModuleGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("MCU_SERIAL_REMOTE_EVENT",
			"SERVER", m_strMCURemoteEventServer))
		{
			return mxfalse;
		}

        if(!loadAudioConfig())
        {
            return mxfalse;
        }

        if(!loadAudioRes())
        {
            return mxfalse;
        }

        m_objPlayVoiceFileRead = std::shared_ptr<CDevPlayVoiceFileRead>(
            new CDevPlayVoiceFileRead());
        if(!m_objPlayVoiceFileRead->init())
        {
            return mxfalse;
        }
        m_objPlayVoiceFileRead->registCallback([&](unsigned char * pcData, 
            int iLen, E_P_TYPE ePacketType,E_VOICE_PLAY_LEVEL eVoiceLevel){
                sendframe(pcData,iLen,ePacketType,eVoiceLevel);
        });
        m_objPlayVoiceFileRead->registPowerUpCallback([&](){
                return getPowerUpEndState();
        });

		m_threadPlayVoice = std::thread([this]() {
			while (1)
			{
				mxbool bRet = this->openSpeaker();
				if (bRet)
                {
                    logPrint(MX_LOG_INFOR, "dev play open speaker success");
                    break;
                }
                usleep(200 * 1000);
			}

			this->run();
		});

		return mxtrue;
	}

	mxbool CDevPlayVoice::unInit()
	{
		return mxbool();
	}

    mxbool CDevPlayVoice::loadAudioConfig()
    {
        if (!m_module->getConfig("AUDIO_RESOURCE", "RESOURCE_CONFIG", m_strAudioResConfig))
        {
            return mxfalse;
        }     
        if (!m_module->getConfig("AUDIO_RESOURCE", "RESOURCE_AUDIO_PATH", m_strAudioResPath))
        {
            return mxfalse;
        } 
        // char * language = NULL;
        // language = getFWParaConfig(PARA_AUDIO_LANGUAGE);
        // if(NULL == language)
        // {
        //     return mxfalse;
        // }
        // m_strLanguage =  std::string(language);
        return mxtrue;
    }

    mxbool CDevPlayVoice::loadAudioRes()
    {
        std::ifstream fin(m_strAudioResConfig,std::ios::in);
        char line[1024] = {0};
        while(fin.getline(line,sizeof(line)))
        {
            std::stringstream lineStrResult(line);
            std::string strKey = "";
            std::string strValue = "";
            std::string strDescribe = "";
            lineStrResult >> strKey;
            lineStrResult >> strValue;
            lineStrResult >> strDescribe;
            m_mapAudioRes[strKey] = strValue;
        }
        logPrint(MX_LOG_INFOR, "resMap size : %d", m_mapAudioRes.size());
        fin.clear();
        fin.close();
        return mxtrue;
    }

    mxbool CDevPlayVoice::getPowerUpEndState()
    {
		cJSON *jsonRoot = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "GetPowerUpPlayEndState");

		char *out = cJSON_Print(jsonRoot);
		std::string strState = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strMCUModuleGUID,
				m_strMCURemoteEventServer,
				(unsigned char*)strState.c_str(), strState.length());

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
					std::string strEvent;
					std::string strParam;
					cJSON *jsonRoot = cJSON_Parse(strErrMsg.c_str());

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

					if (strEvent.compare("GetPowerUpPlayEndState") == 0)
					{
						cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
                        mxbool powerUpEndState;
						if (jsonRoot)
						{
							cJSON *jsonEndState = cJSON_GetObjectItem(jsonRoot, "endState");
							if (!jsonEndState)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
                                if(cJSON_IsTrue(jsonEndState) == 1)
                                {
                                    powerUpEndState = mxtrue;
                                }
                                else
                                {
                                    powerUpEndState = mxfalse;
                                }
							}

							cJSON_Delete(jsonRoot);
						}

						return powerUpEndState;
					}
				}
			}
		}

		return mxfalse;        
    }

	mxbool CDevPlayVoice::openSpeaker()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "openSpeaker");

		cJSON_AddStringToObject(jsonParam, "moduleGUID", 
			m_strDevModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "moduleName", 
			m_strDevModuleName.c_str());
		cJSON_AddStringToObject(jsonParam, "serverName", 
			m_strSpeakerManageChannelServer.c_str());
		cJSON_AddStringToObject(jsonParam, "channelID", 
			m_strSpeakerManageChannelServer.c_str());

		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strOpen = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strSpeakerManageModuleGUID,
				m_strSpeakerManageRemoteEventServer,
				(unsigned char*)strOpen.c_str(), strOpen.length());

			// std::cout << strResult << std::endl;
            logPrint(MX_LOG_INFOR,"open speaker result %s",strResult.c_str());
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
					std::string strEvent;
					std::string strParam;
					cJSON *jsonRoot = cJSON_Parse(strErrMsg.c_str());

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

					if (strEvent.compare("openSpeaker") == 0)
					{
						cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
						std::string strModuleGUID;
						std::string strModuleName;
						std::string strChannelName;
						std::string strServerName;
						int iType = 0;
						std::string strIP;
						int iPort = 0;
						std::string strUnix;
						int iLen = 0;
						if (jsonRoot)
						{
							cJSON *jsonModuleGUID = cJSON_GetObjectItem(jsonRoot, "moduleGUID");
							if (!jsonModuleGUID)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strModuleGUID = std::string(jsonModuleGUID->valuestring);
							}

							cJSON *jsonModuleName = cJSON_GetObjectItem(jsonRoot, "moduleName");
							if (!jsonModuleName)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strModuleName = std::string(jsonModuleName->valuestring);
							}

							cJSON *jsonKey = cJSON_GetObjectItem(jsonRoot, "key");
							if (!jsonKey)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								m_strKey = std::string(jsonKey->valuestring);
							}

							cJSON *jsonServerName = cJSON_GetObjectItem(jsonRoot, "serverName");
							if (!jsonServerName)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strServerName = std::string(jsonServerName->valuestring);
							}

							cJSON *jsonChannelID = cJSON_GetObjectItem(jsonRoot, "channelID");
							if (!jsonChannelID)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strChannelName = std::string(jsonChannelID->valuestring);
							}

							cJSON *jsonType = cJSON_GetObjectItem(jsonRoot, "type");
							if (!jsonType)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								iType = jsonType->valueint;
								if (iType == 5)
									iType = 4;
							}

							cJSON *jsonIP = cJSON_GetObjectItem(jsonRoot, "IP");
							if (!jsonIP)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strIP = std::string(jsonIP->valuestring);
								if (strIP.compare("0.0.0.0") == 0)
								{
									strIP = "127.0.0.1";
								}
							}

							cJSON *jsonPort = cJSON_GetObjectItem(jsonRoot, "port");
							if (!jsonPort)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								iPort = jsonPort->valueint;
							}

							cJSON *jsonUnix = cJSON_GetObjectItem(jsonRoot, "unix");
							if (!jsonUnix)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								strUnix = std::string(jsonUnix->valuestring);
							}

							cJSON *jsonLen = cJSON_GetObjectItem(jsonRoot, "len");
							if (!jsonLen)
							{
								cJSON_Delete(jsonRoot);
								return mxfalse;
							}
							else
							{
								iLen = jsonLen->valueint;
							}

							cJSON_Delete(jsonRoot);
						}

						std::shared_ptr<CIModule> imodule = NULL;
						if (!m_module->getConnectModule(strModuleGUID, imodule))
						{
							std::shared_ptr<CModule> newModule(
								new CModule(strModuleGUID, strModuleName));

							imodule = newModule;

							if (!m_module->connect(imodule))
							{
								return mxfalse;
							}

						}

						CModule *module = dynamic_cast<CModule *>(imodule.get());
						std::shared_ptr<CComRCFClientEndPoint> objClientEndPoint(
							new CComRCFClientEndPoint(m_module));

						T_COM_PROXY_CLIENT_CONFIG clientConfig;
						// memset(&clientConfig, 0, sizeof(T_COM_PROXY_CLIENT_CONFIG));

						clientConfig.m_iType = iType;
						clientConfig.m_strIP = strIP;
						clientConfig.m_iPort = iPort;
						clientConfig.m_strUnix = strUnix;
						clientConfig.m_iTCPMsgLen = iLen;
						clientConfig.m_iUDPMsgLen = iLen;
						clientConfig.m_iUNIXMsgLen = iLen;

						objClientEndPoint->init(clientConfig, E_CLIENT_FRAME);

						if (!module->regClient(strServerName, objClientEndPoint))
						{
							return mxfalse;
						}
						m_bInit = mxtrue;
						return mxtrue;
					}
				}
			}
		}

		return mxfalse;
	}

	mxbool CDevPlayVoice::sendframe(unsigned char * pcData, 
		int iLen, E_P_TYPE ePacketType,E_VOICE_PLAY_LEVEL eVoiceLevel)
	{
		if (!m_bInit || pcData == 0 || iLen < 0)
			return mxfalse;

		if (iLen > 0)
		{
			std::shared_ptr<CMediaFramePacket>  packet(
				new CMediaFramePacket());
			packet->setPacketType(ePacketType);
            packet->setReserve_1(E_SOURCE_FILE);
            packet->setReserve_2(eVoiceLevel);

			if (!packet->setFrameData(pcData, iLen, getCurrentTime(),0))
				return mxfalse;

			if (!pushFrameData(packet))
			{
				std::shared_ptr<CMediaFramePacket> lostPacket = NULL;
				popFrameData(lostPacket);
				m_iLostFrameNum++;

				logPrint(MX_LOG_DEBUG, "dev play lost frame num: %d",
					m_iLostFrameNum);

				if (!pushFrameData(packet))
				{
					logPrint(MX_LOG_DEBUG, "dev play insert frame error");
				}
			}
		}
		return mxtrue;
	}

    mxbool CDevPlayVoice::playWithFileId(std::string strFileId , int iLevel,int iPlayTime)
    {
        auto fileName = m_mapAudioRes.find(strFileId);
        if(fileName == m_mapAudioRes.end())
        {
            return mxfalse;
        }
        std::string strFilePath = "";
        strFilePath.append(m_strAudioResPath);
        // strFilePath.append(m_strLanguage);
        strFilePath.append("/");
        strFilePath.append(fileName->second);
        logPrint(MX_LOG_INFOR,"play file path %s ",strFilePath.c_str());
        return m_objPlayVoiceFileRead->playWithFile(strFilePath,iLevel,iPlayTime);
    }

    mxbool CDevPlayVoice::playWithFilePath(std::string strFilePath , int iLevel,int iPlayTime)
    {
        return m_objPlayVoiceFileRead->playWithFile(strFilePath,iLevel,iPlayTime);
    }

    mxbool CDevPlayVoice::stopWithFileId(std::string strFileId)
    {
        auto fileName = m_mapAudioRes.find(strFileId);
        if(fileName == m_mapAudioRes.end())
        {
            return mxfalse;
        }
        std::string strFilePath = "";
        strFilePath.append(m_strAudioResPath);
        // strFilePath.append(m_strLanguage);
        strFilePath.append("/");
        strFilePath.append(fileName->second);
        logPrint(MX_LOG_INFOR, "stopWithFileId : [%s] ", strFileId.c_str());
        return m_objPlayVoiceFileRead->stopWithFile(strFilePath);
    }

	mxbool CDevPlayVoice::pushFrameData(
		std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CDevPlayVoice::popFrameData(
		std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

	void CDevPlayVoice::run()
	{
		while (1)
		{
			if (m_strKey.length() == 0)
			{
#ifdef	WIN32
				Sleep(100);
#else
				usleep(100 * 1000);
#endif
				continue;
			}

			std::shared_ptr<CMediaFramePacket> packet = NULL;
			popFrameData(packet);

			if (!packet)
			{
// #ifdef	WIN32
// 				Sleep(500);
// #else
// 				usleep(500 * 1000);
// #endif
				continue;
			}

			logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
				m_strSpeakerManageChannelServer.c_str(), 
				packet->getFrameDataLen());

			m_module->output(
				m_strSpeakerManageModuleGUID,
				m_strSpeakerManageChannelServer, 
				packet->getFrameData(),
				packet->getFrameDataLen());
		}
	}

	int64_t CDevPlayVoice::getCurrentTime()
	{
#ifdef _WIN32
		struct timeb rawtime;
		ftime(&rawtime);
		return rawtime.time * 1000 + rawtime.millitm;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

}