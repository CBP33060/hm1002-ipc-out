#ifndef __OTA_MANAGE_APP_H__
#define __OTA_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class COTAManageApp : public CApp
	{
	public:
		COTAManageApp(std::string strName);
		~COTAManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initOTAManage(std::string strConfigPath);
		mxbool unInitOTAManage();

	private:
		std::string m_strOTAModuleName;
		std::string m_strOTAModuleGUID;
	};
}
#endif //__OTA_MANAGE_APP_H__
