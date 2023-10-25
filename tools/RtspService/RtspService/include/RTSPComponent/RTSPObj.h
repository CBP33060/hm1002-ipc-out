/** 
 * @file	RTSPObj.h
 * @brief	
 * 
 * 具体实现RTSP命令之间的交互
 * 
 * @author	
 * @version	1.0
 * @date	
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>	<td>	<td>Create this file</tr>
 * </table>
 * 
 */

#ifndef __ING_RTSPOBJ_H__
#define __ING_RTSPOBJ_H__


typedef void* RTSP_HANDL;
typedef void* SESSION_HANDLE;

#define RTSP_ERROR_OK 0
#define RTSP_ERROR_WOULDBLOCK 1

#define RTSP_ERROR_UNKNOWN -1
#define RTSP_ERROR_TIMEOUT  -2
#define RTSP_ERROR_CONNECTION_BREAK  -3
#define RTSP_ERROR_INVALID_HANDLE -4
#define RTSP_ERROR_INVALID_PARAM -5
#define RTSP_ERROR_MEM_OVERLOAD -6
#define RTSP_ERROR_OPERATION -7

enum RFC_AUDIO_ALG
{
	rfc_alg_pcmu	= 1,
	rfc_alg_g723	= 4,
	rfc_alg_adpcm	= 5,
	rfc_alg_pcma	= 8,
	rfc_alg_g722	= 9,
};

/*
 *	功能描述：
 *		创建一个RTSP服务器，并监听@nListenPort端口；所有RTP数据，都是从@nMedioPort端口发送（复用本地端口，以便端口映射），
 *		RTP发送数据包，只支持基于UDP模式。
 *	参数说明：
 *		@nListenPort				[IN] 本地的TCP监听端口
 *		@nMedioPort					[IN] 在RTSP交互中，告诉客户端的本地发送RTP包的端口
 *		@nSetTrackNumTimeOutInterval [IN] 单位毫秒,设置TrackNum的超时时间，针对的是SESSION_HANDLE
 *		@nSDPTimeOutInterval			[IN] 单位毫秒,在调用RTSPSession::SetTrackNum函数后，如果这么长时间后，Session还不能产生自己的SDP文件，就认为超时，应该断开Session
 *
 *	返回值：
 *		NULL ：创建失败；其他值皆为成功
 */
RTSP_HANDL RTSP_Create(unsigned short nListenPort, unsigned short nMedioPortMin, unsigned short nMedioPortMax, int nSetTrackNumTimeOutInterval,
				  int nSDPTimeOutInterval);

/*
 *	功能描述：
 *		销毁一个RTSP服务
 *	参数说明：
 *		@hRTSPHandle				[IN] 需要关闭的RTSP服务句柄
 *	返回值：
 *		无返回值
 */
void RTSP_Destroy(RTSP_HANDL hRTSPHandle);

/*
 *	功能描述：
 *		RTSP驱动函数，调用者需不停地调用此函数。不支持多线程模式
 *	参数说明：
 *		@hRTSPHandle				[IN] RTSP服务句柄
 *	返回值：
 *		ERROR_OK：RTSP服务工作正常
 *		ERROR_UNKNOWN：工作异常，应该销毁@hRTSPHandle句柄
 */
int RTSP_Run(RTSP_HANDL hRTSPHandle);

/*
 *	功能描述：
 *		接受一个RTSP客户端的请求，并返回相应的Session句柄；调用者需不停地调用此函数。
 *	参数说明：
 *		@hRTSPHandle				[IN] RTSP服务句柄
 *	返回值：
 *		NULL：没有请求
 *		其他值：一个有效客户端请求
 */
SESSION_HANDLE RTSP_AcceptSession(RTSP_HANDL hRTSPHandle);

/*
 *	功能描述：
 *		销毁一个Session句柄。注意，销毁hSession时，其对应的数据通道也应该关闭
 *	参数说明：
 *		@hRTSPHandle				[IN] RTSP服务句柄
 *		@hSession					[IN] 待关闭Session句柄
 *	返回值：
 *		ERROR_OK：成功删除
 *		ERROR_INVALID_HANDLE：无效的句柄
 */
int RTSP_DeleteSession(RTSP_HANDL hRTSPHandle, SESSION_HANDLE hSession);

/*
 *	功能描述：
 *		查询一个Session的状态。
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_CR_TIMEOUT：超时
 *		ERROR_CONNECTION_BREAK：连接已断开
 */
int RTSP_GetSessionStatus(SESSION_HANDLE hSession); 

/*
 *	功能描述：
 *		获取Session请求的URL，根据URL分析出相应的资源
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@pULR						[OUT] 请求的URL地址
 *		@pMaxURLLen					[IN/OUT] pULR的最大长度，返回pULR的实际长度
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_WOULDBLOCK：没有获取到，需要重新获取
 */
int RTSP_GetURL(SESSION_HANDLE hSession, unsigned char *pszULR, int *pMaxURLLen);

/*
 *	功能描述：
 *		设置Session的Track数量。一般应该在URL中含有是否伴音的信息，或者是应用层的一个配置。
 *		应用层尽快得知是否有伴音，来设置Track的数目。目前我们只设置1（1路视频）和2（1路视频加1路音频）。
 *		其余情况都出错。如果设置成-1，就说明媒体流获取有困难，需要断掉这个链接（结束此次请求）。
 *		这函数必须在GetURL成功后SetTrackNumTimeOutInterval内必须设置，否则断开这个连接
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@TrackNum					[IN] Track数量，取值为1、2
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_INVALID_PARAM：重复设置，已经设置过
 *		ERROR_CR_TIMEOUT:	设置超时
 */
int RTSP_SetTrackNum(SESSION_HANDLE hSession, int nTrackNum);

/*
 *	功能描述：
 *		设置视频相关参数，用于生成SDP，目前视频只支持H264
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@nTrackID					[IN] Track ID 必须是一个大于0
 *		@pES						[IN] 待申请视频的关键帧数据,用于生成SPS和PPS。如果客户端不检查此值，可以NULL
 *		@nESLen						[IN] 关键帧数据长度
 *		@nBandWidth					[IN] 码流带宽，单位kbps，如果0，就不用出现在SDP中
 *		@nFrmRate					[IN] 帧率。如果0，就不用出现在SDP中
 *		@uiRangeBegin				[IN] 开始时间，实时流，填0
 *		@uiRangeEnd					[IN] 结束时间，实时流，填0
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_INVALID_PARAM：重复设置，已经设置过
 *		ERROR_CR_TIMEOUT:	设置超时
 */
int RTSP_AddVideoTrack(SESSION_HANDLE hSession, int nTrackID, unsigned char *pES, int nESLen, 
				  int nFrmRate, unsigned short video_media_port);

/*
 *	功能描述：
 *		设置音频相关参数，用于生成SDP
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@nTrackID					[IN] Track ID， 必须是一个大于0
 *		@nAlgID						[IN] 音频算法，RFC文档上规定的ID
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_INVALID_PARAM：重复设置，已经设置过
 *		ERROR_CR_TIMEOUT:	设置超时
 */
int RTSP_AddAudioTrack(SESSION_HANDLE hSession, int nTrackID, int nRFCAlgID);

/*
 *	功能描述：
 *		设置SDP,自定义的SDP。
 *		一般情况下，AddXXXTrack完毕后，对象内部应该生成SDP。但我们也导出一个接口，允许设置自己的SDP.
 *		这个函数只能和AddXXXTrack函数互斥使用。
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@pSDP						[IN] 自定义的SDP
 *		@nVideoTackID				[IN] 视频TrackID,必须是一个大于0
 *		@nRFCVedioAlg				[IN] 根据自定义SDP中具体取值不同， h264通常是96
 *		@nAudioTackID				[IN] 音频TrackID,如果没有音频，该值小于0， 否则大于0
 *		@nRFCAudioAlg				[IN] 根据自定义SDP中具体取值不同
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_INVALID_PARAM：重复设置，已经设置过
 *		ERROR_CR_TIMEOUT:	设置超时
 */
int RTSP_SetPrivateSDP(SESSION_HANDLE hSession, char *pSDP, int nVideoTackID, int nRFCVedioAlg, int nAudioTackID, int nRFCAudioAlg);

/*
 *	功能描述：
 *		获取播放轨道（Track）信息
 *		pClientIP和pClientPort是客户端接收RTP包的端口。RTCP端口一般就是RTP端口加1
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *		@pVideoTrackValidFlg		[OUT] 视频参数是否有效，0无效，其他值有效
 *		@pVideoClientIP				[OUT] 客户端接收视频数据的IP
 *		@pVideoClientPort			[OUT] 客户端接收视频数据的端口
 *		@pVideoPayLoad				[OUT] 视频PayLoad
 *		@pVideoSSRC					[OUT] 视频数据的SSRC
 *		@pAudioTrackValidFlg		[OUT] 音频参数是否有效，0无效，其他值有效
 *		@AudioClientIP				[OUT] 客户端接收音频数据的IP
 *		@pAudioClientPort			[OUT] 客户端接收音频数据的端口
 *		@pAudioPayLoad				[OUT] 音频PayLoad
 *		@pAudioSSRC					[OUT] 音频数据的SSRC
 *	返回值：
 *		ERROR_OK：正常
 *		ERROR_WOULDBLOCK：未获取到，需重新获取
 */
int RTSP_GetPlayTrackInfo(SESSION_HANDLE hSession, int *pVideoTrackValidFlg, unsigned short *pVideoMediaPort, unsigned int *pVideoClientIP, 
					 unsigned short *pVideoClientPort, int *pVideoPayLoad, unsigned int *pVideoSSRC,
					 int *pAudioTrackValidFlg, unsigned int *AudioClientIP, unsigned short *pAudioClientPort,
					 int *pAudioPayLoad, unsigned int *pAudioSSRC, bool *IsTCP);

/*
 *	功能描述：
 *		获取rtp session的tcp连接句柄，用于发送视频流
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *	返回值：
 *		ERROR_OK：正常
 */
unsigned int RTSP_GetConnectFd(SESSION_HANDLE hSession);

/*
 *	功能描述：
 *		点播文件数据发送完或者主动关闭Session的时候，发送此命令
 *	参数说明：
 *		@hSession					[IN] Session句柄
 *	返回值：
 *		ERROR_OK：正常
 */
int RTSP_EndofFile(SESSION_HANDLE hSession);

/*
 *	功能描述：
 *		将C7音频数据的AlgID转换成RFC文档定义的RTP打包的数据。
 *		目前支持的算法是G711A G722 ADPCM
 *	参数说明：
 *		@nC7AudioAlg					[IN] C7音频算法ID
 *	返回值：
 *		RFC对应的算法ID
 */
int RTSP_GetAudioRFCID(int nC7AudioAlg);

unsigned short RTSP_GetMediaPort(RTSP_HANDL hRTSPHandle);

// int RTSP_FetchRange(SESSION_HANDLE hSession,double& dbBeginTime, double& dbEndtime);
// int RTSP_FetchScal(SESSION_HANDLE hSession, double& dbScale);
int RTSP_FetchPauseFlag(SESSION_HANDLE hSession, bool &bPauseFlag);
int RTSP_FetchPlayFlag(SESSION_HANDLE hSession, bool &bPlayFlag);
int RTSP_SetLocalIP(SESSION_HANDLE hSession, const char *pszLocalIP);
#endif/*__CR_RTSPOBJ_H__*/




