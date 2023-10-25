#ifndef __RTSPSOCKET_H__
#define __RTSPSOCKET_H__
#include "RTSPCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


int RTSP_TCPRecvHeadNB(SOCKET sock, char *pBuf, int iBufLen, int *piRecvLen, char *pszTail);
int RTSP_TCPRecvDataNB(SOCKET sock, char *pRecvBuf, int *piRecvLen, int iDataLen);
int RTSP_TCPSendDataNB(SOCKET sock, char *pSendBuf, int *offset, int iDataLen);
SOCKET RTSP_UDPBind(unsigned int u32IP, unsigned short u16Port);
bool RTSP_UDPSendTo(SOCKET sock, char *pBuf, int iBufLen, unsigned int u32IP, unsigned short u16Port);
int RTSP_UDPSendToNB(SOCKET sock, char *pBuf, int iBufLen, unsigned int u32IP, unsigned short u16Port);

int RTSP_TCPRecvNB(SOCKET sock, char *pRecvBuf, int iMaxRecvLen);
void RTSP_PreFork(int min, int max);
unsigned short RTSP_ForkPort();

//#define lastSockErrno GetLastError()
#define TCP_SEND_PACKET_LEN	1400	//实际上以太网中传的是1426个

#ifdef __cplusplus
}
#endif

#endif

