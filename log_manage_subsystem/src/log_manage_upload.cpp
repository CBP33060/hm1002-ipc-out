#include "log_manage_upload.h"
#include <sys/stat.h>
#include "cJSON.h"
#include "log_mx.h"
#include "fw_env_para.h"
#include "common.h"
#include <stdio.h>
#include "unistd.h"
#include "string.h"

#define LOG_FILE_COUNT                  10                                      //applog文件数量
#define LOG_FILE_MCU_COUNT              10                                      //mculog文件数量
#define LOG_FILE_PRE_NAME               "70mai_log_"                            //applog文件名称前半部分
#define LOG_FILE_PRE_MCU_NAME           "70mai_mcu_log_"                        //mculog文件名称前半部分
#define LOG_FILE_BEHIND_NAME            ".log"                                  //log文件名称后缀部分
#define LOG_FILE_TMP_BEHIND_NAME        "_tmp.log"                              //tmp log文件名称后缀部分
#define LOG_FILE_SINGLE_PACKAGE_LENGTH  4096                                    //单包数据读取长度

namespace maix {
    CLogManageUpload::CLogManageUpload(CModule *objModule)
        : m_objModule(objModule)
    {
        m_bInit = mxfalse;
        m_bRun = mxfalse;
        m_bNeedUpload = mxfalse;
    }

    CLogManageUpload::~CLogManageUpload()
    {
    }

    mxbool CLogManageUpload::init()
    {
        if(!m_objModule)
        {
            return mxfalse;
        }

        m_strDID = getDID();

        if (!m_objModule->getConfig("LOG_STORAGE_CONFIG", "LOG_PATH", m_strApplogDir))
        {
            return mxfalse;
        }
        if (!m_objModule->getConfig("LOG_STORAGE_CONFIG", "LOG_PATH", m_strMcuLogDir))
        {
            return mxfalse;
        }

        if (!m_objModule->getConfig("IPC_MANAGE_REMOTE_EVENT", "GUID", m_strIpcManageGUID))
        {
            return mxfalse;
        }
        if (!m_objModule->getConfig("IPC_MANAGE_REMOTE_EVENT", "SERVER", m_strIpcManageServer))
        {
            return mxfalse;
        }

        m_threadUploadLog = std::thread([this]() {
			this->uploadLogRun();
		});

        m_bInit = mxtrue;
        return mxtrue;
    }

    mxbool CLogManageUpload::unInit()
    {
        m_bRun = mxfalse;
        m_bInit = mxfalse;
        return mxtrue;
    }

    void CLogManageUpload::uploadLogRun()
    {
        m_bRun = true;
        while (m_bRun)
        {
            std::unique_lock<std::mutex> lock(m_mutexUploadLog);
            m_conditionUploadLog.wait(lock);
            if(m_bNeedUpload)
            {
                uploadAppLog();
                uploadMcuLog();
                sendLogEndToIpcManage();
                m_bNeedUpload = false;
            }
        }
    }

    std::string CLogManageUpload::startUploadLog(std::string strParam)
    {
        std::string strResultCode;
        std::string strResultErrMsg;
        {
            std::unique_lock<std::mutex> lock(m_mutexUploadLog);
            cJSON *jsonRoot = cJSON_Parse(strParam.c_str());
            std::string strDID;

            if (jsonRoot)
            {
                cJSON *jsonDID = cJSON_GetObjectItem(jsonRoot, "did");
                if (!jsonDID)
                {
                    cJSON_Delete(jsonRoot);
                    strResultCode = "500";
                    strResultErrMsg = "did parse err";
                }
                else
                {
                    strDID = std::string(jsonDID->valuestring);
                    strResultCode = "200";
                    strResultErrMsg = "start upload success";
                }

                cJSON_Delete(jsonRoot);
            }
            if(strResultCode.compare("200") == 0)
            {
                m_bNeedUpload = true;
                m_conditionUploadLog.notify_one();
            }
        }
        logPrint(MX_LOG_ERROR,"start upload %s  %s",strParam.c_str(),strResultErrMsg.c_str());
        return procResult(strResultCode,"",strResultErrMsg);
    }

    void CLogManageUpload::uploadAppLog()
    {
        int iCurrentTmpPosition;
        std::string currentTmpFile = "";
        for (int i = 0; i < LOG_FILE_COUNT; i++)
        {
            currentTmpFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_NAME,(i+1),LOG_FILE_TMP_BEHIND_NAME);
            if(isFileExit(currentTmpFile))
            {
                break;
            }
            else
            {
                currentTmpFile = "";
            }
        }
        logPrint(MX_LOG_ERROR,"currentTmpFile %s\n",currentTmpFile.c_str());
        if(currentTmpFile.empty())
        {
            std::string strUplodFile = "";
            for (int i = 0; i < LOG_FILE_COUNT; i++)
            {
                strUplodFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_NAME,(i+1),LOG_FILE_BEHIND_NAME);
                if(isFileExit(strUplodFile))
                {
                    FILE* sourceFd = fopen(strUplodFile.c_str(),"r");
                    char readBuf[LOG_FILE_SINGLE_PACKAGE_LENGTH];
                    if(sourceFd)
                    {
                        int readNum = 0;
                        while(m_bRun)
                        {
                            memset(readBuf,0,sizeof(readBuf));
                            readNum = fread(readBuf, 1, sizeof(readBuf), sourceFd);
                            if(readNum <= 0)
                            {
                                break;
                            }
                            sendLogToIpcManage(TYPE_UPLOAD_LOG_NORMAL,readBuf,readNum);
                        }
                        fclose(sourceFd);
                    }
                }
                else
                {
                    if(i == 0)
                    {
                        std::string strRead = "get app path faild : not find applog";
                        sendLogToIpcManage(TYPE_UPLOAD_LOG_NORMAL,strRead,strRead.length());
                        return;
                    }
                }
            }
            return;
        }
        else
        {
            iCurrentTmpPosition = checkTmpFilePosition(m_strApplogDir,LOG_FILE_PRE_NAME,currentTmpFile);
        }

        std::string strUplodLogFile = "";
        int iCurrentSavePosition = iCurrentTmpPosition;
        for (int i = 0; i < LOG_FILE_COUNT; i++)
        {
            logPrint(MX_LOG_ERROR,"app iCurrentSavePosition %d   %d\n",iCurrentTmpPosition,iCurrentSavePosition);
            iCurrentSavePosition++;
            if(iCurrentSavePosition > LOG_FILE_COUNT)
            {
                iCurrentSavePosition = 1;
            }
            if(iCurrentSavePosition == iCurrentTmpPosition)
            {
                strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_NAME,iCurrentSavePosition,LOG_FILE_TMP_BEHIND_NAME);
                if(!isFileExit(strUplodLogFile))
                {
                    strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_NAME,iCurrentSavePosition,LOG_FILE_BEHIND_NAME);
                }
            }
            else{
                strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_NAME,iCurrentSavePosition,LOG_FILE_BEHIND_NAME);
            }            
            
            if(isFileExit(strUplodLogFile))
            {
                logPrint(MX_LOG_ERROR,"app strUplodLogFile %s\n",strUplodLogFile.c_str());
                FILE* sourceFd = fopen(strUplodLogFile.c_str(),"r");
                char readBuf[LOG_FILE_SINGLE_PACKAGE_LENGTH];
                if(sourceFd)
                {
                    int readNum = 0;
                    while(m_bRun)
                    {
                        memset(readBuf,0,sizeof(readBuf));
                        readNum = fread(readBuf, 1, sizeof(readBuf), sourceFd);
                        if(readNum <= 0)
                        {
                            break;
                        }
                        sendLogToIpcManage(TYPE_UPLOAD_LOG_NORMAL,readBuf,readNum);
                    }
                    fclose(sourceFd);
                }
            }
        }

    }

    void CLogManageUpload::uploadMcuLog()
    {
        int iCurrentTmpPosition;
        std::string currentTmpFile = "";
        for (int i = 0; i < LOG_FILE_MCU_COUNT; i++)
        {
            currentTmpFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,(i+1),LOG_FILE_TMP_BEHIND_NAME);
            if(isFileExit(currentTmpFile))
            {
                break;
            }
            else
            {
                currentTmpFile = "";
            }
        }
        logPrint(MX_LOG_ERROR,"mcu currentTmpFile %s\n",currentTmpFile.c_str());
        if(currentTmpFile.empty())
        {
            std::string strUplodFile = "";
            for (int i = 0; i < LOG_FILE_MCU_COUNT; i++)
            {
                strUplodFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,(i+1),LOG_FILE_BEHIND_NAME);
                if(isFileExit(strUplodFile))
                {
                    FILE* sourceFd = fopen(strUplodFile.c_str(),"r");
                    char readBuf[LOG_FILE_SINGLE_PACKAGE_LENGTH];
                    if(sourceFd)
                    {
                        int readNum = 0;
                        while(m_bRun)
                        {
                            memset(readBuf,0,sizeof(readBuf));
                            readNum = fread(readBuf, 1, sizeof(readBuf), sourceFd);
                            if(readNum <= 0)
                            {
                                break;
                            }
                            sendLogToIpcManage(TYPE_UPLOAD_LOG_MCU,readBuf,readNum);
                        }
                        fclose(sourceFd);
                    }
                }
                else
                {
                    if(i == 0)
                    {
                        std::string strRead = "get path faild : not find mcu log";
                        sendLogToIpcManage(TYPE_UPLOAD_LOG_MCU,strRead,strRead.length());
                        return;
                    }
                }
            }
            return;
        }
        else
        {
            iCurrentTmpPosition = checkTmpFilePosition(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,currentTmpFile);
        }

        std::string strUplodLogFile = "";
        int iCurrentSavePosition = iCurrentTmpPosition;
        for (int i = 0; i < LOG_FILE_MCU_COUNT; i++)
        {
            logPrint(MX_LOG_ERROR,"mcu iCurrentSavePosition %d   %d\n",iCurrentTmpPosition,iCurrentSavePosition);
            iCurrentSavePosition++;
            if(iCurrentSavePosition > LOG_FILE_MCU_COUNT)
            {
                iCurrentSavePosition = 1;
            }
            if(iCurrentSavePosition == iCurrentTmpPosition)
            {
                strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,iCurrentSavePosition,LOG_FILE_TMP_BEHIND_NAME);
                if(!isFileExit(strUplodLogFile))
                {
                    strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,iCurrentSavePosition,LOG_FILE_BEHIND_NAME);
                }
            }
            else{
                strUplodLogFile = montageLogPath(m_strApplogDir,LOG_FILE_PRE_MCU_NAME,iCurrentSavePosition,LOG_FILE_BEHIND_NAME);
            }     
            if(isFileExit(strUplodLogFile))
            {
                FILE* sourceFd = fopen(strUplodLogFile.c_str(),"r");
                logPrint(MX_LOG_ERROR,"mcu strUplodLogFile %s\n",strUplodLogFile.c_str());
                char readBuf[LOG_FILE_SINGLE_PACKAGE_LENGTH];
                if(sourceFd)
                {
                    int readNum = 0;
                    while(m_bRun)
                    {
                        readNum = fread(readBuf, 1, sizeof(readBuf), sourceFd);
                        if(readNum <= 0)
                        {
                            break;
                        }
                        sendLogToIpcManage(TYPE_UPLOAD_LOG_MCU,readBuf,readNum);
                    }
                    fclose(sourceFd);
                }
            }
        }
    }

    mxbool CLogManageUpload::sendLogToIpcManage(E_UPLOAD_LOG_TYPE type,std::string logdata,int iLen)
    {
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "LogData");

        cJSON_AddStringToObject(jsonParam, "data", logdata.c_str());
        cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
        cJSON_AddNumberToObject(jsonParam, "type", type);
        cJSON_AddNumberToObject(jsonParam, "len", iLen);

        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strLogData = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        // logPrint(MX_LOG_ERROR,"strLogData   %s ",strLogData.c_str());
        
        std::string strResult;
        if(m_objModule != NULL)
        {
            strResult = m_objModule->output(m_strIpcManageGUID,m_strIpcManageServer,(unsigned char*) strLogData.c_str(),strLogData.length());
            if(!isSendSuccess(strResult))
            {
                strResult = m_objModule->output(m_strIpcManageGUID,m_strIpcManageServer,(unsigned char*) strLogData.c_str(),strLogData.length());
            }
        }
        return true;
    }

    void CLogManageUpload::sendLogEndToIpcManage()
    {
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonParam = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "event", "LogDataEnd");
        cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());

        cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

        char *out = cJSON_Print(jsonRoot);
        std::string strLogData = std::string(out);
        cJSON_Delete(jsonRoot);
        if (out)
            free(out);

        logPrint(MX_LOG_ERROR,"sendLogEndToIpcManage strLogData %s\n",strLogData.c_str());
        
        std::string strResult;
        if(m_objModule != NULL)
        {
            strResult = m_objModule->output(m_strIpcManageGUID,m_strIpcManageServer,(unsigned char*) strLogData.c_str(),strLogData.length());
            if(!isSendSuccess(strResult))
            {
                strResult = m_objModule->output(m_strIpcManageGUID,m_strIpcManageServer,(unsigned char*) strLogData.c_str(),strLogData.length());
            }
        }
        logPrint(MX_LOG_ERROR,"sendLogEndToIpcManage strResult %s\n",strResult.c_str());
    }

    mxbool CLogManageUpload::isSendSuccess(std::string strResult)
    {
        if(strResult.length() > 0)
        {
            std::string strCode;
            std::string strErrMsg;
            cJSON *jsonRoot = cJSON_Parse(strResult.c_str());
            if (jsonRoot)
            {
                cJSON *jsonCode = cJSON_GetObjectItem(jsonRoot, "code");
                if (jsonCode)
                {
                    strCode = std::string(jsonCode->valuestring);
                }
                else
                {
                    cJSON_Delete(jsonRoot);
                    return mxfalse;
                }
                cJSON *jsonErrMsg = cJSON_GetObjectItem(jsonRoot, "errMsg");
                if (jsonErrMsg)
                {
                    char *pcErrMsg = cJSON_Print(jsonErrMsg);
                    if (pcErrMsg)
                    {
                        strErrMsg = std::string(pcErrMsg);
                        free(pcErrMsg);
                    }

                }
                cJSON_Delete(jsonRoot);
                if(strCode.compare("200") == 0)
                {
                    return mxtrue;
                }
                else
                {
                    return mxfalse;
                }
            }
            else
            {
                return mxfalse;
            }
        }
        else
        {
            return mxfalse;
        }
    }

    int CLogManageUpload::checkTmpFilePosition(std::string strLogDir,std::string strLogPreName,std::string tmpFilePath)
    {
        std::string numPrePath = "";
        numPrePath.append(strLogDir);
        numPrePath.append(strLogPreName);
        std::string numBehindPath = LOG_FILE_TMP_BEHIND_NAME;
        std::string position = tmpFilePath.substr(numPrePath.length(),tmpFilePath.length() - numPrePath.length() - numBehindPath.length());
        return atoi(position.c_str());
    }

    std::string CLogManageUpload::montageLogPath(std::string strDir,std::string strPreName,int iNum,std::string strBehindName)
    {
        char buf[100] = { 0 };
        snprintf(buf, sizeof(buf), "%s%s%d%s", strDir.c_str(),strPreName.c_str(),iNum,strBehindName.c_str());
        return std::string(buf);
    }

    mxbool CLogManageUpload::isFileExit(std::string filePath)
    {
        return (access(filePath.c_str(),F_OK) == 0);
    }

    std::string CLogManageUpload::procResult(std::string code, 
        std::string strMsg, std::string strErr)
    {
        std::string strResult;
        cJSON *jsonRoot = cJSON_CreateObject();
        cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
        cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
        cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
        cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
        char *pcResult = cJSON_Print(jsonRoot);
        strResult = std::string(pcResult);
        cJSON_Delete(jsonRoot);
        if (pcResult)
            free(pcResult);

        return strResult;
    }

}