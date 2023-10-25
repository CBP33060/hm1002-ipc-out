#ifndef __I_COM_SERVER_END_POINT_H__
#define __I_COM_SERVER_END_POINT_H__
#include "global_export.h"
#include <string>

namespace maix {
	class CIComServerEndPoint
	{
	public:
		virtual std::string input(unsigned char * pcData, int iLen) = 0;
	};
}

#endif //__I_COM_SERVER_END_POINT_H__
