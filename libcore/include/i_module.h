#ifndef __I_MODULE_H__
#define __I_MODULE_H__
#include "type_def.h"
#include "global_export.h"
#include "i_com_client_end_point.h"
#include "i_com_server_end_point.h"
#include <string>
#include <map>
#include <mutex>
#include <future>

namespace maix {
	class CIModule
	{
	public:
		virtual std::string getGUID() = 0;
		virtual std::string getName() = 0;
		
		virtual mxbool connect(std::shared_ptr<CIModule> module) = 0;
		virtual mxbool disconnect(std::string strGUID) = 0;
		virtual mxbool getConnectModule(std::string strGUID,
			std::shared_ptr<CIModule> &module) = 0;

		virtual mxbool reg(std::shared_ptr<CIModule> module, 
			unsigned char* pcData, int iLen) = 0;
		virtual mxbool unReg(std::shared_ptr<CIModule> module) = 0;

		virtual std::string input(std::shared_ptr<CIModule> sender, 
			std::string strServerName, 
			unsigned char* pcData, int iLen) = 0;

		virtual mxbool regClient(std::string strServerName, 
			std::shared_ptr<CIComClientEndPoint> client) = 0;
		virtual mxbool unRegClient(std::string strServerName) = 0;
		virtual mxbool isClientExist(std::string strServerName) = 0;
		virtual int getClientType(std::string strServerName) = 0;
		virtual mxbool regServer(std::string strServerName, 
			std::shared_ptr<CIComServerEndPoint> server) = 0;
		virtual mxbool unRegServer(std::string strServerName) = 0;
	};

}
#endif //__I_MODULE_H__
