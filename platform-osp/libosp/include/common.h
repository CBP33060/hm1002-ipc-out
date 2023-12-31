#ifndef __COMMON_H__
#define __COMMON_H__
#include "global_export.h"
#include "type_def.h"
#include <string>
#include <vector>

using namespace maix;

#ifdef _WIN32
#define ETH0 	"Intel(R) Wi-Fi 6 AX201 160MHz"
#else
#define ETH0 	"wlan0"
#endif

#define BASE_STATION_IP	 	"192.168.10.1"
#define FW_VERSION_PATH		"/etc/version"

MAIX_EXPORT mxbool bindCpu(int cpu_id);
MAIX_EXPORT mxbool getFiles(std::string path, 
	std::vector<std::string>& files, const char* sType);
MAIX_EXPORT mxbool getLocalIPByName(std::string strName, 
	std::string &strIP);
MAIX_EXPORT mxbool getGUIDData(std::string &strGUID);
MAIX_EXPORT mxbool getKey(std::string &strKey);
MAIX_EXPORT mxbool linuxPopenExecCmd(std::string &strOutData, const char * pFormat, ...);
MAIX_EXPORT int versionCompare(const std::string &oldVersion,
	const std::string &newVersion);
MAIX_EXPORT std::string getDID();
MAIX_EXPORT std::string getPSK();
MAIX_EXPORT std::string getMAC();
MAIX_EXPORT std::string getMACWithColon();
MAIX_EXPORT int setDID(std::string strDid);
MAIX_EXPORT int setPSK(std::string strPsk);
MAIX_EXPORT int setMAC(std::string strMac);
MAIX_EXPORT int getRandomDigit(int iNum);
#endif //__COMMON_H__