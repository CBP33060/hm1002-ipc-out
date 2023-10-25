#include "local_storage_app.h"
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
	std::shared_ptr<CLocalStorageApp> objLocalStorageApp(
		new CLocalStorageApp("local_storage_app"));
	if (!objLocalStorageApp->loadConfig(argv[1]))
	{
		std::cout << "load failed" << std::endl;
	}

	if (!objLocalStorageApp->init())
	{
		std::cout << "init failed" << std::endl;
	}

	objLocalStorageApp->setState(E_APP_START);
	objLocalStorageApp->start();

	return 0;
}