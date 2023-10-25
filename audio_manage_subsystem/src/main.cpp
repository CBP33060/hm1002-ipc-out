#include "audio_manage_app.h"
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
	
	std::shared_ptr<CAudioManageApp> objAudioManageApp(
		new CAudioManageApp("audio_manage_app"));
	if (!objAudioManageApp->loadConfig(argv[1]))
	{
		std::cout << "AudioManageApp load failed" << std::endl;
		return -1;
	}
	
	if (!objAudioManageApp->init())
	{
		std::cout << "AudioManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"AudioManageApp init failed");
		return -1;
	}
		
	objAudioManageApp->setState(E_APP_START);
	objAudioManageApp->start();

	return 0;
}
