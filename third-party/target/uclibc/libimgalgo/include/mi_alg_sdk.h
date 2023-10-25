/*******************************************************************************
 *
 *
 *file     mi_alg_sdk.h
 *brief    算法SDK对外接口.
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
#ifndef _MI_ALG_SDK_H_
#define _MI_ALG_SDK_H_

#include "mi_alg_type.h"
#include "mi_alg_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
  *@Function: mi_alg_model_load
  * 
  *@ Description: 算法模型加载函数接口
  * 
  *@ Input: 
  *     p_alg_model_handle: 指向算法模型句柄的指针
  *     p_param: 加载算法模型所需配置参数
  * 
  *@ Output:
  *     p_alg_model_handle: 指向算法模型句柄的指针
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note: 
  *     注意传入二级指针
*******************************************************************************/
mi_alg_error_t mi_alg_model_load(void **p_alg_model_handle, mi_alg_model_param *p_param);

/*******************************************************************************
  *@Function: mi_alg_model_unload
  * 
  *@ Description: 算法模型卸载函数接口
  * 
  *@ Input: 
  *     p_alg_model_handle: 算法模型句柄
  * 
  *@ Output: N/A
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意调用顺序 
*******************************************************************************/
mi_alg_error_t mi_alg_model_unload(void *p_alg_model_handle);

/*******************************************************************************
  *@Function: mi_alg_model_version_get
  * 
  *@ Description: 获取算法模型版本
  * 
  *@ Input: 
  *     p_alg_model_handle: 算法模型句柄
  * 
  *@ Output:
  *     算法模型版本信息
  * 
  *@ Return: N/A
  * 
  *@ Note:
  *     注意调用顺序 
*******************************************************************************/
const char *mi_alg_model_version_get(void *p_model_handle);

/*******************************************************************************
  *@Function: mi_alg_handle_create
  * 
  *@ Description: 创建算法句柄
  * 
  *@ Input: 
  *     p_alg_handle: 指向算法句柄的指针
  *     p_alg_param: 参数
  * 
  *@ Output:
  *     p_alg_handle: 指向算法句柄的指针
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意调用顺序，需要先调用mi_alg_model_load
*******************************************************************************/
mi_alg_error_t mi_alg_handle_create(void **p_alg_handle, mi_alg_init_param_t *p_alg_param);

/*******************************************************************************
  *@Function: mi_alg_handle_destroy
  * 
  *@ Description: 销毁算法句柄
  * 
  *@ Input: 
  *     p_alg_handle: 算法句柄
  * 
  *@ Output: N/A
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意调用顺序
*******************************************************************************/
mi_alg_error_t mi_alg_handle_destroy(void *p_alg_handle);

/*******************************************************************************
  *@Function: mi_alg_param_set
  * 
  *@ Description: 配置算法参数
  * 
  *@ Input: 
  *     p_alg_handle: 算法句柄
  *     mode: 配置类型，用于区分此次配置目的
  *     p_param: 配置参数
  * 
  *@ Output: N/A
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意配置参数需要根据mode采用不同结构体
*******************************************************************************/
mi_alg_error_t mi_alg_param_set(void *p_alg_handle, mi_alg_set_mode_t mode, void *p_param);

/*******************************************************************************
  *@Function: mi_alg_param_get
  * 
  *@ Description: 获取算法当前参数
  * 
  *@ Input: 
  *     p_alg_handle: 算法句柄
  *     mode: 类型，用于区分此次获取目的
  *     p_param: 指向参数地址的指针
  * 
  *@ Output: 
  *     p_param: 获取到的参数
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意参数获取需要根据mode采用不同结构体
*******************************************************************************/
mi_alg_error_t mi_alg_param_get(void *p_alg_handle, mi_alg_set_mode_t mode, void *p_param);

/*******************************************************************************
  *@Function: mi_alg_input
  * 
  *@ Description: 算法输入接口
  * 
  *@ Input: 
  *     p_alg_handle: 算法句柄
  *     p_input: 输入内容
  * 
  *@ Output: N/A
  * 
  *@ Return: 
  *     0: 成功，其他失败
  * 
  *@ Note:
  *     注意调用顺序
*******************************************************************************/
mi_alg_error_t mi_alg_input(void *p_alg_handle, mi_alg_input_param_t *p_input);

/*******************************************************************************
  *@Function: mi_alg_version_get
  * 
  *@ Description: 获取算法版本
  * 
  *@ Input: 
  *     p_alg_handle: 算法句柄
  * 
  *@ Output: 
  *     算法版本信息
  * 
  *@ Return:  N/A
  * 
  *@ Note:
  *     注意调用顺序
*******************************************************************************/
const char *mi_alg_version_get(void *p_alg_handle);


#ifdef __cplusplus
}
#endif
#endif

