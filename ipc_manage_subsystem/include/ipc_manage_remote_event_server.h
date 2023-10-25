#ifndef __IPC_MANAGE_REMOTE_EVENT_SERVER_H__
#define __IPC_MANAGE_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "ipc_manage_module.h"

namespace maix {
	class CIPCManageRemoteEventServer : public CComProxyBase
	{
	public:
		CIPCManageRemoteEventServer(CIPCManageModule
			*objIPCManageModule);
		~CIPCManageRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CIPCManageModule *m_objIPCManageModule;
	};
}
#endif //__EVENT_MANAGE_REMOTE_EVENT_SERVER_H__
