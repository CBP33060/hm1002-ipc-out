#ifndef __MCU_LOG_MANAGE_STORAGE_H__
#define __MCU_LOG_MANAGE_STORAGE_H__
#include "module.h"
#include "b_queue.h"

namespace maix {
	class CMCULogManageStorage
	{
	public:
		CMCULogManageStorage(CModule *objModule);
		~CMCULogManageStorage();

		mxbool init();
		mxbool unInit();

        mxbool pushLogData(std::string &logData);
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

        void popLogData(std::string &logData);

        mxbool m_bInit;
        mxbool m_bRun;

        CModule * m_objModule;

        std::string m_strLogDir;

        std::thread m_threadWriteLog;

        CBQueue<std::string> m_qLogList; 

        FILE* m_pLogFd;
        std::string m_strTmpLogPath;
        long long m_llCurrentLogSize;               //存储当前正在写入的日志文件的大小

	};
}
#endif //__MCU_LOG_MANAGE_STORAGE_H__