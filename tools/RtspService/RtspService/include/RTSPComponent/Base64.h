/* 
 *  Copyright (c) 2005, ���մ����Ƽ����޹�˾
 *  All rights reserved.
 *  
 *  �ļ�����: Base64.h
 *  �ļ���ʶ: 
 *  ժ    Ҫ:	
 *  
 *  ��ǰ�汾: 
 *  ��    ��: Tommy
 *  �������: 2006-02-16 08:39:50
 *  ������ע: 
 *  
 */

#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

/*  ��������: Base64����
 *  ����˵��:
 *      char *pszEncoded [OUT]:	������Base64�ַ���,��0��β
 *								�ϲ���Ҫ��֤�����С�ܹ��㹻��ű���������
 *      char *pData [IN]:		��Ҫ����Ķ���������
 *      int iSize [IN]:			���������ݵĳ���
 *  ����ֵ: ���ر�����ַ����ĳ���,��������β��0
 */
int Base64_Encode(char *pszEncoded, char *pData, int iSize);

/*  ��������: Base64����
 *  ����˵��:
 *      char *pDecoded [OUT]:	�����Ķ��������ݵĴ�Ż����ַ
 *								����Ҫ�ܹ����½���������,�����ض�
  *      char *pszData [INT]:	��Ҫ�����Base64�ַ���,��0��β	
 *      int iSize [IN]:			Base64�ַ�������,������4��������
 *  ����ֵ: ���ؽ����ʵ�����Ķ����ƻ���ĳ���
 */
int Base64_Decode(char *pDecoded, char *pszData, int iSize);

#ifdef __cplusplus
}
#endif

#endif //__BASE64_H__
