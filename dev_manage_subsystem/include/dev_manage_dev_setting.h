#ifndef __DEV_MANAGE_DEV_SETTING_H__
#define __DEV_MANAGE_DEV_SETTING_H__
#include <string>
#include "fw_env_para.h"
#include "cJSON.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageModule;
	class CDevSetting
	{
	public:
		CDevSetting(CDevManageModule *objDevModule);
		~CDevSetting();

    	mxbool handleSetCameraSwitch(std::string &strValue);

    	mxbool handleSetIndicatorSwitch(std::string &strValue);

    	mxbool handleSetWhiteLightSwitch(std::string &strValue);
		mxbool handleSetWhiteLightBrightness(std::string &strValue);

		mxbool handleGetBatteryLevel(std::string &strValue);
		mxbool handleGetChargingState(std::string &strValue);

		mxbool handleGetWifiSignal(std::string &strValue);
	private:
		CDevManageModule *m_objDevModule;

	};
}

#endif /*__DEV_MANAGE_DEV_SETTING_H__*/
