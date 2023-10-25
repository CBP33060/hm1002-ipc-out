#include "app.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef _CRYPTO_ENABLE
#include "crypt_api_mx.h"
#endif
#include "common.h"
#include "log_mx.h"
#include "iLockFileStream.h"
#include "oLockFileStream.h"

namespace maix {
	CApp::CApp(std::string strName)
		: m_strName(strName)
	{

	}

	CApp::~CApp()
	{
	}

	mxbool CApp::init()
	{
		return mxbool();
	}

	mxbool CApp::unInit()
	{
		return mxbool();
	}

	E_APP_STATE CApp::getState()
	{
		return m_state;
	}

	void CApp::setState(E_APP_STATE state)
	{
		m_state = state;
	}

	void CApp::run()
	{
		while (getState() == E_APP_START)
		{
#ifdef _WIN32
			Sleep(15000);
#else
			sleep(15);
#endif
		}
	}

	mxbool CApp::start()
	{
		run();
		return mxtrue;
	}

	mxbool CApp::pause(bool isPause)
	{
		return mxfalse;
	}

	mxbool CApp::stop()
	{
		return false;
	}

	mxbool CApp::addModule(std::string strModuleName, 
		std::shared_ptr<CModule> module)
	{
		std::unique_lock<std::mutex> lock(m_mutexModules);
		m_mapModules[strModuleName] = module;
		return mxtrue;
	}

	std::shared_ptr<CModule> CApp::getModule(std::string strModuleName)
	{
		std::unique_lock<std::mutex> lock(m_mutexModules);
		if (m_mapModules.count(strModuleName) == 1)
			return m_mapModules[strModuleName];
		return std::shared_ptr<CModule>();
	}

	mxbool CApp::delModule(std::string strModuleName)
	{
		std::unique_lock<std::mutex> lock(m_mutexModules);
		m_mapModules.erase(strModuleName);
		return mxtrue;
	}

	mxbool CApp::connect(std::string strSender, std::string strReceiver)
	{
		std::unique_lock<std::mutex> lock(m_mutexModules);
		if (m_mapModules.count(strSender) == 1 &&
			m_mapModules.count(strReceiver) == 1)
		{
			m_mapModules[strSender]->connect(m_mapModules[strReceiver]);
			return mxtrue;
		}
		else
		{
			return mxfalse;
		}
	}

	mxbool CApp::disconnect(std::string strSender, std::string strReceiver)
	{
		std::unique_lock<std::mutex> lock(m_mutexModules);
		m_mapModules[strSender]->disconnect(
			m_mapModules[strReceiver]->getGUID());
		return mxtrue;
	}

	mxbool CApp::loadConfig(std::string strPath)
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
			return mxfalse;


		std::string strCryptoPath(m_strConfigPath);
		strCryptoPath.append(std::string(".enc"));
		iLockFileStream fenc(strCryptoPath.c_str());
		if (fenc.good() == mxtrue)
		{
			fenc.seekg(0, fenc.end);
			int size = fenc.tellg();
			if (size == 0)
				return mxfalse;
	

			fenc.seekg(0, fenc.beg);
			unsigned char *pcEncFileBuffer =
				(unsigned char*)malloc(size + 1);
			if (pcEncFileBuffer == NULL)
				return mxfalse;

			unsigned char *pcFileBuffer =
				(unsigned char*)malloc(size + 1);
			if (pcFileBuffer == NULL)
			{
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
			oLockFileStream fout(strTmpPath.c_str());
			fout.write(pcFileBuffer, iFileLen);
			fout.flush();
			fout.close();

			free(pcEncFileBuffer);
			free(pcFileBuffer);

		}
#else
		strTmpPath = strPath;
#endif
#ifdef _INI_CONFIG
		try
		{
			iLockFileStream f(strTmpPath.c_str());
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
							"save app config failed: %s", strPath.c_str());
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
		}
		catch (std::exception & e)
		{
			/// 重新生成enc文件
			if (access(strCryptoPath.c_str(), F_OK) == 0)
			{
				logPrint(MX_LOG_ERROR, "[CAPP]remove file:%s", strCryptoPath.c_str());
				remove(strCryptoPath.c_str());
			}
			logPrint(MX_LOG_ERROR, "loadConfig app  failed: %s", strPath.c_str());
		}
#endif

#ifdef _CRYPTO_ENABLE
		remove(strTmpPath.c_str());
#endif
		return mxfalse;
	}

	mxbool CApp::saveConfig()
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
		iLockFileStream fin(strTmpPath.c_str());
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
			logPrint(MX_LOG_INFOR, "[CAPP]saveConfig path:%s input len:%d, ret:%d, output len:%d\n", m_strConfigPath.c_str(), size, ret, iEncFileLen);
			if (ret != 0)
			{
				free(pcEncFileBuffer);
				free(pcFileBuffer);
				return mxfalse;
			}
			std::string strPath = m_strConfigPath;
			strPath.append(std::string(".enc"));
			oLockFileStream fout(strPath.c_str());
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