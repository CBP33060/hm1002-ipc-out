#ifndef __Log_MANAGE_APP_H__
#define __Log_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CLogManageApp : public CApp
	{
	public:
		CLogManageApp(std::string strName);
		~CLogManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initLogManage(std::string strConfigPath);
		mxbool unInitLogManage();

	private:
		std::string m_strLogModuleName;
		std::string m_strLogModuleGUID;
	};
}
#endif //__Log_MANAGE_APP_H__
