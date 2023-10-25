#include "log_manage_storage.h"
#include <sys/stat.h>

#define LOG_CACHE_BUFFER            4096                                    //log写缓冲区大小
#define LOG_LIMMIT                  1000                                    //loglist存放上限
#define LOG_FILE_COUNT              10                                      //log文件数量
#define LOG_FILE_TOTAL_MEMORY       1.5 * 1024 *1024                        //log文件总存储大小
#define LOG_FILE_SINGLE_MEMORY      LOG_FILE_TOTAL_MEMORY / LOG_FILE_COUNT  //log文件单个文件大小
#define LOG_FILE_PRE_NAME           "70mai_log_"                            //log文件名称前半部分
#define LOG_FILE_BEHIND_NAME        ".log"                                  //log文件名称后缀部分
#define LOG_FILE_TMP_BEHIND_NAME    "_tmp.log"                              //tmp log文件名称后缀部分
#define LOG_SIGNAL_LOG_DATA_SIZE    1024                                    //单条log的长度，与log_mx中对应

namespace maix {
    CLogManageStorage::CLogManageStorage(CModule *objModule)
        : m_objModule(objModule)
    {
        m_pLogFd = NULL;
        m_strTmpLogPath = "";
        m_llCurrentLogSize = 0;
    }

    CLogManageStorage::~CLogManageStorage()
    {
    }

    mxbool CLogManageStorage::init()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        if(!m_objModule)
        {
            return mxfalse;
        }

        if(!m_qLogList.init(LOG_LIMMIT,0))
            return mxfalse;

        std::string strUnixPath;
        if (!m_objModule->getConfig("LOG_STORAGE_CONFIG", "UNIX", strUnixPath))
        {
            return mxfalse;
        }
        
        if (!m_objModule->getConfig("LOG_STORAGE_CONFIG", "LOG_PATH", m_strLogDir))
        {
            return mxfalse;
        }       

        if(access(m_strLogDir.c_str(),F_OK) == -1)
        {
            printf("applog path null and creat : %s \n",m_strLogDir.c_str());
            mkdir(m_strLogDir.c_str(),S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        }

        m_threadWriteLog = std::thread([this]() {
            this->run();
        });

        m_objEpollServer = std::shared_ptr<CLogManageEpollServer>(new CLogManageEpollServer());
        if(!m_objEpollServer)
        {
            return mxfalse;
        }

        m_objEpollServer->registCallBack([&](E_EPOLL_LOG_TYPE enmType,std::string logData){
            if(enmType == TYPE_LOG_MCU)
            {
                m_objMCULogStorage->pushLogData(logData);
            }else{
                pushLogData(logData);
            }
        });

        if(!m_objEpollServer->init(strUnixPath))
        {
            return mxfalse;
        }

        m_objMCULogStorage = std::shared_ptr<CMCULogManageStorage>(new CMCULogManageStorage(m_objModule));
        if(!m_objMCULogStorage->init())
        {
            return mxfalse;
        }
        
        m_objKernelRead = std::shared_ptr<CLogManageKernelRead>(new CLogManageKernelRead());
        if(!m_objKernelRead)
        {
            return mxfalse;
        }

        m_objKernelRead->registCallBack([&](std::string logData){
            // printf("log kernel ::: %s \n",logData.c_str());
            pushLogData(logData);
        });

        if(!m_objKernelRead->init())
        {
            return mxfalse;
        }

        m_bInit = mxtrue;
        return mxtrue;
    }

    mxbool CLogManageStorage::unInit()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        if(m_pLogFd)
        {
            fflush(m_pLogFd);
            fclose(m_pLogFd);
            m_pLogFd = NULL;
            m_strTmpLogPath = "";
            m_llCurrentLogSize = 0;
        }
        if(m_objMCULogStorage)
        {
            m_objMCULogStorage->unInit();
        }
        m_bRun = mxfalse;
        m_bInit = mxfalse;
        return mxtrue;
    }

    mxbool CLogManageStorage::pushLogData(std::string &logData)
    {
        if(!m_bInit)
        {
            return false;
        }
        return m_qLogList.push(logData);
    }

    void CLogManageStorage::popLogData(std::string &logData)
    {
        m_qLogList.pop(logData);
    }

    void CLogManageStorage::run()
    {
        m_bRun = mxtrue;
        while (m_bRun)
        {
            std::string logData = "";
            popLogData(logData);
            if(logData.empty())
            {
                continue;
            }
            writeLogToFile(logData);
        }
    }

    void CLogManageStorage::writeLogToFile(std::string logData)
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        if(!m_bInit)
        {
            return;
        }
        changeWriteFile();
        if(m_pLogFd)
        {
            m_llCurrentLogSize += (logData.length() + 2);
            fwrite(logData.c_str(), logData.length(), 1, m_pLogFd);

            char enter[] = { '\r' , '\n' }; 
            fwrite(enter, sizeof(char), sizeof(enter), m_pLogFd);
            // fflush(m_pLogFd);
        }
        else
        {
            printf("fd open faild \n");
        }

    }

    /**
     * @brief 切换写日志文件
     * 思路:以文件名来判断当前到哪个文件，需要写的文件命名中增加tmp，例如 70mai_log_1.log  70mai_log_2_tmp.log
     * 
     */
    void CLogManageStorage::changeWriteFile()
    {
        //先判断当前写入的文件是否超过单个文件的上限，不超过继续写入，超过换下一个文件
        if(!m_strTmpLogPath.empty())
        {
            // size_t tempFileSize = getFileSize(m_strTmpLogPath.c_str());
            // printf("fd size %ld\n",tempFileSize);
            if((m_llCurrentLogSize + LOG_SIGNAL_LOG_DATA_SIZE) < LOG_FILE_SINGLE_MEMORY)
            {
                return;
            }
        }
        else
        {
            std::string currentTmpFile = "";
            for (int i = 0; i < LOG_FILE_COUNT; i++)
            {
                currentTmpFile = montageLogPath(m_strLogDir,LOG_FILE_PRE_NAME,(i+1),LOG_FILE_TMP_BEHIND_NAME);
                if(isFileExit(currentTmpFile))
                {
                    break;
                }
                else
                {
                    currentTmpFile = "";
                }
            }
            if(currentTmpFile.empty())
            {
                currentTmpFile = montageLogPath(m_strLogDir,LOG_FILE_PRE_NAME,1,LOG_FILE_TMP_BEHIND_NAME);
            }
            m_strTmpLogPath = currentTmpFile;
            m_llCurrentLogSize = getFileSize(m_strTmpLogPath.c_str());
            printf("fd current log size %lld\n",m_llCurrentLogSize);
            if((m_llCurrentLogSize + LOG_SIGNAL_LOG_DATA_SIZE) < LOG_FILE_SINGLE_MEMORY)
            {
                m_pLogFd = fopen(m_strTmpLogPath.c_str(), "a+");
                if(!m_pLogFd)
                {
                    printf("fd open faild \n");
                    m_strTmpLogPath = "";
                }
                else
                {
                    if(setvbuf(m_pLogFd,NULL,_IOFBF,LOG_CACHE_BUFFER) != 0)
                    {
                        printf("fd setVbuf faild \n");
                    }
                }
                return;
            }
        }
        
        //查询当前tmp位置
        int iCurrentTmpPosition = checkTmpFilePosition(m_strTmpLogPath);
        //判断与tmp同位置的log文件是否存在，存在就删除，将tmp再rename为同位置的log文件
        std::string strCurrentLogPath = "";
        strCurrentLogPath = montageLogPath(m_strLogDir,LOG_FILE_PRE_NAME,iCurrentTmpPosition,LOG_FILE_BEHIND_NAME);
        if(isFileExit(strCurrentLogPath))
        {
            remove(strCurrentLogPath.c_str());
        }
        if(m_pLogFd)
        {
            fflush(m_pLogFd);
            fclose(m_pLogFd);
            m_pLogFd = NULL;
        }
        //找到下一个tmp的位置，打开文件，如果打开失败，则等到下次再写日志的时候重走上面的识别流程
        std::string strNextTmpLogPath = "";
        strNextTmpLogPath = montageLogPath(m_strLogDir,LOG_FILE_PRE_NAME,(iCurrentTmpPosition == LOG_FILE_COUNT ? 1 : (iCurrentTmpPosition + 1)),LOG_FILE_TMP_BEHIND_NAME);
        m_pLogFd = fopen(strNextTmpLogPath.c_str(), "a+");
        if(!m_pLogFd)
        {
            printf("fd open faild \n");
            m_strTmpLogPath = "";
            return;
        }
        m_llCurrentLogSize = 0;
        if(setvbuf(m_pLogFd,NULL,_IOFBF,LOG_CACHE_BUFFER) != 0)
        {
            printf("fd setVbuf faild \n");
        }
        
        //将tmp rename为当前位置的log文件，如果重名失败，则继续将fd更换为当前tmp的fd，继续写到当前的tmp下，等下次再切换
        if(std::rename(m_strTmpLogPath.c_str(),strCurrentLogPath.c_str()) != 0)
        {
            printf("fd rename faild \n");
            fflush(m_pLogFd);
            fclose(m_pLogFd);
            m_pLogFd = NULL;
            m_pLogFd = fopen(m_strTmpLogPath.c_str(), "a+");
            if(!m_pLogFd)
            {
                printf("fd open faild \n");
                m_strTmpLogPath = "";
            }
            else
            {
                m_llCurrentLogSize = 0;
                if(setvbuf(m_pLogFd,NULL,_IOFBF,LOG_CACHE_BUFFER) != 0)
                {
                    printf("fd setVbuf faild \n");
                }
            }
            return;
        }
        m_strTmpLogPath = strNextTmpLogPath;
}

    int CLogManageStorage::checkTmpFilePosition(std::string tmpFilePath)
    {
        std::string numPrePath = "";
        numPrePath.append(m_strLogDir);
        numPrePath.append(LOG_FILE_PRE_NAME);
        std::string numBehindPath = LOG_FILE_TMP_BEHIND_NAME;
        std::string position = tmpFilePath.substr(numPrePath.length(),tmpFilePath.length() - numPrePath.length() - numBehindPath.length());
        return atoi(position.c_str());
    }

    std::string CLogManageStorage::montageLogPath(std::string strDir,std::string strPreName,int iNum,std::string strBehindName)
    {
        char buf[100] = { 0 };
        snprintf(buf, sizeof(buf), "%s%s%d%s", strDir.c_str(),strPreName.c_str(),iNum,strBehindName.c_str());
        return std::string(buf);
    }

    size_t CLogManageStorage::getFileSize(const char* fileName)
    {
        if(fileName == NULL)
        {
            return 0;
        }
        struct stat statBuf;
        stat(fileName,&statBuf);
        return statBuf.st_size;
    }

    mxbool CLogManageStorage::isFileExit(std::string filePath)
    {
        return (access(filePath.c_str(),F_OK) == 0);
    }

    mxbool CLogManageStorage::flushLogCache()
    {
        if(m_pLogFd)
        {
            fflush(m_pLogFd);
        }
        if(m_objMCULogStorage && (m_objMCULogStorage.use_count() != 0))
        {
            m_objMCULogStorage->flushLogCache();
        }
        return mxtrue;
    }

}