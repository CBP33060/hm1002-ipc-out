#include "INGCommon.h"

unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


char* RTSP_U32ToAddr(unsigned int u32IP, char szIP[RTSP_MAX_IPSTR_LEN])
{
	memset(szIP, 0, RTSP_MAX_IPSTR_LEN);
	sprintf(szIP, "%d.%d.%d.%d",
		(u32IP >> 0) & 0x000000FF,
		(u32IP >> 8) & 0x000000FF,
		(u32IP >> 16) & 0x000000FF,
		(u32IP >> 24) & 0x000000FF);
	return (char *)szIP;
}

unsigned int RTSP_AddrToU32(const char *cpszAddr)
{
	return (unsigned int)inet_addr(cpszAddr);
}
