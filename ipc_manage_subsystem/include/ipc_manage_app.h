#ifndef __IPC_MANAGE_APP_H__
#define __IPC_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CIPCManageApp : public CApp
	{
	public:
		CIPCManageApp(std::string strName);
		~CIPCManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initIPCManage(std::string strConfigPath);
		mxbool unInitIPCManage();

	private:
		std::string m_strIPCModuleName;
		std::string m_strIPCModuleGUID;
	};
}
#endif //__IPC_MANAGE_APP_H__