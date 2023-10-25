#ifndef __AI_MANAGE_APP_H__
#define __AI_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CAIManageApp : public CApp
	{
	public:
		CAIManageApp(std::string strName);
		~CAIManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initAIManage(std::string strConfigPath);
		mxbool unInitAIManage();

	private:
		std::string m_strAIModuleName;
		std::string m_strAIModuleGUID;
	};
}
#endif //__AI_MANAGE_APP_H__
