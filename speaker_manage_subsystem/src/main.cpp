#include "speaker_manage_app.h"
#include "log_mx.h"
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
	std::shared_ptr<CSpeakerManageApp> objSpeakerManageApp(
		new CSpeakerManageApp("speaker_manage_app"));

	if (!objSpeakerManageApp->loadConfig(argv[1]))
	{
		std::cout << "SpeakerManageApp load failed" << std::endl;
		return -1;
	}

	if (!objSpeakerManageApp->init())
	{
		std::cout << "SpeakerManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"SpeakerManageApp init failed");
		return -1;
	}
	
	objSpeakerManageApp->setState(E_APP_START);
	objSpeakerManageApp->start();

	return 0;
}