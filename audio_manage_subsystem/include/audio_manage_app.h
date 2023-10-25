#ifndef __AUDIO_MANAGE_APP_H__
#define __AUDIO_MANAGE_APP_H__
#include "app.h"
namespace maix {
	class CAudioManageApp : public CApp
	{
	public:
		CAudioManageApp(std::string strName);
		~CAudioManageApp();

		mxbool init();
		mxbool unInit();
		mxbool initAudioManage(std::string strConfigPath);
		mxbool unInitAudioManage();

	private:
		std::string m_strAudioMoudleName;
		std::string m_strAudioMoudleGUID;
	};
}
#endif //__AUDIO_MANAGE_APP_H__
