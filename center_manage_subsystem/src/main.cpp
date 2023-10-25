#include "center_manage_app.h"
#include "log_mx.h"
#include <iostream>
#include "crypt_api_mx.h"
using namespace maix;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "please input config path!!!" << std::endl;
		return -1;
	}
	verify_boot();
	std::shared_ptr<CCenterManageApp> objCenterManageApp(
		new CCenterManageApp("center_manage_app"));
	if (!objCenterManageApp->loadConfig(argv[1]))
	{
		std::cout << "[CenterManageApp]: load failed" << std::endl;
		return -1;
	}
	
	if (!objCenterManageApp->init())
	{
		std::cout << "[CenterManageApp]: init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"CenterManageApp init failed");
		return -1;
	}

	objCenterManageApp->setState(E_APP_START);
	objCenterManageApp->start();

	return 0;
}