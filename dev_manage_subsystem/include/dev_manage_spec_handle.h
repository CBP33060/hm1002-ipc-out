#ifndef __DEV_MANAGE_SPEC_HANDLE_H__
#define __DEV_MANAGE_SPEC_HANDLE_H__
#include <string>
#include "cJSON.h"

#include "fw_env_para.h"
#include "dev_manage_module.h"
#include "dev_manage_alert_msg.h"
#include "dev_manage_dev_setting.h"
#include "dev_manage_video_setting.h"
#include "dev_manage_spec_base.h"
#include "dev_manage_intelligent_det.h"
#include "dev_manage_pir_det.h"


namespace maix {
	#define BIND_FUNC(a,b) std::bind(a, b, std::placeholders::_1)
	typedef std::function<bool(std::string &strValue)> specHandleFunction;

	typedef struct __SPEC_HANDLE_CONCIFG
	{
		std::string m_strEvent;
		const char *m_pParaName;
		specHandleFunction m_handleFunc;		
	} T_SPEC_HANDLE_CONFIG;

	class CDevAlertMsg;
	class CDevSetting;
	class CDevVideoSetting;
	class CDevManageModule;
	class CDevPirMotionDet;
	class CDevIntelligentDet;

	class CDevSpecHandle
	{
	public:
		CDevSpecHandle(CDevManageModule *objDevModule);
		~CDevSpecHandle();

		mxbool init();
		std::string handleSpec(const std::string &strParam);
	private:
		CDevManageModule *m_objDevModule;		
		std::shared_ptr<CDevAlertMsg> m_objAlertMsg;
		std::shared_ptr<CDevVideoSetting> m_objVideoSetting;
		std::shared_ptr<CDevSetting> m_objDevSetting;
		std::shared_ptr<CDevIntelligentDet> m_objDevIntelligentDet;
		std::shared_ptr<CDevPirMotionDet> m_objDevPirMotionDet;

		std::map<std::string, T_SPEC_HANDLE_CONFIG> m_specHandleSetMap;
		std::map<std::string, T_SPEC_HANDLE_CONFIG> m_specHandleGetMap;

		mxbool handleGetSpec(const std::string &strEvent, const cJSON *pJsonParam, std::string &strCode, std::string &strValue);
		mxbool handleSetSpec(const std::string &strEvent, const cJSON *pJsonParam, std::string &strCode, std::string &strValue);

    	std::string inputRet(cJSON *pJsonParam);
	};
}

#endif /*__DEV_MANAGE_SPEC_HANDLE_H__*/
