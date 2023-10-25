#ifndef __SPEAKER_DECODE_MODEL_H__
#define __SPEAKER_DECODE_MODEL_H__
#include <string>
#include "module.h"
#include "media_frame_packet.h"
#include "log_mx.h"

namespace maix {

    class CSpeakerDecodeModel
    {
    public:
        CSpeakerDecodeModel();
        virtual ~CSpeakerDecodeModel();

        virtual mxbool decodeCreat();
        virtual mxbool decodeDestory();
        /**
         * @brief 解码函数
         * 
         * @param pInData   需要解码的数据
         * @param inLen     解码数据长度
         * @param pOutData  解码后的数据
         * @return int      解码后的数据长度  长度大于0,解码成功
         */
        virtual int decode(uint8_t *pInData,int inLen,uint8_t * pOutData);
        /**
         * @brief       判断解码器是否已经创建
         * 
         * @return mxbool 
         */
        virtual mxbool isDecodeCreat();

        virtual E_P_TYPE getDecodeType();

    private:
        
    };
}
#endif //__SPEAKER_DECODE_MODEL_H__
