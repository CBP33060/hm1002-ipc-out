#ifndef __LOCAL_STORAGE_REMOTE_EVENT_SERVER_H__
#define __LOCAL_STORAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "local_storage_module.h"

namespace maix {
	class CLocalStorageRemoteEventServer : public CComProxyBase
	{
	public:
		CLocalStorageRemoteEventServer(CLocalStorageModule
			*objLocalStorageModule);
		~CLocalStorageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CLocalStorageModule *m_objLocalStorageModule;
	};
}
#endif //__LOCAL_STORAGE_REMOTE_EVENT_SERVER_H__
