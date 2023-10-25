#ifndef __ZERATUL_VIDEO_INTERFACE_H__
#define __ZERATUL_VIDEO_INTERFACE_H__
#include "media_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <imp/imp_system.h>
#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <imp/imp_encoder.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


/******************************************** Sensor Attribute Table *********************************************/
/* 		NAME		I2C_ADDR		RESOLUTION		Default_Boot												*/
/* 		jxf23		0x40 			1920*1080		0:25fps_dvp 1:15fps_dvp 2:25fps_mipi						*/
/* 		jxf37		0x40 			1920*1080		0:25fps_dvp 1:25fps_mipi 2:	25fps_mipi						*/
/* 		imx327		0x1a 			1920*1080		0:25fps 1:25fps_2dol										*/
/* 		sc430ai		0x30 			2688*1520		0:20fps_mipi 1:30fps_mipi 2:25fps_mipi						*/
/* 		sc500ai		0x30 			2880*1620		0:30fps_mipi												*/
/* 		sc5235		0x30 			2592*1944		0:5fps_mipi													*/
/* 		gc4663		0x29 			2560*1440		0:25fps_mipi 1:30fps_mipi									*/
/* 		sc8238		0x30 			3840*2160		0:15fps 1:30fps 											*/
/******************************************** Sensor Attribute Table *********************************************/
/* first sensor */
// #if SENSOR_gc5603
// 	#define FIRST_SNESOR_NAME		   "gc5603"						//sensor name (match with snesor driver name)
// 	#define FIRST_I2C_ADDR			  	0x31							//sensor i2c address
// #elif SENSOR_gc4663
// 	#define FIRST_SNESOR_NAME		   "gc4663"						//sensor name (match with snesor driver name)
// 	#define FIRST_I2C_ADDR			  	0x29							//sensor i2c address
// #else
// 	// #error "sensor not supported"
// #endif 
#define FIRST_SNESOR_NAME		   "gc5603"						//sensor name (match with snesor driver name)
#define FIRST_I2C_ADDR			  	0x31							//sensor i2c address
#define FIRST_I2C_ADAPTER_ID		0							   //sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH		  	2560							//sensor width
#define FIRST_SENSOR_HEIGHT		 	1440							//sensor height
#define FIRST_RST_GPIO			  	GPIO_PA(18)					 //sensor reset gpio
#define FIRST_PWDN_GPIO			 	GPIO_PA(19)					 //sensor pwdn gpio
#define FIRST_POWER_GPIO			-1							  //sensor power gpio
#define FIRST_SENSOR_ID			 	0							   //sensor index
#define FIRST_VIDEO_INTERFACE	   	IMPISP_SENSOR_VI_MIPI_CSI0	  //sensor interface type (dvp/csi0/csi1)
#define FIRST_MCLK				  	IMPISP_SENSOR_MCLK0			 //sensor clk source (mclk0/mclk1/mclk2)
#define FIRST_DEFAULT_BOOT		  	0							   //sensor default mode(0/1/2/3/4)

#define CHN0_EN				 1
#define CHN1_EN				 1
#define CHN2_EN				 1

/* Crop_en Choose */
#define FIRST_CROP_EN					1

#define FIRST_SENSOR_FRAME_RATE_NUM			15
#define FIRST_SENSOR_FRAME_RATE_DEN			1

#define FIRST_SENSOR_WIDTH_SECOND			832
#define FIRST_SENSOR_HEIGHT_SECOND			480

#define FIRST_SENSOR_WIDTH_THIRD			1280
#define FIRST_SENSOR_HEIGHT_THIRD			720

#define BITRATE_2K_Kbs			1500
#define BITRATE_720P_Kbs		700
#define BITRATE_720P_H264_Kbs   600

#define NR_FRAMES_TO_SAVE		200
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC	8
#define OSD_REGION_HEIGHT_SEC   18


#define SLEEP_TIME			1

#define FS_CHN_NUM			3
#define ENCODE_CHN_NUM		4
#define ENCGROUP_CHN_NUM    3
#define IVS_CHN_ID		  1

#define CH0_INDEX  	0
#define CH1_INDEX  	1
#define CH2_INDEX  	2

#define CHN_ENABLE 		1
#define CHN_DISABLE 	0

struct SEncodeChnConf
{
	unsigned int bitrate;
	unsigned int gop;
	unsigned int fps;
	unsigned int width;
	unsigned int height;
	IMPEncoderProfile profile;
	IMPEncoderRcMode rcMode;
};

struct SLogoParameter
{
	int width;
	int height;
	uint8_t* logodata;

	int num_width;
	int num_height;
	uint8_t* num_data[12];
	char date[40];
};

struct SChnConf
{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
	IMPCell osdcell;
};

#define _RINGBUF_LEN 4
typedef struct _ringbuf_t_
{
    volatile unsigned int   ringbuf_head;
    volatile unsigned int   ringbuf_tail;
    volatile unsigned char  p_ringbuf[_RINGBUF_LEN][599040];
}ringbuf;


#define  CHN_NUM  ARRAY_SIZE(chn)

#define TAG "mxZeratulVideoInterface"

#define VIDEO_BUF_SIZE 600*1024

using namespace maix;
class CZeratulVideoInterface : public CMediaInterface
{
public:
	CZeratulVideoInterface(std::string strName);
	~CZeratulVideoInterface();

	IMPRgnHandle * initZeratulOSD(int iGroupID,SLogoParameter *logo,int x0, int y0);
	IMPRgnHandle * initZeratulOSDShowInfo(int iGroupID,int iTimestampCoordinatesX, int iTimestampCoordinatesY);
	int uninitZeratulOSD(int iGroupID,IMPRgnHandle *pOSDHander);
	int uninitZeratulOSDShowInfo(int iGroupID,IMPRgnHandle *pOSDHander);
	mxbool zeratulOSDrefresh(int iChnNum);
	mxbool zeratulOSDInforefresh(int iChnNum);
	int startOSDShowRgn(int iChnNum);
	int stopOSDShowRgn(int iChnNum);


	mxbool initZeratulIMP();
	mxbool unInitZeratulIMP();
	mxbool initZeratulFramesource();
	mxbool unInitZeratulFramesource();
	mxbool zeratulFramesourceStreamOn(int chnNum);
	mxbool zeratulFramesourceStreamOff();
	mxbool initZeratulEncoder();
	mxbool unInitZeratulEncoder();
	mxbool initLogoData();

	int zeratulEncoderStartRcvePic(int chnNum);
	int zeratulGetFrame(IMPEncoderStream *stream,int chnNum,int delay);
	int zeratulCopyFrameToBuffer(IMPEncoderStream *stream, unsigned char *buf, size_t bufSize);
	int zeratulReleaseFrame(IMPEncoderStream *stream,int chnNum);
	int zeratulGetYuvFrame(int chnNum, IMPFrameInfo **frame);
	int zeratulReleaseYuvFrame(int chnNum, IMPFrameInfo *frame);
	int zeratulGetJpegSnap();
	int zeratulSaveStream(int fd, IMPEncoderStream *stream);

	mxbool init();
	mxbool unInit();
	mxbool startRcvFrame(int chnNum);
	mxbool getIDRFrame(int chnNum);
	unsigned char *readFrame(int chnNum, int *size);
	unsigned char *readFrame(int chnNum, int *size, int *frameType, int64_t *timestamp, int *frameSeq);
	int getChnNum();

	mxbool loadConfig(std::string strPath);
	template<class T>
	mxbool getConfig(std::string strSection,
		std::string strKey, T& value);

	template<class T>
	mxbool setConfig(std::string strSection,
		std::string strKey, T& value);

	mxbool saveConfig();
	int getChnNum(int iNum);
	std::string getChnName(int iNum);
	int getChnSN(int iNum);
	E_P_TYPE getPacketType(int iNum);

	mxbool config(std::string strConfig);
	int loadTagConfig();

	int zeratulISPRunModeSwitch(const char *cmdData);
	int zeratulISPWdrModeSwitch(const char *cmdData);
	std::string zeratulGetLightSensorValue();
	std::string readFileToSting(const char* filename);
	void setISPRunningMode(IMPVI_NUM vinum, IMPISPRunningMode mode);
	void LightSensorRun();
	mxbool initSetLedMode();
	
	int getErrorIntNum();

private:
	std::string m_strConfigPath;
	std::mutex m_mutexConfig;
	ringbuf *g_rb;
	int g_yuv_count;
#ifdef _CRYPTO_ENABLE
	unsigned char m_key[32];
#endif
#ifdef _INI_CONFIG
	INI::CINIFile m_configs;
#endif
    mxbool m_bInit;
	uint8_t* m_frameBuff;
    uint8_t* m_frameBuff_1;
    uint8_t* m_frameBuff_2;
    uint8_t* m_frameBuff_3;
    uint8_t* m_frameBuff_yuv;
	IMPRgnHandle* m_pOSDHander[ENCGROUP_CHN_NUM];
	IMPRgnHandle* m_pOSDInfoHander[ENCGROUP_CHN_NUM];
	std::mutex m_mutexOSDShow;
	int m_OSDPushState;
	int m_wdr_mode;
	int m_wdr_mode_init;
	int m_dn_mode;

	int m_restSensorFlag;
	int m_errorIntNum;
    std::mutex m_mutexPlay;

    bool m_bFirstCHnReady = false;
    bool m_bSencondCHnReady = false;
    bool m_bThirdCHnReady = false;
    bool m_bDepthReady = false;    

	std::thread lightSensorThread;
	std::mutex m_mutexLightSensor;
	std::condition_variable m_conditionLightSensor;

	volatile int m_setLedMode;
	int m_LedState;
	int nightCount;
	int dayCount;

	volatile int m_getLedOldMode;

    std::mutex m_mutexSetConfig;

};

#endif //__ZERATUL_VIDEO_INTERFACE_H__

template<class T>
inline mxbool CZeratulVideoInterface::getConfig(std::string strSection,
	std::string strKey, T & value)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
	try
	{
		value = m_configs[strSection][strKey].as<T>();
	}
	catch (std::invalid_argument &ia)
	{
		return mxfalse;
	}
#endif
	return mxtrue;
}

template<class T>
inline mxbool CZeratulVideoInterface::setConfig(std::string strSection,
	std::string strKey, T & value)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
	try
	{
		m_configs[strSection][strKey] = value;
	}
	catch (std::invalid_argument &ia)
	{
		return mxfalse;
	}
#endif
	return mxtrue;
}
