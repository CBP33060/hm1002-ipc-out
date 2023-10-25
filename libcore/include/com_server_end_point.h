#ifndef __COM_SERVER_END_POINT_H__
#define __COM_SERVER_END_POINT_H__
#include "i_com_server_end_point.h"
#include "i_com_server_handle.h"
#include "module.h"

namespace maix {
	class MAIX_EXPORT CComServerEndPoint : public CIComServerEndPoint
	{
	public:
		CComServerEndPoint(CModule* module);
		mxbool init(std::shared_ptr<CIComServerHandle> handle);
		virtual std::string  input(unsigned char * pcData, int iLen);

	private:
		std::shared_ptr<CIComServerHandle> m_handle;
		CModule* m_module;
	};
}
#endif //__COM_SERVER_END_POINT_H__
