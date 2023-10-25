#pragma once
// #include "../libPacket/pack.h"
// #include "../RTSP/RTSPComponent/RTPPacketObj.h"
#include "RTSPComponent/RTSPCommon.h"
#include "RTSPComponent/RTPPacketObj.h"
#include "RTSPComponent/RTSPSocket.h"

// #include "../PSPacket/PSPacket.h"
#include "ING_Guard.h"
#include <vector>
//#include <ingjrtp.h>
#include <math.h>


extern "C" {
	extern int IMP_OSD_SetPoolSize(int size);
}
#define  MAX_MTU_LEN		1420
#define	 RTP_PKT_LEN_MAX    1400
#define  VIDEO_BUFFER_SIZE  (2*1024*1024)

#define SENSOR_FRAME_RATE_NUM		15
#define SENSOR_FRAME_RATE_DEN		1

static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CAPPED_QUALITY;
// static const IMPEncoderRcMode S_RC_METHOD = IMP_ENC_RC_MODE_CBR;

#if SENSOR_gc5603
	#define SENSOR_GC5603
#elif SENSOR_gc4663
	#define SENSOR_GC4663
#elif SENSOR_gc4023
	#define SENSOR_GC4023
#else
	#error "sensor not supported"
#endif 

#if defined SENSOR_AR0141
#define SENSOR_NAME				"ar0141"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x10
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_OV7725
#define SENSOR_NAME				"ov7725"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x21
#define SENSOR_WIDTH			640
#define SENSOR_HEIGHT			480
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					0
#elif defined SENSOR_OV9732
#define SENSOR_NAME				"ov9732"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_OV9750
#define SENSOR_NAME				"ov9750"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_OV9712
#define SENSOR_NAME				"ov9712"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_GC1004
#define SENSOR_NAME				"gc1004"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_JXH42
#define SENSOR_NAME				"jxh42"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			720
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_SC1035
#define SENSOR_NAME				"sc1035"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			1280
#define SENSOR_HEIGHT			960
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_OV2710
#define SENSOR_NAME				"ov2710"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_OV2735
#define SENSOR_NAME				"ov2735"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1

#elif defined SENSOR_OV2735B
#define SENSOR_NAME				"ov2735b"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x3c
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1

#elif defined SENSOR_SC2135
#define SENSOR_NAME				"sc2135"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1
#elif defined SENSOR_JXF22
#define SENSOR_NAME				"jxf22"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1

#elif defined SENSOR_JXF23
#define SENSOR_NAME				"jxf23"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					1

#elif defined SENSOR_JXF28
#define SENSOR_NAME				"jxf28"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x40
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					1

#elif defined SENSOR_GC2053
#define SENSOR_NAME				"gc2053"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x37
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0

#define CROP_EN                 1

#elif defined SENSOR_OV4689
#define SENSOR_NAME				"ov4689"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			2048
#define SENSOR_HEIGHT			1520
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					1

#elif defined SENSOR_OS05A20
#define SENSOR_NAME				"os05a20"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			2448
#define SENSOR_HEIGHT			1760
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					0

#elif defined SENSOR_SC4236H
#define SENSOR_NAME				"sc4236h"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			576
#define SENSOR_HEIGHT			432
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					0

#elif defined SENSOR_GC2063
#define SENSOR_NAME				"gc2063"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x37
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 1

#elif defined SENSOR_GC4663
#define SENSOR_NAME				"gc4663"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x29
#define SENSOR_WIDTH			2560
#define SENSOR_HEIGHT			1440
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 1

#elif defined SENSOR_GC5603
#define SENSOR_NAME				"gc5603"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x31
#define SENSOR_WIDTH			2560
#define SENSOR_HEIGHT			1440
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 1

#elif defined SENSOR_GC4023
#define SENSOR_NAME				"gc4023"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x29
#define SENSOR_WIDTH			2560
#define SENSOR_HEIGHT			1440
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN                 1

#endif

#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360

#define SENSOR_WIDTH_THIRD		1280
#define SENSOR_HEIGHT_THIRD		720

#define BITRATE_720P_Kbs        1000

#define NR_FRAMES_TO_SAVE		200
#define STREAM_BUFFER_SIZE		(2 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC		8
#define OSD_REGION_HEIGHT_SEC   	18


#define SLEEP_TIME			1

#define FS_CHN_NUM			4  //MIN 1,MAX 3
#define IVS_CHN_ID          3

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_ENABLE 1
#define CHN_DISABLE 0

/*#define SUPPORT_RGB555LE*/

struct chn_conf {
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
	chn_conf(int chnNum)
	{
		int chnIndex = CH0_INDEX;
		int chnEn = CHN0_EN;
		switch (chnNum){
			case 0:
			{
				chnIndex = CH0_INDEX;
				chnEn = CHN0_EN;
				break;
			}
			case 1:
			{
				chnIndex = CH1_INDEX;
				chnEn = CHN1_EN;
				break;
			}
			case 2:
			{
				chnIndex = CH2_INDEX;
				chnEn = CHN2_EN;
				break;
			}
			case 3:
			{
				chnIndex = CH3_INDEX;
				chnEn = CHN3_EN;
				break;
			}
		}
		index = chnIndex;
		enable = chnEn;
		payloadType = IMP_ENC_PROFILE_HEVC_MAIN;

		fs_chn_attr.i2dattr.i2d_enable = 0,
		fs_chn_attr.i2dattr.flip_enable = 1,
		fs_chn_attr.i2dattr.mirr_enable = 0,
		fs_chn_attr.i2dattr.rotate_enable = 0,
		fs_chn_attr.i2dattr.rotate_angle = 270,

		fs_chn_attr.pixFmt = PIX_FMT_NV12;
		fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE_NUM;
		fs_chn_attr.outFrmRateDen = SENSOR_FRAME_RATE_DEN;
		fs_chn_attr.nrVBs = 2;
		fs_chn_attr.type = FS_PHY_CHANNEL;

		fs_chn_attr.crop.enable = 1;
		fs_chn_attr.crop.top = 0;
		fs_chn_attr.crop.left = 0;
		fs_chn_attr.crop.width = SENSOR_WIDTH;
		fs_chn_attr.crop.height = SENSOR_HEIGHT;

		fs_chn_attr.scaler.enable = 1;
		fs_chn_attr.scaler.outwidth = SENSOR_WIDTH,
		fs_chn_attr.scaler.outheight = SENSOR_HEIGHT,

		fs_chn_attr.picWidth = SENSOR_WIDTH; 
		fs_chn_attr.picHeight = SENSOR_HEIGHT; 

		framesource_chn = { DEV_ID_FS, chnIndex, 0 };
		imp_encoder = { DEV_ID_ENC, chnIndex, 0 };
	}
};

#define  CHN_NUM  ARRAY_SIZE(chn)

class ING_Stream_Vdo
{
public:
	typedef std::vector<MediaDataPacket* > MEDIADATAARR;

	enum
	{
		stream_type_realtime = 0x01,
		stream_type_vod,
		stream_type_download,
	};
	ING_Stream_Vdo(unsigned int dlg_handle, int stream_type, std::string type, int payload, SOCKET tcpFd);    //tcp
	ING_Stream_Vdo(unsigned int dlg_handle, unsigned short port, const char *remote_ip, unsigned short remote_port, int stream_type,
		std::string type, int payload);				//udp

	virtual ~ING_Stream_Vdo(void);
	virtual int open();
	virtual int close();

	virtual int run();

	unsigned dlg_handle() {
		return dlg_handle_;
	}

// 	virtual int send_frame(char *frame, int frame_len, int frame_type, unsigned short frame_no);
//	virtual int recv_frame(char *frame, int &frame_len, int &frame_type, int &key_frame);

	// 设置流断开标记
// 	void disconn() {
// 		ING_Guard guard(mutex_);
// 		is_close_flag_ = true;
// 		dlg_handle_ = 0;//保险起见真正关闭前将会话句柄置空，防止拖拽播放时查找流，创建流 逻辑出错
// 		//真正需要关闭流的时候才断开和平台的RTPSession，拖拽时重新申请视频流不断开，将原来的jrtp_handle塞回新的流对象
// 		printf("disconn--------------------------------------------------------------------\n");
// 	}
// 	bool is_close_flag() {
// 		ING_Guard guard(mutex_);
// 		return is_close_flag_;
// 	}
	// 设置流清除标记
	void remove() {
		ING_Guard guard(mutex_);
		is_remove_flag_ = true;
	}
	bool is_remove_flag() {
		ING_Guard guard(mutex_);
		return is_remove_flag_;
	}

	// 暂停
	void pause() {
		ING_Guard guard(mutex_);
		pause_flag_ = true;
	}
	// 继续
	void resume() {
		ING_Guard guard(mutex_);
		pause_flag_ = false;
	}
protected:
	// 按照将每帧数据直接拆的方法打包
	int XSipStream_PacketRTP_i(char *pData, char cFrameRate, int nLen);

private:
	// 	RTP_H264_HANDLE h264_handle_;
	// 	RTP_C7Audio_HANDLE audio_handle_;	

	unsigned int dlg_handle_;	// 会话句柄
	ING_Mutex mutex_;
	unsigned short port_;
	std::string remote_ip_;
	unsigned short remote_port_;
	std::string type_;		// 流类型
	int payload_;

	RTP_H264_HANDLE rtp_pack_handle_;
// 	INGJRTP_HANDLE  jrtp_handle_;

// 	bool is_jrtp_delete_flag_;

// 	bool is_close_flag_;			// 是否关闭标记
	bool is_remove_flag_;			// 是否删除标记

	unsigned int time_stamp_;
	bool wait_key_frame_;

// 	PACK_HANDLE audio_pack_handle_;

	int stream_type_;
	unsigned int first_time_stamp_;
	unsigned int cur_time_stamp_;

	bool last_send_status_;
	unsigned int last_send_frame_;

	bool pause_flag_;

	double scale_;

	int scale_frame_num_;

	unsigned int frame_tm_;

	// added by Rocky 2014.08.30(当流类型为音频时用于缓存完整的一帧音频数据，内存为动态分配，当发送一帧完整音频数据时，内存被释放)
// 	char *recv_buf_;	// 缓存一帧音频数据的buf
// 	int recv_buf_len_;	// 缓存buf的实时长度

// #define MAX_PS_PACKET_LEN (1024 * 1024)
// 	char ps_buffer_[MAX_PS_PACKET_LEN];

// 	unsigned short usVideoSeqNum_;
// 	unsigned int uiVideoCurTimestamp_;
// 	MEDIADATAARR send_list_;
	unsigned long LastPoolTmOutPrint_;
	SOCKET socket_rtp_;
	int sendCount_;
	unsigned short trans_mode_;
	unsigned char szRtpBuf_[MAX_MTU_LEN];

	char video_file_name[256];
	int save_stream;
};

typedef std::vector<ING_Stream_Vdo *> STREAM_ARR;
