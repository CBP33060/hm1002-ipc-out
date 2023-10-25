#ifndef __MCU_SERIAL_PORT_REMOTE_EVENT_SERVER_H__
#define __MCU_SERIAL_PORT_REMOTE_EVENT_SERVER_H__
#include "com_proxy_base.h"
#include "mcu_serial_port_module.h"

namespace maix {
	class CMCUSerialPortRemoteEventServer : public CComProxyBase
	{
	public:
		CMCUSerialPortRemoteEventServer(CMCUSerialPortModule
			*objMCUSerialPortModule);
		~CMCUSerialPortRemoteEventServer();

		mxbool init();
		mxbool unInit();

		std::string eventProc(std::string strMsg);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
	private:
		CMCUSerialPortModule *m_objMCUSerialPortModule;
	};
}
#endif //__MCU_SERIAL_PORT_REMOTE_EVENT_SERVER_H__
