#ifndef __LOW_POWER_REMOTE_EVENT_SERVER_H__
#define __LOW_POWER_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "low_power_module.h"

namespace maix {
	class CLowPowerRemoteEventServer : public CComProxyBase
	{
	public:
		CLowPowerRemoteEventServer(CLowPowerModule
			*objLowPowerModule);
		~CLowPowerRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CLowPowerModule *m_objLowPowerModule;
	};
}
#endif //__LOW_POWER_REMOTE_EVENT_SERVER_H__
