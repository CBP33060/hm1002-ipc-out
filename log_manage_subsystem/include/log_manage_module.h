#ifndef __LOG_MANAGE_MODULE_H__
#define __LOG_MANAGE_MODULE_H__
#include "module.h"
#include "log_manage_storage.h"
#include "log_manage_upload.h"

namespace maix {
	class MAIX_EXPORT CLogManageModule : public CModule
	{
	public:
		CLogManageModule(std::string strGUID, std::string strName);
		~CLogManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
        mxbool initLogStorage();
        mxbool initMcuRemote();
        mxbool initLogUpload();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);

		std::string enterLowPower(std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:

        void sendToMcuLogOnline();

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

        std::shared_ptr<CLogManageStorage> m_ptrLogStorage;
        std::shared_ptr<CLogManageUpload> m_ptrLogUpload;

        std::string m_strMcuManageGUID;
        std::string m_strMcuManageServer;
	};
}
#endif //__LOG_MANAGE_MODULE_H__
