#ifndef __VIDEO_MANAGE_APP_H__
#define __VIDEO_MANAGE_APP_H__
#include "app.h"
namespace maix {
	class CVideoManageApp : public CApp
	{
	public:
		CVideoManageApp(std::string strName);
		~CVideoManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initVideoManage(std::string strConfigPath);
		mxbool unInitVideoManage();

	private:
		std::string m_strVideoModuleName;
		std::string m_strVideoModuleGUID;
	};
}
#endif //__VIDEO_MANAGE_APP_H__
