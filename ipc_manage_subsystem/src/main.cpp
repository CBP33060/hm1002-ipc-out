#include "ipc_manage_app.h"
#include <iostream>
#include "crypt_api_mx.h"
#include "log_mx.h"
#include "common.h"

using namespace maix;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "please input config path!!!" << std::endl;
		return -1;
	}
	bindCpu(1);
	verify_boot();
	std::shared_ptr<CIPCManageApp> objIPCManageApp(
		new CIPCManageApp("ipc_manage_app"));
	if (!objIPCManageApp->loadConfig(argv[1]))
	{
		std::cout << "IPCManageApp load failed" << std::endl;
		return -1;
	}

	if (!objIPCManageApp->init())
	{
		std::cout << "IPCManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"IPCManageApp init failed");
		return -1;
	}

	objIPCManageApp->setState(E_APP_START);
	objIPCManageApp->start();

	return 0;
}
