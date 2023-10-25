#include "log_manage_app.h"
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
	std::shared_ptr<CLogManageApp> objLogManageApp(
		new CLogManageApp("log_manage_app"));
    if (!objLogManageApp->loadConfig(argv[1]))
	{
		std::cout << "LOGManageApp load failed" << std::endl;
		return -1;
	}

	if (!objLogManageApp->init())
	{
		std::cout << "LOGManageApp init failed" << std::endl;
        logPrint(MX_LOG_ERROR,"LOGManageApp init failed");
		return -1;
	}

	objLogManageApp->setState(E_APP_START);
	objLogManageApp->start();

	return 0;
}