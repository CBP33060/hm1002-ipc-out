#ifndef __DEV_MANAGE_ALERT_MSG_H__
#define __DEV_MANAGE_ALERT_MSG_H__
#include <string>
#include "fw_env_para.h"
#include "cJSON.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageModule;
	class CDevAlertMsg
	{
	public:
		CDevAlertMsg(CDevManageModule *objDevModule);
		~CDevAlertMsg();

		mxbool handleSetAutoAlarmSwitch(std::string &strValue);
		mxbool handleSetManualAlarmSwitch(std::string &strValue);

	private:
		CDevManageModule *m_objDevModule;

		int m_iAutoAlarmValue;

	};
}

#endif /*__DEV_MANAGE_ALERT_MSG_H__*/
