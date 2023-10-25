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
	*	����������
	*		��һ֡����Ƶ����֡���������RTP���ݰ�
	*/
	int PushCommFrm(unsigned char *pFrm, int nFrmLen, unsigned int uiTimeStamp);

	/*
	*	����������
	*		��ȡ����õ�RTP���ݰ���ͨ��һ֡����������RTP������Ҫ��λ�ȡ
	*/
	int PopRTPCommPacket(unsigned char *pRTP, int *pMaxRTPLen);

protected:

	unsigned int uiSSRC_;
	unsigned char PayLoadType_;
	int nMaxRTPLen_;

	//RTP������
	unsigned short usSeqNum_;
	MEDIADATA_ARR data_arr_;

	unsigned int uiVideoLastTimestamp_;
	unsigned int uiVideoCurTimestamp_;
};


#endif/*__CR_RTPPACKETCOMM_H__*/
