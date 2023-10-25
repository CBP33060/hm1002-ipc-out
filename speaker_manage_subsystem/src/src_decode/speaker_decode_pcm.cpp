#include "speaker_decode_pcm.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

namespace maix {

    CSpeakerDecodePcm::CSpeakerDecodePcm()
    {
        m_bInit = false;
    }

    CSpeakerDecodePcm::~CSpeakerDecodePcm()
    {
        m_bInit = false;
    }

    mxbool CSpeakerDecodePcm::decodeCreat()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        logPrint(MX_LOG_INFOR, "pcm_creat");
        if(m_bInit)
        {
            logPrint(MX_LOG_INFOR, "pcm has been creat");
            return false;
        }
        m_bInit = true;
        return true;
    }

    mxbool CSpeakerDecodePcm::decodeDestory()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        logPrint(MX_LOG_INFOR, "pcm_destroy");
        if(!m_bInit)
        {
            logPrint(MX_LOG_INFOR, "pcm destroy err : not init");
            return false;
        }
        m_bInit = false;
        return true;
    }

    /**
     * @brief 解码函数
     * 
     * @param pInData   需要解码的数据
     * @param inLen     解码数据长度
     * @param pOutData  解码后的数据
     * @return int      解码后的数据长度
     */
    int CSpeakerDecodePcm::decode(uint8_t *pInData,int inLen,uint8_t * pOutData)
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        int resultLen = -1;
        if(!m_bInit)
        {
            logPrint(MX_LOG_ERROR, "pcm_decode: not create\n");
            return resultLen;
        }

        if(!pInData || !pOutData || inLen <= 0)
        {
            logPrint(MX_LOG_ERROR, "pcm_decode err : decode data err\n");
            return resultLen;
        }
        memcpy(pOutData, pInData, inLen);
        resultLen = inLen;
        return resultLen;
    }

    /**
     * @brief       判断解码器是否已经创建
     * 
     * @return mxbool 
     */
    mxbool CSpeakerDecodePcm::isDecodeCreat()
    {
        return m_bInit;
    }

    E_P_TYPE CSpeakerDecodePcm::getDecodeType()
    {
        return E_P_AUDIO_PCM;
    }

	
}
