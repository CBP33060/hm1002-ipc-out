#include "zeratul_video_interface.h"
#include "logoMIBgra.h"
#include "res/osd_2k_common.h"
#include "res/osd_480p_common.h"
#include "res/osd_720p_common.h"
#include "bitMapInfo.h"
#include <iostream>
#include <math.h>
#include "cJSON.h"
#ifdef _CRYPTO_ENABLE
#include "crypt_api_mx.h"
#endif
#include "common.h"
#include "log_mx.h"
#include "fw_env_para.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "iLockFileStream.h"
#include "oLockFileStream.h"

#define ALS_CHECK_COUNT 10
#define NIGHT_SENSOR_VALUE 0.2
#define IRLED_SENSOR_VALUE 3
#define WLED_SENSOR_VALUE 300


struct SEncodeChnConf g_SEncodeChnConf[ENCODE_CHN_NUM] = { 0 };
struct SLogoParameter g_SLogoParameter[3] = { 0 };

struct SChnConf g_SChnConf[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
		.payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.i2dattr = {
				.i2d_enable = 0,
				.flip_enable = 0,
				.mirr_enable = 0,
				.rotate_enable = 1,
				.rotate_angle = 270,
			},

			.picWidth = FIRST_SENSOR_WIDTH,
			.picHeight = FIRST_SENSOR_HEIGHT,
			.pixFmt = PIX_FMT_NV12,
			.crop = {
				.enable = FIRST_CROP_EN,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH,
				.height = FIRST_SENSOR_HEIGHT,
			},
			.scaler = {
				.enable = 1,
				.outwidth = FIRST_SENSOR_WIDTH,
				.outheight = FIRST_SENSOR_HEIGHT,
			},
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,
			.fcrop = {
				.enable = FIRST_CROP_EN,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH,
				.height = FIRST_SENSOR_HEIGHT,
			},
			.mirr_enable = 0,
		},
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
		.osdcell = {DEV_ID_OSD, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
		.payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.i2dattr = {
				.i2d_enable = 0,
				.flip_enable = 0,
				.mirr_enable = 0,
				.rotate_enable = 1,
				.rotate_angle = 270,
			},
			.picWidth = FIRST_SENSOR_WIDTH_THIRD,
			.picHeight = FIRST_SENSOR_HEIGHT_THIRD,
			.pixFmt = PIX_FMT_NV12,
			.crop = {
				.enable = 0,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH_THIRD,
				.height = FIRST_SENSOR_HEIGHT_THIRD,
			},
			.scaler = {
				.enable = 1,
				.outwidth = FIRST_SENSOR_WIDTH_THIRD,
				.outheight = FIRST_SENSOR_HEIGHT_THIRD,
			},
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,
			.fcrop = {
				.enable = 0,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH_THIRD,
				.height = FIRST_SENSOR_HEIGHT_THIRD,
			},
			.mirr_enable = 0,
		},
		.framesource_chn =	{ DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
		.osdcell = {DEV_ID_OSD, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
		.payloadType = IMP_ENC_PROFILE_HEVC_MAIN,
		.fs_chn_attr = {
			.i2dattr = {
				.i2d_enable = 0,
				.flip_enable = 0,
				.mirr_enable = 0,
				.rotate_enable = 1,
				.rotate_angle = 270,
			},
			.picWidth = FIRST_SENSOR_WIDTH_SECOND,
			.picHeight = FIRST_SENSOR_HEIGHT_SECOND,
			.pixFmt = PIX_FMT_NV12,
			.crop = {
				.enable = 0,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH_SECOND,
				.height = FIRST_SENSOR_HEIGHT_SECOND,
			},
			.scaler = {
				.enable = 1,
				.outwidth = FIRST_SENSOR_WIDTH_SECOND,
				.outheight = FIRST_SENSOR_HEIGHT_SECOND,
			},

			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,
			.fcrop = {
				.enable = 0,
				.left = 0,
				.top = 0,
				.width = FIRST_SENSOR_WIDTH_SECOND,
				.height = FIRST_SENSOR_HEIGHT_SECOND,
			},
			.mirr_enable = 0,
		},
		.framesource_chn =	{ DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
		.osdcell = {DEV_ID_OSD, CH2_INDEX, 0},
	}
};

IMPSensorInfo Def_Sensor_Info[1] = {
	{
		FIRST_SNESOR_NAME,
		TX_SENSOR_CONTROL_INTERFACE_I2C,
		//.i2c = 
		{
			FIRST_SNESOR_NAME, 
			FIRST_I2C_ADDR, 
			FIRST_I2C_ADAPTER_ID
		},
		FIRST_RST_GPIO,
		FIRST_PWDN_GPIO,
		FIRST_POWER_GPIO,
		FIRST_SENSOR_ID,
		FIRST_VIDEO_INTERFACE,
		FIRST_MCLK,
		FIRST_DEFAULT_BOOT
	}
};
IMPSensorInfo sensor_info[1];


CZeratulVideoInterface::CZeratulVideoInterface(std::string strName)
	: CMediaInterface(strName)
{
	m_bInit = false;
	m_OSDPushState = 0;
	m_wdr_mode = 0;
	m_dn_mode = 0;
	// m_wdr_mode_init = 0;
	m_restSensorFlag = 0;
	m_errorIntNum = 0;
	m_setLedMode = 0;
	m_LedState = 0;
	nightCount = 0;
	dayCount = 0;

}

CZeratulVideoInterface::~CZeratulVideoInterface()
{
}

int initOSDRgn(IMPRgnHandle *handle,int iGroupID,int x0,int y0,int x1,int y1,unsigned char * data)
{
	*handle = IMP_OSD_CreateRgn(NULL);
	if (*handle == INVHANDLE) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_CreateRgn Logo error !\n");
		return -1;
	}

	int ret = IMP_OSD_RegisterRgn(*handle, iGroupID, NULL);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IVS IMP_OSD_RegisterRgn failed !\n");
		return -1;
	}

	IMPOSDRgnAttr rAttrLogo;
	memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
	rAttrLogo.type = OSD_REG_PIC;
	rAttrLogo.rect.p0.x = x0;
	rAttrLogo.rect.p0.y = y0;

	//p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrLogo.rect.p1.x = x1;
	rAttrLogo.rect.p1.y = y1;
	rAttrLogo.fmt = PIX_FMT_BGRA;
	rAttrLogo.data.picData.pData = data;
	ret = IMP_OSD_SetRgnAttr(*handle, &rAttrLogo);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_SetRgnAttr Logo error !\n");
		return -1;
	}

	IMPOSDGrpRgnAttr grAttrLogo;
	if (IMP_OSD_GetGrpRgnAttr(*handle, iGroupID, &grAttrLogo) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return -1;

	}
	memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrLogo.show = 0;
	/* Set Logo global alpha to 0x7f, it is semi-transparent. */
	grAttrLogo.gAlphaEn = 1;
	grAttrLogo.fgAlhpa = 0x7f;
	grAttrLogo.layer = 2;

	if (IMP_OSD_SetGrpRgnAttr(*handle, iGroupID, &grAttrLogo) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return -1;
	}

	return 0;
}

IMPRgnHandle * CZeratulVideoInterface::initZeratulOSD(int iGroupID,SLogoParameter *logo,int x0, int y0)
{
	int ret = 0;
	IMPRgnHandle *prHander = NULL;
	
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulOSD start !\n");

	prHander = (IMPRgnHandle *)malloc(19 * sizeof(IMPRgnHandle));
	if (prHander == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMPRgnHandle malloc() error!\n");
		return NULL;
	}

	if (IMP_OSD_CreateGroup(iGroupID) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_CreateGroup(%d) error !\n",iGroupID);
        return NULL;
    }
	//query osd rgn create status

	initOSDRgn(&prHander[18], iGroupID, x0, y0,x0 + logo->width - 1, y0 + logo->height - 1, logo->logodata);//logo
	
	for(int i = 0; i < 10 ; i++) {
		int x = x0 + logo->width + logo->num_width * i ;
		initOSDRgn(&prHander[i], iGroupID, x, y0, x + logo->num_width -1, y0 + logo->height -1, NULL);
	}

	for(int i = 10; i < 18 ; i++) {
		int x = x0 + logo->width + logo->num_width * i + logo->num_width;
		initOSDRgn(&prHander[i], iGroupID, x, y0, x + logo->num_width -1, y0 + logo->height - 1, NULL);
	}

	time_t tCurrTime;
	time(&tCurrTime);
	struct tm *T_SCurrDate = localtime(&tCurrTime);
	memset(logo->date, 0, 40);
	strftime(logo->date, 40, "%Y/%m/%d%H:%M:%S", T_SCurrDate);

	int iLen = strlen(logo->date);
	for (int i = 0; i < iLen; i++) {
		IMPOSDRgnAttr prAttr;
		IMP_OSD_GetRgnAttr(prHander[i], &prAttr);
		switch(logo->date[i]) {
			case '0' ... '9':
				prAttr.data.picData.pData = logo->num_data[logo->date[i]-'0'];
				break;
			case '/':
				prAttr.data.picData.pData = logo->num_data[10];
				break;
			case ':':
				prAttr.data.picData.pData = logo->num_data[11];
				break;
			default:
				break;
		}
		IMP_OSD_SetRgnAttr(prHander[i], &prAttr);
	}

	ret = IMP_OSD_Start(iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return NULL;
	}

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface ZvmOSDInit success !\n");
	return prHander;
}

IMPRgnHandle * CZeratulVideoInterface::initZeratulOSDShowInfo(int iGroupID,int x0, int y0)
{
	int ret = 0;

	IMPRgnHandle *prHander = NULL;
	IMPRgnHandle rHanderInfo = 0;
	
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulOSDShowInfo start\n");

	prHander = (IMPRgnHandle *)malloc(1 * sizeof(IMPRgnHandle));
	if (prHander == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMPRgnHandle malloc() error !\n");
		return NULL;
	}

	if (IMP_OSD_CreateGroup(iGroupID) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_CreateGroup(%d) error !\n", iGroupID);
        return NULL;
    }

	rHanderInfo = IMP_OSD_CreateRgn(NULL);
	if (rHanderInfo == INVHANDLE) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_CreateRgn rHanderInfo error !\n");
		return NULL;
	}
	//query osd rgn create status
	IMPOSDRgnCreateStat stStatus;
	memset(&stStatus,0x0,sizeof(IMPOSDRgnCreateStat));
	ret = IMP_OSD_RgnCreate_Query(rHanderInfo,&stStatus);
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_RgnCreate_Query error !\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderInfo, iGroupID, NULL);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	//query osd rgn register status
	IMPOSDRgnRegisterStat stRigStatus;
	memset(&stRigStatus,0x0,sizeof(IMPOSDRgnRegisterStat));
	ret = IMP_OSD_RgnRegister_Query(rHanderInfo, iGroupID,&stRigStatus);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_RgnRegister_Query failed\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrInfo;
	memset(&rAttrInfo, 0, sizeof(IMPOSDRgnAttr));
	rAttrInfo.type = OSD_REG_BITMAP;
	rAttrInfo.rect.p0.x = x0;
	rAttrInfo.rect.p0.y = y0;
	rAttrInfo.rect.p1.x = rAttrInfo.rect.p0.x + 20 * 32 - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrInfo.rect.p1.y = rAttrInfo.rect.p0.y + 33 - 1;
#ifdef SUPPORT_COLOR_REVERSE
	rAttrInfo.fontData.invertColorSwitch = 1;   //Color reverse switch 0-close 1-open.
	rAttrInfo.fontData.luminance = 200;
	rAttrInfo.fontData.data.fontWidth = 16;
	rAttrInfo.fontData.data.fontHeight = 33;
	rAttrInfo.fontData.length = 19;
	rAttrInfo.fontData.istimestamp = 1;
#endif
#ifdef SUPPORT_RGB555LE
	rAttrInfo.fmt = PIX_FMT_RGB555LE;
#else
	rAttrInfo.fmt = PIX_FMT_MONOWHITE;
#endif
	rAttrInfo.data.bitmapData = valloc(20 * 32 * 33);
	if (rAttrInfo.data.bitmapData == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface alloc rAttr.data.bitmapData info error !\n");
		return NULL;
	}
	memset(rAttrInfo.data.bitmapData, 0, 20 * 32 * 33);

	ret = IMP_OSD_SetRgnAttr(rHanderInfo, &rAttrInfo);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_SetRgnAttr info error !\n");
		return NULL;
	}

	IMPOSDGrpRgnAttr grAttrFont;

	if (IMP_OSD_GetGrpRgnAttr(rHanderInfo, iGroupID, &grAttrFont) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_GetGrpRgnAttr info error !\n");
		return NULL;

	}
	memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrFont.show = 0;

	/* Disable Font global alpha, only use pixel alpha. */
	grAttrFont.gAlphaEn = 1;
	grAttrFont.fgAlhpa = 0xff;
	grAttrFont.layer = 3;
	if (IMP_OSD_SetGrpRgnAttr(rHanderInfo, iGroupID, &grAttrFont) < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_SetGrpRgnAttr info error !\n");
		return NULL;
	}

	ret = IMP_OSD_Start(iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_Start OSDShowInfo, Cover and Rect error !\n");
		return NULL;
	}

	prHander[0] = rHanderInfo;
	
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulOSDShowInfo OSDShowInfo success !\n");
	return prHander;
}

int CZeratulVideoInterface::uninitZeratulOSD(int iGroupID,IMPRgnHandle *pOSDHander)
{
	int ret;

	// todo 关闭所有句柄
	ret = IMP_OSD_ShowRgn(pOSDHander[0], iGroupID, 0);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_ShowRgn close timeStamp error !\n");
	}

	ret = IMP_OSD_ShowRgn(pOSDHander[1], iGroupID, 0);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_ShowRgn close Logo error !\n");
	}

	ret = IMP_OSD_UnRegisterRgn(pOSDHander[0], iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_UnRegisterRgn timeStamp error !\n");
	}
	
	ret = IMP_OSD_UnRegisterRgn(pOSDHander[1], iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_UnRegisterRgn logo error !\n");
	}

	IMP_OSD_DestroyRgn(pOSDHander[0]);
	IMP_OSD_DestroyRgn(pOSDHander[1]);

	ret = IMP_OSD_DestroyGroup(iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_DestroyGroup(%d) error !\n", iGroupID);
		return -1;
	}
	free(pOSDHander);
	pOSDHander = NULL;

	return 0;
}

int CZeratulVideoInterface::uninitZeratulOSDShowInfo(int iGroupID,IMPRgnHandle *pOSDHander)
{
	int ret;

	// todo
	ret = IMP_OSD_ShowRgn(pOSDHander[0], iGroupID, 0);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_ShowRgn close info error !\n");
	}

	ret = IMP_OSD_UnRegisterRgn(pOSDHander[0], iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_UnRegisterRgn info error !\n");
	}

	IMP_OSD_DestroyRgn(pOSDHander[0]);

	ret = IMP_OSD_DestroyGroup(iGroupID);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_OSD_DestroyGroup(%d) error !\n", iGroupID);
		return -1;
	}
	free(pOSDHander);
	pOSDHander = NULL;

	return 0;
}

mxbool CZeratulVideoInterface::initZeratulIMP()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulIMP start !\n");

	int ret = 0;

	
    int shm_id;
    key_t shm_key;

    if ((shm_key = ftok("/etc/passwd", 12345)) == -1) {
        perror("ftok()");
        exit(1);
    }

    shm_id = shmget(shm_key, sizeof(ringbuf), IPC_CREAT |0600);
    if (shm_id < 0) {
        perror("shmget()");
        exit(1);
    }

    g_rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)g_rb == (void *)-1) {
        perror("shmat()");
        exit(1);
    }
	g_rb->ringbuf_tail = 0;
	g_rb->ringbuf_head = 0;
	g_yuv_count = 0;

	IMP_OSD_SetPoolSize(512*1024);
	//IMP_OSD_SetPoolSize_ISP(512 * 1024);

	memset(&sensor_info, 0, sizeof(sensor_info));
	memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo));
		
	IMPISPTuningOpsMode wdr_enable = m_wdr_mode ? IMPISP_TUNING_OPS_MODE_ENABLE : IMPISP_TUNING_OPS_MODE_DISABLE;
	sensor_info[0].default_boot  = m_wdr_mode ? 1 : 0;

	//直通模式下
	// IMPISPWdrOpenAttr wdr_attr;	
	// wdr_attr.rmode = m_wdr_mode ? IMPISP_TYPE_RUN_WDR : IMPISP_TYPE_RUN_LINEAR;
	// wdr_attr.imode = m_wdr_mode_init ? IMPISP_TYPE_WDR_DOL : IMPISP_TYPE_WDR_FS;

	// IMP_ISP_WDR_OPEN(IMPVI_MAIN, &wdr_attr);

	/* open isp */
	ret = IMP_ISP_Open();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed to open ISP !\n");
		return false;
	}

    ret = IMP_ISP_WDR_ENABLE(IMPVI_MAIN,&wdr_enable);
    if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed IMP_ISP_WDR_ENABLE !\n");
        return -1;
    }

	/* add sensor */
	ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed to AddSensor !\n");
		return false;
	}

	/* enable sensor */
	ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed to EnableSensor !\n");
		return false;
	}

	ret = IMP_Encoder_SetIvpuBsSize(1);
    if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetIvpuBsSize failed !\n");
        return false;
    }

    // ret = IMP_Encoder_SetAvpuBsSize(1695232);
    // if(ret < 0){
    //  IMP_LOG_ERR(TAG, "IMP_Encoder_SetAvpuBsSize failed\n");
    //  return false;
    // }

    // ret = IMP_Encoder_SetAvpuBsShare(1, 1846592);
    // if(ret < 0){
	// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetAvpuBsShare failed !\n");
    //     return false;
    // }

	/* init imp system */
	ret = IMP_System_Init();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_System_Init failed !\n");
		return false;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_EnableTuning failed !\n");
		return false;
	}

    // unsigned char sharpnessValue = 100;
    // IMP_ISP_Tuning_SetSharpness(IMPVI_MAIN, &sharpnessValue);
    // if(ret){
    // 	logPrint(MX_LOG_ERROR,  "IMP_ISP_Tuning_SetSharpness error !\n");
    // 	return -1;
    // }

	/* set contrast, sharpness, saturation, brightness */
	unsigned char value = 128;
	IMP_ISP_Tuning_SetContrast(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetSharpness(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetSaturation(IMPVI_MAIN, &value);
	IMP_ISP_Tuning_SetBrightness(IMPVI_MAIN, &value);

	/* set runningmode */
	IMPISPRunningMode dn = IMPISP_RUNNING_MODE_DAY;
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_MAIN, &dn);
	if (ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_SetISPRunningMode failed !\n");
		return false;
	}

	/* set fps */
	IMPISPSensorFps fpsAttr;
	fpsAttr.num = FIRST_SENSOR_FRAME_RATE_NUM;
	fpsAttr.den = FIRST_SENSOR_FRAME_RATE_DEN;
	ret = IMP_ISP_Tuning_SetSensorFPS((IMPVI_NUM)0, &fpsAttr);
	if (ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_SetSensorFPS failed !\n");
		return -1;
	}

	// 该函数根据sensor属性调整图像方位,不增加内存
	// IMPISPHVFLIPAttr attr;
    // ret = IMP_ISP_Tuning_GetHVFLIP(IMPVI_MAIN, &attr);
    // if(ret){
	// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_GetHVFLIP error !\n");
    //     return -1;
    // }
    // attr.sensor_mode = IMPISP_FLIP_HV_MODE;
    // ret = IMP_ISP_Tuning_SetHVFLIP(IMPVI_MAIN, &attr);
    // if(ret){
	// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_SetHVFLIP error !\n");
    //     return -1;
    // }

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface IMPInit success !\n");
	return true;
}

mxbool CZeratulVideoInterface::unInitZeratulIMP()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface unInitZeratulIMP start !\n");

	int ret = 0;

	/* exit imp system */
	ret = IMP_System_Exit();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_System_Exit failed !\n");
		return false;
	}

	/* disable sensor */
	ret = IMP_ISP_DisableSensor(IMPVI_MAIN);
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_DisableSensor failed !\n");
		return false;
	}

	/* delete sensor */
	ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info[0]);
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_DelSensor failed !\n");
		return false;
	}

	/* disable turning */
	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_DisableTuning failed !\n");
		return false;
	}

	/* close isp */
	ret = IMP_ISP_Close();
	if(ret < 0){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed to Close ISP !\n");
		return false;
	}

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface sample_system_exit success !\n");
	return true;
}

mxbool CZeratulVideoInterface::initZeratulFramesource()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulFramesource start !\n");

	int i, ret;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (g_SChnConf[i].enable) {
			ret = IMP_FrameSource_CreateChn(g_SChnConf[i].index, &g_SChnConf[i].fs_chn_attr);
			if(ret < 0){
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_CreateChng_SChnConf[%d] error !\n", g_SChnConf[i].index);
				return false;
			}

			ret = IMP_FrameSource_SetChnAttr(g_SChnConf[i].index, &g_SChnConf[i].fs_chn_attr);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_SetChnAttr(g_SChnConf%d) error !\n", g_SChnConf[i].index);
				return false;
			}
		}
	}

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface FramesourceInit success !\n");
	return true;
}

mxbool CZeratulVideoInterface::unInitZeratulFramesource()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface unInitZeratulFramesource start !\n");

	int i, ret;

	/* destroy framesource channels */
	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (g_SChnConf[i].enable) {
			ret = IMP_FrameSource_DestroyChn(g_SChnConf[i].index);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_DestroyChn[%d] error: [%d] !\n", g_SChnConf[i].index, ret);
				return -1;
			}
		}
	}
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface unInitZeratulFramesource success !\n");
	return true;
}

mxbool CZeratulVideoInterface::zeratulFramesourceStreamOn(int chnNum)
{
    std::unique_lock<std::mutex> lock(m_mutexPlay);
	int ret = 0;
    if(chnNum == 0)
    {
        /* enable framesource channels */
        if (g_SChnConf[0].enable && !m_bFirstCHnReady) {
            ret = IMP_FrameSource_EnableChn(g_SChnConf[0].index);
            if (ret < 0) {
                logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_EnableChn failed, error g_SChnConf index: [%d] !\n", g_SChnConf[0].index);
                return false;
            }
            m_bFirstCHnReady = true;
        }
    }
    if(chnNum == 1 || chnNum == 2)
    {
        /* enable framesource channels */
        if (g_SChnConf[1].enable && !m_bSencondCHnReady) {
            ret = IMP_FrameSource_EnableChn(g_SChnConf[1].index);
            if (ret < 0) {
                logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_EnableChn failed, error g_SChnConf index: [%d] !\n", g_SChnConf[1].index);
                return false;
            }
            m_bSencondCHnReady = true;
        }
    }
    if (chnNum == 3 || chnNum == 4)
    {
        /* enable framesource channels */
        if (g_SChnConf[2].enable && !m_bThirdCHnReady) {
            ret = IMP_FrameSource_EnableChn(g_SChnConf[2].index);
            if (ret < 0) {
                logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_EnableChn failed, error g_SChnConf index: [%d] !\n", g_SChnConf[2].index);
                return false;
            }
            m_bThirdCHnReady = true;
        }     
        if(!m_bDepthReady)
        {
            ret = IMP_FrameSource_SetFrameDepth(2,1);
            if (ret < 0) {
                logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_SetFrameDepth[%d]failed !\n", 2);
                return false;
            }  

			if(zeratulGetJpegSnap() != 0)
			{
				return mxfalse;
			}
            m_bDepthReady = true;
        }

    }

	return true;
}

mxbool CZeratulVideoInterface::zeratulFramesourceStreamOff()
{
	int ret = 0, i = 0;

	/* disable framesource channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (g_SChnConf[i].enable){
			ret = IMP_FrameSource_DisableChn(g_SChnConf[i].index);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_DisableChn failed, error g_SChnConf index: [%d] !\n", g_SChnConf[i].index);
				return false;
			}
		}
		if(i == 2)
			IMP_FrameSource_SetFrameDepth(2, 0);
	}
	return true;
}

mxbool CZeratulVideoInterface::initZeratulEncoder()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulEncoder start !\n");

	int i, ret, chnNum = 0;
	int s32picWidth = 0,s32picHeight = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;
	IMPFSI2DAttr sti2dattr;

	for (i = 0; i < ENCGROUP_CHN_NUM; i++) {
		ret = IMP_Encoder_CreateGroup(i);
		if (ret < 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_CreateGroup[%d] error !\n", g_SChnConf[i].index);
			return false;
		}
		if (g_SChnConf[i].enable) {
			imp_chn_attr_tmp = &g_SChnConf[i].fs_chn_attr;
			chnNum = g_SChnConf[i].index;

			memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
			memset(&sti2dattr,0,sizeof(IMPFSI2DAttr));

			ret = IMP_FrameSource_GetI2dAttr(g_SChnConf[i].index,&sti2dattr);
			if(ret < 0){
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_GetI2dAttr[%d] error !\n", g_SChnConf[i].index);
				return false;
			}

			if((1 == sti2dattr.i2d_enable) &&
					((sti2dattr.rotate_enable) && (sti2dattr.rotate_angle == 90 || sti2dattr.rotate_angle == 270))){
				s32picWidth = (g_SChnConf[i].fs_chn_attr.picHeight);/*this depend on your sensor or channels*/
				s32picHeight = (g_SChnConf[i].fs_chn_attr.picWidth);
			}else{
				s32picWidth = g_SChnConf[i].fs_chn_attr.picWidth;
				s32picHeight =g_SChnConf[i].fs_chn_attr.picHeight;
			}

			if(chnNum == 0)
			{
				//计算目标码流
				float ratio = 1;
				if (((uint64_t)s32picWidth * s32picHeight) > (1280 * 720)) {
					ratio = log10f(((uint64_t)s32picWidth * s32picHeight) / (1280 * 720.0)) + 1;
				} else {
					ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)s32picWidth * s32picHeight)) + 1);
				}
				ratio = ratio > 0.1 ? ratio : 0.1;
				g_SEncodeChnConf[chnNum].bitrate = BITRATE_2K_Kbs;
			}
			else
			{
				//计算目标码流
				float ratio = 1;
				if (((uint64_t)s32picWidth * s32picHeight) > (1280 * 720)) {
					ratio = log10f(((uint64_t)s32picWidth * s32picHeight) / (1280 * 720.0)) + 1;
				} else {
					ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)s32picWidth * s32picHeight)) + 1);
				}
				ratio = ratio > 0.1 ? ratio : 0.1;
				g_SEncodeChnConf[chnNum].bitrate = BITRATE_720P_Kbs;
			}
				

			// 动态设置码率控制属性I帧和P帧的MaxQP和MinQP
			// int i_minqp = 0, i_maxqp = 0, p_minqp = 0, p_maxqp = 0;
			// char* iminqp = getFWParaConfig("i_minqp");
			// char* imaxqp = getFWParaConfig("i_maxqp");
			// char* pminqp = getFWParaConfig("p_minqp");
			// char* pmaxqp = getFWParaConfig("p_maxqp");

			// if (iminqp == NULL || imaxqp == NULL || pminqp == NULL || pmaxqp == NULL) {
			// 	return mxfalse;
			// }

			// i_minqp = atoi(iminqp);
			// i_maxqp = atoi(imaxqp);
			// p_minqp = atoi(pminqp);
			// p_maxqp = atoi(pmaxqp);

			// channel_attr.rcAttr.attrRcMode.attrCbr.iIPDelta = 30;
			// channel_attr.gopAttr.uGopCtrlMode = IMP_ENC_GOP_CTRL_MODE_SMARTP;
			// logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface i_minqp:[%d] i_maxqp:[%d] p_minqp:[%d] p_maxqp:[%d] !\n", i_minqp, i_maxqp, p_minqp, p_maxqp);

			if(i < 2)
			{
				g_SEncodeChnConf[chnNum].gop = 45;//imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen;
				g_SEncodeChnConf[chnNum].fps = imp_chn_attr_tmp->outFrmRateNum / imp_chn_attr_tmp->outFrmRateDen;
				g_SEncodeChnConf[chnNum].height = s32picHeight;
				g_SEncodeChnConf[chnNum].width = s32picWidth;
				g_SEncodeChnConf[chnNum].profile = IMP_ENC_PROFILE_HEVC_MAIN;
				g_SEncodeChnConf[chnNum].rcMode =  IMP_ENC_RC_MODE_CAPPED_QUALITY;

				logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface [%d]rcMode:[%d] gop:[%d] fps:[%d] !\n", chnNum,g_SEncodeChnConf[chnNum].rcMode,g_SEncodeChnConf[i].gop,g_SEncodeChnConf[i].fps);
				
				ret = IMP_Encoder_SetDefaultParam(&channel_attr, g_SEncodeChnConf[chnNum].profile, g_SEncodeChnConf[chnNum].rcMode,
						g_SEncodeChnConf[chnNum].width, g_SEncodeChnConf[chnNum].height,
						imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
						g_SEncodeChnConf[chnNum].gop, 1,
						(g_SEncodeChnConf[chnNum].rcMode == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
						g_SEncodeChnConf[chnNum].bitrate);
				if (ret < 0) 
				{
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_HEVC_MAIN IMP_Encoder_SetDefaultParam[%d] error !\n", chnNum);
					return false;
				}


					// if (0 == chnNum)
					// 	channel_attr.bEnableIvdc = true;

				if(i == 0)
                {
                    ret = IMP_Encoder_SetStreamBufSize(chnNum, 600*1024);
                    if(ret < 0){
                        logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed 0:0x%x!\n",ret);
                        return false;
                    }
                }else{
                    ret = IMP_Encoder_SetStreamBufSize(chnNum, 300*1024);
                    if(ret < 0){
                        logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed 0:0x%x!\n",ret);
                        return false;
                    }
                }
				channel_attr.rcAttr.attrRcMode.attrCappedQuality.uMaxBitRate = 3000;
				channel_attr.rcAttr.attrRcMode.attrCappedQuality.iIPDelta = -1;
                // channel_attr.rcAttr.attrRcMode.attrCappedQuality.uMaxPictureSize = 4000;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMinQP = 32;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMaxQP = 51;

				ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_HEVC_MAIN IMP_Encoder_CreateChn[%d] error !\n", chnNum);
					return false;
				}

				ret = IMP_Encoder_RegisterChn(i, chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_HEVC_MAIN IMP_Encoder_RegisterChn[%d, %d] error: [%d] !\n", i, chnNum, ret);
					return false;
				}

				ret = IMP_Encoder_SetChnMaxPictureSize(chnNum, 1600, 160);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "IMP_Encoder_SetChnMaxPictureSize[%d] error !\n", chnNum);
					return mxfalse;
				}

				// ret = IMP_Encoder_SetChnQpBoundsPerFrame(chnNum,32,38,32,48);
				// if (ret < 0) {
				// 	logPrint(MX_LOG_ERROR, "MP_Encoder_SetChnQpBoundsPerFrame[%d] error !\n", chnNum);
				// 	return mxfalse;
				// }
			}

			// if(i == 0)//jpeg
			// {
			// 	ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
			// 			g_SEncodeChnConf[chnNum].width, g_SEncodeChnConf[chnNum].height,
			// 			imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0);
			// 	if (ret < 0) 
			// 	{
			// 		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_SetDefaultParam[%d] error  !\n", chnNum);
			// 		return mxfalse;
			// 	}

			// 	ret = IMP_Encoder_SetStreamBufSize(4, 400*1024);
            //     if(ret < 0){
            //         logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed 0:0x%x!\n",ret);
            //         return false;
            //     }

			// 	// Create Channel 
			// 	ret = IMP_Encoder_CreateChn(4, &channel_attr);
			// 	if (ret < 0) {
			// 		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_CreateChn[%d] error:[%d] !\n", 4, ret);
			// 		return mxfalse;
			// 	}

			// 	// Resigter Channel 
			// 	ret = IMP_Encoder_RegisterChn(i, 4);
			// 	if (ret < 0) {
			// 		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_RegisterChn[0, %d] error: [%d] !\n", 4, ret);
			// 		return mxfalse;
			// 	}
			// }
	
			if(i == 1) //H264
			{
				chnNum = 2;
				g_SEncodeChnConf[chnNum].bitrate = BITRATE_720P_H264_Kbs;
				g_SEncodeChnConf[chnNum].gop = 45;
				g_SEncodeChnConf[chnNum].fps = imp_chn_attr_tmp->outFrmRateNum / imp_chn_attr_tmp->outFrmRateDen;
				g_SEncodeChnConf[chnNum].height = s32picHeight;
				g_SEncodeChnConf[chnNum].width = s32picWidth;
				g_SEncodeChnConf[chnNum].profile = IMP_ENC_PROFILE_AVC_MAIN;
				g_SEncodeChnConf[chnNum].rcMode =  IMP_ENC_RC_MODE_CAPPED_QUALITY;

				logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface [%d]rcMode:%d. gop:%d fps:%d !\n", chnNum,g_SEncodeChnConf[chnNum].rcMode,g_SEncodeChnConf[i].gop,g_SEncodeChnConf[i].fps);

				ret = IMP_Encoder_SetDefaultParam(&channel_attr, g_SEncodeChnConf[chnNum].profile, g_SEncodeChnConf[chnNum].rcMode,
						g_SEncodeChnConf[chnNum].width, g_SEncodeChnConf[chnNum].height,
						imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
						g_SEncodeChnConf[chnNum].gop, 1,
						(g_SEncodeChnConf[chnNum].rcMode == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
						g_SEncodeChnConf[chnNum].bitrate);
				if (ret < 0) 
				{
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_SetDefaultParam[%d] error !\n", chnNum);
					return false;
				}

				ret = IMP_Encoder_SetStreamBufSize(chnNum, 600*1024);
                if(ret < 0){
                    logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed 0:0x%x!\n",ret);
                    return false;
                }

				channel_attr.rcAttr.attrRcMode.attrCappedQuality.uMaxBitRate = 1000;
				channel_attr.rcAttr.attrRcMode.attrCappedQuality.iIPDelta = -1;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.uMaxPictureSize = 1000;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMinQP = 25;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMaxQP = 48;

				ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_CreateChn[%d] error !\n", chnNum);
					return false;
				}

				ret = IMP_Encoder_RegisterChn(i, chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_RegisterChn[%d, %d] error: [%d] !\n", i, chnNum, ret);
					return false;
				}

				// ret = IMP_Encoder_SetChnQpBoundsPerFrame(chnNum,32,38,32,48);
				// if (ret < 0) {
				// 	logPrint(MX_LOG_ERROR, "MP_Encoder_SetChnQpBoundsPerFrame[%d] error !\n", chnNum);
				// 	return mxfalse;
				// }
			}
			if(i == 2) //H265---480p--jpeg
			{
				chnNum = 3;
				g_SEncodeChnConf[chnNum].bitrate = 500;
				g_SEncodeChnConf[chnNum].gop = 45;
				g_SEncodeChnConf[chnNum].fps = imp_chn_attr_tmp->outFrmRateNum / imp_chn_attr_tmp->outFrmRateDen;
				g_SEncodeChnConf[chnNum].height = s32picHeight;
				g_SEncodeChnConf[chnNum].width = s32picWidth;
				g_SEncodeChnConf[chnNum].profile = IMP_ENC_PROFILE_HEVC_MAIN;
				g_SEncodeChnConf[chnNum].rcMode =  IMP_ENC_RC_MODE_CAPPED_QUALITY;

				logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface [%d]rcMode:[%d] gop:[%d] fps:[%d]!\n", chnNum,g_SEncodeChnConf[chnNum].rcMode,g_SEncodeChnConf[i].gop,g_SEncodeChnConf[i].fps);

				ret = IMP_Encoder_SetDefaultParam(&channel_attr, g_SEncodeChnConf[chnNum].profile, g_SEncodeChnConf[chnNum].rcMode,
						g_SEncodeChnConf[chnNum].width, g_SEncodeChnConf[chnNum].height,
						imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
						g_SEncodeChnConf[chnNum].gop, 1,
						(g_SEncodeChnConf[chnNum].rcMode == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
						g_SEncodeChnConf[chnNum].bitrate);
				if (ret < 0) 
				{
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_SetDefaultParam[%d] error !\n", chnNum);
					return false;
				}

				channel_attr.rcAttr.attrRcMode.attrCappedQuality.iIPDelta = -1;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.uMaxPictureSize = 3000;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMinQP = 25;
                channel_attr.rcAttr.attrRcMode.attrCappedQuality.iMaxQP = 48;
				ret = IMP_Encoder_SetStreamBufSize(chnNum, 200*1024);
                if(ret < 0){
                    logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed 0:0x%x!\n",ret);
                    return false;
                }

				ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_CreateChn[%d] error !\n", chnNum);
					return false;
				}

				ret = IMP_Encoder_RegisterChn(i, chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_AVC_MAIN IMP_Encoder_RegisterChn[%d, %d] error: [%d] !\n", i, chnNum, ret);
					return false;
				}

				// ret = IMP_Encoder_SetChnQpBoundsPerFrame(chnNum,32,38,32,48);
				// if (ret < 0) {
				// 	logPrint(MX_LOG_ERROR, "MP_Encoder_SetChnQpBoundsPerFrame[%d] error !\n", chnNum);
				// 	return mxfalse;
				// }


				//480p---jpeg
				ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
						g_SEncodeChnConf[chnNum].width, g_SEncodeChnConf[chnNum].height,
						imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, 25, 0);
				if (ret < 0) 
				{
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_SetDefaultParam[%d] error  !\n", chnNum);
					return mxfalse;
				}

				ret = IMP_Encoder_SetStreamBufSize(4, 200*1024);
				if(ret < 0){
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_SetStreamBufSize failed11 0:0x%x!\n",ret);
					return false;
				}

				// Create Channel 
				ret = IMP_Encoder_CreateChn(4, &channel_attr);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_CreateChn[%d] error:[%d] !\n", 4, ret);
					return mxfalse;
				}

				// Resigter Channel 
				ret = IMP_Encoder_RegisterChn(i, 4);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ENC_PROFILE_JPEG IMP_Encoder_RegisterChn[0, %d] error: [%d] !\n", 4, ret);
					return mxfalse;
				} 	  
			}
		}	 	   
	}

	for (i = 0; i < ENCGROUP_CHN_NUM; i++) {
		if (g_SChnConf[i].enable) {
			ret = IMP_System_Bind(&g_SChnConf[i].framesource_chn, &g_SChnConf[i].osdcell);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface Bind FrameSource channel[%d] and OSD failed !\n", i);
				return false;
			}
			ret = IMP_System_Bind(&g_SChnConf[i].osdcell, &g_SChnConf[i].imp_encoder);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface Bind OSD channel[%d] and Encoder failed !\n", i);
				return false;
			}
			// ret = IMP_System_Bind(&g_SChnConf[i].framesource_chn, &g_SChnConf[i].imp_encoder);
			// if (ret < 0) {
			// logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface Bind FrameSource channel[0] and Encoder failed !\n", i);
			// 	return false;
			// }
		}
	}

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface initZeratulEncoder success !\n");
	return true;
}

mxbool CZeratulVideoInterface::unInitZeratulEncoder()
{
	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface unInitZeratulEncoder start !\n");
	
	int ret = 0, i = 0, chnNum = 0;
	IMPEncoderChnStat chn_stat;

	for (i = 0; i < ENCGROUP_CHN_NUM; i++) {
		if (g_SChnConf[i].enable) {
			ret = IMP_System_UnBind(&g_SChnConf[i].osdcell, &g_SChnConf[i].imp_encoder);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface UnBind OSD and Encoder failed !\n");
				return false;
			}
			ret = IMP_System_UnBind(&g_SChnConf[i].framesource_chn, &g_SChnConf[i].osdcell);
			if (ret < 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface UnBind FrameSource and OSD failed !\n");
				return false;
			}
		}
	}

	for (i = 0; i <  ENCODE_CHN_NUM; i++) {
		chnNum = i;
		memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
		ret = IMP_Encoder_Query(chnNum, &chn_stat);
		if (ret < 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_Query[%d] error: [%d] !\n", chnNum, ret);
			return false;
		}
		if(chnNum == 0)
		{
			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_UnRegisterChn[%d] error: [%d] !\n", chnNum, ret);
					return false;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyChn[%d] error: [%d] !\n", chnNum, ret);
					return false;
				}

				ret = IMP_Encoder_UnRegisterChn(4);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_UnRegisterChn[%d] error:[%d] !\n", 4, ret);
					return false;
				}

				ret = IMP_Encoder_DestroyChn(4);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyChn[%d] error:[%d] !\n", 4, ret);
					return false;
				}

				ret = IMP_Encoder_DestroyGroup(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyGroup[%d] error:[%d] !\n", chnNum, ret);
					return false;
				}
			}
		}
		else if(chnNum == 1)
		{
			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_UnRegisterChn[%d] error: [%d] !\n", chnNum, ret);
					return false;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyChn[%d] error:[%d] !\n", chnNum, ret);
					return false;
				}
			}
		}
		else if(chnNum == 2)
		{
			if (chn_stat.registered) {
				ret = IMP_Encoder_UnRegisterChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_UnRegisterChn[%d] error:[%d] !\n", chnNum, ret);
					return false;
				}

				ret = IMP_Encoder_DestroyChn(chnNum);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyChn[%d] error:[%d] !\n", chnNum, ret);
					return false;
				}
				ret = IMP_Encoder_DestroyGroup(chnNum-1);
				if (ret < 0) {
					logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_DestroyGroup[%d] error:[%d] !\n", chnNum, ret);
					return false;
				}
			}
		}
	}

	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface unInitZeratulEncoder success !\n");
	return false;
}
mxbool CZeratulVideoInterface::initLogoData()
{
	//480p
	g_SLogoParameter[2].logodata = log_480p_bgra;
	g_SLogoParameter[2].height = 32;
	g_SLogoParameter[2].width = 31;
	g_SLogoParameter[2].num_data[0] = number_0_480p_bgra;
	g_SLogoParameter[2].num_data[1] = number_1_480p_bgra;
	g_SLogoParameter[2].num_data[2] = number_2_480p_bgra;
	g_SLogoParameter[2].num_data[3] = number_3_480p_bgra;
	g_SLogoParameter[2].num_data[4] = number_4_480p_bgra;
	g_SLogoParameter[2].num_data[5] = number_5_480p_bgra;
	g_SLogoParameter[2].num_data[6] = number_6_480p_bgra;
	g_SLogoParameter[2].num_data[7] = number_7_480p_bgra;
	g_SLogoParameter[2].num_data[8] = number_8_480p_bgra;
	g_SLogoParameter[2].num_data[9] = number_9_480p_bgra;
	g_SLogoParameter[2].num_data[10] = slash_480p_bgra;
	g_SLogoParameter[2].num_data[11] = colon_480p_bgra;
	g_SLogoParameter[2].num_height = 32;
	g_SLogoParameter[2].num_width = 14;

	//720p
	g_SLogoParameter[1].logodata = log_720p_bgra;
	g_SLogoParameter[1].height = 46;
	g_SLogoParameter[1].width = 46;
	g_SLogoParameter[1].num_data[0] = number_0_720p_bgra;
	g_SLogoParameter[1].num_data[1] = number_1_720p_bgra;
	g_SLogoParameter[1].num_data[2] = number_2_720p_bgra;
	g_SLogoParameter[1].num_data[3] = number_3_720p_bgra;
	g_SLogoParameter[1].num_data[4] = number_4_720p_bgra;
	g_SLogoParameter[1].num_data[5] = number_5_720p_bgra;
	g_SLogoParameter[1].num_data[6] = number_6_720p_bgra;
	g_SLogoParameter[1].num_data[7] = number_7_720p_bgra;
	g_SLogoParameter[1].num_data[8] = number_8_720p_bgra;
	g_SLogoParameter[1].num_data[9] = number_9_720p_bgra;
	g_SLogoParameter[1].num_data[10] = slash_720p_bgra;
	g_SLogoParameter[1].num_data[11] = colon_720p_bgra;
	g_SLogoParameter[1].num_height = 46;
	g_SLogoParameter[1].num_width = 21;

	//2.5k
	g_SLogoParameter[0].logodata = log_2k_bgra;
	g_SLogoParameter[0].height = 92;
	g_SLogoParameter[0].width = 92;
	g_SLogoParameter[0].num_data[0] = number_0_2k_bgra;
	g_SLogoParameter[0].num_data[1] = number_1_2k_bgra;
	g_SLogoParameter[0].num_data[2] = number_2_2k_bgra;
	g_SLogoParameter[0].num_data[3] = number_3_2k_bgra;
	g_SLogoParameter[0].num_data[4] = number_4_2k_bgra;
	g_SLogoParameter[0].num_data[5] = number_5_2k_bgra;
	g_SLogoParameter[0].num_data[6] = number_6_2k_bgra;
	g_SLogoParameter[0].num_data[7] = number_7_2k_bgra;
	g_SLogoParameter[0].num_data[8] = number_8_2k_bgra;
	g_SLogoParameter[0].num_data[9] = number_9_2k_bgra;
	g_SLogoParameter[0].num_data[10] = slash_2k_bgra;
	g_SLogoParameter[0].num_data[11] = colon_2k_bgra;
	g_SLogoParameter[0].num_height = 92;
	g_SLogoParameter[0].num_width = 42;

	return mxtrue;
}

mxbool CZeratulVideoInterface::init()
{
    if(m_bInit)
    {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface CVideoInputEntity init err : has been init !\n");
        return mxfalse;
    }

	if(loadTagConfig() != 0)
	{
		return mxfalse;
	}

	if (!initZeratulIMP())
		return mxfalse;

	if (!initZeratulFramesource())
		return mxfalse;

	initLogoData();
	m_pOSDHander[0] = initZeratulOSD(0,&g_SLogoParameter[0],10,10);
	if(m_pOSDHander[0] == NULL)
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface initZeratulOSD and initZeratulOSDShowInfo failed !\n");
		return mxfalse;
	}
	
	m_pOSDHander[1] = initZeratulOSD(1,&g_SLogoParameter[1],10,10);
	if(m_pOSDHander[1] == NULL)
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface initZeratulOSD and initZeratulOSDShowInfo failed !\n");
		return mxfalse;
	}

	m_pOSDHander[2] = initZeratulOSD(2,&g_SLogoParameter[2],10,10);
	if(m_pOSDHander[2] == NULL)
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface initZeratulOSD and initZeratulOSDShowInfo failed !\n");
		return mxfalse;
	}

	if (!initZeratulEncoder())
		return mxfalse;

	// if (!zeratulFramesourceStreamOn())
	// 	return mxfalse;

	// if(m_wdr_mode)
    // {
    //     zeratulISPWdrModeSwitch("wdr_mode:0");  //临时关闭HDR，便于调试
    // }

	

	if(m_OSDPushState)
    {
        startOSDShowRgn(0);
        startOSDShowRgn(1);
		startOSDShowRgn(2);
    }

	// if(zeratulGetJpegSnap() != 0)
	// {
	// 	return mxfalse;
	// }

	m_frameBuff = (uint8_t *)malloc(VIDEO_BUF_SIZE);
	if(m_frameBuff == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface readFrame malloc video buf error !\n");
		return mxfalse;
	}
    m_frameBuff_1 = (uint8_t *)malloc(VIDEO_BUF_SIZE);
	if(m_frameBuff_1 == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface readFrame malloc video m_frameBuff_1 error !\n");
		return mxfalse;
	}
    m_frameBuff_2 = (uint8_t *)malloc(VIDEO_BUF_SIZE);
	if(m_frameBuff_2 == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface readFrame malloc video m_frameBuff_2 error !\n");
		return mxfalse;
	}
    m_frameBuff_3 = (uint8_t *)malloc(VIDEO_BUF_SIZE);
	if(m_frameBuff_3 == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface readFrame malloc video m_frameBuff_3 error !\n");
		return mxfalse;
	}
    m_frameBuff_yuv = (uint8_t *)malloc(VIDEO_BUF_SIZE);
	if(m_frameBuff_yuv == NULL) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface readFrame malloc video m_frameBuff_yuv error !\n");
		return mxfalse;
	}

	std::unique_lock<std::mutex> lock(m_mutexLightSensor);
	m_setLedMode = m_dn_mode;
	if (!initSetLedMode())
			return mxfalse;

	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface zeratul_video_interface Init success !\n");
	m_bInit = true;
    return mxtrue;
}

mxbool CZeratulVideoInterface::unInit()
{
    if(!m_bInit)
    {
        return mxfalse;
    }
	if (!zeratulFramesourceStreamOff())
		return mxfalse;
	if (!unInitZeratulEncoder())
		return mxfalse;

	for (int i = 0; i < ENCGROUP_CHN_NUM+1; i++) 
	{
		if(uninitZeratulOSD(i, m_pOSDHander[i]) != 0)
		{
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface uninitZeratulOSD failed !\n");
			return mxfalse;
		}

		// if(uninitZeratulOSDShowInfo(i, m_pOSDHander[i]) != 0)
		// {
		// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface uninitZeratulOSDShowInfo failed !\n");
		// 	return mxfalse;
		// }
	}

	if (!unInitZeratulFramesource())
		return mxfalse;
	if (!unInitZeratulIMP())
		return mxfalse;

	if(m_frameBuff != NULL)
	{
		free(m_frameBuff);
        m_frameBuff = NULL;
	}
    if(m_frameBuff_1 != NULL)
	{
		free(m_frameBuff_1);
        m_frameBuff_1 = NULL;
	}
    if(m_frameBuff_2 != NULL)
	{
		free(m_frameBuff_2);
        m_frameBuff_2 = NULL;
	}
    if(m_frameBuff_3 != NULL)
	{
		free(m_frameBuff_3);
        m_frameBuff_3 = NULL;
	}
    if(m_frameBuff_yuv != NULL)
	{
		free(m_frameBuff_yuv);
        m_frameBuff_yuv = NULL;
	}

	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface zeratul_video_interface DeInit success !\n");
	m_bInit = false;
    return mxtrue;
}

int CZeratulVideoInterface::zeratulEncoderStartRcvePic(int chnNum)
{
	if(chnNum > 4 || chnNum < 0)
	{
		return -1;
	}
    if (!zeratulFramesourceStreamOn(chnNum))
    {
        printf("zeratulFramesourceStreamOn err %d \n",chnNum);
        return -1;
    }

    if(chnNum <= 3 && chnNum >= 0)
    {
        int ret = IMP_Encoder_StartRecvPic(chnNum);
        if (ret < 0) 
        {
            logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_StartRecvPic[%d] failed !\n", chnNum);
            return -1;
        }
    }
	return 0;
}

int CZeratulVideoInterface::zeratulGetFrame(IMPEncoderStream *stream,int chnNum,int delay)
{
	int ret = IMP_Encoder_PollingStream(chnNum, delay);
	if (ret < 0) 
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_PollingStream[%d] timeout !\n", chnNum);
		return ret;
	}

	/* Get H264 or H265 Stream */
	ret = IMP_Encoder_GetStream(chnNum, stream, 1);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_GetStream[%d] failed !\n", chnNum);
		return -1;
	}
	return ret;
}

int CZeratulVideoInterface::zeratulCopyFrameToBuffer(IMPEncoderStream *stream, unsigned char *buf, size_t bufSize)
{
    if (stream == nullptr || buf == nullptr || bufSize == 0) {
        return -1;
    }

    int nr_pack = stream->packCount;
    unsigned char *buf_tmp = buf;
    size_t ui32FrameSize = 0;

    for (int i = 0; i < nr_pack; i++) {
        IMPEncoderPack *pack = &stream->pack[i];

        if (pack->length) {
            size_t remSize = stream->streamSize - pack->offset;

            if (remSize < pack->length) {
                if (ui32FrameSize + remSize > bufSize) {
                    return -1;
                }
                memcpy(buf_tmp, (void *)(stream->virAddr + pack->offset), remSize);
                buf_tmp += remSize;
                ui32FrameSize += remSize;

                if (ui32FrameSize + (pack->length - remSize) > bufSize) {
                    return -1;
                }
                memcpy(buf_tmp, (void *)stream->virAddr, pack->length - remSize);
                buf_tmp += (pack->length - remSize);
                ui32FrameSize += (pack->length - remSize);
            } else {
                if (ui32FrameSize + pack->length > bufSize) {
                    return -1;
                }
                memcpy(buf_tmp, (void *)(stream->virAddr + pack->offset), pack->length);
                buf_tmp += pack->length;
                ui32FrameSize += pack->length;
            }
        }
    }

    return ui32FrameSize;
}

int CZeratulVideoInterface::zeratulReleaseFrame(IMPEncoderStream *stream,int chnNum)
{
	return IMP_Encoder_ReleaseStream(chnNum, stream);
}

int CZeratulVideoInterface::zeratulGetYuvFrame(int chnNum, IMPFrameInfo **frame)
{
	// int ret = IMP_FrameSource_SetFrameDepth(chnNum-2, g_SChnConf[chnNum-2].fs_chn_attr.nrVBs * 2);
	// if (ret < 0) {
	// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_SetFrameDepth[%d]failed !\n", chnNum);
	// 	return ret;
	// }

	int ret = IMP_FrameSource_GetFrame(chnNum-2, frame);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_GetFrame[%d] failed !\n", chnNum);
		return ret;
	}
	// logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface ZvmGetYuvFrame, frame->size=[%d] !\n",(*frame)->size);
	return ret;
}

int CZeratulVideoInterface::zeratulReleaseYuvFrame(int chnNum, IMPFrameInfo *frame)
{
	int ret = IMP_FrameSource_ReleaseFrame(chnNum-2, frame);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_FrameSource_ReleaseFrame[%d] failed !\n",chnNum);
		return ret;
	}
	// ret = IMP_FrameSource_SetFrameDepth(chnNum-2, 0);
	return ret;
}

mxbool CZeratulVideoInterface::startRcvFrame(int chnNum)
{
	if(zeratulEncoderStartRcvePic(chnNum)!= 0)
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Get_Video_Thread zeratulEncoderStartRcvePic error,chnNum[%d] !\n",chnNum);
		return mxfalse;
	}
	return mxtrue;
}

mxbool CZeratulVideoInterface::getIDRFrame(int chnNum)
{
	if(IMP_Encoder_RequestIDR(chnNum) != 0)
	{
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_Encoder_RequestIDR error,chnNum[%d] !\n",chnNum);
        return mxfalse;
	}
	return mxtrue;
}

mxbool CZeratulVideoInterface::zeratulOSDrefresh(int iChnNum)
{

	//logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface The iChnNum:[%d] that needs to be watermarked !\n",iChnNum);
	if(iChnNum == 1 || iChnNum == 2)
	{
		iChnNum = 1;
	}else if(iChnNum == 3)
	{
		iChnNum = 2;
	}

	char acDateStr[40];
	time_t tCurrTime;
	struct tm *T_SCurrDate;
	unsigned i = 0;
	IMPOSDRgnAttr rAttr;
	unsigned int iLen = 0;

	IMP_OSD_GetRgnAttr(m_pOSDHander[iChnNum][0], &rAttr);
	time(&tCurrTime);
	T_SCurrDate = localtime(&tCurrTime);
	memset(acDateStr, 0, 40);
	strftime(acDateStr, 40, "%Y/%m/%d%H:%M:%S", T_SCurrDate);
	iLen = strlen(acDateStr);
	for (i = 0; i < iLen; i++) {
		if(g_SLogoParameter[iChnNum].date[i] != acDateStr[i]){
			
			IMPOSDRgnAttr prAttr;
			IMP_OSD_ShowRgn(m_pOSDHander[iChnNum][i], iChnNum, 0);
			IMP_OSD_GetRgnAttr(m_pOSDHander[iChnNum][i], &prAttr);
			switch(acDateStr[i]) {
				case '0' ... '9':
					prAttr.data.picData.pData = g_SLogoParameter[iChnNum].num_data[acDateStr[i]-'0'];
					break;
				case '/':
					prAttr.data.picData.pData = g_SLogoParameter[iChnNum].num_data[10];
					break;
				case ':':
					prAttr.data.picData.pData = g_SLogoParameter[iChnNum].num_data[11];
					break;
				default:
					break;
			}
			IMP_OSD_SetRgnAttr(m_pOSDHander[iChnNum][i], &prAttr);
			if(T_SCurrDate->tm_year != 70)
			{
				IMP_OSD_ShowRgn(m_pOSDHander[iChnNum][i], iChnNum, 1);
			}
		}
	}

	strcpy(g_SLogoParameter[iChnNum].date,acDateStr);

	int error_int_num = getErrorIntNum();
	if(m_errorIntNum != error_int_num)
	{
		m_restSensorFlag = 1;
	}

	return true;
}

mxbool CZeratulVideoInterface::zeratulOSDInforefresh(int iChnNum)
{
	if(iChnNum == 1 || iChnNum == 2)
	{
		iChnNum = 1;
	}else if(iChnNum == 3)
	{
		iChnNum = 2;
	}

	IMPRgnHandle *prHander = m_pOSDInfoHander[iChnNum];
	int iRet = IMP_OSD_ShowRgn(prHander[0], iChnNum, 1);
	if (iRet != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface StartOSDShowRgn IMP_OSD_ShowRgn iChnNum[%d],iRet[%d] OSDinfo error!\n",iChnNum,iRet);
		return -1;
	}

	/*generate time*/
	char acInfoStr[40];
	unsigned i = 0, j = 0;
	void *pInfoData = NULL;
	uint32_t *pData = NULL;
	IMPOSDRgnAttr rAttr;

	int iPenpos = 0;
	int iPenpos_t = 0;
	int iInfoadv = 0;
	unsigned int iLen = 0;

	iRet = IMP_OSD_GetRgnAttr(m_pOSDInfoHander[iChnNum][0], &rAttr);
	// int result = IMP_OSD_GetRegionLuma(m_pOSDInfoHander[iChnNum][0], &rAttr);
	// if(result == -1 )
	// {
	// 	return mxfalse;
	// }

	pData = (uint32_t *)rAttr.data.bitmapData;
	memset(acInfoStr, 0, 40);

	IMPISPAEExprInfo info;                                      //获取AE统计信息
	iRet = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
	if(iRet){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_GetAeExprInfo error !\n");
		return mxfalse;
	}

	IMPISPWBAttr awb_attr;                                      //获取AWB属性
	iRet = IMP_ISP_Tuning_GetAwbAttr(IMPVI_MAIN, &awb_attr);
	if(iRet){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_GetAwbAttr error !\n");
		return mxfalse;
	}

	IMPISPAEScenceAttr scence_attr;                             //获取AE场景模式信息
	iRet = IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_MAIN, &scence_attr);
	if(iRet){
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface IMP_ISP_Tuning_GetAeScenceAttr error !\n");
		return mxfalse;
	}
	//依次为：AE 手动模式下的曝光值； AE total gain；AE短帧，AE Luma值；AE场景luma值；白平衡当前色温值
	sprintf(acInfoStr," %d %d %d %d %d %d %d",info.AeIntegrationTime,info.AeShortIntegrationTime,info.TotalGainDb,info.TotalGainDbShort,
		scence_attr.luma,scence_attr.luma_scence,awb_attr.ct);
	iLen = strlen(acInfoStr);
	// logPrint(MX_LOG_ERROR, "[%d]%s\n",iLen,acInfoStr);
	for (i = 0; i < iLen; i++) {
		switch(acInfoStr[i]) {
			case '0' ... '9':
#ifdef SUPPORT_COLOR_REVERSE
				if(rAttr.fontData.colType[i] == 1) {
					pInfoData = (void *)gBitmap_black[acInfoStr[i] - '0'].pdata;
				} else {
					pInfoData = (void *)T_SBitmapInfo[acInfoStr[i] - '0'].pdata;
				}
#else
				pInfoData = (void *)T_SBitmapInfo[acInfoStr[i] - '0'].pdata;
#endif
				iInfoadv = T_SBitmapInfo[acInfoStr[i] - '0'].width;
				iPenpos_t += T_SBitmapInfo[acInfoStr[i] - '0'].width;
				break;
			case '-':
#ifdef SUPPORT_COLOR_REVERSE
				if(rAttr.fontData.colType[i] == 1) {
					pInfoData = (void *)gBitmap_black[10].pdata;
				} else {
					pInfoData = (void *)T_SBitmapInfo[10].pdata;
				}
#else
				pInfoData = (void *)T_SBitmapInfo[10].pdata;
#endif
				iInfoadv = T_SBitmapInfo[10].width;
				iPenpos_t += T_SBitmapInfo[10].width;
				break;
			case ' ':
				pInfoData = (void *)T_SBitmapInfo[11].pdata;
				iInfoadv = T_SBitmapInfo[11].width;
				iPenpos_t += T_SBitmapInfo[11].width;
				break;
			case ':':
#ifdef SUPPORT_COLOR_REVERSE
				if(rAttr.fontData.colType[i] == 1) {
					pInfoData = (void *)gBitmap_black[12].pdata;
				} else {
					pInfoData = (void *)T_SBitmapInfo[12].pdata;
				}
#else
				pInfoData = (void *)T_SBitmapInfo[12].pdata;
#endif
				iInfoadv = T_SBitmapInfo[12].width;
				iPenpos_t += T_SBitmapInfo[12].width;
				break;
			default:
				break;
		}
#ifdef SUPPORT_RGB555LE
		for (j = 0; j < OSD_REGION_HEIGHT; j++) {
			memcpy((void *)((uint16_t *)pData + j*40*OSD_REGION_WIDTH + iPenpos_t),
					(void *)((uint16_t *)pInfoData + j*iInfoadv), iInfoadv*sizeof(uint16_t));
		}
#else
		for (j = 0; j < gBitmapHight; j++) {
			memcpy((void *)((intptr_t)pData + j*20*32 + iPenpos),
					(void *)((intptr_t)pInfoData + j*iInfoadv), iInfoadv);
		}
		iPenpos = iPenpos_t;
#endif
	}
	return true;
}

unsigned char *CZeratulVideoInterface::readFrame(int chnNum, int *size)
{
	if(chnNum >=0 && chnNum <= 3)
	{
		IMPEncoderStream stream;
		
		while(zeratulGetFrame(&stream,chnNum,1000) != 0)
		{
			continue;
		}
		uint32_t ui32FrameSize = zeratulCopyFrameToBuffer(&stream, m_frameBuff, VIDEO_BUF_SIZE);
		*size = ui32FrameSize;
		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface IMP_Get_Video_Thread ZvmGetFrame ui32FrameSize=[%d],chnNum[%d],stream.seq[%d] !\n",ui32FrameSize,chnNum,stream.seq);
		zeratulReleaseFrame(&stream,chnNum);
		if(m_OSDPushState == 1)
			zeratulOSDrefresh(chnNum);
		return m_frameBuff;
	}else{
		IMPFrameInfo *yuvframe;
		while(zeratulGetYuvFrame(chnNum, &yuvframe) != 0)
		{
			continue;
		}
		// *size = yuvframe->size;
		*size = 0;
		if ((g_rb->ringbuf_tail + 1) % 3 != g_rb->ringbuf_head)
		{
			memcpy((void *)g_rb->p_ringbuf[g_rb->ringbuf_tail],(unsigned char*)yuvframe->virAddr, yuvframe->size);
			g_rb->ringbuf_tail= (g_rb->ringbuf_tail + 1) % 3;
		}

		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface ZvmGetYuvFrame  date size is ------------------------yuvframe->size[%d]-- !\n",yuvframe->size);
		zeratulReleaseYuvFrame(chnNum, yuvframe);
		return m_frameBuff;
	}
}

int count = 0;
// int count_1 = 0;
// int count_2 = 0;
// int count_3 = 0;

unsigned char *CZeratulVideoInterface::readFrame(int chnNum, int *size, int *frameType, int64_t *timestamp, int *frameSeq)
{
	if(m_restSensorFlag)   //重置sensor传感器
	{
		if(!unInit())
		{
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface zeratul_video_interface unInit failed!\n");
		}
		if(!init())
		{
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface zeratul_video_interface Init failed!\n");
		}
		m_restSensorFlag = 0;
	}
	if(chnNum >=0 && chnNum <= 3)
	{
		IMPEncoderStream stream;
		
		while(zeratulGetFrame(&stream,chnNum,1000) != 0)
		{
			continue;
		}

		*timestamp = stream.pack[stream.packCount-1].timestamp;
		*frameSeq = stream.seq;

		if(stream.pack[stream.packCount-1].sliceType == IMP_ENC_SLICE_I)
		{
			*frameType = 1;//I帧
		}else if(stream.pack[stream.packCount-1].sliceType == IMP_ENC_SLICE_P)
		{
			*frameType = 0;//P帧
		}else if(stream.pack[stream.packCount-1].sliceType == IMP_ENC_SLICE_B)
		{
			*frameType = 2;//B帧
		}
        if(m_OSDPushState == 1 && stream.seq % 5 == 0)
		{
			zeratulOSDrefresh(chnNum);
			// zeratulOSDInforefresh(chnNum);
		}

        if (chnNum == 0)
        {
            uint32_t ui32FrameSize = zeratulCopyFrameToBuffer(&stream, m_frameBuff, VIDEO_BUF_SIZE);
            *size = ui32FrameSize;
            // logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface IMP_Get_Video_Thread ZvmGetFrame ui32FrameSize=[%d],chnNum[%d],frameType[%d],stream.seq[%d]!\n", ui32FrameSize,chnNum,*frameType,stream.seq);
            // if(chnNum == 0 && count <= 500){
            //     if(count == 0)
            //     {
            //         system("rm -rf /userfs/msgrcvV1234_3000-4000-old_2_5p_h265");
            //     }
            //     int fd = open("/userfs/1500-1800-old_2_5p_h265", O_RDWR | O_CREAT | O_APPEND, 0x644);
            //     write(fd, m_frameBuff, ui32FrameSize);
            //     close(fd);
            //     count++;
            // }
			// if(count == 500)
			// {
			// 	printf("===============save end\n");
			// }
            zeratulReleaseFrame(&stream,chnNum);
            return m_frameBuff;
        }
        else if(chnNum == 1)
        {
            uint32_t ui32FrameSize = zeratulCopyFrameToBuffer(&stream, m_frameBuff_1, VIDEO_BUF_SIZE);
            *size = ui32FrameSize;
            // if(chnNum == 1 && count_1 <= 4500){
            //     if(count_1 == 0)
            //     {
            //         system("rm -rf /tmp/mnt/sdcard/msgrcvV1234_media_source_720p_h265");
            //     }
            //     int fd = open("/tmp/mnt/sdcard/msgrcvV1234_media_source_720p_h265", O_RDWR | O_CREAT | O_APPEND, 0x644);
            //     write(fd, m_frameBuff_1, ui32FrameSize);
            //     close(fd);
            //     count_1++;
            // }
            zeratulReleaseFrame(&stream,chnNum);
            return m_frameBuff_1;
        }
        else if(chnNum == 2)
        {
            uint32_t ui32FrameSize = zeratulCopyFrameToBuffer(&stream, m_frameBuff_2, VIDEO_BUF_SIZE);
            *size = ui32FrameSize;
            // if(chnNum == 2 && count_2 <= 4500){
            //     if(count_2 == 0)
            //     {
            //         system("rm -rf /tmp/mnt/sdcard/msgrcvV1234_media_source_720p_h264");
            //     }
            //     int fd = open("/tmp/mnt/sdcard/msgrcvV1234_media_source_720p_h264", O_RDWR | O_CREAT | O_APPEND, 0x644);
            //     write(fd, m_frameBuff_2, ui32FrameSize);
            //     close(fd);
            //     count_2++;
            // }
            zeratulReleaseFrame(&stream,chnNum);
            return m_frameBuff_2;
        }
        else if(chnNum == 3)
        {
            uint32_t ui32FrameSize = zeratulCopyFrameToBuffer(&stream, m_frameBuff_3, VIDEO_BUF_SIZE);
            *size = ui32FrameSize;
            // if(chnNum == 3 && count_3 <= 4500){
            //     if(count_3 == 0)
            //     {
            //         system("rm -rf /tmp/mnt/sdcard/msgrcvV1234_media_source_480p_h265");
            //     }
            //     int fd = open("/tmp/mnt/sdcard/msgrcvV1234_media_source_480p_h265", O_RDWR | O_CREAT | O_APPEND, 0x644);
            //     write(fd, m_frameBuff_3, ui32FrameSize);
            //     close(fd);
            //     count_3++;
            // }
            zeratulReleaseFrame(&stream,chnNum);
            return m_frameBuff_3;
        }
	}
    else{
		IMPFrameInfo *yuvframe;
		while(zeratulGetYuvFrame(chnNum, &yuvframe) != 0)
		{
			continue;
		}
		// printf("\nWCQ==_g_yuv_count:%d.g_rb->ringbuf_tail:%d.g_rb->ringbuf_head:%d-_RINGBUF_LEN:%d\n",g_yuv_count,g_rb->ringbuf_tail,g_rb->ringbuf_head,_RINGBUF_LEN);
		if(g_yuv_count++ % 5 == 0)
		{
			if ((g_rb->ringbuf_tail + 1) % _RINGBUF_LEN != g_rb->ringbuf_head)
			{
				memcpy((void *)g_rb->p_ringbuf[g_rb->ringbuf_tail],(unsigned char*)yuvframe->virAddr, yuvframe->size);
				g_rb->ringbuf_tail= (g_rb->ringbuf_tail + 1) % _RINGBUF_LEN;
			}
			// logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface ZvmGetYuvFrame  date size is ------------------------yuvframe->size[%d]--!\n",yuvframe->size);
		}

		zeratulReleaseYuvFrame(chnNum, yuvframe);
		*size = 0;
		return m_frameBuff_yuv;
	}

	return NULL;
}

int CZeratulVideoInterface::startOSDShowRgn(int iChnNum)
{
	std::unique_lock<std::mutex> lock(m_mutexOSDShow);

	IMPRgnHandle *prHander = m_pOSDHander[iChnNum];

	// todo
	for (int i = 0; i < 19 ; i++)
	{
		int iRet = IMP_OSD_ShowRgn(prHander[i], iChnNum, 1);
		if (iRet != 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface StartOSDShowRgn IMP_OSD_ShowRgn iChnNum[%d],iRet[%d] timeStamp error!\n",iChnNum,iRet);
			return -1;
		}
	}


	m_OSDPushState = 1;
	return 0;
}

int CZeratulVideoInterface::stopOSDShowRgn(int iChnNum)
{
	std::unique_lock<std::mutex> lock(m_mutexOSDShow);
	m_OSDPushState = 0;

	//todo
	IMPRgnHandle *prHander = m_pOSDHander[iChnNum];
	for (int i = 0; i < 19 ; i++)
	{
		int iRet = IMP_OSD_ShowRgn(prHander[i], iChnNum, 0);
		if (iRet != 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface StopOSDShowRgn IMP_OSD_ShowRgn() iChnNum[%d],iRet[%d] 	timeStamp error!\n",iChnNum,iRet);
			return -1;
		}
	}

	return 0;
}


int CZeratulVideoInterface::getChnNum()
{
	int iChnNum = 0;

	if (!getConfig("INTERFACE", "CHN_NUM", iChnNum))
	{
		return 0;
	}

	return iChnNum;
}

std::string CZeratulVideoInterface::getChnName(int iNum)
{
	std::string strName;
	switch (iNum)
	{
	case 0 :
		if (getConfig("CHN_NUM_1", "NAME", strName))
		{
			return strName;
		}
		break;
	case 1 :
		if (getConfig("CHN_NUM_2", "NAME", strName))
		{
			return strName;
		}
		break;
	case 2 :
		if (getConfig("CHN_NUM_3", "NAME", strName))
		{
			return strName;
		}
		break;
	case 3 :
		if (getConfig("CHN_NUM_4", "NAME", strName))
		{
			return strName;
		}
		break;
	case 4 :
		if (getConfig("CHN_NUM_5", "NAME", strName))
		{
			return strName;
		}
		break;
	default:
		return strName;
	}

	return strName;
}

int CZeratulVideoInterface::getChnSN(int iNum)
{
	int sn = -1;
	switch (iNum)
	{
	case 0 :
		if (getConfig("CHN_NUM_1", "SN", sn))
		{
			return sn;
		}
		break;
	case 1 :
		if (getConfig("CHN_NUM_2", "SN", sn))
		{
			return sn;
		}
		break;
	case 2 :
		if (getConfig("CHN_NUM_3", "SN", sn))
		{
			return sn;
		}
		break;
	case 3 :
		if (getConfig("CHN_NUM_4", "SN", sn))
		{
			return sn;
		}
		break;
	case 4 :
		if (getConfig("CHN_NUM_5", "SN", sn))
		{
			return sn;
		}
		break;
	default:
		return sn;
	}
	return sn;
}

E_P_TYPE CZeratulVideoInterface::getPacketType(int iNum)
{
	std::string strType;
	E_P_TYPE eType = E_P_NULL;
	if (iNum == 0)
	{
		if (getConfig("CHN_NUM_1", "TYPE", strType))
		{
			if (strType.compare("H265") == 0)
			{
				eType = E_P_VIDEO_H265;
			}
		}
	}else if(iNum == 1)
	{
		if (getConfig("CHN_NUM_2", "TYPE", strType))
		{
			if (strType.compare("H265") == 0)
			{
				eType = E_P_VIDEO_H265;
			}
		}
	}else if(iNum == 2)
	{
		if (getConfig("CHN_NUM_3", "TYPE", strType))
		{
			if (strType.compare("H264") == 0)
			{
				eType = E_P_VIDEO_H264;
			}
		}
	}else if(iNum == 3)
	{
		if (getConfig("CHN_NUM_4", "TYPE", strType))
		{
			if (strType.compare("H265") == 0)
			{
				eType = E_P_VIDEO_H265;
			}
		}
	}else if(iNum == 4)
	{
		if (getConfig("CHN_NUM_5", "TYPE", strType))
		{
			if (strType.compare("YUV") == 0)
			{
				eType = E_P_VIDEO_YUV;
			}
		}
	}
	return eType;
}

mxbool CZeratulVideoInterface::loadConfig(std::string strPath)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
	m_strConfigPath = strPath;
	std::string strTmpPath = strPath;
	
#ifdef _CRYPTO_ENABLE
	std::string strKey = getPSK();
	std::string strDID = getDID();
	std::string strMac = "maix";

	int ret = crypto_hmac_sha256(strKey, strDID, strMac, m_key);
	if (ret != 0)
		return mxfalse;

	std::string strCryptoPath(m_strConfigPath);
	strCryptoPath.append(std::string(".enc"));

	iLockFileStream fenc(strCryptoPath.c_str());
	if (fenc.good() == mxtrue)
	{
		fenc.seekg(0, fenc.end);
		int size = fenc.tellg();
		if (size == 0)
			return mxfalse;

		fenc.seekg(0, fenc.beg);
		unsigned char *pcEncFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcEncFileBuffer == NULL)
			return mxfalse;

		unsigned char *pcFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcFileBuffer == NULL)
		{
			free(pcEncFileBuffer);
			return mxfalse;
		}

		memset(pcEncFileBuffer, 0, size + 1);
		memset(pcFileBuffer, 0, size + 1);

		fenc.read((char*)pcEncFileBuffer, size);

		int iFileLen = 0;
		ret = crypto_aes256_decrypt_base64(m_key,
			pcEncFileBuffer, size, pcFileBuffer, &iFileLen);
		if (ret != 0)
		{
			free(pcEncFileBuffer);
			free(pcFileBuffer);
			return mxfalse;
		}

#ifdef _WIN32
		strTmpPath.append(std::string(".read.tmp"));
#else
		strTmpPath = std::string("/tmp/");
		std::string strGUID;
		getGUIDData(strGUID);
		strTmpPath.append(strGUID);
#endif
		oLockFileStream fout(strTmpPath.c_str());
		fout.write((char*)pcFileBuffer, iFileLen);
		fout.close();

		free(pcEncFileBuffer);
		free(pcFileBuffer);
		
	}
#else
		strTmpPath = strPath;
#endif
#ifdef _INI_CONFIG
	try
	{
		iLockFileStream f(strTmpPath.c_str());
		if (f.good() == mxtrue)
		{
			m_configs.load(strTmpPath);
#ifdef _CRYPTO_ENABLE
			if (fenc.good() != mxtrue)
			{
				lock.unlock();
				if (!saveConfig())
				{
					logPrint(MX_LOG_ERROR,
						"save app config failed: %s", strTmpPath.c_str());
				}
			}
			else
			{
				remove(strTmpPath.c_str());
			}

			f.close();
			
#endif
			return mxtrue;
		}
	}
	catch (std::exception & e)
	{
		logPrint(MX_LOG_ERROR, "loadConfig app  failed: %s", strTmpPath.c_str());
	}
#endif

#ifdef _CRYPTO_ENABLE
		remove(strTmpPath.c_str());
#endif
	return mxfalse;
}

mxbool CZeratulVideoInterface::saveConfig()
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _WIN32
	std::string strTmpPath(m_strConfigPath);
	strTmpPath.append(std::string(".write.tmp"));
#else
	std::string strTmpPath("/tmp/");
	std::string strGUID;
	getGUIDData(strGUID);
	strTmpPath.append(strGUID);
#endif
#ifdef _INI_CONFIG
#ifdef _CRYPTO_ENABLE
		m_configs.save(strTmpPath);
#else
		m_configs.save(m_strConfigPath);
#endif
#endif
#ifdef _CRYPTO_ENABLE
	iLockFileStream fin(strTmpPath.c_str());
	if (fin.good() == mxtrue)
	{
		fin.seekg(0, fin.end);
		int size = fin.tellg();
		if (size == 0)
			return mxfalse;

		fin.seekg(0, fin.beg);
		unsigned char *pcEncFileBuffer =
			(unsigned char*)malloc(size + 2048);
		if (pcEncFileBuffer == NULL)
			return mxfalse;

		unsigned char *pcFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcFileBuffer == NULL)
		{
			free(pcEncFileBuffer);
			return mxfalse;
		}

		memset(pcEncFileBuffer, 0, size + 2048);
		memset(pcFileBuffer, 0, size + 1);

		fin.read((char*)pcFileBuffer, size);

		size_t iEncFileLen = 0;
		int ret = crypto_aes256_encrypt_base64(m_key,
			pcFileBuffer, size, pcEncFileBuffer,
			size + 2048,
			&iEncFileLen);

		if (ret != 0)
		{
			free(pcEncFileBuffer);
			free(pcFileBuffer);
			return mxfalse;
		}
		
		std::string strPath = m_strConfigPath;
		strPath.append(std::string(".enc"));
		oLockFileStream fout(strPath.c_str());
		fout.write((char*)pcEncFileBuffer, iEncFileLen);
		fout.close();
		
		free(pcEncFileBuffer);
		free(pcFileBuffer);
	}

	fin.close();
	remove(strTmpPath.c_str());
#endif
	return mxtrue;
}

mxbool CZeratulVideoInterface::config(std::string strConfig)
{
    std::unique_lock<std::mutex> lock(m_mutexSetConfig);
	//在这里需要根据不同的配置信息采取不同的措施：
	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface get set info from video source:-[%s] !\n",strConfig.c_str());

	cJSON *jsonRoot = cJSON_Parse(strConfig.c_str());
	if (jsonRoot == nullptr) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface Failed to parse JSON input !\n");
		return mxfalse;
	}
	cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "configName");
	if (jsonEvent == nullptr) {
			cJSON_Delete(jsonRoot);
			return mxfalse;
	}
	cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "configValue");
	if (jsonParam == nullptr) {
			cJSON_Delete(jsonRoot);
			return mxfalse;
	}
	cJSON *jsonValue = cJSON_GetObjectItem(jsonParam, "value");
	if (jsonValue == nullptr) {
			cJSON_Delete(jsonRoot);
			return mxfalse;
	}
	std::string strEvent = std::string(jsonEvent->valuestring);
	std::string strValue = std::string(jsonValue->valuestring);

	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface get strEvent is:[%s] --get strValue is:[%s]\r\n",strEvent.c_str(), strValue.c_str());

	cJSON_Delete(jsonRoot);

	if (0 == strEvent.compare("SetWatermark"))
	{
		if(0 == strValue.compare("1"))
		{
			// 打开水印
			startOSDShowRgn(0);
			startOSDShowRgn(1);
			startOSDShowRgn(2);
			return mxtrue;
		}else if(0 == strValue.compare("0"))
		{
			// 关闭水印
			stopOSDShowRgn(0);
			stopOSDShowRgn(1);
			stopOSDShowRgn(2);
			return mxtrue;
		}else
		{
			logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface There are no supported configuration value\r\n");
			return mxfalse;
		}
	}
	else if(0 == strEvent.compare("SetWdr"))
	{
		if(0 == strValue.compare("0"))
		{
			// 关闭HDR模式
			zeratulISPWdrModeSwitch("wdr_mode:0");
			return mxtrue;
		}else if(0 == strValue.compare("1"))
		{
			// 开启HDR模式
			zeratulISPWdrModeSwitch("wdr_mode:1");
			return mxtrue;
		}else
		{
			logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface There are no supported configuration value\r\n");
			return mxfalse;
		}
	}
	else if(0 == strEvent.compare("SetNightShot"))
	{
		int value = std::stoi(strValue);
		m_getLedOldMode = m_setLedMode;
		m_setLedMode = std::stoi(strValue);
		if(value != 4 && m_getLedOldMode == 5)
		{
			// value = 5;
			logPrint(MX_LOG_DEBUG, "Audible and visual warnings are in progress[%d]\n", m_setLedMode);
			return mxtrue;
		}
		
		m_setLedMode = value;

		m_conditionLightSensor.notify_one();
		return mxtrue;
	}
    else if(0 == strEvent.compare("GetJpegCover"))
	{
        logPrint(MX_LOG_ERROR, "GetJpegCover\n");
        if(zeratulGetJpegSnap() == 0)
        {
            return mxtrue;
        }
        return mxfalse;
	}
	else
	{
		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface There are no supported configuration types\r\n");
		return mxfalse;
	}
}

int CZeratulVideoInterface::loadTagConfig()
{
    int fd ,size=0;
    char buffer[1024]={0};
	memset(buffer, 0, sizeof(buffer));
    if((fd = open("/proc/cmdline", O_RDONLY)) < 0)
    {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface open /proc/cmdline error %d, %s\r\n", errno,strerror(errno));
        return -1;
    }
    size = read(fd,buffer,sizeof(buffer));
    if(size <= 0)
    {
        close(fd);
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface open /proc/cmdline error size = 0\r\n");
        return -1;
    }
    char *work = NULL;
    if ((work = strstr(buffer, "70mai_wm="))!= NULL)
    {
        sscanf(work, "70mai_wm=%d", &m_OSDPushState); 
		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface 70mai_wm=[%d]\r\n", m_OSDPushState);
    }
	if ((work = strstr(buffer, "70mai_wdr="))!= NULL)
    {
        sscanf(work, "70mai_wdr=%d", &m_wdr_mode); 
		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface 70mai_wdr=[%d]\r\n", m_wdr_mode);
    }
	// if ((work = strstr(buffer, "user_wdr_mode="))!= NULL)
    // {
    //     sscanf(work, "70mai_wdr=%d", &m_wdr_mode_init); 
	// 	logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface 70mai_wdr=[%d]\r\n", m_wdr_mode_init);
    // }
	if ((work = strstr(buffer, "70mai_dn_sw="))!= NULL)
    {
        sscanf(work, "70mai_dn_sw=%d", &m_dn_mode); 
		logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface 70mai_dn_sw=[%d]\r\n", m_dn_mode);
    }
    close(fd);
	
	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface LoadConfig success\r\n");
    return 0;
}

mxbool CZeratulVideoInterface::initSetLedMode()
{
	lightSensorThread = std::thread([this]() {
			this->LightSensorRun();
	});
	return mxtrue;
}

void CZeratulVideoInterface::LightSensorRun()
{
    std::string strCmdValue;
    int iPos = -1;
	int iAlsCriterionAdc = 25000;
	int iWhiteLightBrightness = 50;

	linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW als_400_adc");

	iPos = strCmdValue.find("Value=");
	if (iPos > 0) 
	{
		std::string striAlsCriterionAdc = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		iAlsCriterionAdc = std::stoi(striAlsCriterionAdc);
		// 如果读到默认值，从factory分区里面读一次
		if(iAlsCriterionAdc == 45000)
		{
			while(access("/dev/env", F_OK) != 0)
			{
				usleep(100*1000);
			}
			char *cValue = getFWParaConfig("factory","als_400_adc");
			if(cValue != NULL)
			{
				iAlsCriterionAdc = atoi(cValue);
				if(iAlsCriterionAdc != 45000)
					linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW als_400_adc %d",iAlsCriterionAdc);
				else
					linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW als_400_adc %d",iAlsCriterionAdc-1);
			}
			else
			{
				linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW als_400_adc %d",iAlsCriterionAdc-1);
			}
		}
    }
	else
	{
		while(access("/dev/env", F_OK) != 0)
		{
			usleep(100*1000);
		}
		char *cValue = getFWParaConfig("factory","als_400_adc");
		if(cValue != NULL)
		{
			iAlsCriterionAdc = atoi(cValue);
			if(iAlsCriterionAdc != 45000)
				linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW als_400_adc %d",iAlsCriterionAdc);
			else
				linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW als_400_adc %d",iAlsCriterionAdc-1);
		}
	}
	
	// 设置boot光敏阈值，如果不为0.2，强制设置成0.2
	strCmdValue = "";
	linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW adc_value");
	iPos = strCmdValue.find("Value=");
	if(-1 == iPos)
	{
		linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW adc_value 2");
	}
	else
	{
		std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		int iAdcValue= atoi(strValue.c_str());
		if(iAdcValue != 2)
		{
			linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW adc_value 2");
		}
	}
	
	strCmdValue = "";
	linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW white_light_brightness");
	iPos = strCmdValue.find("Value=");
	if(-1 == iPos)
	{
		while(access("/dev/env", F_OK) != 0)
		{
			usleep(100*1000);
		}
		char *cValue = getFWParaConfig("white_light_brightness");
		if(cValue == NULL)
		{
			linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW white_light_brightness 50");
			setFWParaConfig("white_light_brightness","50",1);
		}
		else
		{
			iWhiteLightBrightness = atoi(cValue);
			linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW white_light_brightness %d",iWhiteLightBrightness);
		}
	}
	else
	{
		std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
		iWhiteLightBrightness= atoi(strValue.c_str());
	}

	// 0-100映射到0-50，灯亮度调整之后要修改
	if(iWhiteLightBrightness != 1)
		iWhiteLightBrightness = iWhiteLightBrightness / 2;

    logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface striAlsCriterionAdc [%d],iWhiteLightBrightness [%d]",iAlsCriterionAdc,iWhiteLightBrightness);

	while (1)
	{
		std::unique_lock<std::mutex> lock(m_mutexLightSensor);

		IMPVI_NUM vinum = IMPVI_MAIN;
		
		// 0：关灯 1：红外 2：白光 3：强制开白光 4：关闭强制开白光 5：声光警告 6：刷新白光灯亮度
		switch(m_setLedMode)
		{
			case 0:
			{
				logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry day mode IMPISP_RUNNING_MODE_DAY### !\n");
				setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
				system("wled off");
				system("ircut on");
				system("irled off");
				m_conditionLightSensor.wait(lock);
				break;
			}
			case 1:
			{
				int lightSensorValue = std::stoi(readFileToSting("/sys/als/device/property/value"));
				if(lightSensorValue < 0)
				{
					usleep(300 * 1000);
					continue;
				}

                float Lux = 400.0 / static_cast<float>(iAlsCriterionAdc) * lightSensorValue; 
				// IMPISPAEScenceAttr scence_attr;
				// IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_MAIN, &scence_attr);
				// printf("raw:%d,lux:%f,luma:%d\n", lightSensorValue,Lux,scence_attr.luma);
            
				switch (m_LedState)
				{
				case 0:
					if(Lux < NIGHT_SENSOR_VALUE) {
						nightCount++;
						dayCount = 0;
						if(nightCount < ALS_CHECK_COUNT)
							break;
						m_LedState = 1;
						logPrint(MX_LOG_ERROR, "### Black and white mode entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
						setISPRunningMode(vinum, IMPISP_RUNNING_MODE_NIGHT);
						system("wled off");
						system("ircut off");
						system("irled on");
					} else if(Lux > IRLED_SENSOR_VALUE) {
						dayCount++;
						nightCount = 0;
						if(dayCount < ALS_CHECK_COUNT)
							break;
						m_LedState = 2;
						logPrint(MX_LOG_ERROR, "### Black and white mode entry day mode IMPISP_RUNNING_MODE_DAY###\n");
						system("wled off");
						system("ircut on");
						system("irled off");
						setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
					}
					break;
				case 1:
					if(Lux > IRLED_SENSOR_VALUE) {
						if(dayCount++ >= ALS_CHECK_COUNT)
							m_LedState = 0;
					}
					else
						dayCount = 0;
					break;
				case 2:
					if(Lux < NIGHT_SENSOR_VALUE) {
						if(nightCount++ >= ALS_CHECK_COUNT)
							m_LedState = 0;
					}
					else
						nightCount = 0;
					break;
				default:
					break;
				}
				break;
			}
			case 2:
			{
				int lightSensorValue = std::stoi(readFileToSting("/sys/als/device/property/value"));
				if(lightSensorValue < 0)
				{
					usleep(300 * 1000);
					continue;
				}

                float Lux = 400.0 / static_cast<float>(iAlsCriterionAdc) * lightSensorValue;

				switch (m_LedState)
				{
				case 0:
					if(Lux < NIGHT_SENSOR_VALUE) {
						nightCount++;
						dayCount = 0;
						if(nightCount < ALS_CHECK_COUNT)
							break;
						m_LedState = 1;
						logPrint(MX_LOG_INFOR, "###Full color mode entry day mode IMPISP_RUNNING_MODE_DAY### !\n");
						setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
						system("irled off");
						system("ircut on");
						std::string command = "wled on 10000 " + std::to_string(iWhiteLightBrightness);
						system(command.c_str());
					} else if(Lux > WLED_SENSOR_VALUE) {
						dayCount++;
						nightCount = 0;
						if(dayCount < ALS_CHECK_COUNT)
							break;
						m_LedState = 2;
						logPrint(MX_LOG_ERROR, "###Full color mode entry day mode IMPISP_RUNNING_MODE_DAY###\n");
						setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
						system("wled off");
						system("ircut on");
						system("irled off");
					}
					break;
				case 1:
					if(Lux > IRLED_SENSOR_VALUE) {
						if(dayCount++ >= ALS_CHECK_COUNT)
							m_LedState = 0;
					}
					else
						dayCount = 0;
					break;
				case 2:
					if(Lux < NIGHT_SENSOR_VALUE) {
						if(nightCount++ >= ALS_CHECK_COUNT)
							m_LedState = 0;
					}
					else
						nightCount = 0;
					break;
				default:
					break;
				}
				break;
			}
			case 3:
			{
				logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry to open wled### !\n");
				setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
				system("ircut on");
				system("irled off");
				std::string command = "wled on 10000 " + std::to_string(iWhiteLightBrightness);
				system(command.c_str());
				// system("ircut off");
				// system("irled off");
				m_LedState = 0;
				m_conditionLightSensor.wait(lock);
				break;
			}
			case 4:
			{
				logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry to close wled### !\n");
				// setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
				system("wled off");
				system("irled off");
				std::string strCmdValue;
				linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW 70mai_dn_sw");

				int iPos = strCmdValue.find("Value=");
				if(-1 == iPos)
				{
					logPrint(MX_LOG_ERROR, "CDevVideoSetting not found 70mai_dn_sw");
					break;
				}

				std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
				m_setLedMode = std::stoi(strValue);
				m_LedState = 0;
				m_conditionLightSensor.wait_for(lock, std::chrono::milliseconds(300));
				// m_conditionLightSensor.wait(lock);
				break;
			}
			case 5:
			{
				logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry to open Audible and visual warning wled### !\n");
				// setISPRunningMode(vinum, IMPISP_RUNNING_MODE_DAY);
				system("irled off");
				system("ircut on");
				system("wled on");
				m_conditionLightSensor.wait_for(lock, std::chrono::seconds(40));
				// m_setLedMode = m_getLedOldMode;
				m_setLedMode = 4;
				m_LedState = 0;
				break;
			}
			case 6:
			{
				iWhiteLightBrightness = 50;
				strCmdValue = "";
				linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW white_light_brightness");
				iPos = strCmdValue.find("Value=");
				if(-1 == iPos)
				{
					char *cValue = getFWParaConfig("white_light_brightness");
					if(cValue == NULL)
					{
						linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW white_light_brightness 50");
						setFWParaConfig("white_light_brightness","50",1);
					}
					else
					{
						iWhiteLightBrightness = atoi(cValue);
						linuxPopenExecCmd(strCmdValue, "tag_env_info --set HW white_light_brightness %d",iWhiteLightBrightness);
					}
				}
				else
				{
					std::string strValue = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
					iWhiteLightBrightness= atoi(strValue.c_str());
				}
				
				if(iWhiteLightBrightness != 1)
					iWhiteLightBrightness = iWhiteLightBrightness / 2;
				
				m_setLedMode = m_getLedOldMode;
				if(m_setLedMode == 2)
				{
					system("wled off");
				}
				m_LedState = 0;

				break;
			}
			default:
				break;
		}
		usleep(120*1000);
	}
}

void CZeratulVideoInterface::setISPRunningMode(IMPVI_NUM vinum, IMPISPRunningMode mode)
{
    int ret = IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface handleSetDayMode IMP_ISP_Tuning_SetISPRunningMode failed, ret=[%d] !\n", ret);
    }
}

int CZeratulVideoInterface::zeratulISPWdrModeSwitch(const char *cmdData)
{
	int ret = 0;
	IMPISPTuningOpsMode wdr_enable = IMPISP_TUNING_OPS_MODE_DISABLE;

	if(strncmp(cmdData, "wdr_mode:0", 10) == 0)
	{
		ret = IMP_ISP_WDR_ENABLE(IMPVI_MAIN,&wdr_enable);
		if (ret < 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed IMP_ISP_WDR_ENABLE to DISABLE !\n");
			return -1;
		}
	}
	else if(strncmp(cmdData, "wdr_mode:1", 10) == 0)
	{
		wdr_enable = IMPISP_TUNING_OPS_MODE_ENABLE;
		ret = IMP_ISP_WDR_ENABLE(IMPVI_MAIN,&wdr_enable);
		if (ret < 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface failed IMP_ISP_WDR_ENABLE to ENABLE !\n");
			return -1;
		}
	}
	logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface success IMP_ISP_WDR_ENABLE !\n");
	return 0;
}

int CZeratulVideoInterface::zeratulISPRunModeSwitch(const char *cmdData)
{
	IMPVI_NUM vinum = IMPVI_MAIN;
	IMPISPRunningMode pmode;

	int ret = IMP_ISP_Tuning_GetISPRunningMode(vinum, &pmode);
	if (ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface ZvmISPRunModeSwitch IMP_ISP_Tuning_GetISPRunningMode failed,ret=[%d] !\n",ret);
		return -1;
	}
	
	//70mai_dn 0：关闭补光灯(白天模式)   1：夜晚开红外夜视(黑白)   2：夜晚开白光补光灯(白光夜视)(全彩)
	if(strncmp(cmdData, "70mai_dn:0", 10) == 0)
	{
		logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry day mode IMPISP_RUNNING_MODE_DAY### !\n");
		IMPISPRunningMode mode = IMPISP_RUNNING_MODE_DAY;
		ret = IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface ZvmISPRunModeSwitch IMP_ISP_Tuning_SetISPRunningMode failed,ret=[%d] !\n",ret);
			return -1;
		}

		system("wled off");
		system("ircut on");
		system("irled off");
	}
	else if(strncmp(cmdData, "70mai_dn:1", 10) == 0)
	{
		//判断光感值
		// std::string retValue = zeratulGetLightSensorValue();
		// int iLightSensorValue = atoi(retValue.c_str());
		// if (iLightSensorValue < 0) {
		// 	logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface ZvmISPRunModeSwitch zeratulGetLightSensorValue failed !\n");
		// 	return -1;
		// }
		
		// if(iLightSensorValue <= 1) //夜晚临界值
		// {
			logPrint(MX_LOG_ERROR, "### entry night mode IMPISP_RUNNING_MODE_NIGHT###\n");
			IMPISPRunningMode mode = IMPISP_RUNNING_MODE_NIGHT;
			IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
			if (ret != 0) {
				logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface ZvmISPRunModeSwitch IMP_ISP_Tuning_SetISPRunningMode failed,ret=[%d] !\n",ret);
				return -1;
			}

			system("wled off");
			system("ircut off");
			system("irled on");
		// }
	}
	else if(strncmp(cmdData, "70mai_dn:2", 10) == 0)
	{
		logPrint(MX_LOG_INFOR, "mxZeratulVideoInterface ### entry day mode IMPISP_RUNNING_MODE_DAY### !\n");
		IMPISPRunningMode mode = IMPISP_RUNNING_MODE_DAY;
		IMP_ISP_Tuning_SetISPRunningMode(vinum, &mode);
		if (ret != 0) {
			logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface ZvmISPRunModeSwitch IMP_ISP_Tuning_SetISPRunningMode failed,ret=[%d] !\n",ret);
			return -1;
		}

		system("irled off");
		system("ircut on");
		system("wled on");
	}

	return 0;
}

std::string CZeratulVideoInterface::zeratulGetLightSensorValue()
{
	std::string strCmdValue;
	linuxPopenExecCmd(strCmdValue, "tag_env_info --get HW als_400_adc");

	int iPos = strCmdValue.find("Value=");
	if(-1 == iPos) {
		return "-1";
	}

	std::string striAlsCriterionAdc = strCmdValue.substr(iPos + strlen("Value="), strCmdValue.length() - strlen("Value="));
	int iAlsCriterionAdc = atoi(striAlsCriterionAdc.c_str());
	if(iAlsCriterionAdc == 0) {
		return "-1";
	}
	
	int Sum = 0;
	for (int i = 0; i < 3;i++) {
		std::string strRet = readFileToSting("/sys/als/device/property/value");
		int AlsAdc = atoi(strRet.c_str());
		if(AlsAdc < 0) {
			return "-1";
		}
		
		Sum += AlsAdc;
		usleep(200 * 1000);
	}
		
	float Lux = 400.0 / (float)iAlsCriterionAdc * (float)Sum / 3.0;
	
	return std::to_string(Lux);
}

std::string CZeratulVideoInterface::readFileToSting(const char* filename)
{
	FILE* file = fopen(filename, "r"); 
	if (file == NULL) {
		// perror("Error opening file"); 
		return "-2";
	}

	char buffer[128];

	size_t result = fread(buffer, 1, sizeof(buffer), file); 
	buffer[result] = '\0';

	fclose(file); 
	return std::string(buffer);
}

int CZeratulVideoInterface::getErrorIntNum()
{
    int fd = open("/proc/jz/isp/isp-w02", O_RDONLY | O_SYNC);
    if (fd < 0)
    {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface open mem fd failed,[%s] !\n",strerror(errno));
        return -1;
    }

    char buf[512];
    int ret = read(fd, buf, sizeof(buf) - 1);
    if (ret < 0)
    {
		logPrint(MX_LOG_ERROR, "mxZeratulVideoInterface read error: [%s] !\n",strerror(errno));
        close(fd);
        return -1;
    }

    buf[ret] = '\0';
    close(fd);

    char *p_int_num = strstr(buf, "\n") + 1;
    int int_num = atoi(p_int_num);

	// logPrint(MX_LOG_DEBUG, "mxZeratulVideoInterface getErrorIntNum:[%d] !\n",int_num);

    return int_num;
}

int CZeratulVideoInterface::zeratulSaveStream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;
	for (i = 0; i < nr_pack; i++) 
	{
		IMPEncoderPack *pack = &stream->pack[i];
		if(pack->length){
			uint32_t remSize = stream->streamSize - pack->offset;
			if(remSize < pack->length){
				ret = write(fd, (void *)(stream->virAddr + pack->offset), remSize);
				if (ret != (int)remSize) {
					logPrint(MX_LOG_ERROR, "zeratulSaveStream stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
					return -1;
				}
				ret = write(fd, (void *)stream->virAddr, pack->length - remSize);
				if (ret != (int)(pack->length - remSize)) {
					logPrint(MX_LOG_ERROR, "zeratulSaveStream stream->virAddr:%x stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", stream->virAddr, ret, i, (pack->length - remSize), strerror(errno));
					return -1;
				}
			} else {
				ret = write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
				if (ret != (int)pack->length) {
					logPrint(MX_LOG_ERROR, "zeratulSaveStream stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
					return -1;
				}
			}
		}
	}
	return 0;
}

int CZeratulVideoInterface::zeratulGetJpegSnap()
{
    logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap start\n");
	char snap_path[64]={0};
	int ret = IMP_Encoder_StartRecvPic(4);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap IMP_Encoder_StartRecvPic(%d) failed\n", 4);
		return -1;
	}

	sprintf(snap_path, "%s","/tmp/cover.jpeg");
	int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (snap_fd < 0) {
		logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap open motion.jpg failed: %s\n", strerror(errno));
		return -1;
	}

	/* Polling JPEG Snap, set timeout as 1000msec */
	ret = IMP_Encoder_PollingStream(4, 1000);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap Polling stream timeout\n");
		close(snap_fd);
		return -1;
	}

	IMPEncoderStream stream;
	/* Get JPEG Snap */
	ret = IMP_Encoder_GetStream(4, &stream, 1);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap IMP_Encoder_GetStream() failed\n");
		close(snap_fd);
		return -1;
	}

	ret = zeratulSaveStream(snap_fd, &stream);
	if (ret < 0) {
		close(snap_fd);
		return -1;
	}

	IMP_Encoder_ReleaseStream(4, &stream);

	close(snap_fd);

	ret = IMP_Encoder_StopRecvPic(4);
	if (ret < 0) {
		logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap IMP_Encoder_StopRecvPic() failed\n");
		return -1;
	}
    logPrint(MX_LOG_ERROR, "ZvmGetJpegSnap end\n");
	return 0;
}
