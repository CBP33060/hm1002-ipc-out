#ifndef __SPEAKER_DECODE_OPUS_H__
#define __SPEAKER_DECODE_OPUS_H__

#include "speaker_decode_model.h"
#include "opus.h"

namespace maix
{
class CSpeakerDecodeOpus : public CSpeakerDecodeModel
{
public:
    CSpeakerDecodeOpus();
    ~CSpeakerDecodeOpus();

    mxbool decodeCreat();
    mxbool decodeDestory();
    /**
     * @brief 解码函数
     * 
     * @param pInData   需要解码的数据
     * @param inLen     解码数据长度
     * @param pOutData  解码后的数据
     * @return int      解码后的数据长度  长度大于0,解码成功
     */
    int decode(uint8_t *pInData,int inLen,uint8_t * pOutData);
    /**
     * @brief       判断解码器是否已经创建
     * 
     * @return mxbool 
     */
    mxbool isDecodeCreat();

    E_P_TYPE getDecodeType();

private:
    mutable std::mutex m_initMutex;
    mxbool m_bInit;
    OpusDecoder *dec=NULL;
};
}

#endif //__SPEAKER_DECODE_OPUS_H__
