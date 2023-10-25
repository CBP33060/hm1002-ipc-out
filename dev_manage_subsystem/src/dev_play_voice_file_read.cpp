#include "dev_play_voice_file_read.h"
#include "log_mx.h"
#include <algorithm> 
#include <unistd.h>
#include <stdio.h>
#include <string.h>

namespace maix {
    CDevPlayVoiceFileRead::CDevPlayVoiceFileRead()
        : m_bInit(mxfalse)
        , m_bRun(mxfalse)
        ,m_funCallback(NULL)
        ,m_bPowerUpPlayState(mxfalse)
    {
    }

    CDevPlayVoiceFileRead::~CDevPlayVoiceFileRead()
    {
    }

    mxbool CDevPlayVoiceFileRead::init()
    {
        m_threadPlay = std::thread([this]() {
            //mcu与media_soure连接需要一定的时间，开机音在mcu连接之后才会播报，
            usleep(1 * 1000 * 1000);
            this->run();
        });
        m_bInit = mxtrue;
        return mxtrue;
    }

    mxbool CDevPlayVoiceFileRead::unInit()
    {
        m_bRun = mxfalse;
        m_bInit = mxfalse;
        return mxtrue;
    }

    void CDevPlayVoiceFileRead::registCallback(PlayCallback funCallback)
    {
        m_funCallback = funCallback;
    }

    void CDevPlayVoiceFileRead::registPowerUpCallback(PowerUpCallback callback)
    {
        m_powerUpCallback = callback;
    }

    mxbool CDevPlayVoiceFileRead::playWithFile(std::string strFilePath , int iLevel,int playTime)
    {
        std::unique_lock<std::mutex> lock(m_mutexPlay);
        if(m_dqPlayFileList.size() > 100)
        {
            return mxfalse;
        }
        T_VOICE_PLAY_INFO playInfo;
        playInfo.m_strFilePath = strFilePath;
        playInfo.m_eLevel = getPlayLevel(iLevel);
        playInfo.m_iPlayTime = playTime;
        playInfo.m_lCurrentPlayTime = 0;
        m_dqPlayFileList.push_front(playInfo);
        sort(m_dqPlayFileList.begin(),m_dqPlayFileList.end(),std::bind(&CDevPlayVoiceFileRead::playFileCompare,this,std::placeholders::_1, std::placeholders::_2));
        m_conditionPlay.notify_one();
        return mxtrue;
    }

    mxbool CDevPlayVoiceFileRead::stopWithFile(std::string strFilePath)
    {
        std::unique_lock<std::mutex> lock(m_mutexPlay);
        logPrint(MX_LOG_INFOR, "stopWithFilePath : [%s] ", strFilePath.c_str());

        if(!m_strStopPath.empty())
        {
            m_strStopPath = "";
        }
        m_strStopPath.append(strFilePath);
        for (std::deque<T_VOICE_PLAY_INFO>::iterator it = m_dqPlayFileList.begin(); it != m_dqPlayFileList.end();it++)
        {
            if(it->m_strFilePath.compare(m_strStopPath) == 0)
            {
                logPrint(MX_LOG_INFOR, "stopWithFileId erase : [%s] ", it->m_strFilePath.c_str());
                m_dqPlayFileList.erase(it);
            }
        }
        return mxtrue;
    }

    void CDevPlayVoiceFileRead::run()
    {
        m_bRun = mxtrue;
        while (m_bRun)
        {

            if(!m_bPowerUpPlayState)
            {
                m_bPowerUpPlayState = m_powerUpCallback();
                logPrint(MX_LOG_ERROR,"m_bPowerUpPlayState : %d",m_bPowerUpPlayState);
                usleep(200 * 1000);
                continue;
            }

            T_VOICE_PLAY_INFO playInfo;
            {
                std::unique_lock<std::mutex> lock(m_mutexPlay);
                while(m_dqPlayFileList.empty())
                {
                    m_conditionPlay.wait(lock);
                }

                playInfo = m_dqPlayFileList.front();
                m_dqPlayFileList.pop_front();
            }
            logPrint(MX_LOG_ERROR,"current play file : %s",playInfo.m_strFilePath.c_str());
            playWithFileOPUS(playInfo);
        }
    }

    void CDevPlayVoiceFileRead::playWithFilePCM(std::string strPath)
    {
        int readBufSize = 640;
        unsigned char *inBuf = NULL;
        inBuf = (unsigned char *)calloc(readBufSize,sizeof(short));
        if (inBuf == NULL) {
            return;
        }

        FILE *play_file = fopen(strPath.c_str(), "rb");
        if (play_file == NULL) {
            free(inBuf);
            return;
        }

        size_t num_read;

        mxbool needPlay = true;
        while (needPlay)
        {
            num_read = fread(inBuf, 1, readBufSize*sizeof(short), play_file);
            if (num_read <= 0)
            {
                needPlay = false;                
                break;
            }
            if(m_funCallback != NULL)
            {
                m_funCallback(inBuf, readBufSize*sizeof(short), E_P_AUDIO_PCM,E_LEVEL_LOWEST);
            }
            usleep(40 * 1000);
        }

        
        free(inBuf);
        fclose(play_file);
    }

    void CDevPlayVoiceFileRead::playWithFileOPUS(T_VOICE_PLAY_INFO playInfo)
    {
        int readBufSize = 640;
        unsigned char *inBuf = NULL;
        inBuf = (unsigned char *)malloc(readBufSize * sizeof(short));
        if (inBuf == NULL) 
        {
            return;
        }
        memset(inBuf, 0, sizeof(readBufSize * sizeof(short)));

        if(access(playInfo.m_strFilePath.c_str(),F_OK) != 0)
        {
            logPrint(MX_LOG_ERROR,"play file not exsit: %s",playInfo.m_strFilePath.c_str());
            free(inBuf);
            return;
        }

        FILE *play_file = fopen(playInfo.m_strFilePath.c_str(), "rb");
        if (play_file == NULL) {
            free(inBuf);
            return;
        }
        
        //unsigned int encFinalRange;
        size_t num_read;
        int decodeLen;
        mxbool needPlay = true;
        while (needPlay)
        {
            if(!m_strStopPath.empty() && playInfo.m_strFilePath.compare(m_strStopPath) == 0)
            {
                break;
            }

            unsigned char ch[4];
            num_read = fread(ch, 1, 4, play_file);
            if (num_read!=(4))
            {
                needPlay = false;
                break;
            }
            decodeLen = char_to_int(ch);
            if (decodeLen>1280 || decodeLen<0)
            {
                needPlay = false;
                break;
            }
            num_read = fread(ch, 1, 4, play_file);
            if (num_read!=4)
            {
                needPlay = false;
                break;
            }  
            //encFinalRange = char_to_int(ch);
            num_read = fread(inBuf, 1, decodeLen, play_file);
            if (num_read!=(size_t)decodeLen)
            {
                needPlay = false;                
                break;
            }

            if(m_funCallback != NULL)
            {
                m_funCallback(inBuf, decodeLen, E_P_AUDIO_OPUS,playInfo.m_eLevel);
            }
            usleep(19 * 1000);
            playInfo.m_lCurrentPlayTime += 20;
        }
      
        free(inBuf);
        fclose(play_file);

        if(!m_strStopPath.empty() && playInfo.m_strFilePath.compare(m_strStopPath) == 0)
        {
            logPrint(MX_LOG_INFOR, "play audio stop : [%s] ", playInfo.m_strFilePath.c_str());
            m_strStopPath = "";
            return;
        }
        if(playInfo.m_iPlayTime > 0)
        {
            if((playInfo.m_iPlayTime * 1000) > playInfo.m_lCurrentPlayTime)
            {
                std::unique_lock<std::mutex> lock(m_mutexPlay);
                m_dqPlayFileList.push_front(playInfo);
                sort(m_dqPlayFileList.begin(),m_dqPlayFileList.end(),std::bind(&CDevPlayVoiceFileRead::playFileCompare,this,std::placeholders::_1, std::placeholders::_2));
            }
        }

    }

    uint32_t CDevPlayVoiceFileRead::char_to_int(unsigned char ch[4])
    {
        return ((uint32_t)ch[0]<<24) | ((uint32_t)ch[1]<<16)
            | ((uint32_t)ch[2]<< 8) |  (uint32_t)ch[3];
    }


    bool CDevPlayVoiceFileRead::playFileCompare(T_VOICE_PLAY_INFO first,T_VOICE_PLAY_INFO second)
    {
        return first.m_eLevel > second.m_eLevel;
    }

    E_VOICE_PLAY_LEVEL CDevPlayVoiceFileRead::getPlayLevel(int level)
    {
        if(level < E_LEVEL_LOWEST)
        {
            return E_LEVEL_LOWEST;
        }
        else if (level > E_LEVEL_HIGHEST)
        {
            return E_LEVEL_HIGHEST;
        }
        else 
        {
            return (E_VOICE_PLAY_LEVEL) level;
        }
    }

}