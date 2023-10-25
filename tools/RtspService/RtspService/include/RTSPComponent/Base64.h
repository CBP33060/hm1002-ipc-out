/* 
 *  Copyright (c) 2005, 安徽创世科技有限公司
 *  All rights reserved.
 *  
 *  文件名称: Base64.h
 *  文件标识: 
 *  摘    要:	
 *  
 *  当前版本: 
 *  作    者: Tommy
 *  完成日期: 2006-02-16 08:39:50
 *  修正备注: 
 *  
 */

#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

/*  功能描述: Base64编码
 *  参数说明:
 *      char *pszEncoded [OUT]:	编码后的Base64字符串,以0结尾
 *								上层需要保证缓冲大小能够足够存放编码后的数据
 *      char *pData [IN]:		需要编码的二进制数据
 *      int iSize [IN]:			二进制数据的长度
 *  返回值: 返回编码后字符串的长度,不包括结尾的0
 */
int Base64_Encode(char *pszEncoded, char *pData, int iSize);

/*  功能描述: Base64解码
 *  参数说明:
 *      char *pDecoded [OUT]:	解码后的二进制数据的存放缓冲地址
 *								必须要能够放下解码后的数据,否则会截断
  *      char *pszData [INT]:	需要解码的Base64字符串,以0结尾	
 *      int iSize [IN]:			Base64字符串长度,必须是4的整数倍
 *  返回值: 返回解码后实际填充的二进制缓冲的长度
 */
int Base64_Decode(char *pDecoded, char *pszData, int iSize);

#ifdef __cplusplus
}
#endif

#endif //__BASE64_H__
