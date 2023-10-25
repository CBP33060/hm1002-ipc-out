#include "speaker_decode_ctrl.h"
#include "speaker_decode_opus.h"
#include "speaker_decode_pcm.h"

#define DECODE_MAP_KEY_PCM "speaker_decode_pcm"
#define DECODE_MAP_KEY_OPUS "speaker_decode_opus"
#define DECODE_MAP_KEY_AAC "speaker_decode_aac"

namespace maix {

    CSpeakerDecodeCtrl::CSpeakerDecodeCtrl()
    {
    }

    CSpeakerDecodeCtrl::~CSpeakerDecodeCtrl()
    {
    }

    /**
     * @brief 解码函数
     * 
     * @param decodeType    解码类型
     * @param pInData       需要解码的数据
     * @param inLen         解码数据长度
     * @param pOutData      解码后的数据
     * @return int          解码后的数据长度  长度大于0,解码成功
     */
    int CSpeakerDecodeCtrl::decode(E_P_TYPE decodeType,uint8_t *pInData,int inLen,uint8_t * pOutData)
    {
        std::lock_guard<std::mutex> lck(m_mutexDecode);
        int decodeLen = -1;
        for (auto iter = m_mapSpeakerModel.begin(); iter != m_mapSpeakerModel.end(); iter++)
        {
            if(iter->second->getDecodeType() != decodeType)
            {
                if(iter->second->isDecodeCreat())
                {
                    iter->second->decodeDestory();
                }
            }
        }
        CSpeakerDecodeModel* decodeModel = NULL;
        switch (decodeType)
        {
            case E_P_AUDIO_OPUS:
                {
                    auto model = m_mapSpeakerModel.find(DECODE_MAP_KEY_OPUS);
                    if(model == m_mapSpeakerModel.end())
                    {
                        m_mapSpeakerModel[DECODE_MAP_KEY_OPUS] = new CSpeakerDecodeOpus();
                    }
                    decodeModel = m_mapSpeakerModel[DECODE_MAP_KEY_OPUS];
                }
                break;
            case E_P_AUDIO_AAC:
                break;
            case E_P_AUDIO_PCM:
                {
                    auto model = m_mapSpeakerModel.find(DECODE_MAP_KEY_PCM);
                    if(model == m_mapSpeakerModel.end())
                    {
                        m_mapSpeakerModel[DECODE_MAP_KEY_PCM] = new CSpeakerDecodePcm();
                    }
                    decodeModel = m_mapSpeakerModel[DECODE_MAP_KEY_PCM];
                }
                break;
            case E_P_NULL:
            case E_P_VIDEO_YUV:
            case E_P_VIDEO_H264:
            case E_P_VIDEO_H265:
            case E_P_VIDEO_JPEG:
                break;

        }
        if(decodeModel)
        {
            if(!decodeModel->isDecodeCreat())
            {
                decodeModel->decodeCreat();
            }
            decodeLen = decodeModel->decode(pInData,inLen,pOutData);
        }
        return decodeLen;
    }

}
