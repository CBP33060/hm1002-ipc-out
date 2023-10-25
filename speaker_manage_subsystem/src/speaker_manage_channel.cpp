#include "speaker_manage_channel.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "cJSON.h"
#include "com_rpc_client_end_point.h"
#include "log_mx.h"
#include "inc_decode/speaker_decode_ctrl.h"

namespace maix {
	CSpeakerManageChannel::CSpeakerManageChannel(CModule * module,
		std::string strName,
		CSpeakerSourceInputServer * objSpeakerSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objSpeakerSourceInputServer(objSpeakerSourceInputServer)
	{
	}

	CSpeakerManageChannel::~CSpeakerManageChannel()
	{
	}

	mxbool CSpeakerManageChannel::init()
	{
		if (!m_module->getConfig("MODULE", "GUID", m_strModuleGUID))
			return mxfalse;

		if (!m_module->getConfig("MODULE", "NAME", m_strModuleName))
			return mxfalse;

		if (!m_module->getConfig("SPEAKER_SOURCE_REMOTE_EVENT", 
			"GUID", m_strSpeakerSourceGUID))
		{
			return mxfalse;
		}

		if (!m_module->getConfig("SPEAKER_SOURCE_REMOTE_EVENT",
			"SERVER", m_strSpeakerSourceServer))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CSpeakerManageChannel::unInit()
	{
		return mxbool();
	}

	mxbool CSpeakerManageChannel::open()
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "openSpeaker");

		cJSON_AddStringToObject(jsonParam, "moduleGUID", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "moduleName", m_strModuleGUID.c_str());
		cJSON_AddStringToObject(jsonParam, "serverName", m_strName.c_str());
		cJSON_AddStringToObject(jsonParam, "channelID", m_strName.c_str());

		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strOpen = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		if (m_module != NULL)
		{
			std::string strResult = m_module->output(m_strSpeakerSourceGUID,
				m_strSpeakerSourceServer,
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
									strIP = "127.0.0.1";
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

						std::unique_lock<std::mutex> lock(m_mutexChannelConfig);
						T_ServerConfig tServerConfig;
						tServerConfig.strGUID = strModuleGUID;
						tServerConfig.strServer = strServerName;
						m_listChannelConfig.push_back(tServerConfig);
						return mxtrue;
					}
				}
			}
		}
		
		return mxfalse;
	}

	mxbool CSpeakerManageChannel::close()
	{
		return mxbool();
	}

	mxbool CSpeakerManageChannel::config(std::string strConfig)
	{
		return mxbool();
	}

	void CSpeakerManageChannel::run()
	{
		while (1)
		{
			if (m_objSpeakerSourceInputServer)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;
				m_objSpeakerSourceInputServer->popFrameData(packet);

				if (!packet)
				{
#ifdef	WIN32
                     Sleep(30);
#else
                     usleep(30*1000);
#endif        
					continue;
				}

				T_MediaFramePacketHeader tMediaFramePacketHeader;
				int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
				if (packet->getFrameDataLen() > iPacketHeaderLen)
				{
					memcpy(&tMediaFramePacketHeader, packet->getFrameData(),
						iPacketHeaderLen);
				}
				else
				{
					continue;
				}

				// if (packet->getPacketType() == E_FP_AUDIO_PCM)
				// {
				// 	std::unique_lock<std::mutex> lock(m_mutexChannelConfig);
				// 	std::list<T_ServerConfig>::iterator iter;
				// 	for (iter = m_listChannelConfig.begin();
				// 	iter != m_listChannelConfig.end(); iter++)
				// 	{
				// 		m_module->output(iter->strGUID,
				// 			iter->strServer, packet->getFrameData(), 
				// 			packet->getFrameDataLen());

				// 		logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
				// 			m_strName.c_str(), packet->getFrameDataLen());
				// 	}
				// }

                unsigned char *pcData = packet->getFrameData() + 
                    iPacketHeaderLen;

                unsigned char pcDecoder[8192] = { 0 };
               
                int iDecoderLen = -1;
               
                iDecoderLen = CSpeakerDecodeCtrl::getInstance().decode(packet->getPacketType(),pcData,tMediaFramePacketHeader.nPacketSize,pcDecoder);
                
                // logPrint(MX_LOG_ERROR, "iDecoderLen = %d",iDecoderLen);
                if(iDecoderLen > 0)
                {
                    std::shared_ptr<CMediaFramePacket>  decoderPacket(
                        new CMediaFramePacket());
                    decoderPacket->setPacketType(E_P_AUDIO_PCM);
                    decoderPacket->setReserve_1(tMediaFramePacketHeader.iReserve_1);
                    decoderPacket->setReserve_2(tMediaFramePacketHeader.iReserve_2);

                    if (!decoderPacket->setFrameData(pcDecoder,
                        iDecoderLen, tMediaFramePacketHeader.lTimeStamp, tMediaFramePacketHeader.iFrameSeq))
                    {
                        logPrint(MX_LOG_ERROR, "set decoder data failed");
                        continue;
                    }

                    std::unique_lock<std::mutex> lock(m_mutexChannelConfig);
                    std::list<T_ServerConfig>::iterator iter;
                    for (iter = m_listChannelConfig.begin();
                    iter != m_listChannelConfig.end(); iter++)
                    {
                        m_module->output(iter->strGUID,
                            iter->strServer, decoderPacket->getFrameData(),
                            decoderPacket->getFrameDataLen());
                        logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
                            m_strName.c_str(), decoderPacket->getFrameDataLen());
                    }
                }

			}
			else
			{
#ifdef	WIN32
				Sleep(1000);
#else
				usleep(1000*1000);
#endif
			}
		}
	}

}