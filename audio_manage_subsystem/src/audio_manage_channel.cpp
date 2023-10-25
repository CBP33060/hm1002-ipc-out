#include "audio_manage_channel.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log_mx.h"
#include <iostream>
#include "crypt_api_mx.h"

#define ENCRYPT_DATA_LEN (5*1024)
namespace maix {
	CAudioManageChannel::CAudioManageChannel(CModule * module,
		std::string strName,
		CAudioSourceInputServer * objAudioSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objAudioSourceInputServer(objAudioSourceInputServer)
		, m_pcEncryptData(NULL)
		, m_iEncryptDataLen(0)
	{
	}

	CAudioManageChannel::~CAudioManageChannel()
	{

	}

	mxbool CAudioManageChannel::init()
	{
		std::shared_ptr <CAudioCodecEntity> pObjAudioCodec(new CAudioCodecEntity());
		m_objAudioCodec = pObjAudioCodec;
		if (!m_objAudioCodec->initOpusEncode())
		{
			return mxfalse;
		}
		
		if (!m_pcEncryptData)
		{
			m_pcEncryptData = (unsigned char*)malloc(ENCRYPT_DATA_LEN);
			if (!m_pcEncryptData)
				return mxfalse;
		}

		m_iEncryptDataLen = ENCRYPT_DATA_LEN;
		return mxtrue;
	}

	mxbool CAudioManageChannel::unInit()
	{
		if (!m_objAudioCodec->unInitOpusEncode())
		{
			return mxfalse;
		}
		return mxtrue;
	}

	mxbool CAudioManageChannel::open(std::string strGUID,
		std::string strServerName,
		std::string strKey)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		m_mapClient[strGUID][strServerName] = strKey;

		return mxtrue;
	}

	mxbool CAudioManageChannel::close(std::string strGUID,
		std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) != 0)
		{
			if (m_mapClient[strGUID].count(strServerName) != 0)
				m_mapClient[strGUID].erase(strServerName);

			if (m_mapClient[strGUID].size() == 0)
				m_mapClient.erase(strGUID);
		}
		return mxtrue;
	}

	mxbool CAudioManageChannel::config(std::string strConfig)
	{
		return mxbool();
	}

	mxbool CAudioManageChannel::reset()
	{
		return mxbool();
	}

	void CAudioManageChannel::run()
	{
		while (1)
		{
			if (m_mapClient.size() == 0)
			{
#ifdef	WIN32
				Sleep(500);
#else 
				usleep(500 * 1000);
#endif
				continue;
			}

			if (m_objAudioSourceInputServer)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;
				m_objAudioSourceInputServer->popFrameData(packet);

				if (!packet)
				{
#ifdef	WIN32
 					Sleep(10);
#else
 					usleep(10*1000);
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

				if (packet->getPacketType() == E_P_AUDIO_PCM)
				{
					int iPcmDataLen = packet->getFrameDataLen() - iPacketHeaderLen;
					unsigned char *pcData = packet->getFrameData() + iPacketHeaderLen;
					unsigned char *pcEncoder = 
						(unsigned char*)malloc(iPcmDataLen);
					memset(pcEncoder, 0, iPcmDataLen);


					opus_int16 pcmInt16Data[iPcmDataLen / 2] = {0};
					for (int i = 0; i < iPcmDataLen / 2; i++) {
						pcmInt16Data[i] = pcData[2 * i + 1] << 8 | pcData[2 * i];
					}

					int iEncoderLen = 0;
					if (!pcEncoder)
					{
						logPrint(MX_LOG_ERROR, "malloc encoder failed");
						continue;
					}
					else
					{
						iEncoderLen = m_objAudioCodec->opusEncode((int16_t *)pcmInt16Data, iPcmDataLen / 2, pcEncoder, iPcmDataLen);
						if (iEncoderLen < 0)
						{
							logPrint(MX_LOG_ERROR, "opus encoder failed, ret:%d", iEncoderLen);
							continue;
						}
					}
					std::shared_ptr<CMediaFramePacket>  encoderPacket(
						new CMediaFramePacket());
					encoderPacket->setPacketType(E_P_AUDIO_OPUS);

					if (!encoderPacket->setFrameData(pcEncoder,
						iEncoderLen, tMediaFramePacketHeader.lTimeStamp, tMediaFramePacketHeader.iFrameSeq))
					{
						logPrint(MX_LOG_ERROR, "set encoder data failed");
						if (pcEncoder)
						{
							free(pcEncoder);
							pcEncoder = NULL;
						}
						continue;
					}

					if (pcEncoder)
					{
						free(pcEncoder);
						pcEncoder = NULL;
					}

					std::unique_lock<std::mutex> lock(m_mutexClient);
					std::map<std::string, std::map<std::string, std::string>>::iterator  iter;
					for (iter = m_mapClient.begin(); iter != m_mapClient.end();)
					{
						std::string strGUID = iter->first;
						std::map<std::string, std::string> mapServer = iter->second;

						std::map<std::string, std::string>::iterator iterServer;
						for (iterServer = mapServer.begin();
						iterServer != mapServer.end(); iterServer++)
						{
							logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
								m_strName.c_str(), encoderPacket->getFrameDataLen());

							std::string strServer = iterServer->first;
							std::string strKey = iterServer->second;
							std::string strResult;
							int clientType = 0;

							std::shared_ptr<CIModule> module;

							if (!m_module->getConnectModule(strGUID, module))
								continue;

							if(module != NULL)
							{
								clientType = module->getClientType(strServer);
							}else{
								continue;
							}

							if ((clientType ==1 || clientType ==2) && (strKey.length() > 15))
							{
								logPrint(MX_LOG_DEBUG, "crypt packet");
								int iEncryptDataLen = 0;
								if (encoderPacket->getFrameDataLen() > ENCRYPT_DATA_LEN)
								{
									free(m_pcEncryptData);
									m_pcEncryptData = NULL;
									m_pcEncryptData = (unsigned char*)malloc(
										encoderPacket->getFrameDataLen() + 48);
									if (m_pcEncryptData)
									{
										m_iEncryptDataLen = encoderPacket->getFrameDataLen() + 48;
									}
									else
									{
										logPrint(MX_LOG_ERROR, "malloc encrypt data buffer failed");
										continue;
									}

								}
								else
								{
									if (!m_pcEncryptData)
									{
										m_pcEncryptData = (unsigned char*)malloc(ENCRYPT_DATA_LEN);
										if (!m_pcEncryptData)
											continue;

										m_iEncryptDataLen = ENCRYPT_DATA_LEN;
									}

								}


								if (crypto_aes128_encrypt((unsigned char*)strKey.c_str(),
									encoderPacket->getFrameData(),
									encoderPacket->getFrameDataLen(),
									m_pcEncryptData, &iEncryptDataLen) == 0)
								{
									strResult = m_module->output(
										strGUID, strServer, m_pcEncryptData,
										iEncryptDataLen);

									// logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
									// 	m_strName.c_str(), encoderPacket->getFrameDataLen());
								}
								else
								{
									logPrint(MX_LOG_ERROR, "encrypt data  failed");
									continue;
								}
							}
							else if(clientType == -1)
							{
								m_mapClient[strGUID].erase(strServer);
							}
							else
							{
								strResult = m_module->output(
									strGUID, strServer, encoderPacket->getFrameData(),
									encoderPacket->getFrameDataLen());
								// logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
								// 	m_strName.c_str(), encoderPacket->getFrameDataLen());
							}
							

							if (strResult.compare("disconnect") == 0)
							{
								m_mapClient[strGUID].erase(strServer);
							}
						}

						if (m_mapClient[strGUID].size() == 0)
						{
							m_mapClient.erase(iter++);
							continue;
						}
						iter++;
					}
				}
				else
				{
					logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
						m_strName.c_str(), packet->getFrameDataLen());

					std::unique_lock<std::mutex> lock(m_mutexClient);
					std::map<std::string, std::map<std::string, std::string>>::iterator  iter;
					for (iter = m_mapClient.begin(); iter != m_mapClient.end();)
					{
						std::string strGUID = iter->first;
						std::map<std::string, std::string> mapServer = iter->second;

						std::map<std::string, std::string>::iterator iterServer;
						for (iterServer = mapServer.begin();
						iterServer != mapServer.end(); iterServer++)
						{
							logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
								m_strName.c_str(), packet->getFrameDataLen());

							std::string strServer = iterServer->first;
							std::string strKey = iterServer->second;
							std::string strResult;

							std::shared_ptr<CIModule> module;

							if (!m_module->getConnectModule(strGUID, module))
								continue;

							if (module->getClientType(strServer) == 1 ||
								module->getClientType(strServer) == 2)
							{
								logPrint(MX_LOG_DEBUG, "crypt packet");
								int iEncryptDataLen = 0;
								if (packet->getFrameDataLen() > ENCRYPT_DATA_LEN)
								{
									free(m_pcEncryptData);
									m_pcEncryptData = NULL;
									m_pcEncryptData = (unsigned char*)malloc(
										packet->getFrameDataLen() + 48);
									if (m_pcEncryptData)
									{
										m_iEncryptDataLen = packet->getFrameDataLen() + 48;
									}
									else
									{
										logPrint(MX_LOG_ERROR, "malloc encrypt data buffer failed");
										continue;
									}

								}
								else
								{
									if (!m_pcEncryptData)
									{
										m_pcEncryptData = (unsigned char*)malloc(ENCRYPT_DATA_LEN);
										if (!m_pcEncryptData)
											continue;

										m_iEncryptDataLen = ENCRYPT_DATA_LEN;
									}

								}

								if (crypto_aes128_encrypt((unsigned char*)strKey.c_str(),
									packet->getFrameData(),
									packet->getFrameDataLen(),
									m_pcEncryptData, &iEncryptDataLen) == 0)
								{
									strResult = m_module->output(
										strGUID, strServer, m_pcEncryptData,
										iEncryptDataLen);

									// logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
									// 	m_strName.c_str(), packet->getFrameDataLen());
								}
								else
								{
									logPrint(MX_LOG_ERROR, "encrypt data  failed");
									continue;
								}
							}
							else
							{
								strResult = m_module->output(
									strGUID, strServer, packet->getFrameData(),
									packet->getFrameDataLen());
								// logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
								// 	m_strName.c_str(), packet->getFrameDataLen());
							}


							if (strResult.compare("disconnect") == 0)
							{
								m_mapClient[strGUID].erase(strServer);
							}
						}

						if (m_mapClient[strGUID].size() == 0)
						{
							m_mapClient.erase(iter++);
							continue;
						}
						iter++;
					}
				}
			}
			else
			{
#ifdef _WIN32
				Sleep(1000);
#else
				usleep(1000 * 1000);
#endif
			}
		}
	}

}
