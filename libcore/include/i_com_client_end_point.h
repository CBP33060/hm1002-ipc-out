#ifndef __I_COM_CLIENT_END_POINT_H__
#define __I_COM_CLIENT_END_POINT_H__
#include "global_export.h"
#include "type_def.h"
#include <string>

namespace maix {
	class CIComClientEndPoint
	{
	public:
		virtual std::string output(std::string strServerName,
			unsigned char * pcData, int iLen) = 0;
		virtual int getClientType() = 0;
	};
}
#endif //__I_COM_CLIENT_END_POINT_H__
