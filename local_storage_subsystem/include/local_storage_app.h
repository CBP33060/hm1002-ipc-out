#ifndef __LOCAL_STORAGE_APP_H__
#define __LOCAL_STORAGE_APP_H__
#include "app.h"

namespace maix {
	class CLocalStorageApp : public CApp
	{
	public:
		CLocalStorageApp(std::string strName);
		~CLocalStorageApp();

		mxbool init();
		mxbool unInit();
		mxbool initLocalStorageModule(std::string strConfigPath);
		mxbool unInitLocalStorageModule();

	private:
		std::string m_strLocalStorageModuleName;
		std::string m_strLocalStorageModuleGUID;
	};
	
}
#endif //__LOCAL_STORAGE_APP_H__
