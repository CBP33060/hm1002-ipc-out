#ifndef __CR_RTPPACKETCOMM_H__
#define __CR_RTPPACKETCOMM_H__

#include "RTSPCommon.h"
#include <vector>

class RTPPacketComm
{
public:
	typedef std::vector<MediaDataPacket*> MEDIADATA_ARR;

	RTPPacketComm(unsigned int uiSSRC, unsigned char PayLoadType, int nMaxRTPLen);

	~RTPPacketComm();

	/*
	*	功能描述：
	*		将一帧音视频数据帧打包成若干RTP数据包
	*/
	int PushCommFrm(unsigned char *pFrm, int nFrmLen, unsigned int uiTimeStamp);

	/*
	*	功能描述：
	*		获取打包好的RTP数据包，通常一帧数据有若干RTP包，需要多次获取
	*/
	int PopRTPCommPacket(unsigned char *pRTP, int *pMaxRTPLen);

protected:

	unsigned int uiSSRC_;
	unsigned char PayLoadType_;
	int nMaxRTPLen_;

	//RTP打包相关
	unsigned short usSeqNum_;
	MEDIADATA_ARR data_arr_;

	unsigned int uiVideoLastTimestamp_;
	unsigned int uiVideoCurTimestamp_;
};


#endif/*__CR_RTPPACKETCOMM_H__*/
