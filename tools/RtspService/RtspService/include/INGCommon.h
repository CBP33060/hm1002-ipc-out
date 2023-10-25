#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h> 
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>

typedef void *HANDLE;
#define RTSP_MAX_IPSTR_LEN 16

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetTickCount();
char* RTSP_U32ToAddr(unsigned int u32IP, char szIP[RTSP_MAX_IPSTR_LEN]);
unsigned int RTSP_AddrToU32(const char *cpszAddr);
