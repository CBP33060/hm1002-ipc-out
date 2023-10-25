#ifndef __CR_RTPPACKETH264_H__
#define __CR_RTPPACKETH264_H__

#include "RTSPCommon.h"
#include <vector>

class RTPPacketH264
{
public:
	typedef std::vector<MediaDataPacket*> MEDIADATA_ARR;

	RTPPacketH264(unsigned int uiSSRC, unsigned char PayLoadType, int nMaxRTPLen, int FrmRate);

	~RTPPacketH264();

	/*
	*	功能描述：
	*		将一帧裸H264数据帧打包成若干RTP数据包
	*/
	int PushH264ES(unsigned char *pES, int nESLen, unsigned int uiTimeStamp);
	int PushH265ES(unsigned char *pES, int nESLen, unsigned int uiTimeStamp);

	/*
	*	功能描述：
	*		获取打包好的RTP数据包，通常一帧数据有若干RTP包，需要多次获取
	*/
	int PopRTPPacket(unsigned char *pRTP, int *pMaxRTPLen);

protected:

	char* GetNUAL(char *pData, int nLen, NALU_t *nalu);

protected:
	
	unsigned int uiSSRC_;
	unsigned char PayLoadType_;
	int nMaxRTPLen_;

	//RTP打包相关
	unsigned short usSeqNum_;
	unsigned int uiTimestampIncrese_;
	unsigned int uiCurrentTimestamp_;

	MEDIADATA_ARR data_arr_;
};

#endif/*__CR_RTPPACKETH264_H__*/

