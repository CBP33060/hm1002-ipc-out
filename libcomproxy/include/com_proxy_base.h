#ifndef __COM_PROXY_BASE_H__
#define __COM_PROXY_BASE_H__
#include "global_export.h"
#include <RCF/RCF.hpp>

RCF_BEGIN(I_ComProxy, "ComProxy")
RCF_METHOD_V1(void, frameProc, RCF::ByteBuffer);
RCF_METHOD_R1(std::string, eventProc, std::string);

RCF_END(I_ComProxy);

//#define SHM_LIST_NUM 3
//#define SHM_DATA_LEN 629145
namespace maix {
	typedef struct _SHMRingBuf
	{
		unsigned int   iDataLen;
		unsigned char  pDataBuf[0];
	}T_SHMRingBuf;

	class MAIX_EXPORT CComProxyBase
	{
	public:
		CComProxyBase();
		virtual ~CComProxyBase();

		virtual void frameProc(RCF::ByteBuffer byteBuffer);
		virtual void frameProc(unsigned char* byteBuffer, int len);
		virtual std::string eventProc(std::string strMsg);
	};
}
#endif //__COM_PROXY_BASE_H__
