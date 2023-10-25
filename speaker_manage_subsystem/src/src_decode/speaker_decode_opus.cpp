#include "speaker_decode_opus.h"

#define MX_DECODE_RATE 16000
#define MX_DECODE_BIT 16
#define MX_DECODE_CHANNEL 1
#define MX_DECODE_MAX_FRAME_SIZE 2560*2
#define MX_MAX_PACKET 1500

namespace maix {

    CSpeakerDecodeOpus::CSpeakerDecodeOpus()
    {
        m_bInit = false;
    }

    CSpeakerDecodeOpus::~CSpeakerDecodeOpus()
    {
        m_bInit = false;
    }

    mxbool CSpeakerDecodeOpus::decodeCreat()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        logPrint(MX_LOG_INFOR, "opus_creat");
        if(m_bInit)
        {
            logPrint(MX_LOG_INFOR, "opus has been creat");
            return false;
        }
        int err;
        dec = opus_decoder_create(MX_DECODE_RATE, MX_DECODE_CHANNEL, &err);
        if (err != OPUS_OK)
        {
            fprintf(stderr, "Cannot create decoder: %s\n", opus_strerror(err));
            return false;
        }
        m_bInit = true;
        return true;
    }

    mxbool CSpeakerDecodeOpus::decodeDestory()
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        logPrint(MX_LOG_INFOR, "opus_destroy");
        if(!m_bInit)
        {
            logPrint(MX_LOG_INFOR, "opus destroy err : not init");
            return false;
        }
        opus_decoder_destroy(dec);
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
    int CSpeakerDecodeOpus::decode(uint8_t *pInData,int inLen,uint8_t * pOutData)
    {
        std::lock_guard<std::mutex> lck(m_initMutex);
        int resultLen = -1;
        if(!m_bInit)
        {
            logPrint(MX_LOG_ERROR, "opus_decode: not create\n");
            return resultLen;
        }

        if(!pInData || !pOutData || inLen <= 0)
        {
            logPrint(MX_LOG_ERROR, "opus_decode err : decode data err\n");
            return resultLen;
        }

        unsigned int decFinalRange;
        short *out = (short*)malloc(MX_DECODE_MAX_FRAME_SIZE*MX_DECODE_CHANNEL*sizeof(short));
        opus_int32 output_samples = opus_decode(dec, pInData, inLen, out, MX_DECODE_MAX_FRAME_SIZE, 0);
        if (output_samples>0)
        {
            int i;
            for(i=0;i<(output_samples)*MX_DECODE_CHANNEL;i++)
            {
                short s;
                s=out[i];
                pOutData[2*i]=s&0xFF;
                pOutData[2*i+1]=(s>>8)&0xFF;
            }
            resultLen = output_samples * sizeof(short)*MX_DECODE_CHANNEL;
        } else {
            logPrint(MX_LOG_ERROR, "error decoding frame %d \n",output_samples);
        }
        opus_decoder_ctl(dec, OPUS_GET_FINAL_RANGE(&decFinalRange));
        free(out);
        return resultLen;
    }

    /**
     * @brief       判断解码器是否已经创建
     * 
     * @return mxbool 
     */
    mxbool CSpeakerDecodeOpus::isDecodeCreat()
    {
        return m_bInit;
    }

    E_P_TYPE CSpeakerDecodeOpus::getDecodeType()
    {
        return E_P_AUDIO_OPUS;
    }

	
}
