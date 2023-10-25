#include "ota_manage_app.h"
#include <iostream>
#include "crypt_api_mx.h"
#include "log_mx.h"

using namespace maix;

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "please input config path!!!" << std::endl;
		return -1;
	}
	verify_boot();
	std::shared_ptr<COTAManageApp> objOTAManageApp(
		new COTAManageApp("ota_manage_app"));
	if (!objOTAManageApp->loadConfig(argv[1]))
	{
		std::cout << "OTAManageApp load failed" << std::endl;
		return -1;
	}

	if (!objOTAManageApp->init())
	{
		std::cout << "OTAManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"OTAManageApp init failed");
		return -1;
	}

	objOTAManageApp->setState(E_APP_START);
	objOTAManageApp->start();

	return 0;
}