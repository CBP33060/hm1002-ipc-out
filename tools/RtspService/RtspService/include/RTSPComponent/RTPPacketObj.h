/** 
 * @file	RTPObj.h
 * @brief	
 * 
 * 封装了三种RTP打包方式，分别是H264标准打包、通用RTP打包以及针对音频数据的打包
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
 *  <tr> <td>1.0		<td>			<td>		<td>Create this file</tr>
 * </table>
 * 
 */

#ifndef __ING_RTPOBJ_H__
#define __ING_RTPOBJ_H__

typedef void* RTP_H264_HANDLE;
typedef void* RTP_Common_HANDLE;
typedef void* RTP_C7Audio_HANDLE;

#define RTPPACKET_ERROR_NOPACKET 2
#define RTPPACKET_ERROR_CONTINUE 1
#define RTPPACKET_ERROR_OK 0
#define RTPPACKET_ERROR_UNKWON -1
#define RTPPACKET_ERROR_MEM_OVERLOAD -2


/*
 *	功能描述：
 *		创建一个标准H264打包的句柄，打包格式是按照RFC3511标准
 *	参数说明：
 *		@uiSSRC						[IN] RTP报文头中的SSRC值
 *		@PayLoadType				[IN] RTP报文头中的payload值,默认值为96
 *		@nMaxRTPLen					[IN] 每个RTP包最大值，包括RTP报文头的长度,不要超过2048
 *		@nFrmRate					[IN] 帧率
 *	返回值：
 *		NULL ：创建失败；其他值皆为成功
 */
RTP_H264_HANDLE CreateH264PacketHandle(unsigned int uiSSRC, unsigned char PayLoadType, int nMaxRTPLen, int nFrmRate);

/*
 *	功能描述：
 *		销毁句柄
 *	参数说明：
 *		@h264Handle					[IN] 待销毁的句柄值
 *	返回值：
 *		无
 */
void DestroyH264PackHandle(RTP_H264_HANDLE h264Handle);

/*
 *	功能描述：
 *		将一帧裸H264数据帧打包成若干RTP数据包
 *	参数说明：
 *		@h264Handle					[IN] 句柄值
 *		@pES						[IN] 裸H264数据帧
 *		@nESLen						[IN] H264数据帧长度
 *		@uiTimeStamp				[IN] 时间戳
 *	返回值：
 *		总是成功饿
 */
int PushH264ES(RTP_H264_HANDLE h264Handle, unsigned char *pES, int nESLen, unsigned int uiTimeStamp = 0);
int PushH265ES(RTP_H264_HANDLE h264Handle, unsigned char *pES, int nESLen, unsigned int uiTimeStamp = 0);

/*
 *	功能描述：
 *		获取打包好的RTP数据包，通常一帧数据有若干RTP包，需要多次获取
 *	参数说明：
 *		@h264Handle					[IN] 句柄值
 *		@pRTP						[OUT] RTP包数据
 *		@pMaxRTPLen					[IN/OUT] @pRTP的最大长度，实际数据的长度
 *	返回值：
 *		RTPPACKET_ERROR_MEM_OVERLOAD: pRTP太小
 *		RTPPACKET_ERROR_OK：取到一个RTP包且是最后一个包
 *		RTPPACKET_ERROR_CONTINUE：取到一个RTP包且后面还有数据包
 *		RTPPACKET_ERROR_NOPACKET：没有数据包
 */
int PopRTPPacket(RTP_H264_HANDLE h264Handle, unsigned char *pRTP, int *pMaxRTPLen);

#endif/*__ING_RTPOBJ_H__*/

