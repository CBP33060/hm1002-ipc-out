#ifndef __DEV_MANAGE_PIR_DET_H__
#define __DEV_MANAGE_PIR_DET_H__
#include <string>
#include "fw_env_para.h"
#include "cJSON.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageModule;
	class CDevPirMotionDet
	{
	public:
		CDevPirMotionDet(CDevManageModule *objDevModule);
		~CDevPirMotionDet();

    	mxbool handleSetPirSensitivity(std::string &strValue);
		mxbool handleSetPirSwitch(std::string &strValue);
		mxbool handleSetPirInterval(std::string &strValue);
		mxbool handleSetTimingDection(std::string &strValue);
	private:
		CDevManageModule* m_objDevModule;
	};
}

#endif /*__DEV_MANAGE_PIR_DET_H__*/
