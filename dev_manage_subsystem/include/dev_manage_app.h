#ifndef __DEV_MANAGE_APP_H__
#define __DEV_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CDevManageApp : public CApp
	{
	public:
		CDevManageApp(std::string strName);
		~CDevManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initDevManage(std::string strConfigPath);
		mxbool unInitDevManage();

	private:
		std::string m_strDevModuleName;
		std::string m_strDevModuleGUID;
	};

}
#endif //__DEV_MANAGE_APP_H__
