#ifndef __EVENT_MANAGE_APP_H__
#define __EVENT_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CEventManageApp : public CApp
	{
	public:
		CEventManageApp(std::string strName);
		~CEventManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initEventManage(std::string strConfigPath);
		mxbool unInitEventManage();

	private:
		std::string m_strEventModuleName;
		std::string m_strEventModuleGUID;
	};

}
#endif //__EVENT_MANAGE_APP_H__
