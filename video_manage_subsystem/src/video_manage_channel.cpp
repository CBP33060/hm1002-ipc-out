#include "video_manage_channel.h"
#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <iostream>
#include "log_mx.h"
#include "crypt_api_mx.h"

#define ENCRYPT_DATA_LEN (500*1024)

namespace maix {
	CVideoManageChannel::CVideoManageChannel(CModule * module,
		std::string strName,
		CVideoSourceInputServer * objVideoSourceInputServer)
		: m_module(module)
		, m_strName(strName)
		, m_objVideoSourceInputServer(objVideoSourceInputServer)
		, m_pcEncryptData(NULL)
		, m_iEncryptDataLen(0)
	{
		m_jpeg = mxtrue;
	}

	CVideoManageChannel::~CVideoManageChannel()
	{
	}

	mxbool CVideoManageChannel::init()
	{
		if (!m_pcEncryptData)
		{
			m_pcEncryptData = (unsigned char*)malloc(ENCRYPT_DATA_LEN);
			if (!m_pcEncryptData)
				return mxfalse;
		}

		m_iEncryptDataLen = ENCRYPT_DATA_LEN;
		return mxtrue;
	}

	mxbool CVideoManageChannel::unInit()
	{
		return mxbool();
	}

	mxbool CVideoManageChannel::open(std::string strGUID,
		std::string strServerName, std::string strKey)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		m_mapClient[strGUID][strServerName] = strKey;

		return mxtrue;
	}

	mxbool CVideoManageChannel::close(std::string strGUID,
		std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) != 0)
		{
			if(m_mapClient[strGUID].count(strServerName) != 0)
				m_mapClient[strGUID].erase(strServerName);

			if (m_mapClient[strGUID].size() == 0)
				m_mapClient.erase(strGUID);
		}
			
		return mxtrue;
	}

	mxbool CVideoManageChannel::config(std::string strConfig)
	{
		return mxbool();
	}

	mxbool CVideoManageChannel::reset()
	{
		return mxbool();
	}

	mxbool CVideoManageChannel::getJpegFrameData(std::shared_ptr<CMediaFramePacket> &packet)
	{
		FILE * fpJpegFile = fopen("/tmp/cover.jpeg", "rb");
		if (fpJpegFile == NULL) {
			logPrint(MX_LOG_ERROR,"fopen /tmp/cover.jpeg error!");
			return mxfalse;
		}

		fseek(fpJpegFile,0,SEEK_END);
		long filesize = ftell(fpJpegFile);
		fseek(fpJpegFile,0,SEEK_SET);

		char * framedata = (char *)malloc(filesize);
		if(framedata == NULL)
		{
			logPrint(MX_LOG_ERROR,"malloc jpeg filesize %d error!", filesize);
			fclose(fpJpegFile);
			return mxfalse;
		}
		memset(framedata,0,filesize);

		char buf[1024] = { 0 };
		int size = 0, dataLen = 0;
		while(1) {
			memset(buf,0,sizeof(buf));
			size = fread(buf, 1, sizeof(buf), fpJpegFile);
			if (size > 0) {
				memcpy(framedata + dataLen, buf, size);
				dataLen += size;
			} else {
				break;
			}
		}
		fclose(fpJpegFile);

		packet = std::shared_ptr<CMediaFramePacket>(new CMediaFramePacket());
		packet->setPacketType(E_P_VIDEO_JPEG);
		if (!packet->setFrameData((unsigned char*)framedata, filesize, getCurrentTime()))
		{
			logPrint(MX_LOG_ERROR,"packet setFrameData jpeg error!");
			free(framedata);
			return mxfalse;
		}

		free(framedata);
		return mxtrue;
	}

	int64_t CVideoManageChannel::getCurrentTime()
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

    // struct timeval prevEnd, currEnd;
    // int count_test = 0;

	void CVideoManageChannel::run()
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

			if (m_objVideoSourceInputServer)
			{
				std::shared_ptr<CMediaFramePacket> packet = NULL;

				if(m_jpeg)
				{
					getJpegFrameData(packet);
					m_jpeg = mxfalse;
				}
				else
					m_objVideoSourceInputServer->popFrameData(packet);

				if (!packet)
				{
// #ifdef	WIN32
// 					Sleep(500);
// #else
// 					usleep(500 * 1000);
// #endif
					continue;
				}
				
				std::unique_lock<std::mutex> lock(m_mutexClient);
				std::map<std::string, std::map<std::string, std::string>>::iterator  iter;
				for (iter = m_mapClient.begin(); iter != m_mapClient.end(); )
				{
					std::string strGUID = iter->first;
					std::map<std::string, std::string> mapServer = iter->second;

					std::map<std::string, std::string>::iterator iterServer;
					for (iterServer = mapServer.begin();
					iterServer != mapServer.end(); iterServer++)
					{
						std::string strServer = iterServer->first;
						std::string strKey = iterServer->second;
						std::string strResult;
						int clientType = 0;

						std::shared_ptr<CIModule> module = NULL;

						if (!m_module->getConnectModule(strGUID, module))
							continue;

						if(module != NULL)
						{
							clientType = module->getClientType(strServer);
						}else{
							continue;
						}

						if (clientType ==1 || clientType ==2 || clientType == 16)
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

                            // gettimeofday(&prevEnd, NULL);
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
                            // gettimeofday(&currEnd, NULL);
                            // long int timeDiff = (currEnd.tv_sec - prevEnd.tv_sec) * 1000 + (currEnd.tv_usec - prevEnd.tv_usec) / 1000;
                            // if (timeDiff > 200){
							// 		count_test++;
							// 		printf("Time difference between two send calls1: %ld ms--count:[%d]\n", timeDiff, count_test);
							// 	}
                            // printf("Time difference between two send calls1: %ld ms\n", timeDiff);                            
							
						}
						else if(clientType == -1)
						{
							m_mapClient[strGUID].erase(strServer);
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
