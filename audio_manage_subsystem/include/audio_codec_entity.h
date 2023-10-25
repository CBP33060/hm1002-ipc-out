#ifndef __MX_VOICE_CODEC_ENTITY_HPP__
#define __MX_VOICE_CODEC_ENTITY_HPP__

// #include "MXVoiceManageComm.hpp"
// #include "MXUsageEnvironment.hpp"
#include <stdint.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "opus.h"
#include "type_def.h"
#include "log_mx.h"

#define MX_DECODE_RATE 16000
#define MX_DECODE_BIT 16
#define MX_DECODE_CHANNEL 1
#define MX_DECODE_MAX_FRAME_SIZE 2560  //16000*16/8*1/40 = 1280byte (1秒采样40次),为确保安全，预留一倍的空间


namespace maix {

    class CAudioCodecEntity
    {
    public:
        CAudioCodecEntity();
        ~CAudioCodecEntity();

        mxbool initOpusDecode();
        mxbool unInitOpusDecode();
        int opusDecode(uint8_t * pInData, int inLen, uint8_t *pOutData,int inEncFinalRange);

        mxbool initOpusEncode();
        mxbool unInitOpusEncode();
        int opusEncode(int16_t * pInData, int inLen, uint8_t *pOutData, int iOutLen);
    private:
        mxbool m_bDecodeInit;
        std::mutex m_decodeMutex;
        OpusDecoder *m_pOpusDecoder;

        mxbool m_bEncodeInit;
        std::mutex m_encodeMutex;
        OpusEncoder *m_pOpusEncoder;


    };
}

#endif /* __MX_VOICE_CODEC_ENTITY_HPP__ */