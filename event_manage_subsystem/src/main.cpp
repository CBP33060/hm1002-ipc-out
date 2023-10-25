#include "event_manage_app.h"
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
	std::shared_ptr<CEventManageApp> objEventManageApp(
		new CEventManageApp("event_manage_app"));
	if (!objEventManageApp->loadConfig(argv[1]))
	{
		std::cout << "[EventManageApp]: load config failed" << std::endl;
		return -1;
	}

	if (!objEventManageApp->init())
	{
		std::cout << "[EventManageApp]: init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"EventManageApp init failed");
		return -1;
	}

	objEventManageApp->setState(E_APP_START);
	objEventManageApp->start();

	return 0;
}