#ifndef __I_COM_SERVER_HANDLE_H__
#define __I_COM_SERVER_HANDLE_H__
#include "global_export.h"
#include <string>

namespace maix {
	class MAIX_EXPORT CIComServerHandle
	{
	public:
		virtual std::string msgHandle(unsigned char * pcData, int iLen) = 0;
	};
}
#endif //__I_COM_SERVER_HANDLE_H__
