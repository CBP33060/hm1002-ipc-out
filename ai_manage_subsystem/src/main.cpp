#include "ai_manage_app.h"
#include "log_mx.h"
#include <iostream>
#include "crypt_api_mx.h"
#include "common.h"

using namespace maix;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "please input config path!!!" << std::endl;
		return -1;
	}
	bindCpu(0);
	verify_boot();
	std::shared_ptr<CAIManageApp> objAIManageApp(
		new CAIManageApp("ai_manage_app"));
	
	if (!objAIManageApp->loadConfig(argv[1]))
	{
		std::cout << "AIManageApp load failed" << std::endl;
		return -1;
	}
	
	if (!objAIManageApp->init())
	{
		std::cout << "AIManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"AIManageApp init failed");
		return -1;
	}
	
	objAIManageApp->setState(E_APP_START);
	objAIManageApp->start();

	return 0;
}