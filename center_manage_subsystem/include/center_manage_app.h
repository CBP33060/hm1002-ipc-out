#ifndef __CENTER_MANAGE_APP_H__
#define __CENTER_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CCenterManageApp : public CApp
	{
	public:
		CCenterManageApp(std::string strName);
		~CCenterManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initCenterManage(std::string strConfigPath);
		mxbool unInitCenterManage();
		mxbool initLowPowerManage(std::string strConfigPath);
		mxbool unInitLowPowerManage();
		
	private:
		std::string m_strCenterMoudleName;
		std::string m_strCenterMoudleGUID;

		std::string m_strLowPowerMoudleName;
		std::string m_strLowPowerMoudleGUID;
	};
}
#endif //__CENTER_MANAGE_APP_H__
