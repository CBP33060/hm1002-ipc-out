#ifndef __SPEAKER_DECODE_CTRL_H__
#define __SPEAKER_DECODE_CTRL_H__

#include "media_frame_packet.h"
#include <string>
#include "module.h"
#include "speaker_decode_model.h"

namespace maix {

    class  CSpeakerDecodeCtrl
    {
    public:
        ~CSpeakerDecodeCtrl();

        CSpeakerDecodeCtrl(const CSpeakerDecodeCtrl&) = delete;
        CSpeakerDecodeCtrl& operator = (const CSpeakerDecodeCtrl&) = delete;
        static CSpeakerDecodeCtrl& getInstance(){
            static CSpeakerDecodeCtrl instance;
            return instance;
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
        int decode(E_P_TYPE decodeType,uint8_t *pInData,int inLen,uint8_t * pOutData);

    private:
        CSpeakerDecodeCtrl();
        mutable std::mutex m_mutexDecode;
        std::map<std::string, CSpeakerDecodeModel*> m_mapSpeakerModel;
    };
}
#endif //__SPEAKER_DECODE_CTRL_H__
