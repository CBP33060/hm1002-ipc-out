#ifndef __SPEAKER_MANAGE_APP_H__
#define __SPEAKER_MANAGE_APP_H__
#include "app.h"

namespace maix {
	class CSpeakerManageApp : public CApp
	{
	public:
		CSpeakerManageApp(std::string strName);
		~CSpeakerManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initSpeakerManage(std::string strConfigPath);
		mxbool unInitSpeakerManage();

	private:
		std::string m_strSpeakerModuleName;
		std::string m_strSpeakerModuleGUID;
	};
}
#endif //__SPEAKER_MANAGE_APP_H__
