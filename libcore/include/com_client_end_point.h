#ifndef __COM_CLIENT_END_POINT_H__
#define __COM_CLIENT_END_POINT_H__
#include "i_com_client_end_point.h"
#include <string>
#include <map>
#include "module.h"

namespace maix {
	class MAIX_EXPORT CComClientEndPoint : public CIComClientEndPoint
	{
	public:
		CComClientEndPoint(CModule* module);
		virtual std::string  output(std::string strServerName,
			unsigned char * pcData, int iLen);

		virtual int getClientType();
	private:
		CModule* m_module;
	};
}
#endif //__COM_CLIENT_END_POINT_H__
