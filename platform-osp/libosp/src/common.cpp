#include "common.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <time.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <sys/timeb.h>
#else
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h> 
#include <unistd.h> 
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h> 
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <stdarg.h>
#include <sys/time.h>
#endif
#include "fw_env_para.h"
#include <string.h>
#include <thread>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

mxbool bindCpu(int cpu_id)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu_id,&mask);
	if(sched_setaffinity(0,sizeof(mask),&mask) == -1)
	{
		printf("warning: could not set CPU affinity\n");
		return mxfalse;
	}	

	return mxtrue;
}

mxbool getFiles(std::string path,
	std::vector<std::string>& files, const char* sType)
{
#ifdef _WIN32
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),
		&fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && 
					strcmp(fileinfo.name, "..") != 0)
				{
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files, sType);
				}
			}
			else
			{
				char* pName = fileinfo.name;
				char* pFind = strstr(pName, sType);
				if (pFind != NULL)
				{
					files.push_back(p.assign(path).
						append("\\").append(fileinfo.name));
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
#else
    DIR* dir = NULL;
    if ((dir = opendir(path.c_str())) != NULL)
    {
		struct dirent* d_ent = NULL;
        while ( (d_ent = readdir(dir)) != NULL )
        {
			if ( d_ent->d_type == DT_DIR )
			{
				if (strcmp(d_ent->d_name, ".") != 0 && 
				 	strcmp(d_ent->d_name, "..") != 0 )
				{
					getFiles(path + "/" + d_ent->d_name, files, sType);
				}
			}
			else
			{
				if ( NULL != strstr(d_ent->d_name, sType) )
				{
					files.push_back(path + "/" + d_ent->d_name);
				}
			}
        }
        closedir(dir);
    }
#endif

	return mxtrue;
}


mxbool getLocalIPByName(std::string strName, std::string &strIP)
{
#ifdef _WIN32
    bool bFind = false;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

	if (pAdapterInfo == NULL)
	{
		return mxfalse;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			return mxfalse;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			if (strName.compare(pAdapter->Description) == 0)
			{
				strIP = pAdapter->IpAddressList.IpAddress.String;
				bFind = true;
				break;
			}

			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		if (pAdapterInfo)
		{
			free(pAdapterInfo);
		}
		return mxfalse;
	}

	if (pAdapterInfo)
	{
		free(pAdapterInfo);
	}
	
	return bFind;
#else
	int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        // printf("socket error: %s\n", strerror(errno));
        return mxfalse;
    }
 
    strncpy(ifr.ifr_name, strName.c_str(), IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
 
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        // printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return mxfalse;
    }
    
    //printf("interfac: %s, ip: %s\n", strName.c_str(), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr)); 
	strIP = inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
	//printf("interfac: %s, ip: %s\n", strName.c_str(), strIP.c_str()); 

    close(sd);
    return mxtrue;
#endif

}


mxbool linuxPopenExecCmd(std::string &strOutData, const char * pFormat, ...)
{
    char acBuff[512] ={0};

    va_list ap;
    int ret = -1;
    va_start(ap, pFormat);
    ret = vsprintf(acBuff, pFormat, ap);
    va_end(ap);
#ifndef _WIN32
	FILE *pFile = popen(acBuff, "r");
	if (NULL == pFile)
	{	
		return mxfalse;
	}

	char acValue[512] = {0};
	while(fgets(acValue, sizeof(acValue), pFile) != NULL)
	{
		strOutData += acValue;
	}
	pclose(pFile);
#endif

	return mxtrue;
}

mxbool getGUIDData(std::string & strGUID)
{
	char acGUID[64] = { 0 };
	struct timeval tv;
	gettimeofday(&tv, NULL);    
	int64_t iConTime = tv.tv_sec * 1000 * 1000 + tv.tv_usec;	
	srand(iConTime);
	snprintf(acGUID, sizeof(acGUID),
		"%08X-%04X-%04X-%04X-%04X%04X%04X",
		rand() & 0xffffffff,
		rand() & 0xffff,
		rand() & 0xffff,
		rand() & 0xffff,
		rand() & 0xffff, rand() & 0xffff, rand() & 0xffff
		);

	strGUID = acGUID;
	return  mxtrue;
}

mxbool getKey(std::string &strKey)
{
	char acKey[64] = { 0 };
	int iTime = time(NULL);
	srand(iTime);
	snprintf(acKey, sizeof(acKey),
		"%08X%08X",
		rand() & 0xffffffff,
		rand() & 0xffffffff
		);

	strKey = acKey;
	return  mxtrue;
}

int versionCompare(const std::string &oldVersion, const std::string &newVersion)
{
    int iOldPos = 0, iNewPos = 0;
    unsigned long long int iOldNum = 0, iNewNum = 0;

    while (iOldPos < oldVersion.length() || iNewPos < newVersion.length()) 
    {
        iOldNum = 0;
        while (iOldPos < oldVersion.length() && oldVersion[iOldPos] != '.') 
        {
            iOldNum = iOldNum * 10 + (oldVersion[iOldPos] - '0');
            iOldPos++;
        }

        iNewNum = 0;
        while (iNewPos < newVersion.length() && newVersion[iNewPos] != '.') 
        {
            iNewNum = iNewNum * 10 + (newVersion[iNewPos] - '0');
            iNewPos++;
        }

        if (iOldNum < iNewNum) {
            return 1;
        } else if (iOldNum > iNewNum) {
            return -1;
        }

        iOldPos++;
        iNewPos++;
    }

    return 0;
}

extern "C" {
	#include "efuse.h"
	int readEfuse(int seg_id, int offset, unsigned char *buf, int len);
	int writeEfuse(int seg_id, int offset, unsigned char *buf, int len, int force);
}

std::string getDID()
{
#ifdef WIN32
	return std::string("667179728");
#else
	const char* lock_file_path = "/tmp/efuse.lock";
    int times = 0;
    int fd = open(lock_file_path,O_RDWR | O_CREAT);
	if(fd < 0) {
		return "";
	}

    while(0 != flock(fd,LOCK_EX | LOCK_NB ))
    {
		if(++times > 10) {
			flock(fd,LOCK_UN);
    		close(fd);
			return "";
		}
        usleep(100 * 1000);
    }

	char did[DID_LEN + 1] = { 0 };
	if(readEfuse(SCB_DATA, DID_OFFSET, (unsigned char *)did, DID_LEN) == 0){
		std::string str(did);
		flock(fd,LOCK_UN);
    	close(fd);
		return str;
	} else {
		flock(fd,LOCK_UN);
    	close(fd);
		return "";
	}
#endif
}

std::string getPSK()
{
#ifdef WIN32
	return std::string("XXQKbV51INfatj3f");
#else
	const char* lock_file_path = "/tmp/efuse.lock";
    int times = 0;
    int fd = open(lock_file_path,O_RDWR | O_CREAT);
	if(fd < 0) {
		return "";
	}

    while(0 != flock(fd,LOCK_EX | LOCK_NB ))
    {
		if(++times > 10) {
			flock(fd,LOCK_UN);
    		close(fd);
			return "";
		}
        usleep(100 * 1000);
    }

	char psk[PSK_LEN + 1] = { 0 };
	if(readEfuse(USER_DATA, PSK_OFFSET, (unsigned char *)psk, PSK_LEN) == 0){
		std::string str(psk);
		flock(fd,LOCK_UN);
    	close(fd);
		return str;
	} else {
		flock(fd,LOCK_UN);
    	close(fd);
		return "";
	}
#endif
}

std::string getMAC()
{
#ifdef WIN32
	return std::string("7CC294FF3A2A");
#else
	char* get_mac = getFWParaConfig("factory","mac");
	if (get_mac != nullptr) {
		int len = strlen(get_mac);
		char *mac = (char*)malloc(len + 1);
		int j = 0;
		for (int i = 0; i < len; i++) {
			if (get_mac[i] != ':') {
				mac[j++] = get_mac[i];
			}
		}
		mac[j] = '\0';
		std::string str(mac);
		free(mac);
		return str;
	} else {
		return "";
	}
#endif
}

int setDID(std::string strDid)
{
	return writeEfuse(SCB_DATA, DID_OFFSET, (unsigned char *)strDid.c_str(), DID_LEN, 1);
}

int setPSK(std::string strPsk)
{
	return writeEfuse(USER_DATA, PSK_OFFSET, (unsigned char *)strPsk.c_str(), PSK_LEN, 1);
}

int setMAC(std::string strMac)
{

	return writeEfuse(SCB_DATA, MAC_OFFSET, (unsigned char *)strMac.c_str(), MAC_LEN, 1);
}

std::string getMACWithColon()
{
#ifdef WIN32
	return std::string("7C:C2:94:FF:3A:2A");
#else
	char* get_mac = getFWParaConfig("factory","mac");
	if (get_mac != nullptr) {
		std::string str(get_mac);
		return str;
	} else {
		return "";
	}
#endif
}

int getRandomDigit(int iNum)
{
#ifdef WIN32
	int iDataLen = 0;
	if (iNum > 0)
	{
		for (int i = 0; i < iNum; i++)
		{
			iDataLen = iDataLen * 10;
		}
	}
	else
	{
		return 1;
	}

	int iConTime = time(NULL);
	srand(iConTime);
	return rand() % iNum;
#else
	FILE *fp = NULL;
	if (iNum > 64)
		return 1;

	char random[65] = { 0 };
   	char tmp[100] = {0};
   	fp = fopen("/dev/urandom", "r");
   	for (int i=0; i<iNum; i++)
   	{
   		fread(tmp, sizeof(tmp), 1, fp);
		for(int j=0; j<sizeof(tmp); j++)
		{
			if(tmp[j] >= '0' && tmp[j] <= '9')
			{
				random[i] = tmp[j];
				break;
			}
		}
   	}
   	fclose(fp);
	
	int ret = atoi(random);
	return ret;
#endif
}
