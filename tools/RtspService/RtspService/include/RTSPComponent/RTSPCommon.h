#ifndef __CR_RTSPSERVERCOMMON_H__
#define __CR_RTSPSERVERCOMMON_H__

// TODO: reference additional headers your program requires here
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif
#define BOOL bool

#define SOCKET_ERROR -1
typedef int SOCKET;
#define INVALID_SOCKET -1

#define RTSP_MAX_IPSTR_LEN 16

//将socket设置成阻塞模式,s为socket句柄,以下同
#define rtsp_setsockblock(s) \
{ \
	int __opts; \
	__opts = fcntl((int)s, F_GETFL); \
	__opts = (__opts & ~O_NONBLOCK); \
	fcntl((int)s, F_SETFL, __opts); \
}

//将socket设置成非阻塞模式
#define rtsp_setsocknonblock(s) \
{	\
	int __opts; \
	__opts = fcntl((int)s, F_GETFL); \
	__opts = (__opts | O_NONBLOCK); \
	fcntl((int)s, F_SETFL, __opts); \
}
//设置socket接收缓冲大小,i为缓冲大小
#define rtsp_setsockrecvbuf(s, i) \
{ \
	int __i = i; \
	setsockopt((int)s, SOL_SOCKET, SO_RCVBUF, (char *)&__i, sizeof(__i)); \
}

//设置socket发送缓冲大小,i同上
#define rtsp_setsocksendbuf(s, i) \
{ \
	int __i = i; \
	setsockopt((int)s, SOL_SOCKET, SO_SNDBUF, (char *)&__i, sizeof(__i)); \
}

//设置socket等待时间,o表示是否使能等待,l表示等待时间,单位秒
//LINUX是否要设置还未知,好像不设置会出问题的,所以又设置了
//LINUX不能设置,如果设置了,关闭的时候就会等待设置的linger时间
#define rtsp_setsocklinger(s, o, l) \
{ \
	struct linger __Linger; \
	__Linger.l_onoff = o; \
	__Linger.l_linger = l; \
	setsockopt((int)s, SOL_SOCKET, SO_LINGER, (char*)&__Linger, sizeof(__Linger)); \
}

//设置socket为非延迟模式,
//具体含义不是太明白.根据平台不同而不同,一般不设这个
#define rtsp_setsocknodelay(s) \
/*{ \
	int __one = 1; \
	setsockopt((int)s, IPPROTO_TCP, TCP_NODELAY, (char*)&__one, sizeof(int)); 
}
*/

//设置socket为保持连接
//不是所有平台都支持
#define rtsp_setsockkeepalive(s) \
{ \
	int __one = 1; \
	setsockopt((int)s, SOL_SOCKET, SO_KEEPALIVE, (char*)&__one, sizeof(int)); \
}

#define rtsp_setsockrecverr(s) \
{ \
	int __optval = 1; \
	setsockopt((int)s, SOL_IP, IP_RECVERR, (char *)&__optval, sizeof(__optval)); \
}

typedef struct 
{
	/**//* byte 0 */
	unsigned char csrc_len:4;        /**//* expect 0 */
	unsigned char extension:1;        /**//* expect 0, see RTP_OP below */
	unsigned char padding:1;        /**//* expect 0 */
	unsigned char version:2;        /**//* expect 2 */
// 	unsigned char c;
	/**//* byte 1 */
	unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
	unsigned char marker:1;        /**//* expect 1 */
// 	unsigned char d;
	/**//* bytes 2, 3 */
	unsigned short seq_no;            
	/**//* bytes 4-7 */
	unsigned  long timestamp;        
	/**//* bytes 8-11 */
	unsigned long ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct {
	//byte 0
	unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;    
// 	unsigned char c;
} NALU_HEADER; /**//* 1 BYTES */

typedef struct {
	//byte 0
	unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;    
// 	unsigned char c;
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct {
	//byte 0
	unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;    
// 	unsigned char c;
} FU_HEADER; /**//* 1 BYTES */

typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx    
	char *buf;                    //! contains the first byte followed by the EBSP
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;

//C7 存储帧头定义
typedef struct _N_FRAME_HEAD
{
	unsigned int utc;
	unsigned int time_stamp;
	unsigned char key_frm;
	unsigned char rsv1[3];
} TN_FRAME_HEAD;

typedef struct _N_FRAME_INFO
{
	union 
	{
		TN_FRAME_HEAD fh;
		struct 
		{
			TN_FRAME_HEAD fh;
			unsigned short width;
			unsigned short height;
			unsigned short producer_id;
			unsigned char frm_rate;
			unsigned char rsv1;
			unsigned char rsv2[4];
			unsigned char rsv[4];
		} video;
		struct  
		{
			TN_FRAME_HEAD fh;
			unsigned short block_align;
			unsigned char channels;
			unsigned char bits_per_sample;
			unsigned short sample_per_sec;
			unsigned short frm_cnt;
			unsigned short producer_id;
			unsigned short pcm_len;
			unsigned char rsv[4];
		} audio;
	} un;
} TN_FRAME_INFO;

//C7 视频私有数据头
typedef struct _N_VIDEO_DATA_HEAD 
{
	unsigned char alg;
	char   rsv[3];
	char   private_data[8];
}TN_VIDEO_DATA_HEAD;

//C7 音频私有数据头
typedef struct _N_AUDIO_DATA_HEAD
{
	unsigned char alg;
	char   rsv[3];
}TN_AUDIO_DATA_HEAD;

typedef struct  _TMediaDataPacket
{
	char	szData[2048];
	int		len;
} MediaDataPacket;

#ifdef __cplusplus
}
#endif

#endif/*__CR_RTSPSERVERCOMMON_H__*/
