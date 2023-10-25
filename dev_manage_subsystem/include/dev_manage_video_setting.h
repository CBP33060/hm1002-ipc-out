#ifndef __DEV_MANAGE_VIDEO_SETTING_H__
#define __DEV_MANAGE_VIDEO_SETTING_H__
#include <string>
#include "fw_env_para.h"
#include "cJSON.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageModule;
	class CDevVideoSetting
	{
	public:
		CDevVideoSetting(CDevManageModule *objDevModule);
		~CDevVideoSetting();

    	mxbool handleSetNightShot(std::string &strValue);
		mxbool handleGetNightShot(std::string &strValue);

		mxbool handleSetWdrSwitch(std::string &strValue);
		mxbool handleGetWdrSwitch(std::string &strValue);

		mxbool handleSetWaterMark(std::string &strValue);
		mxbool handleGetWaterMark(std::string &strValue);

		// mxbool handleSetRecordDuration(std::string &strValue);
		// mxbool handleGetRecordDuration(std::string &strValue);


		// mxbool handleSetNightShot(std::string &strValue);
	private:
		CDevManageModule *m_objDevModule;

	};
}

#endif /*__DEV_MANAGE_VIDEO_SETTING_H__*/
