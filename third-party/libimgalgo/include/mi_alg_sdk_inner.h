/*******************************************************************************
 *
 *
 *file     mi_alg_sdk_inner.h
 *brief    算法SDK内部头文件
 *
 *author   wangningbo
 *version  1.0.0
 *date     05/12/2021
 *
 *history  1.0.0 05/12/2021, wangningbo, Create file.
 *
 *Copyright (c) 2020 Beijing Xiaomi Mobile Software Co., Ltd. All rights reserved.
 *
 *******************************************************************************/
#ifndef _MI_ALG_SDK_INNER_H_
#define _MI_ALG_SDK_INNER_H_

#include "mi_alg_type.h"
#include <pthread.h>
#include <stdio.h>
#ifdef USE_MIKE
#include "mi_log.h"
#endif



#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_MIKE
#define ALG_SDK_TAG         "MI_ALG_SDK"
#define PRT(fmt, arg...)    MI_LOG_DEBUG(ALG_SDK_TAG, fmt, ##arg)
#define PRTI(fmt, arg...)   MI_LOG_INFO(ALG_SDK_TAG, fmt, ##arg)
#define PRTE(fmt, arg...)   MI_LOG_ERROR(ALG_SDK_TAG, fmt, ##arg)
#else
#define PRT(fmt, arg...) \
	do{\
			printf("[D][%s][%d]..." fmt , __FUNCTION__, __LINE__, ##arg);\
	  }while(0)

#define PRTI(fmt, arg...) \
    do{\
            printf("[I][%s][%d]..." fmt , __FUNCTION__, __LINE__, ##arg);\
      }while(0)

        
#define PRTE(fmt, arg...) \
    do{\
            printf("[E][%s][%d]..." fmt , __FUNCTION__, __LINE__, ##arg);\
      }while(0)
      
#endif


/*算法模型控制信息*/   
typedef struct _alg_model_ctl_{
    int valid;                                          /*是否有效 0:无效，其他有效*/
    void *p_handle;                                     /*算法模型句柄*/
    mi_alg_model_param param;                           /*算法模型配置参数*/
}alg_model_ctl_t;

/*算法控制信息*/
typedef struct _alg_ctl_{
    int valid;                                          /*是否有效 0:无效，其他有效*/
    void *p_handle;                                     /*算法模型句柄*/
    mi_alg_init_param_t init_param;                     /*算法模型配置参数*/
}alg_ctl_t;

/*算法SDK控制信息*/
typedef struct _alg_sdk_ctl_{
    pthread_mutex_t lock;                               /*资源锁*/
    void *p_lib_handle;                                 /*算法动态库句柄*/    
    alg_model_ctl_t model_ctl_info;                     /*算法模型控制信息*/   
    alg_ctl_t alg_ctl_info;                             /*算法控制信息*/
    mi_alg_essential_interface_info_t *p_fun_handle;    /*算法接口*/
}alg_sdk_ctl_t;


/*句柄状态*/
typedef struct _alg_sdk_handle_state_t_{
    int valid;                                          /*0无效，其他有效*/
    void *p_handle;                                     /*alg sdk句柄*/
}alg_sdk_handle_state_t;


#ifdef __cplusplus
}
#endif
#endif


