#include "ota_manage_upgrade_process.h"
#include "ota_manage_module.h"
#include "log_mx.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
namespace maix {
	COTAManageUpgradeProcess::COTAManageUpgradeProcess(CModule * module)
		: m_module(module)
		, m_bRun(mxfalse)
		, m_eState(E_OTA_UPGRADE_IDLE)
	{
	}

	COTAManageUpgradeProcess::~COTAManageUpgradeProcess()
	{
	}

	mxbool COTAManageUpgradeProcess::init()
	{
		return mxtrue;
	}

	mxbool COTAManageUpgradeProcess::unInit()
	{
		return mxtrue;
	}

	T_E_OTA_UPGRADE COTAManageUpgradeProcess::getOTAState()
	{
		return m_eState;
	}

	mxbool COTAManageUpgradeProcess::setOTAStage(T_E_OTA_UPGRADE eState)
	{
		m_eState = eState;
		return mxtrue;
	}

	void COTAManageUpgradeProcess::run()
	{
		m_bRun = mxtrue;
		while (m_bRun)
		{
			switch (m_eState)
			{
			case E_OTA_UPGRADE_CHECK:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeCheck())
					{
						m_eState = E_OTA_UPGRADE_KERNEL;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_KERNEL:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeKernel())
					{
						m_eState = E_OTA_UPGRADE_ROOTFS;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_ROOTFS:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeRootfs())
					{
						m_eState = E_OTA_UPGRADE_SYSTEM;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_SYSTEM:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeSystem())
					{
						m_eState = E_OTA_UPGRADE_MODEL;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_MODEL:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeModel())
					{
						m_eState = E_OTA_UPGRADE_MCU;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_MCU:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeMCU())
					{
						m_eState = E_OTA_UPGRADE_COMPLETE;
					}
					else
					{
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_COMPLETE:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeComplete())
					{
						m_eState = E_OTA_UPGRADE_IDLE;
					}
					else
					{
						logPrint(MX_LOG_ERROR, "ota upgraed complete failed");
						m_eState = E_OTA_UPGRADE_FAILED;
					}
				}
				break;
			case E_OTA_UPGRADE_FAILED:
				{
					if (m_module && 
						((COTAManageModule*)m_module)->upgradeFailed())
					{
						m_eState = E_OTA_UPGRADE_IDLE;
					}

					logPrint(MX_LOG_DEBUG, "ota upgrade failed");
				}
				break;
			case E_OTA_UPGRADE_IDLE:
				{
					logPrint(MX_LOG_TRACE, "ota upgrade idle state");
				}
				break;
			default:
				break;
			}

#ifdef _WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
			continue;
		}
	}
}