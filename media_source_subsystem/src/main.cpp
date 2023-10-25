#include "media_source_app.h"
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

	bindCpu(1);

	// verify_boot();
	std::shared_ptr<CMediaSourceApp> objMediaSourceApp(
		new CMediaSourceApp("media_source_app"));
	if (!objMediaSourceApp->loadConfig(argv[1]))
	{
		std::cout << "MediaSourceApp load config failed" << std::endl;
		return -1;
	}

	if(!objMediaSourceApp->init())
	{
		std::cout << "MediaSourceApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"MediaSourceApp init failed");
		return -1;
	}
	
	objMediaSourceApp->setState(E_APP_START);
	objMediaSourceApp->start();
	
	return 0;
}
