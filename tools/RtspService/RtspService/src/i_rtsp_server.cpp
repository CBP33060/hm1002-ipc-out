#include "INGCommon.h"
#include "ING_Controller.h"



int main()
{
	if (!access("/tmp/share/stream5.h264", F_OK))
	{
		remove("/tmp/share/stream5.h264");
	}
	if (!access("/tmp/share/stream5pack.h264", F_OK))
	{
		remove("/tmp/share/stream5pack.h264");
	}
	ING_Controller *controller = new ING_Controller();
	if (controller == NULL) {
		return -1;
		printf("new controller failed\n");
	}
	int ret = controller->open(NULL);
	if (ret != 0)
	{
		controller->close();
		delete controller;
		return -1;
	}
	while (1) {
		controller->run();
		usleep(1000*5);
	}

	return 0;
}
