#include "speaker_decode_model.h"


namespace maix {

    CSpeakerDecodeModel::CSpeakerDecodeModel()
    {
    }

    CSpeakerDecodeModel::~CSpeakerDecodeModel()
    {
    }

    mxbool CSpeakerDecodeModel::decodeCreat()
    {
        return false;
    }

    mxbool CSpeakerDecodeModel::decodeDestory()
    {
        return false;
    }

    /**
     * @brief 解码函数
     * 
     * @param pInData   需要解码的数据
     * @param inLen     解码数据长度
     * @param pOutData  解码后的数据
     * @return int      解码后的数据长度
     */
    int CSpeakerDecodeModel::decode(uint8_t *pInData,int inLen,uint8_t * pOutData)
    {
        return -1;
    }

    /**
     * @brief       判断解码器是否已经创建
     * 
     * @return mxbool 
     */
    mxbool CSpeakerDecodeModel::isDecodeCreat()
    {
        return false;
    }

    E_P_TYPE CSpeakerDecodeModel::getDecodeType()
    {
        return E_P_NULL;
    }

	
}
