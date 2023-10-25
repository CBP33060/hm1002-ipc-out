#ifndef __APP_H__
#define __APP_H__
#include "i_app.h"
#include "module.h"
#include <string>
#include <mutex>
#include <future>
#ifdef _INI_CONFIG
#include "ini_config.h"
#endif

namespace maix {
	class MAIX_EXPORT CApp : public CIApp
	{
	public:
		CApp(std::string strName);
		virtual ~CApp();

		virtual mxbool init();
		virtual mxbool unInit();
		virtual E_APP_STATE getState();
		virtual void setState(E_APP_STATE state);

		virtual void run();
		virtual mxbool start();
		virtual mxbool pause(bool isPause);
		virtual mxbool stop();

		virtual mxbool addModule(std::string strModuleName, 
			std::shared_ptr<CModule> module);
		virtual std::shared_ptr<CModule> getModule(std::string strModuleName);
		virtual mxbool delModule(std::string strModuleName);

		virtual mxbool connect(std::string strSender, 
			std::string strReceiver);
		virtual mxbool disconnect(std::string strSender, 
			std::string strReceiver);

		mxbool loadConfig(std::string strPath);

		template<class T>
		mxbool getConfig(std::string strSection,
			std::string strKey, T& value);

		template<class T>
		mxbool setConfig(std::string strSection,
			std::string strKey, T& value);

		mxbool saveConfig();
	private:
		std::string m_strName;
		E_APP_STATE m_state;
		std::mutex m_mutexModules;
		std::map<std::string, std::shared_ptr<CModule>> m_mapModules;

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
	inline mxbool CApp::getConfig(std::string strSection, std::string strKey, T & value)
	{
		std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
		try
		{
			value = m_configs[strSection][strKey].as<T>();
		}
		catch (std::invalid_argument &ia)
		{
			return mxfalse;
		}
#endif
		return mxtrue;
	}

	template<class T>
	inline mxbool CApp::setConfig(std::string strSection, std::string strKey, T & value)
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
#endif //__APP_H__