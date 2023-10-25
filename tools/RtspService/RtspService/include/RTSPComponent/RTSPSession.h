#ifndef __CR_RTSPSESSION_H__
#define __CR_RTSPSESSION_H__

#include "RTSPCommon.h"

class RTSPSession
{
public:
	RTSPSession(SOCKET s, unsigned short nMediaPort, int nSetTrackNumTimeOutInterval, int nSDPTimeOutInterval);
	~RTSPSession();
	int Run();
	//应该有WOULDBLOCK, ERROR, OK
	int GetURL(unsigned char *pULR, int *pMaxURLLen);
	//一般应该在URL中含有是否伴音的信息，或者是应用层的一个配置。应用层尽快得知是否有伴音，来设置Track的数目。
	//目前我们只设置1（1路视频）和2（1路视频加1路音频）。其余情况都出错。
	//如果设置成-1，就说明媒体流获取有困难，需要断掉这个链接。
	int SetTrackNum(int TrackNum); //这函数必须在GetURL成功后SetTrackNumTimeOutInterval内必须设置，否则断开这个连接 
	//BandWidth是码流带宽，单位kbps，如果0，就不用出现在SDP中
	//FrmRate是帧率。如果0，就不用出现在SDP中
	//RangeBegin, RangeEnd是播放的时间段， 数据类型待确定。如果两个值都是0，说明是播放实时视频。用now-
	int AddVideoTrack(int TrackID, unsigned char *pES, int ESLen, int FrmRate, unsigned short video_media_port);
	int AddAudioTrack(int TrackID, int nRFCAlgID);
	//一般情况下，AddXXXTrack完毕后，对象内部应该生成SDP。但我们也导出一个接口，允许设置自己的SDP.
	//这个函数只能和AddXXXTrack函数互斥使用。（因为一旦生成好SDP文件，底层就通过网络将Describe响应发出去了）
	int SetPrivateSDP(char *pSDP, int VideoTrackID, int VideoRFCAlg, int AudioTrackID, int AudioRFCAlg);

	//应该有WOULDBLOCK, ERROR, OK
	//VideoClientPort是客户端接收端口
	//pVideoTrackValidFlg返回1，表明后面关于视频的信息有效
	//pAudioTrackValidFlg返回1，表明后面关于音频的信息有效
	//ClientIP和ClientPort是客户端接收RTP包的端口。RTCP端口一般就是RTP端口加1
	int GetPlayTrackInfo(int *pVideoTrackValidFlg, unsigned short *pVideoMediaPort, unsigned int *pVideoClientIP, unsigned short *pVideoClientPort, int *pVideoPayLoad, unsigned int *pVideoSSRC,
		int *pAudioTrackValidFlg, unsigned int *AudioClientIP, unsigned short *pAudioClientPort, int *pAudioPayLoad, unsigned int *pAudioSSRC, bool *IsTCP);

	int GetSessionStatus(); //查询这个Session是否还连着。随时可以查询

	/*
		在点播的时候，文件结束了，发送此命令
	*/
	int EndofFile();

protected:

	int RecvData();

	int SendData();

	int AnalyseRecvData();

	int AnalyseOptions();

	int AnalyseDescribe();

	//合成Describe的响应
	int DescribeRsp();

	int AnalyseSetup();

	int AnalysePlay();

	int AnalyseSetParameter();

	int AnalysePause();

	int AnalyseGetParameter();

	//获取请求报文中的CSeq
	int GetCSeq();

	//解析SETUP时的客户端的数据端口
	int GetClientPort(int &ClientRTPPort, int &ClientRTCPPort);

	//获取请求的TrackID(SETUP)
	int GetTrackID(const char* pReq);

	//将SSRC变成字符串形式
	bool SSRCtoStr(char* pszSSRC, unsigned int uiSSRC);

	//获取一个新的SSRC
	int GetSSRC(unsigned int &uiSSRC);

	//通过nAlgID获取i算法名称
	int GetAlgName(int nAlgID, char* pszAlgName);

	//通过H264关键帧数据获取PPS和SPS
	int GetPPSAndSPS(unsigned char* pH264ES, int nEsLen, char* pszPPS, char* pszSPS);

	int GetTimeFromUrl(const char *pUrl, unsigned int &uiBeginTime, unsigned int &uiEndTime);

	//从H264数据中找到一个完整的NUAL单元, 返回值是这个NUAL的起始， nLen是其长度
	unsigned char* FindUNAL(unsigned char* pData, int nEslen, unsigned char FlagCode, int& nLen);
public:
	int GetPause(bool &bPause)
	{
		if (bPauseValidFlag_)
		{
			bPause = bPauseValidFlag_;
			bPauseValidFlag_ = false;
			return 0;
		}
		return 1;
	}
	int GetPlay(bool &bPlay)
	{
		if (bPlayValidFlag_)
		{
			bPlay = bPlayValidFlag_;
			bPlayValidFlag_ = false;
			return 0;
		}
		return 1;
	}
	int GetSock() { return sock; }

	int SetLoaclIP(const char *pszLocalIP)
	{
		memset(szLocalIP, 0, 16);
		memcpy(szLocalIP, pszLocalIP, 15);
		return 0;
	}
protected:
	SOCKET sock;

	char szRecvBuf[5 * 1024];
	int  nRecvLen;
	int  nRecvTotalLen;
	bool bRecvHead;

	char szSendBuf[5 * 1024];
	int  nSendLen;
	int  nSendTotalLen;
	bool bSendFinish;
	bool bStopRecvSend;//即不接受数据也不发送数据
	int nSessionStatus_;

	char szLocalIP[16];
	bool bURLOK;
	char szURL[1024];
	char szDescribeSDP[2048];
	char szVideoSDP[1024];
	char szAudioSDP[1024];
	int  nVideoTrackID;//小于零表示无效
	int  nAudioTrackID;//小于零表示无效
	int  nAudioPayLoad;
	int  nVideoPayLoad;
// 	char szBeginTime[16];
// 	char szEndTime[16];
	int nTrackNum;
	bool bSetTrackNum;
	int  nDescribeCSeq;

	bool bPrivateSDP;
	char szPrivateSDP[4096];

	bool bReqVideo;//是否请求了视频
	bool bReqAudio;//是否请求了音频
	bool bRecvPlay;
	char szSessionID[32];
	double dScale;//播放速度

	//播放速度
// 	bool  bScaleValidFlag_;
// 	double dbScale_;

// 	bool  bRangeFlagValid_;
// 	double dbRangeBegin_;
// 	double dbRangeEnd_;

	bool bPauseValidFlag_;
	bool bPlayValidFlag_;
	bool bIsTCP_;
	unsigned int uiClientIP_;
	char szClientIP_[RTSP_MAX_IPSTR_LEN];
	unsigned short usVideoPort_;
	unsigned short usAudioPort_;
	unsigned int uiVideoSSRC_;
	unsigned int uiAudioSSRC_;

	int nMediaPort_;
	int nSetTrackNumTimeOutInterval_;
	int nSDPTimeOutInterval_;

	bool bStopFlag_;  // 停止标记
	bool bTeardownFlag_;  // 停止标记

	unsigned short video_media_port_;
	unsigned short audio_media_port_;
};


#endif/*__CR_RTSPSESSION_H__*/
