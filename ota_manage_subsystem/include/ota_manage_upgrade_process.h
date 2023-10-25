#ifndef __OTA_MANAGE_UPGRADE_PROCESS_H__
#define __OTA_MANAGE_UPGRADE_PROCESS_H__
#include "module.h"

namespace maix {
	typedef enum
	{
		E_OTA_UPGRADE_CHECK = 0,
		E_OTA_UPGRADE_KERNEL,
		E_OTA_UPGRADE_ROOTFS,
		E_OTA_UPGRADE_SYSTEM,
		E_OTA_UPGRADE_MODEL,
		E_OTA_UPGRADE_MCU,
		E_OTA_UPGRADE_COMPLETE,
		E_OTA_UPGRADE_FAILED,
		E_OTA_UPGRADE_IDLE,
	} T_E_OTA_UPGRADE;

	class COTAManageUpgradeProcess
	{
	public:
		COTAManageUpgradeProcess(CModule *module); 
		~COTAManageUpgradeProcess();
		mxbool init();
		mxbool unInit();

		T_E_OTA_UPGRADE getOTAState();
		mxbool setOTAStage(T_E_OTA_UPGRADE eState);

		void run();
	private:
		CModule *m_module;
		mxbool m_bRun;
		T_E_OTA_UPGRADE m_eState;
	};
}
#endif //__OTA_MANAGE_UPGRADE_PROCESS_H__
