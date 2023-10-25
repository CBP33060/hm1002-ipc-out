#include "ING_Stream_Mgr.h"
#include "bitmapinfo.h"
#include "logodata_100x100_bgra.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

bool GetFWParaConfig(std::string strKey, std::string &strValue);

chn_conf chn[FS_CHN_NUM] = { chn_conf(0), chn_conf(1), chn_conf(2), chn_conf(3) };

#define OSD_LETTER_NUM 300
IMPRgnHandle *prHander;

int grpNum = 0;
volatile int rest_sensor_flag = 0;
volatile int error_int_num = 0;

static int osd_show(void)
{
	int ret;

	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 1);
	if (ret != 0) {
		printf("IMP_OSD_ShowRgn() timeStamp error\n");
		return -1;
	}
	// ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 1);
	// if (ret != 0) {
	// 	printf("IMP_OSD_ShowRgn() Logo error\n");
	// 	return -1;
	// }
	// ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 1);
	// if (ret != 0) {
	// 	printf("IMP_OSD_ShowRgn() Cover error\n");
	// 	return -1;
	// }
	// ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 1);
	// if (ret != 0) {
	// 	printf("IMP_OSD_ShowRgn() Rect error\n");
	// 	return -1;
	// }
	// ret = IMP_OSD_ShowRgn(prHander[4], grpNum, 1);
	// if (ret != 0) {
	// 	printf("IMP_OSD_ShowRgn() Line error\n");
	// 	return -1;
	// }

	return 0;
}

int get_error_int_num()
{
	int fd = 0;
    char buf[512];
    char *p_int_num;
    int int_num,flag = 0;

    fd = open("/proc/jz/isp/isp-w02", O_RDONLY | O_SYNC);        
    if (fd < 0)    
    {  
        printf("open mem fd failed,%s\n", strerror(errno));      
        return -1;  
    }

    int ret = read(fd,buf,512);

    // printf("3333-%d\n%s\n",ret,buf);

    for(int i = 0; i < ret; i++)
    {
        if(buf[i] == '\n')
        {
            flag++;
            if(flag == 2)
            {
                p_int_num = buf + i + 1;
            }
        }

        if(flag == 2 && buf[i] == ',')
        {
            buf[i] = 0;
            break;
        }
    }
    
    int_num = atoi(p_int_num);
    // printf("%d\n",int_num);

	return int_num;
}

static void *update_thread(void *p)
{
	int ret;

	/*generate time*/
	char DateStr[OSD_LETTER_NUM];
	time_t currTime;
	struct tm *currDate;
	unsigned i = 0, j = 0;
	void *dateData = NULL;
	uint32_t *data = NULL;
	IMPOSDRgnAttr rAttr;
	int test_times = 1111;
	

	ret = osd_show();
	if (ret < 0) {
		printf("OSD show error\n");
		return NULL;
	}

	int fd = 0;
    int page_size;
    int mapped_size = page_size = getpagesize();

    printf("mapped_size:%d\n", mapped_size);  

    fd = open("/dev/mem", O_RDONLY | O_SYNC);        
    if (fd < 0)    
    {  
        printf("open mem fd failed,%s\n", strerror(errno));        
    } 

    char * mmap_base_10023=(char *)mmap(NULL, mapped_size, PROT_READ , MAP_SHARED, fd, 0x10023000);  
	
	if (NULL == mmap_base_10023)
	{
		printf("mmap failed,%s\n", strerror(errno));
	}

	char * mmap_base_10054=(char *)mmap(NULL, mapped_size, PROT_READ , MAP_SHARED, fd, 0x10054000);  
	
	if (NULL == mmap_base_10054)
	{
		printf("mmap failed,%s\n", strerror(errno));
	}

    int *preg_10054014 = (int *)(mmap_base_10054 + 0x14);
	int *preg_10054020 = (int *)(mmap_base_10054 + 0x20);
	int *preg_10054024 = (int *)(mmap_base_10054 + 0x24);
	int *preg_10023014 = (int *)(mmap_base_10023 + 0x14);
    int *preg_10023020 = (int *)(mmap_base_10023 + 0x20);
	int *preg_10023024 = (int *)(mmap_base_10023 + 0x24);

    printf("preg_10054014:%d preg_10054020:%d preg_10054024:%d preg_10023014:%d preg_10023020:%d preg_10023024:%d\n", *preg_10054014, *preg_10054020, *preg_10054024, *preg_10023014, *preg_10023020, *preg_10023024);

	while(1) {
		ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
		if (ret != 0) {
			printf("IMP_OSD_ShowRgn() timeStamp error\n");
		}
		int penpos = 0;
		int penpos_t = 0;
		int fontadv = 0;
		unsigned int len = 0;

		ret = IMP_OSD_GetRgnAttr(prHander[0], &rAttr);
		// int result = IMP_OSD_GetRegionLuma(prHander[0], &rAttr);
		// if(result == -1 )
		// 	continue;

		data = (uint32_t *)rAttr.data.bitmapData;
		time(&currTime);
		currDate = localtime(&currTime);
		memset(DateStr, 0, OSD_LETTER_NUM);
		strftime(DateStr, OSD_LETTER_NUM, "%Y-%m-%d %I:%M:%S", currDate);
		len = strlen(DateStr);

		len = 0;

		IMPISPAEExprInfo info;
		ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
		if(ret){
			printf("IMP_ISP_Tuning_GetAeExprInfo error !\n");
			continue;
		}

		IMPISPWBAttr awb_attr;
		ret = IMP_ISP_Tuning_GetAwbAttr(IMPVI_MAIN, &awb_attr);
		if(ret){
			printf("IMP_ISP_Tuning_GetAwbAttr error !\n");
			continue;
		}
		
		IMPISPAEScenceAttr scence_attr;
		ret = IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_MAIN, &scence_attr);
		if(ret){
			printf("IMP_ISP_Tuning_GetAeScenceAttr error !\n");
			continue;
		}
		sprintf(DateStr + len - 1," %d %d %d %d %d",info.AeIntegrationTime,info.AeShortIntegrationTime,info.AeAGain,info.AeIspDGain,info.AeIntegrationTimeUnit);
		// sprintf(DateStr + len - 1," %d %d %d %d %d %d %d",info.AeIntegrationTime,info.AeShortIntegrationTime,info.TotalGainDb,info.TotalGainDbShort,scence_attr.luma,scence_attr.luma_scence,awb_attr.ct);
		// sprintf(DateStr + len - 1," %d %d %d %d %d %d %d",info.AeIntegrationTime,info.AeAGain,info.AeIspDGain,info.AeIntegrationTimeMode,scence_attr.luma,info.AeMode,info.AeIspDGainManualMode);
		len = strlen(DateStr);
		// printf("[%d]%s\n",len,DateStr);
		for (i = 0; i < len; i++) {
			switch(DateStr[i]) {
				case '0' ... '9':
#ifdef SUPPORT_COLOR_REVERSE
					if(rAttr.fontData.colType[i] == 1) {
						dateData = (void *)gBitmap_black[DateStr[i] - '0'].pdata;
					} else {
						dateData = (void *)gBitmap[DateStr[i] - '0'].pdata;
					}
#else
					dateData = (void *)gBitmap[DateStr[i] - '0'].pdata;
#endif
					fontadv = gBitmap[DateStr[i] - '0'].width;
					penpos_t += gBitmap[DateStr[i] - '0'].width;
					break;
				case '-':
#ifdef SUPPORT_COLOR_REVERSE
					if(rAttr.fontData.colType[i] == 1) {
						dateData = (void *)gBitmap_black[10].pdata;
					} else {
						dateData = (void *)gBitmap[10].pdata;
					}
#else
					dateData = (void *)gBitmap[10].pdata;
#endif
					fontadv = gBitmap[10].width;
					penpos_t += gBitmap[10].width;
					break;
				case ' ':
					dateData = (void *)gBitmap[11].pdata;
					fontadv = gBitmap[11].width;
					penpos_t += gBitmap[11].width;
					break;
				case ':':
#ifdef SUPPORT_COLOR_REVERSE
					if(rAttr.fontData.colType[i] == 1) {
						dateData = (void *)gBitmap_black[12].pdata;
					} else {
						dateData = (void *)gBitmap[12].pdata;
					}
#else
					dateData = (void *)gBitmap[12].pdata;
#endif
					fontadv = gBitmap[12].width;
					penpos_t += gBitmap[12].width;
					break;
				default:
					break;
			}
#ifdef SUPPORT_RGB555LE
			for (j = 0; j < OSD_REGION_HEIGHT; j++) {
				memcpy((void *)((uint16_t *)data + j*OSD_LETTER_NUM*OSD_REGION_WIDTH + penpos_t),
						(void *)((uint16_t *)dateData + j*fontadv), fontadv*sizeof(uint16_t));
			}
#else
			for (j = 0; j < gBitmapHight; j++) {
				memcpy((void *)((intptr_t)data + j*20*32 + penpos),
						(void *)((intptr_t)dateData + j*fontadv), fontadv);
			}
			penpos = penpos_t;
#endif
		}
		ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 1);
		if (ret != 0) {
			printf("IMP_OSD_ShowRgn() timeStamp error\n");
		}

		// static IMPISPAFMetricsInfo metric;
		// IMPISPAFMetricsInfo metric_tmp;
		// IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_MAIN, &metric_tmp);
		// // printf("IMP_ISP_Tuning_GetAFMetricesInfo:%d %d %d\n", metric_tmp.af_metrics,metric_tmp.af_metrics_alt,metric_tmp.af_frame_num);
		// if(metric_tmp.af_metrics == metric.af_metrics &&
		// 	metric_tmp.af_metrics_alt == metric.af_metrics_alt &&
		// 	metric_tmp.af_frame_num == metric.af_frame_num){			
		// 	rest_sensor_flag = 1;
		// }
		// else
		// {
		// 	metric.af_metrics = metric_tmp.af_metrics;
		// 	metric.af_metrics_alt = metric_tmp.af_metrics_alt;
		// 	metric.af_frame_num = metric_tmp.af_frame_num;
		// }

		// if(*preg_10054020 != 0 || *preg_10054024 != 0 || *preg_10023020 != 0 || *preg_10023024 != 0)
		// {
		// 	rest_sensor_flag = 1;
		// }
		// printf("rest_sensor_flag:%d \npreg_10054014:%d preg_10054020:%d preg_10054024:%d preg_10023014:%d preg_10023020:%d preg_10023024:%d\n",rest_sensor_flag, *preg_10054014, *preg_10054020, *preg_10054024, *preg_10023014, *preg_10023020, *preg_10023024);

		int _error_int_num = get_error_int_num();
		if(error_int_num != _error_int_num)
		{
			rest_sensor_flag = 1;
		}

		// printf("error_int_num %d,%d,%d\n", error_int_num,rest_sensor_flag,_error_int_num);

		sleep(1);
		while(rest_sensor_flag == 1) {
			sleep(1);
		}
	}

	return NULL;
}

IMPRgnHandle *system_osd_init(int grpNum)
{
	int ret = 0;
	IMPRgnHandle *prHander = NULL;
	IMPRgnHandle rHanderFont = 0;
	// IMPRgnHandle rHanderLogo = 0;
	// IMPRgnHandle rHanderCover = 0;
	// IMPRgnHandle rHanderRect = 0;
	// IMPRgnHandle rHanderLine = 0;

	prHander = (IMPRgnHandle *)malloc(5 * sizeof(IMPRgnHandle));
	if (prHander <= 0) {
		printf("malloc() error !\n");
		return NULL;
	}

	rHanderFont = IMP_OSD_CreateRgn(NULL);
	if (rHanderFont == INVHANDLE) {
		printf("IMP_OSD_CreateRgn TimeStamp error !\n");
		return NULL;
	}
	//query osd rgn create status
	IMPOSDRgnCreateStat stStatus;
	memset(&stStatus,0x0,sizeof(IMPOSDRgnCreateStat));
	ret = IMP_OSD_RgnCreate_Query(rHanderFont,&stStatus);
	if(ret < 0){
		printf("IMP_OSD_RgnCreate_Query error !\n");
		return NULL;
	}

	// rHanderLogo = IMP_OSD_CreateRgn(NULL);
	// if (rHanderLogo == INVHANDLE) {
	// 	printf("IMP_OSD_CreateRgn Logo error !\n");
	// 	return NULL;
	// }

	// rHanderCover = IMP_OSD_CreateRgn(NULL);
	// if (rHanderCover == INVHANDLE) {
	// 	printf("IMP_OSD_CreateRgn Cover error !\n");
	// 	return NULL;
	// }

	// rHanderRect = IMP_OSD_CreateRgn(NULL);
	// if (rHanderRect == INVHANDLE) {
	// 	printf("IMP_OSD_CreateRgn Rect error !\n");
	// 	return NULL;
	// }
	// rHanderLine = IMP_OSD_CreateRgn(NULL);
	// if (rHanderLine == INVHANDLE) {
	// 	printf("IMP_OSD_CreateRgn Line error !\n");
	// 	return NULL;
	// }
	ret = IMP_OSD_RegisterRgn(rHanderFont, grpNum, NULL);
	if (ret < 0) {
		printf("IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}
	//query osd rgn register status
	IMPOSDRgnRegisterStat stRigStatus;
	memset(&stRigStatus,0x0,sizeof(IMPOSDRgnRegisterStat));
	ret = IMP_OSD_RgnRegister_Query(rHanderFont, grpNum,&stRigStatus);
	if (ret < 0) {
		printf("IMP_OSD_RgnRegister_Query failed\n");
		return NULL;
	}

	// ret = IMP_OSD_RegisterRgn(rHanderLogo, grpNum, NULL);
	// if (ret < 0) {
	// 	printf("IVS IMP_OSD_RegisterRgn failed\n");
	// 	return NULL;
	// }

	// ret = IMP_OSD_RegisterRgn(rHanderCover, grpNum, NULL);
	// if (ret < 0) {
	// 	printf("IVS IMP_OSD_RegisterRgn failed\n");
	// 	return NULL;
	// }

	// ret = IMP_OSD_RegisterRgn(rHanderRect, grpNum, NULL);
	// if (ret < 0) {
	// 	printf("IVS IMP_OSD_RegisterRgn failed\n");
	// 	return NULL;
	// }
	// ret = IMP_OSD_RegisterRgn(rHanderLine, grpNum, NULL);
	// if (ret < 0) {
	// 	printf("IVS IMP_OSD_RegisterRgn failed\n");
	// 	return NULL;
	// }
	IMPOSDRgnAttr rAttrFont;
	memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
    rAttrFont.type = OSD_REG_BITMAP;
    rAttrFont.rect.p0.x = 0;
    rAttrFont.rect.p0.y = 0;
    rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + 20 * 32 - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
    rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + 33 - 1;
#ifdef SUPPORT_COLOR_REVERSE
    rAttrFont.fontData.invertColorSwitch = 1;   //Color reverse switch 0-close 1-open.
    rAttrFont.fontData.luminance = 200;
    rAttrFont.fontData.data.fontWidth = 16;
    rAttrFont.fontData.data.fontHeight = 33;
    rAttrFont.fontData.length = 19;
    rAttrFont.fontData.istimestamp = 1;
#endif
#ifdef SUPPORT_RGB555LE
    rAttrFont.fmt = PIX_FMT_RGB555LE;
#else
    rAttrFont.fmt = PIX_FMT_MONOWHITE;
#endif
    rAttrFont.data.bitmapData = valloc(20 * 32 * 33);
    if (rAttrFont.data.bitmapData == NULL) {
        printf("alloc rAttr.data.bitmapData TimeStamp error !\n");
        return NULL;
    }
    memset(rAttrFont.data.bitmapData, 0, 20 * 32 * 33);

	ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
	if (ret < 0) {
		printf("IMP_OSD_SetRgnAttr TimeStamp error !\n");
		return NULL;
	}

	IMPOSDGrpRgnAttr grAttrFont;

	if (IMP_OSD_GetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		printf("IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrFont.show = 0;

	/* Disable Font global alpha, only use pixel alpha. */
	grAttrFont.gAlphaEn = 1;
	grAttrFont.fgAlhpa = 0xff;
	grAttrFont.layer = 3;
	if (IMP_OSD_SetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		printf("IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}
	// IMPOSDRgnAttr rAttrLogo;
	// memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
	// int picw = 100;
	// int pich = 100;
	// rAttrLogo.type = OSD_REG_PIC;
	// rAttrLogo.rect.p0.x = 0;
	// rAttrLogo.rect.p0.y = 0;

	// //p0 is start，and p1 well be epual p0+width(or heigth)-1
	// rAttrLogo.rect.p1.x = rAttrLogo.rect.p0.x+picw-1;
	// rAttrLogo.rect.p1.y = rAttrLogo.rect.p0.y+pich-1;
	// rAttrLogo.fmt = PIX_FMT_BGRA;
	// rAttrLogo.data.picData.pData = logodata_100x100_bgra;
	// ret = IMP_OSD_SetRgnAttr(rHanderLogo, &rAttrLogo);
	// if (ret < 0) {
	// 	printf("IMP_OSD_SetRgnAttr Logo error !\n");
	// 	return NULL;
	// }
	// IMPOSDGrpRgnAttr grAttrLogo;

	// if (IMP_OSD_GetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
	// 	printf("IMP_OSD_GetGrpRgnAttr Logo error !\n");
	// 	return NULL;

	// }
	// memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttrLogo.show = 0;
	// /* Set Logo global alpha to 0x7f, it is semi-transparent. */
	// grAttrLogo.gAlphaEn = 1;
	// grAttrLogo.fgAlhpa = 0x7f;
	// grAttrLogo.layer = 2;

	// if (IMP_OSD_SetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
	// 	printf("IMP_OSD_SetGrpRgnAttr Logo error !\n");
	// 	return NULL;
	// }

	// IMPOSDRgnAttr rAttrCover;
	// memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
	// rAttrCover.type = OSD_REG_COVER;
	// rAttrCover.rect.p0.x = 100;
	// rAttrCover.rect.p0.y = 100;
	// rAttrCover.rect.p1.x = rAttrCover.rect.p0.x+300 -1;
	// rAttrCover.rect.p1.y = rAttrCover.rect.p0.y+300 -1 ;
	// rAttrCover.fmt = PIX_FMT_BGRA;
	// rAttrCover.data.coverData.color = OSD_IPU_RED;
	// ret = IMP_OSD_SetRgnAttr(rHanderCover, &rAttrCover);
	// if (ret < 0) {
	// 	printf("IMP_OSD_SetRgnAttr Cover error !\n");
	// 	return NULL;
	// }
	// IMPOSDGrpRgnAttr grAttrCover;

	// if (IMP_OSD_GetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
	// 	printf("IMP_OSD_GetGrpRgnAttr Cover error !\n");
	// 	return NULL;
	// }
	// memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttrCover.show = 0;

	// /* Disable Cover global alpha, it is absolutely no transparent. */
	// grAttrCover.gAlphaEn = 1;
	// grAttrCover.fgAlhpa = 0x7f;
	// grAttrCover.layer = 2;
	// if (IMP_OSD_SetGrpRgnAttr(rHanderCover, grpNum, &grAttrCover) < 0) {
	// 	printf("IMP_OSD_SetGrpRgnAttr Cover error !\n");
	// 	return NULL;
	// }

	// IMPOSDRgnAttr rAttrRect;
	// memset(&rAttrRect, 0, sizeof(IMPOSDRgnAttr));

	// rAttrRect.type = OSD_REG_RECT;
	// rAttrRect.rect.p0.x = 400;
	// rAttrRect.rect.p0.y = 400;
	// rAttrRect.rect.p1.x = rAttrRect.rect.p0.x + 300 - 1;
	// rAttrRect.rect.p1.y = rAttrRect.rect.p0.y + 300 - 1;
	// rAttrRect.fmt = PIX_FMT_MONOWHITE;
	// rAttrRect.data.lineRectData.color = OSD_YELLOW;
	// rAttrRect.data.lineRectData.linewidth = 5;
	// ret = IMP_OSD_SetRgnAttr(rHanderRect, &rAttrRect);
	// if (ret < 0) {
	// 	printf("IMP_OSD_SetRgnAttr Rect error !\n");
	// 	return NULL;
	// }
	// IMPOSDGrpRgnAttr grAttrRect;
	// if (IMP_OSD_GetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
	// 	printf("IMP_OSD_GetGrpRgnAttr Rect error !\n");
	// 	return NULL;

	// }
	// memset(&grAttrRect, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttrRect.show = 0;
	// grAttrRect.layer = 1;
	// grAttrRect.scalex = 1;
	// grAttrRect.scaley = 1;
	// if (IMP_OSD_SetGrpRgnAttr(rHanderRect, grpNum, &grAttrRect) < 0) {
	// 	printf("IMP_OSD_SetGrpRgnAttr Rect error !\n");
	// 	return NULL;
	// }

	// IMPOSDRgnAttr rAttrLine;
	// memset(&rAttrLine, 0, sizeof(IMPOSDRgnAttr));

	// rAttrLine.type = OSD_REG_HORIZONTAL_LINE;
	// rAttrLine.line.p0.x = 800;
	// rAttrLine.line.p0.y = 800;
	// rAttrLine.data.lineRectData.color = OSD_RED;
	// rAttrLine.data.lineRectData.linewidth = 5;
	// rAttrLine.data.lineRectData.linelength = 200;

	// ret = IMP_OSD_SetRgnAttr(rHanderLine, &rAttrLine);
	// if (ret < 0) {
	// 	printf("IMP_OSD_SetRgnAttr Line error !\n");
	// 	return NULL;
	// }
	// IMPOSDGrpRgnAttr grAttrLine;
	// if (IMP_OSD_GetGrpRgnAttr(rHanderLine, grpNum, &grAttrLine) < 0) {
	// 	printf("IMP_OSD_GetGrpRgnAttr Line error !\n");
	// 	return NULL;

	// }
	// memset(&grAttrLine, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttrLine.show = 0;
	// grAttrLine.layer = 1;
	// grAttrLine.scalex = 1;
	// grAttrLine.scaley = 1;
	// if (IMP_OSD_SetGrpRgnAttr(rHanderLine, grpNum, &grAttrLine) < 0) {
	// 	printf("IMP_OSD_SetGrpRgnAttr Line error !\n");
	// 	return NULL;
	// }

	ret = IMP_OSD_Start(grpNum);
	if (ret < 0) {
		printf("IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return NULL;
	}

	prHander[0] = rHanderFont;
	// prHander[1] = rHanderLogo;
	// prHander[2] = rHanderCover;
	// prHander[3] = rHanderRect;
	// prHander[4] = rHanderLine;
	return prHander;
}

int system_osd_exit(IMPRgnHandle *prHander,int grpNum)
{
	int ret;

	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 0);
	if (ret < 0) {
		printf("IMP_OSD_ShowRgn close timeStamp error\n");
	}

	// ret = IMP_OSD_ShowRgn(prHander[1], grpNum, 0);
	// if (ret < 0) {
	// 	printf("IMP_OSD_ShowRgn close Logo error\n");
	// }

	// ret = IMP_OSD_ShowRgn(prHander[2], grpNum, 0);
	// if (ret < 0) {
	// 	printf("IMP_OSD_ShowRgn close cover error\n");
	// }

	// ret = IMP_OSD_ShowRgn(prHander[3], grpNum, 0);
	// if (ret < 0) {
	// 	printf("IMP_OSD_ShowRgn close Rect error\n");
	// }


	ret = IMP_OSD_UnRegisterRgn(prHander[0], grpNum);
	if (ret < 0) {
		printf("IMP_OSD_UnRegisterRgn timeStamp error\n");
	}

	// ret = IMP_OSD_UnRegisterRgn(prHander[1], grpNum);
	// if (ret < 0) {
	// 	printf("IMP_OSD_UnRegisterRgn logo error\n");
	// }

	// ret = IMP_OSD_UnRegisterRgn(prHander[2], grpNum);
	// if (ret < 0) {
	// 	printf("IMP_OSD_UnRegisterRgn Cover error\n");
	// }

	// ret = IMP_OSD_UnRegisterRgn(prHander[3], grpNum);
	// if (ret < 0) {
	// 	printf("IMP_OSD_UnRegisterRgn Rect error\n");
	// }

	IMP_OSD_DestroyRgn(prHander[0]);
	// IMP_OSD_DestroyRgn(prHander[1]);
	// IMP_OSD_DestroyRgn(prHander[2]);
	// IMP_OSD_DestroyRgn(prHander[3]);

	ret = IMP_OSD_DestroyGroup(grpNum);
	if (ret < 0) {
		printf("IMP_OSD_DestroyGroup(0) error\n");
		return -1;
	}
	free(prHander);
	prHander = NULL;

	return 0;
}


ING_Stream_Mgr::ING_Stream_Mgr(void)
	:stop_(false)
{
	printf("ING_Stream_Mgr constructor.\n");
}

ING_Stream_Mgr::~ING_Stream_Mgr(void)
{
	printf("ING_Stream_Mgr destructor.\n");
}


int ING_Stream_Mgr::open()
{
	// 初始化和反初始化
// 	INGJRTP_Startup();
	int ret = system_init();
	if (ret < 0) {
		return -1;
	}
	printf("111111123123123\n");
	ret = framesource_init();
	if (ret < 0) {
		return -2;
	}
	printf("1asdasdasdasda\n");
	ret = encoder_init();
	if (ret < 0) {
		return -3;
	}
	printf("1111111231231asdasdasdasdasd\n");
	ret = IMP_OSD_CreateGroup(0);
	if (ret < 0) {
		printf("IMP_OSD_CreateGroup(%d) error !\n", 0);
		return -6;
	}

	prHander = system_osd_init(grpNum);
	if (prHander <= 0) {
		printf("OSD init failed\n");
		return -1;
	}

	ret = system_bind();
	if (ret < 0) {
		return -4;
	}

	pthread_t tid;
#ifdef SUPPORT_RGB555LE
	uint32_t *timeStampData = (uint32_t *)malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint16_t));
#else
	uint32_t *timeStampData = (uint32_t *)malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint32_t));
#endif
	if (timeStampData == NULL) {
		printf("valloc timeStampData error\n");
		return -1;
	}
	memset(timeStampData, 0, OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint32_t));

	std::string wdr;

	if( GetFWParaConfig("water_mark",wdr) )
	{
		if(wdr.compare("1") == 0)
		{
			printf("================ Water Mark ================\n");
			ret = pthread_create(&tid, NULL, update_thread, timeStampData);
			if (ret) {
				printf("thread create error\n");
				return -1;
			}
		}
	}

	printf("WCQ=-====ASDASD\n");
	ret = framesource_enable();
	if (ret < 0) {
		return -5;
	}

	return 0;
}

int ING_Stream_Mgr::close()
{
	stop_ = true;
// 	INGJRTP_Cleanup();
	int ret = framesource_disable();
	if (ret < 0) {
		printf("stream_vdo close error framesource_disable %d\n", ret);
		return ret;
	}

	ret = system_unbind();
	if (ret < 0) {
		printf("stream_vdo close error system_unbind %d\n", ret);
		return ret;
	}

	ret = system_osd_exit(prHander,grpNum);
	if (ret < 0) {
		printf("OSD exit failed\n");
		return -1;
	}

	ret = encoder_exit();
	if (ret < 0) {
		printf("stream_vdo close error encoder_exit %d\n", ret);
		return ret;
	}

	ret = framesource_exit();
	if (ret < 0) {
		printf("stream_vdo close error framesource_exit %d\n", ret);
		return ret;
	}

	ret = system_exit();
	if (ret < 0) {
		printf("stream_vdo close error system_exit %d\n", ret);
		return ret;
	}
	
	return 0;
}

int ING_Stream_Mgr::run()
{
	int ret = 0;
	static unsigned int time_now = 0;
	error_int_num = get_error_int_num();
	printf("--------------------------------------ING_Stream_Mgr::run------------------stop=%d\n", stop_);
#if 0
	if (access("1.txt", F_OK) == 0)
	{
		ING_Stream_Vdo *new_stream = new ING_Stream_Vdo(0x12341111,
			24400, "192.168.41.100", 60000, ING_Stream_Vdo::stream_type_realtime, "video", 96, NULL);

		ret = new_stream->open();
		if (ret != 0)
		{
			printf("ING_Controller::handle_session_control ING_Stream_Vdo open err! ret=%d\n", ret);
			new_stream->close();
			return ret;
		}
		// 根据SDP信息创建流
		add_stream(new_stream);
	}
#endif
	// 遍历所有的流
	while (!stop_) {
		unsigned long ulTickCount = GetTickCount();
		// 	printf("***************ING_Controller::run() TickCount=%lu time_now=%d Differ=%lu***************\n", ulTickCount, time_now, ulTickCount - time_now);
		if (ulTickCount - time_now > 1000 * 20)
		{
			printf("***************ING_Stream_Mgr::run***************\n");
			time_now = ulTickCount;
		}

		if(rest_sensor_flag == 1)
		{
			printf("***************rest_sensor:%d***************\n",rest_sensor_flag);
			close();
			open();
			rest_sensor_flag = 0;
			error_int_num = get_error_int_num();
			printf("WCQ=====================error_int_num %d,%d\n", error_int_num,rest_sensor_flag);
			stop_ = false;
		}

// 		printf("--------------------------------------ING_Stream_Mgr::run   2------------------stop=%d\n", stop_);
		STREAM_ARR::iterator iter = stream_arr.begin();
		for (; iter != stream_arr.end();)
		{
			ING_Guard guard(StreamMgrMutex_);
			ING_Stream_Vdo *this_stream = *iter;
			if (this_stream->is_remove_flag())
			{
// 				printf("--------------------------------------ING_Stream_Mgr::run close 1--------------------------\n");
				this_stream->close();
				delete this_stream;
				iter = stream_arr.erase(iter);
// 				printf("--------------------------------------ING_Stream_Mgr::run close 2--------------------------\n");
				break;
			}
			else
			{
				this_stream->run();
				iter++;
			}
		}
		usleep(1000*5);
	}
	printf("--------------------------------------ING_Stream_Mgr::run stop------------------\n");

	return ret;
}

int ING_Stream_Mgr::add_stream(ING_Stream_Vdo *stream)
{
	ING_Guard guard(StreamMgrMutex_);
	stream_arr.push_back(stream);
	return 0;
}

/*
	ING_Controller::handle_session_closing  RTSP通道关闭
	ING_Controller::handle_dlg_closing		收到BYE请求
	以上两种情况会调用此函数
*/
int ING_Stream_Mgr::remove_stream(unsigned int dlg_handle)
{
	ING_Guard guard(StreamMgrMutex_);
	STREAM_ARR::iterator iter = stream_arr.begin();
	for (; iter != stream_arr.end(); )
	{
		ING_Stream_Vdo *this_stream = *iter;
		printf("*********remove_stream 0x%X 0x%X\n", this_stream->dlg_handle(), dlg_handle);
		if (this_stream->dlg_handle() == dlg_handle)
		{
// 			this_stream->disconn(); // 已经关闭
			this_stream->remove(); // 已经关闭
			iter++;
		}
		else
		{
			iter++;
		}
	}
	return 0;
}

// int ING_Stream_Mgr::pause_stream(unsigned int dlg_handle)
// {
// 	ING_Guard guard(mutex_);
// 	STREAM_ARR::iterator iter = stream_arr.begin();
// 	for (; iter != stream_arr.end(); )
// 	{
// 		ING_Stream_Vdo *this_stream = *iter;
// 		if (this_stream->dlg_handle() == dlg_handle)
// 		{
// 			this_stream->pause();
// 			iter++;
// 		}
// 		else
// 		{
// 			iter++;
// 		}
// 	}
// 	return 0;
// }

// int ING_Stream_Mgr::resume_stream(unsigned int dlg_handle)
// {
// 	ING_Guard guard(mutex_);
// 	STREAM_ARR::iterator iter = stream_arr.begin();
// 	for (; iter != stream_arr.end(); )
// 	{
// 		ING_Stream_Vdo *this_stream = *iter;
// 		if (this_stream->dlg_handle() == dlg_handle)
// 		{
// 			this_stream->resume();
// 			return 0;
// 			break;
// 		}
// 		else
// 		{
// 			iter++;
// 		}
// 	}
// 	return -1;
// }

ING_Stream_Vdo* ING_Stream_Mgr::find_stream(unsigned int dlg_handle)
{
	ING_Guard guard(StreamMgrMutex_);
	ING_Stream_Vdo *this_stream = NULL;
	STREAM_ARR::iterator iter = stream_arr.begin();
	for (; iter != stream_arr.end(); )
	{
		ING_Stream_Vdo *c2x_stream = *iter;
		if (c2x_stream->dlg_handle() == dlg_handle)
		{
			this_stream = c2x_stream;
			break;
		}
		else
		{
			iter++;
		}
	}
	return this_stream;
}

int ING_Stream_Mgr::framesource_init()
{
	int i = 0, ret = 0;

	if (chn[i].enable) {
		ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			printf("IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
			return -1;
		}

		ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			printf("IMP_FrameSource_SetChnAttr(chn%d) error !\n", chn[i].index);
			return -1;
		}
	}

	return 0;
}

int ING_Stream_Mgr::encoder_init()
{
	int i = 0, ret = 0, chnNum = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;

	if (chn[i].enable) {
		imp_chn_attr_tmp = &chn[i].fs_chn_attr;
		chnNum = chn[i].index;
		memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
		int bitrate = 1500;
		int param_set = 0;
		// wcq add
		std::string str;
		if( GetFWParaConfig("bitrate",str) )
		{
			bitrate = atoi(str.c_str());
		}

		if( GetFWParaConfig("param_set",str) )
		{
			param_set = atoi(str.c_str());
		}

		float ratio = 1;
		int s32picWidth = 2560,s32picHeight = 1440;
		if (((uint64_t)s32picWidth * s32picHeight) > (1280 * 720)) {
			ratio = log10f(((uint64_t)s32picWidth * s32picHeight) / (1280 * 720.0)) + 1;
		} else {
			ratio = 1.0 / (log10f((1280 * 720.0) / ((uint64_t)s32picWidth * s32picHeight)) + 1);
		}
		ratio = ratio > 0.1 ? ratio : 0.1;
		bitrate = bitrate * ratio;

		ret = IMP_Encoder_SetDefaultParam(&channel_attr, 
											chn[i].payloadType,
											S_RC_METHOD,
											2560,//2560, 1280, 640
											1440,//1440, 720, 360
											SENSOR_FRAME_RATE_NUM,
											SENSOR_FRAME_RATE_DEN,
											45,
								((S_RC_METHOD == IMP_ENC_RC_MODE_CAPPED_VBR) || (S_RC_METHOD == IMP_ENC_RC_MODE_CAPPED_QUALITY)) ? 3 : 1,
											(S_RC_METHOD == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
											bitrate);
		if (ret < 0) {
			printf("IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
			return -1;
		}
		ret = IMP_Encoder_CreateGroup(chn[i].index);
		if (ret < 0) {
			printf("IMP_Encoder_CreateGroup(%d) error !\n", i);
			return -1;
		}

		if(param_set != 0)
		{
			if( GetFWParaConfig("minqp",str) )
			{
				channel_attr.rcAttr.attrRcMode.attrCbr.iMinQP = atoi(str.c_str());
			}

			if( GetFWParaConfig("maxqp",str) )
			{
				channel_attr.rcAttr.attrRcMode.attrCbr.iMaxQP = atoi(str.c_str());
			}

			channel_attr.rcAttr.attrRcMode.attrCbr.iIPDelta = 30;

			printf("WCQ====bitrate:%d iMinQP:%d iMaxQP:%d\n",bitrate,channel_attr.rcAttr.attrRcMode.attrCbr.iMinQP,channel_attr.rcAttr.attrRcMode.attrCbr.iMaxQP);
		}

// 		IMP_Encoder_SetMaxStreamCnt(0, 1);
		ret = IMP_Encoder_CreateChn(0, &channel_attr);
		if (ret < 0) {
			printf("IMP_Encoder_CreateChn(%d) error :0x%x!\n", chnNum,ret);
			return -1;
		}

		ret = IMP_Encoder_RegisterChn(0, 0);
		if (ret < 0) {
			printf("IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
			return -1;
		}

		if(param_set != 0)
		{
			int i_minqp = 32;
			int i_maxqp = 38; // 38 42
			int p_minqp = 32;
			int p_maxqp = 48; // 48 51
			
			if( GetFWParaConfig("i_minqp",str) )
			{
				i_minqp= atoi(str.c_str());
			}
		
			if( GetFWParaConfig("i_maxqp",str) )
			{
				i_maxqp = atoi(str.c_str());
			}

			if( GetFWParaConfig("p_minqp",str) )
			{
				p_minqp = atoi(str.c_str());
			}

			if( GetFWParaConfig("p_maxqp",str) )
			{
				p_maxqp = atoi(str.c_str());
			}

			ret = IMP_Encoder_SetChnQpBoundsPerFrame(0,i_minqp,i_maxqp,p_minqp,p_maxqp);
			if (ret < 0) {
				printf("IMP_Encoder_SetChnQpBoundsPerFrame(%d) error !\n", chnNum);
				return -1;
			}

			printf("WCQ====i_minqp:%d i_maxqp:%d p_minqp:%d p_maxqp:%d\n",i_minqp,i_maxqp,p_minqp,p_maxqp);
		}
	}

	return 0;
}

int ING_Stream_Mgr::system_exit()
{
	int ret = 0;
	printf("system_exit start\n");
	IMP_System_Exit();

	ret = IMP_ISP_DisableSensor(IMPVI_MAIN);
	if (ret < 0) {
		printf("failed to IMP_ISP_DisableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info_);
	if (ret < 0) {
		printf("failed to IMP_ISP_DelSensor\n");
		return -1;
	}

	ret = IMP_ISP_DisableTuning();
	if (ret < 0) {
		printf("IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	if (IMP_ISP_Close()) {
		printf("failed to open ISP\n");
		return -1;
	}

	printf("system_exit success\n");
	return 0;
}

int ING_Stream_Mgr::framesource_exit()
{
	int ret = 0, i = 0;
	if (chn[i].enable) {
		/*Destroy channel */
		ret = IMP_FrameSource_DestroyChn(chn[i].index);
		if (ret < 0) {
			printf("IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
			return -1;
		}
	}
	return 0;
}

int ING_Stream_Mgr::encoder_exit()
{
	int ret = 0, i = 0, chnNum = 0;
	IMPEncoderChnStat chn_stat;

	if (chn[i].enable) {
		chnNum = chn[i].index;
		memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
		ret = IMP_Encoder_Query(chnNum, &chn_stat);
		if (ret < 0) {
			printf("IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
			return -1;
		}

		if (chn_stat.registered) {
			ret = IMP_Encoder_UnRegisterChn(chnNum);
			if (ret < 0) {
				printf("IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}

			ret = IMP_Encoder_DestroyChn(chnNum);
			if (ret < 0) {
				printf("IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
				return -1;
			}
		}
	}
	return 0;
}

bool GetFWParaConfig(std::string strKey, std::string &strValue)
{
	bool bRet = false;
	char acCmd[128] = {0};
	snprintf(acCmd, sizeof(acCmd), "fw_printenv user %s", strKey.c_str());
	FILE *pFile = popen(acCmd, "r");
	char acValue[512] = {0};
	if(fgets(acValue, sizeof(acValue), pFile) != NULL)
	{
		acValue[strlen(acValue) - 1] = 0;
		strValue = acValue;
		bRet = true;
	}
	else
	{
		bRet = false;
	}
	pclose(pFile);
	return bRet;
}


int ING_Stream_Mgr::system_init()
{
	int ret = 0;
	IMPISPSensorFps sensor_fps;
	IMPISPTuningOpsMode wdr_enable = IMPISP_TUNING_OPS_MODE_DISABLE;

	IMP_OSD_SetPoolSize(512 * 1024);

	memset(&sensor_info_, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info_.name, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info_.cbus_type = SENSOR_CUBS_TYPE;
	memcpy(sensor_info_.i2c.type, SENSOR_NAME, sizeof(SENSOR_NAME));
	sensor_info_.i2c.addr = SENSOR_I2C_ADDR;
	

	printf("system_system_init start I2C_ADDR=0x%x\n", SENSOR_I2C_ADDR);

	IMPISPWdrOpenAttr wdr_attr;
	wdr_attr.rmode = IMPISP_TYPE_RUN_WDR;
	wdr_attr.imode = IMPISP_TYPE_WDR_DOL;

	IMP_ISP_WDR_OPEN(IMPVI_MAIN, &wdr_attr);

	ret = IMP_ISP_Open();
	if (ret < 0) {
		printf("failed to open ISP\n");
		return -1;
	}

	std::string wdr;

	if( GetFWParaConfig("wdr_mode",wdr) )
	{
		if(wdr.compare("1") == 0)
		{
			sensor_info_.default_boot = 1;
			wdr_enable = IMPISP_TUNING_OPS_MODE_ENABLE;
			printf("================ IMPISP_TUNING_OPS_MODE_ENABLE ================\n");
		}
		else
		{
			sensor_info_.default_boot = 0;
		}
	}

	ret = IMP_ISP_WDR_ENABLE(IMPVI_MAIN,&wdr_enable);
	if (ret < 0) {
		printf("failed IMP_ISP_WDR_ENABLE\n");
		return -1;
	}

	ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info_);
	if (ret < 0) {
		printf("failed to AddSensor\n");
		return -1;
		// ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info_);
		// if (ret < 0) {
		// 	printf("failed to IMP_ISP_DelSensor\n");
		// 	return -1;
		// }

		// ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info_);
		// if (ret < 0) {
		// 	printf("failed to AddSensor\n");
		// 	return -1;
		// }
	}

	ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info_);
	if (ret < 0) {
		printf("failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_Encoder_SetIvpuBsSize(1);
    if(ret < 0){
		printf("mxZeratulVideoInterface IMP_Encoder_SetIvpuBsSize failed !\n");
        return false;
    }

    // ret = IMP_Encoder_SetAvpuBsSize(1695232);
    // if(ret < 0){
    //  IMP_LOG_ERR(TAG, "IMP_Encoder_SetAvpuBsSize failed\n");
    //  return false;
    // }

    ret = IMP_Encoder_SetAvpuBsShare(1, 1846592);
    if(ret < 0){
		printf( "mxZeratulVideoInterface IMP_Encoder_SetAvpuBsShare failed !\n");
        return false;
    }

	ret = IMP_System_Init();
	if (ret < 0) {
		printf("IMP_System_Init failed\n");
		return -1;
	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if (ret < 0) {
		printf("IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	sensor_fps.num = SENSOR_FRAME_RATE_NUM;
	sensor_fps.den = SENSOR_FRAME_RATE_DEN;
	ret = IMP_ISP_Tuning_SetSensorFPS(IMPVI_MAIN, &sensor_fps);
	if (ret < 0) {
		printf("IMP_ISP_Tuning_SetSensorFPS failed:%d\n",ret);
		return -1;
	}

	IMPISPHVFLIPAttr attr;

	// ret = IMP_ISP_Tuning_GetHVFLIP(IMPVI_MAIN, &attr);
	// if(ret){
	// 	printf("IMP_ISP_Tuning_GetHVFLIP error !\n");
	// 	return -1;
	// }
	// attr.sensor_mode = IMPISP_FLIP_HV_MODE;
	// ret = IMP_ISP_Tuning_SetHVFLIP(IMPVI_MAIN, &attr);
	// if(ret){
	// 	printf("IMP_ISP_Tuning_SetHVFLIP error !\n");
	// 	return -1;
	// }

	// printf("\n\n\n\n\n\n\n\nWC11111Q=\n\n\n\n\n\n\n\n\n=================");
	printf("Imp System Init success\n");

	// IMPISPAEExprInfo info;
	// ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
	// if(ret){
	// 	printf("IMP_ISP_Tuning_GetAeExprInfo error !\n");
	// 	return -1;
	// }

	// info.AeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
	// // info.AeIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
	// // info.AeAGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
	// // info.AeIspDGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
	// info.AeIntegrationTime = 3492;
	// info.AeAGain = 5000;
	// info.AeIspDGain = 512;

	// ret = IMP_ISP_Tuning_SetAeExprInfo(IMPVI_MAIN, &info);
	// if(ret){
	// 	printf("IMP_ISP_Tuning_SetAeExprInfo error !\n");
	// 	return -1;
	// }

	return 0;
}

int ING_Stream_Mgr::system_bind()
{
	int i = 0, ret = 0;
	IMPCell osd0 = { DEV_ID_OSD, 0, 0 };

	ret = IMP_System_Bind(&chn[i].framesource_chn, &osd0);
	if (ret < 0) {
		printf("Bind FrameSource channel%d and Encoder failed 1\n", i);
		return -1;
	}

	ret = IMP_System_Bind(&osd0, &chn[i].imp_encoder);
	if (ret < 0) {
		printf("Bind FrameSource channel%d and Encoder failed 2\n", i);
		return -2;
	}
	return 0;
}

int ING_Stream_Mgr::system_unbind()
{
	int i = 0, ret = 0;
	IMPCell osd0 = { DEV_ID_OSD, 0, 0 };
	if (chn[i].enable) {
		ret = IMP_System_UnBind(&chn[i].framesource_chn, &osd0);
		if (ret < 0) {
			printf("UnBind FrameSource channel%d and Encoder failed\n", i);
			return -1;
		}
		
		ret = IMP_System_UnBind(&osd0, &chn[i].imp_encoder);
		if (ret < 0) {
			printf("UnBind FrameSource channel%d and Encoder failed\n", i);
			return -1;
		}
	}
	return 0;
}

int ING_Stream_Mgr::framesource_enable()
{
	int ret = 0, i = 0;
	/* Enable channels */
	if (chn[i].enable) {
		ret = IMP_FrameSource_EnableChn(chn[i].index);
		if (ret < 0) {
			printf("IMP_FrameSource_EnableChn(%d) error: %d\n", chn[i].index, ret);
			return -5;
		}
		ret = IMP_Encoder_StartRecvPic(i);
		if (ret < 0) {
			printf("IMP_Encoder_StartRecvPic(%d) failed\n", i);
			return -6;
		}

		// IMPISPAEExprInfo info;
		// ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
		// if(ret){
		// 	printf("IMP_ISP_Tuning_GetAeExprInfo error !\n");
		// 	return -1;
		// }

		// printf("WCQ=========================\n");

		// info.AeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
		// // info.AeIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
		// // info.AeAGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
		// // info.AeIspDGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
		// info.AeIntegrationTime = 3492;
		// info.AeAGain = 30720;
		// info.AeIspDGain = 1024;

		// ret = IMP_ISP_Tuning_SetAeExprInfo(IMPVI_MAIN, &info);
		// if(ret){
		// 	printf("IMP_ISP_Tuning_SetAeExprInfo error !\n");
		// 	return -1;
		// }

	}

	return 0;
}

int ING_Stream_Mgr::framesource_disable()
{
	int ret = 0, i = 0;
	/* Enable channels */
	if (chn[i].enable) {
		printf("ING_Stream_Vdo::framesource_disable 00000000000000000000000000000000000000000\n");
		ret = IMP_Encoder_StopRecvPic(i);
		if (ret < 0) {
			printf("IMP_Encoder_StopRecvPic() failed\n");
			return -5;
		}
		printf("ING_Stream_Vdo::framesource_disable 111111111111111111111111111111111111111111\n");
		ret = IMP_FrameSource_DisableChn(chn[i].index);
		if (ret < 0) {
			printf("IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
			return -6;
		}
		printf("ING_Stream_Vdo::framesource_disable 2222222222222222222222222222222222222222222\n");
	}

	return 0;
}
