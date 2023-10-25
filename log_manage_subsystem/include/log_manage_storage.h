#ifndef __LOG_MANAGE_STORAGE_H__
#define __LOG_MANAGE_STORAGE_H__
#include "module.h"
#include "log_manage_epoll_server.h"
#include "log_manage_mcu_storage.h"
#include "log_manage_kernel_read.h"
#include "b_queue.h"

namespace maix {
	class CLogManageStorage
	{
	public:
		CLogManageStorage(CModule *objModule);
		~CLogManageStorage();

		mxbool init();
		mxbool unInit();

        //刷新log缓冲区
        mxbool flushLogCache();

	private:

        void run();
        void writeLogToFile(std::string logData);
        void changeWriteFile();
        int checkTmpFilePosition(std::string tmpFilePath);
        size_t getFileSize(const char* fileName);
        mxbool isFileExit(std::string filePath);
        std::string montageLogPath(std::string strDir,std::string strPreName,int iNum,std::string strBehindName);

        mxbool pushLogData(std::string &logData);
        void popLogData(std::string &logData);

        mutable std::mutex m_initMutex;
        mxbool m_bInit;
        mxbool m_bRun;

        CModule * m_objModule;
        std::shared_ptr<CLogManageEpollServer> m_objEpollServer;
        std::shared_ptr<CMCULogManageStorage> m_objMCULogStorage;       
        std::shared_ptr<CLogManageKernelRead> m_objKernelRead; 
        std::string m_strLogDir;

        std::thread m_threadWriteLog;

        CBQueue<std::string> m_qLogList; 

        FILE* m_pLogFd;
        std::string m_strTmpLogPath;
        long long m_llCurrentLogSize;               //存储当前正在写入的日志文件的大小

	};
}
#endif //__LOG_MANAGE_STORAGE_H__