#include "video_manage_app.h"
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
	std::shared_ptr<CVideoManageApp> objVideoManageApp(
		new CVideoManageApp("video_manage_app"));
	if (!objVideoManageApp->loadConfig(argv[1]))
	{
		std::cout << "VideoManageApp load failed" << std::endl;
		return -1;
	}

	if (!objVideoManageApp->init())
	{
		std::cout << "VideoManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"VideoManageApp init failed");
		return -1;
	}

	objVideoManageApp->setState(E_APP_START);
	objVideoManageApp->start();

	return 0;
}
