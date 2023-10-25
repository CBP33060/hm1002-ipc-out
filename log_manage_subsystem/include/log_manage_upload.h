#ifndef __LOG_MANAGE_UPLOAD_H__
#define __LOG_MANAGE_UPLOAD_H__
#include "module.h"

namespace maix {

    typedef enum
    {
        TYPE_UPLOAD_LOG_NORMAL,
        TYPE_UPLOAD_LOG_MCU,
    }E_UPLOAD_LOG_TYPE;

	class CLogManageUpload
	{
	public:
		CLogManageUpload(CModule *objModule);
		~CLogManageUpload();

		mxbool init();
		mxbool unInit();

        std::string startUploadLog(std::string strParam);

	private:

        void uploadAppLog();
        void uploadMcuLog();
        mxbool sendLogToIpcManage(E_UPLOAD_LOG_TYPE type,std::string logdata,int iLen);
        void sendLogEndToIpcManage();
        //判断发送是否成功
        mxbool isSendSuccess(std::string strResult);

        void uploadLogRun();

        int checkTmpFilePosition(std::string strLogDir,std::string strLogPreName,std::string tmpFilePath);
        mxbool isFileExit(std::string filePath);
        std::string montageLogPath(std::string strDir,std::string strPreName,int iNum,std::string strBehindName);

        std::string procResult(std::string code,
            std::string strMsg, std::string strErr);

        mxbool m_bInit;
        mxbool m_bRun;

        std::string m_strDID;

        CModule * m_objModule;

        std::string m_strApplogDir;
        std::string m_strMcuLogDir;
        
        std::string m_strIpcManageGUID;
        std::string m_strIpcManageServer;

        std::thread m_threadUploadLog;
        std::mutex m_mutexUploadLog;
        std::condition_variable m_conditionUploadLog;

        mxbool m_bNeedUpload;
	};
}
#endif //__LOG_MANAGE_UPLOAD_H__