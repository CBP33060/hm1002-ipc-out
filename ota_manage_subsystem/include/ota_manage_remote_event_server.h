#ifndef __OTA_MANAGE_REMOTE_EVENT_SERVER_H__
#define __OTA_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "ota_manage_module.h"

namespace maix {
	class COTAManageRemoteEventServer : public CComProxyBase
	{
	public:
		COTAManageRemoteEventServer(COTAManageModule
			*objOTAManageModule);
		~COTAManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		COTAManageModule *m_objOTAManageModule;
	};
}
#endif //__OTA_MANAGE_REMOTE_EVENT_SERVER_H__
