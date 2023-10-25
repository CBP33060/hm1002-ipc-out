#ifndef __MODULE_H__
#define __MODULE_H__
#include "type_def.h"
#include "i_module.h"
#include <string>
#include <map>
#include <mutex>
#include <functional>

#ifdef _INI_CONFIG
#include "ini_config.h"
#endif

namespace maix {
	class MAIX_EXPORT CModule : public CIModule
	{
	public:
		CModule(std::string strGUID, std::string strName);
		virtual ~CModule();

		virtual std::string getGUID();
		virtual std::string getName();

		virtual mxbool init();
		virtual mxbool unInit();
		 
		virtual mxbool connect(std::shared_ptr<CIModule> module);
		virtual mxbool disconnect(std::string strGUID);
		virtual mxbool getConnectModule(std::string strGUID,
			std::shared_ptr<CIModule> &module);

		virtual mxbool reg(std::shared_ptr<CIModule> module, 
			unsigned char* pcData, int iLen);
		virtual mxbool unReg(std::shared_ptr<CIModule> module);

		virtual std::string input(
			std::shared_ptr<CIModule> sender, 
			std::string strServerName,
			unsigned char* pcData, int iLen);

		virtual std::string output(
			std::string strGUID, std::string strServerName,
			unsigned char * pcData, int iLen);

		virtual mxbool regClient(std::string strServerName, 
			std::shared_ptr<CIComClientEndPoint> client);
		virtual mxbool unRegClient(std::string strServerName);
		virtual mxbool isClientExist(std::string strServerName);
		virtual int getClientType(std::string strServerName);

		virtual mxbool regServer(std::string strServerName, 
			std::shared_ptr<CIComServerEndPoint> server);
		virtual mxbool unRegServer(std::string strServerName);

		virtual std::shared_ptr<CIComServerEndPoint> getServer(
			std::string strServerName);

		mxbool loadConfig(std::string strPath);

		template<class T>
		mxbool getConfig(std::string strSection,
			std::string strKey, T& value);

		template<class T>
		mxbool setConfig(std::string strSection,
			std::string strKey, T& value);

		mxbool saveConfig();
	private:
		std::string m_strGUID;
		std::string	m_strName;
	
		std::mutex m_mutexConnects;
		std::map<std::string, std::shared_ptr<CIModule>> m_mapConnects;

		std::mutex m_mutexClients;
		std::map<std::string, 
			std::shared_ptr<CIComClientEndPoint>> m_mapClients;

		std::mutex m_mutexServers;
		std::map<std::string, 
			std::shared_ptr<CIComServerEndPoint>> m_mapServers;

		std::string m_strConfigPath;
		std::mutex m_mutexConfig;
#ifdef _CRYPTO_ENABLE
		unsigned char m_key[32];
#endif
#ifdef _INI_CONFIG
		INI::CINIFile m_configs;
#endif
	};
	template<class T>
	inline mxbool CModule::getConfig(std::string strSection,
		std::string strKey, T& value)
	{
		std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
		try
		{
			value = m_configs[strSection][strKey].as<T>();
		}
		catch (std::invalid_argument &ia)
		{
			//std::cerr << " Invalid_argument " << ia.what() << std::endl;
			return mxfalse;
		}
#endif
		return mxtrue;
	}

	template<class T>
	inline mxbool CModule::setConfig(std::string strSection,
		std::string strKey, T& value)
	{
		std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
		try
		{
			m_configs[strSection][strKey] = value;
		}
		catch (std::invalid_argument &ia)
		{
			return mxfalse;
		}
#endif
		return mxtrue;
	}
}
#endif //__MODULE_H__
