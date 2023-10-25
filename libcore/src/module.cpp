#include "module.h"
#include "iLockFileStream.h"
#include "oLockFileStream.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#ifdef _WIN32
#include "windows.h"
#endif
#ifdef _CRYPTO_ENABLE
#include "crypt_api_mx.h"
#endif
#include "common.h"
#include "log_mx.h"

namespace maix {
	CModule::CModule(std::string strGUID, std::string strName)
	{
		m_strGUID = strGUID;
		m_strName = strName;
#ifdef _CRYPTO_ENABLE
		memset(m_key, 0, sizeof(m_key));
#endif
	}

	CModule::~CModule()
	{
	}

	std::string CModule::getGUID()
	{
		return m_strGUID;
	}

	std::string CModule::getName()
	{
		return m_strName;
	}

	mxbool CModule::init()
	{
		return mxfalse;
	}

	mxbool CModule::unInit()
	{
		return mxfalse;
	}

	mxbool CModule::loadConfig(std::string strPath)
	{
		std::unique_lock<std::mutex> lock(m_mutexConfig);
		m_strConfigPath = strPath;
		std::string strTmpPath = strPath;
#ifdef _CRYPTO_ENABLE
		std::string strKey = getPSK();
		std::string strDID = getDID();
		std::string strMac = "maix";

		int ret = crypto_hmac_sha256(strKey, strDID, strMac, m_key);
		if (ret != 0)
		{
			std::cout << "[CModule]: load config crypto failed" << std::endl;
			return mxfalse;
		}
			
		std::string strCryptoPath(m_strConfigPath);
		strCryptoPath.append(std::string(".enc"));

		iLockFileStream fenc(strCryptoPath.c_str());
		if (fenc.good() == mxtrue)
		{
			fenc.seekg(0, fenc.end);
			int size = fenc.tellg();
			if (size == 0)
			{
				std::cout << "[CModule]: load config enc size is 0" << std::endl;
				return mxfalse;
			}

			fenc.seekg(0, fenc.beg);
			unsigned char *pcEncFileBuffer = 
				(unsigned char*)malloc(size + 1);
			if (pcEncFileBuffer == NULL)
			{
				std::cout << "[CModule]: load config pcEncFileBuffer NULL" << std::endl;
				return mxfalse;
			}

			unsigned char *pcFileBuffer = 
				(unsigned char*)malloc(size + 1);
			if (pcFileBuffer == NULL)
			{
				std::cout << "[CModule]: load config pcFileBuffer NULL" << std::endl;
				free(pcEncFileBuffer);
				return mxfalse;
			}
				
			memset(pcEncFileBuffer, 0, size + 1);
			memset(pcFileBuffer, 0, size + 1);

			fenc.read((char*)pcEncFileBuffer, size);
			
			int iFileLen = 0;
			ret = crypto_aes256_decrypt_base64(m_key,
				pcEncFileBuffer, size, pcFileBuffer, &iFileLen);
			if (ret != 0)
			{
				std::cout << "[CModule]: load config decrypt failed" << std::endl;
				free(pcEncFileBuffer);
				free(pcFileBuffer);
				return mxfalse;
			}

#ifdef _WIN32
			strTmpPath.append(std::string(".read.tmp"));
#else
			strTmpPath = std::string("/tmp/");
			std::string strGUID;
			getGUIDData(strGUID);
			strTmpPath.append(strGUID);
#endif
			oLockFileStream fout(strTmpPath);
			fout.write(pcFileBuffer, iFileLen);
			fout.flush();
			fout.close();
			fenc.close();
			free(pcEncFileBuffer);
			free(pcFileBuffer);

		} 
#else
		strTmpPath = strPath;
#endif

#ifdef _INI_CONFIG
		try
		{
			iLockFileStream f(strTmpPath);
			//std::cout << "[CModule]: load config: " <<strTmpPath<< std::endl;
			if (f.good() == mxtrue)
			{
				m_configs.load(strTmpPath);
#ifdef _CRYPTO_ENABLE
				if (fenc.good() != mxtrue)
				{
					lock.unlock();
					if (!saveConfig())
					{
						logPrint(MX_LOG_ERROR,
							"save mod config failed: %s", strPath.c_str());
					}

				}
				else
				{
					remove(strTmpPath.c_str());
				}
				
				f.close();
				
#endif
				
				return mxtrue;
			}
			else
			{
				std::cout << "[CModule]: last load config failed: " <<strTmpPath << std::endl;
			}
		}
		catch (std::exception & e)
		{
			/// 重新生成enc文件
			if (access(strCryptoPath.c_str(), F_OK) == 0)
			{
				logPrint(MX_LOG_ERROR, "[CModule]remove file:%s", strCryptoPath.c_str());
				remove(strCryptoPath.c_str());
			}

			logPrint(MX_LOG_ERROR, "loadConfig mod  failed: %s, remove file", strCryptoPath.c_str());
			std::cout << "[CModule]: load config failed: " <<strPath<< std::endl;
		}
#endif

#ifdef _CRYPTO_ENABLE
		remove(strTmpPath.c_str());
#endif
		return mxfalse;
	}

	mxbool CModule::connect(std::shared_ptr<CIModule> module)
	{
		std::unique_lock<std::mutex> lock(m_mutexConnects);
		if(m_mapConnects.count(module->getGUID()) == 0)
			m_mapConnects[module->getGUID()] = module;

		return mxtrue;
	}

	mxbool CModule::disconnect(std::string strGUID)
	{
		std::unique_lock<std::mutex> lock(m_mutexConnects);
		if (m_mapConnects.count(strGUID))
			m_mapConnects.erase(strGUID);

		return mxtrue;
	}

	mxbool CModule::getConnectModule(std::string strGUID,
		std::shared_ptr<CIModule> &module)
	{
		std::unique_lock<std::mutex> lock(m_mutexConnects);
		if (m_mapConnects.count(strGUID) != 0)
		{
			module = m_mapConnects[strGUID];
			return mxtrue;
		}

		return mxfalse;
	}

	mxbool CModule::reg(std::shared_ptr<CIModule>, 
		unsigned char * pcData, int iLen)
	{
		return mxtrue;
	}

	mxbool CModule::unReg(std::shared_ptr<CIModule> module)
	{
		return mxtrue;
	}

	std::string CModule::input(std::shared_ptr<CIModule> sender, 
		std::string strServerName, unsigned char * pcData, int iLen)
	{
		std::unique_lock<std::mutex> lock(m_mutexClients);
		std::string result;
		std::shared_ptr<CIComClientEndPoint> client = m_mapClients[strServerName];
		if (client.use_count() != 0)
		{
			result = client->output(strServerName, pcData, iLen);
		}

		return result;
	}

	std::string CModule::output(std::string strGUID,
		std::string strServerName, unsigned char * pcData, int iLen)
	{
		std::string result;
		std::shared_ptr<CIModule> module = m_mapConnects[strGUID];
		if (module.use_count() != 0)
		{
			result = module->input(module, strServerName, pcData, iLen);
		}
		return result;
	}

	mxbool CModule::regClient(std::string strServerName, 
		std::shared_ptr<CIComClientEndPoint> client)
	{
		std::unique_lock<std::mutex> lock(m_mutexClients);
		if(m_mapClients.count(strServerName) == 0)
			m_mapClients[strServerName] = client;
		else
		{
			m_mapClients.erase(strServerName);
			m_mapClients[strServerName] = client;
		}

		return mxtrue;
	}

	mxbool CModule::unRegClient(std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClients);
		m_mapClients.erase(strServerName);
		return mxtrue;
	}

	mxbool CModule::isClientExist(std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClients);
		if (m_mapClients.count(strServerName) != 0)
			return mxtrue;
		return mxfalse;
	}

	int CModule::getClientType(std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexClients);
		if (m_mapClients.count(strServerName) != 0)
		{
			if(m_mapClients[strServerName] == NULL)
			{
				return -1;
			}
			return m_mapClients[strServerName]->getClientType();
		}
		return -1;
	}

	mxbool CModule::regServer(std::string strServerName, 
		std::shared_ptr<CIComServerEndPoint> server)
	{
		std::unique_lock<std::mutex> lock(m_mutexServers);
		m_mapServers[strServerName] = server;
		return mxtrue;
	}

	mxbool CModule::unRegServer(std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexServers);
		m_mapServers.erase(strServerName);
		return mxtrue;
	}

	std::shared_ptr<CIComServerEndPoint> CModule::getServer(
		std::string strServerName)
	{
		std::unique_lock<std::mutex> lock(m_mutexServers);
		return m_mapServers[strServerName];
	}

	mxbool CModule::saveConfig()
	{
		std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _WIN32
		std::string strTmpPath(m_strConfigPath);
		strTmpPath.append(std::string(".write.tmp"));
#else
		std::string strTmpPath("/tmp/");
		std::string strGUID;
		getGUIDData(strGUID);
		strTmpPath.append(strGUID);
#endif

#ifdef _INI_CONFIG
#ifdef _CRYPTO_ENABLE
		m_configs.save(strTmpPath);
#else
		m_configs.save(m_strConfigPath);
#endif
#endif
#ifdef _CRYPTO_ENABLE
		iLockFileStream fin(strTmpPath);
		if (fin.good() == mxtrue)
		{
			fin.seekg(0, fin.end);
			int size = fin.tellg();
			if (size == 0)
				return mxfalse;

			fin.seekg(0, fin.beg);
			unsigned char *pcEncFileBuffer =
				(unsigned char*)malloc(size + 2048);
			if (pcEncFileBuffer == NULL)
				return mxfalse;

			unsigned char *pcFileBuffer =
				(unsigned char*)malloc(size + 1);
			if (pcFileBuffer == NULL)
			{
				free(pcEncFileBuffer);
				return mxfalse;
			}

			memset(pcEncFileBuffer, 0, size + 2048);
			memset(pcFileBuffer, 0, size + 1);

			fin.read((char*)pcFileBuffer, size);

			size_t iEncFileLen = 0;
			int ret = crypto_aes256_encrypt_base64(m_key,
				pcFileBuffer, size, pcEncFileBuffer,
				size + 2048,
				&iEncFileLen);
			logPrint(MX_LOG_INFOR, "[CMODULE]saveConfig path:%s input len:%d, ret:%d, output len:%d\n", m_strConfigPath.c_str(), size, ret, iEncFileLen);
			if (ret != 0)
			{
				free(pcEncFileBuffer);
				free(pcFileBuffer);
				return mxfalse;
			}
			std::string strPath = m_strConfigPath;
			strPath.append(std::string(".enc"));
			oLockFileStream fout(strPath);
			fout.write(pcEncFileBuffer, iEncFileLen);
			fout.close();

			free(pcEncFileBuffer);
			free(pcFileBuffer);
		}

		fin.close();
		remove(strTmpPath.c_str());
#endif
		return mxtrue;
	}
}
