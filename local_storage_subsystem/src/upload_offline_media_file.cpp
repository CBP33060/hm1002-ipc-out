#include "upload_offline_media_file.h"
#include "log_mx.h"
#include "crypt_api_mx.h"
#include "media_frame_packet.h"
#ifdef _WIN32
#include <windows.h>
#else
#endif
#define ENCRYPT_DATA_LEN (50*1024)

namespace maix {
	CUploadOfflineMediaFile::CUploadOfflineMediaFile(CModule * module)
		: m_module(module)
		, m_pcEncryptData(NULL)
		, m_iEncryptDataLen(0)
	{
		m_pMP4FileHandle = MP4_INVALID_FILE_HANDLE;
	}

	CUploadOfflineMediaFile::~CUploadOfflineMediaFile()
	{
	}

	mxbool CUploadOfflineMediaFile::init()
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

	mxbool CUploadOfflineMediaFile::unInit()
	{
		return mxbool();
	}

	mxbool CUploadOfflineMediaFile::open(
		std::string strGUID, std::string strServerName, 
		std::string strKey, std::string strFileName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		m_mapClient[strGUID][strServerName] = strKey;

		if(m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE)
		{
			//¸ù¾ÝÎÄ¼þÃû£¬´ò¿ªÖ¸¶¨ÎÄ¼þ
		}

		return mxtrue;
	}

	mxbool CUploadOfflineMediaFile::close(
		std::string strGUID, std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClient);
		if (m_mapClient.count(strGUID) != 0)
		{
			if (m_mapClient[strGUID].count(strServerName) != 0)
				m_mapClient[strGUID].erase(strServerName);

			if (m_mapClient[strGUID].size() == 0)
				m_mapClient.erase(strGUID);
		}

		if (m_pMP4FileHandle != MP4_INVALID_FILE_HANDLE)
		{
			//¹Ø±ÕÎÄ¼þ
		}

		return mxtrue;
	}

	void CUploadOfflineMediaFile::run()
	{
		while (1)
		{
			if (m_mapClient.size() == 0 || 
				m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE)
			{
#ifdef	WIN32
				Sleep(500);
#else 
				usleep(500 * 1000);
#endif
				continue;
			}

			std::shared_ptr<CMediaFramePacket> packet;
			//¶ÁÈ¡Ö¡Êý¾Ý

			if (!packet)
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(500 * 1000);
#endif
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

							logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
								m_strName.c_str(), packet->getFrameDataLen());
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
						logPrint(MX_LOG_DEBUG, "channel: %s send frame len: %d",
							m_strName.c_str(), packet->getFrameDataLen());
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
}
