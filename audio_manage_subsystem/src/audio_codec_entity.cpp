#include "audio_codec_entity.h"

namespace maix {

    CAudioCodecEntity::CAudioCodecEntity()
    {
        m_bDecodeInit = mxfalse;
        m_pOpusDecoder = NULL;

        m_bEncodeInit = mxfalse;
        m_pOpusEncoder = NULL;
    }

    CAudioCodecEntity::~CAudioCodecEntity()
    {
        
    }

    mxbool CAudioCodecEntity::initOpusDecode()
    {
        std::unique_lock<std::mutex> lck(m_decodeMutex);

        logPrint(MX_LOG_INFOR, "opus_creat\n");
        int iErr = 0;
        m_pOpusDecoder = opus_decoder_create(MX_DECODE_RATE, MX_DECODE_CHANNEL, &iErr);
        if ((iErr != OPUS_OK) || (m_pOpusDecoder == NULL))
        {
            logPrint(MX_LOG_INFOR, "opus decoder create failed:%s", opus_strerror(iErr));
            return mxfalse;
        }
        m_bDecodeInit = mxtrue;

        return mxtrue;
    }

    mxbool CAudioCodecEntity::unInitOpusDecode()
    {
        std::unique_lock<std::mutex> lck(m_decodeMutex);
        logPrint(MX_LOG_INFOR, "opus_destroy\n");
        opus_decoder_destroy(m_pOpusDecoder);

        m_bDecodeInit = mxfalse;

        return mxtrue; 
    }

    int CAudioCodecEntity::opusDecode(uint8_t * pInData, int inLen, uint8_t *pOutData, int inEncFinalRange)
    {
        std::unique_lock<std::mutex> lck(m_decodeMutex);
        int resultLen = -1;
        if(!m_bDecodeInit)
        {
            logPrint(MX_LOG_ERROR, "opus_decode: not create\n");
            return resultLen;
        }

        unsigned int decFinalRange;
        short *out = (short*)malloc(MX_DECODE_MAX_FRAME_SIZE * MX_DECODE_CHANNEL * sizeof(short));
        opus_int32 output_samples = opus_decode(m_pOpusDecoder, pInData, inLen, out, MX_DECODE_MAX_FRAME_SIZE, 0);
        if (output_samples>0)
        {
            int i;
            for(i = 0; i < (output_samples) * MX_DECODE_CHANNEL; i++)
            {
                short s;
                s = out[i];
                pOutData[2 * i] = s&0xFF;
                pOutData[2 * i + 1] = (s >> 8) & 0xFF;
            }
            resultLen = output_samples * sizeof(short) * MX_DECODE_CHANNEL;
        } else {
            logPrint(MX_LOG_ERROR, "error decoding frame %d \n",output_samples);
        }
        opus_decoder_ctl(m_pOpusDecoder, OPUS_GET_FINAL_RANGE(&decFinalRange));
        /* compare final range encoder rng values of encoder and decoder */
        if( inEncFinalRange !=0  && decFinalRange != (unsigned int)inEncFinalRange) {
    //        resultLen = -1;
            logPrint(MX_LOG_ERROR, "Error: Range coder state mismatch inEncFinalRange %d  decFinalRange %d", inEncFinalRange, decFinalRange);
        }

        if (out) 
        {
            free(out);
            out = NULL;
        }

        return resultLen;
    }

    mxbool CAudioCodecEntity::initOpusEncode()
    {
        std::unique_lock<std::mutex> lck(m_encodeMutex);

        int iErr = 0;

        m_pOpusEncoder = opus_encoder_create(MX_DECODE_RATE, MX_DECODE_CHANNEL, OPUS_APPLICATION_VOIP, &iErr);
        if ((iErr != OPUS_OK) || (m_pOpusEncoder == NULL))
        {
            logPrint(MX_LOG_INFOR, "opus encoder create failed:%s", opus_strerror(iErr));
            return mxfalse;
        }
        m_bEncodeInit = mxtrue;
        logPrint(MX_LOG_DEBUG, "opus init encode success:%p\n", m_pOpusEncoder);
        return mxtrue;
    }

    mxbool CAudioCodecEntity::unInitOpusEncode()
    {
        std::unique_lock<std::mutex> lck(m_encodeMutex);

        logPrint(MX_LOG_INFOR, "opus encode destroy\n");
        opus_encoder_destroy(m_pOpusEncoder);

        m_bEncodeInit = mxfalse;
        return true;
    }

    int CAudioCodecEntity::opusEncode(opus_int16 * pInData, int inLen, uint8_t *pOutData, int iOutLen)
    {
        std::unique_lock<std::mutex> lck(m_encodeMutex);
        if(!m_bEncodeInit)
        {
            logPrint(MX_LOG_ERROR, "opus encode faied, encoder is not inited\n");
            return -1;
        }

        int iLen = opus_encode(m_pOpusEncoder, pInData, inLen, pOutData, iOutLen);
        if ((iLen < 0) || (iLen >  iOutLen))
        {
            logPrint(MX_LOG_ERROR, "opus encode failed, len is %d", iLen);
            return -1;
        }

        return iLen;
    }


    


}