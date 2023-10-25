#ifndef __I_APP_H__
#define __I_APP_H__
#include "type_def.h"

namespace maix {
	enum E_APP_STATE
	{
		E_APP_ERROR,
		E_APP_STOP,
		E_APP_START,
		E_APP_PASE,
	};

	class CIApp
	{
		virtual mxbool init() = 0;
		virtual mxbool unInit() = 0;
		virtual E_APP_STATE getState() = 0;
		virtual void setState(E_APP_STATE state) = 0;
	};
}
#endif //__I_APP__
