#include "log_mx.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/socket.h>

#define MCU_LOG_PRINT_FLAG                  "-muc_log-"     //mcu打印log的标识

namespace maix {
	T_LogConfig g_tLogConfig;

    static int g_iClientFd = -1;
    static int g_iConnectState = -1;
	static mxbool g_bBackFile = mxfalse;

	static void fileWrite(char *pBuf, int iLen)
	{
        if(g_tLogConfig.m_strUnix.empty())
        {
            printf("unix path err\n");
            return;
        }

        if(g_iClientFd < 0)
        {
            g_iClientFd = socket(AF_UNIX,SOCK_DGRAM,0);
            if(g_iClientFd < 0)
            {
                // printf("socket open err [%s] %s \n",g_tLogConfig.m_strName.c_str(),strerror(errno));
                return;
            }
        }

        if(g_iConnectState < 0)
        {
            sockaddr_un clientAddr;
            memset(&clientAddr,0,sizeof(sockaddr_un));
            clientAddr.sun_family = AF_UNIX;
            strcpy(clientAddr.sun_path,g_tLogConfig.m_strUnix.c_str());
            g_iConnectState = connect(g_iClientFd,(sockaddr * )&clientAddr,sizeof(clientAddr));
            if(g_iConnectState < 0)
            {
                // printf("connect err [%s] %s \n",g_tLogConfig.m_strName.c_str(),strerror(errno));
                return;
            }
        }

        int res = write(g_iClientFd,pBuf,iLen);
        if(res < 0)
        {
            // printf("write err [%s] %s \n",g_tLogConfig.m_strName.c_str(),strerror(errno));
            close(g_iClientFd);
            g_iClientFd = -1;
            g_iConnectState = -1;
            return;
        }

		if (g_bBackFile)
		{

		}
	}

	static void remoteWrite(char *pBuf, int iLen)
	{

	}

	std::string getLogTime(void)
	{
		std::string str;
		char    szTime[120];
#ifdef _WIN32
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		sprintf(szTime, "%d %02d-%02d %02d:%02d:%02d.%03d",
			sysTime.wYear, sysTime.wMonth, sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
			sysTime.wMilliseconds);
		str = szTime;
#else
		struct  tm      *ptm;
		struct  timeb   stTimeb;
		ftime(&stTimeb);
		ptm = localtime(&stTimeb.time);
		sprintf(szTime, "%d %02d-%02d %02d:%02d:%02d.%03d",
			ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, stTimeb.millitm);
		str = szTime;
#endif

		return str;
	}

	void logInit(T_LogConfig tLogConfig)
	{
		g_tLogConfig.m_strName = tLogConfig.m_strName;
		g_tLogConfig.m_eLevel = tLogConfig.m_eLevel;
		g_tLogConfig.m_eType = tLogConfig.m_eType;
		g_tLogConfig.m_strFileName = tLogConfig.m_strFileName;
		g_tLogConfig.m_eNetType = tLogConfig.m_eNetType;
		g_tLogConfig.m_strIP = tLogConfig.m_strIP;
		g_tLogConfig.m_iPort = tLogConfig.m_iPort;
        g_tLogConfig.m_strUnix = tLogConfig.m_strUnix;
	}

	void logBackFile()
	{
		g_bBackFile = true;
	}

	void logPrint(E_MX_LOG_LEVEL eLevel, const char *format, ...)
	{
		if (eLevel <= g_tLogConfig.m_eLevel)
		{
			char buf[1024] = { 0 };

            snprintf(buf, sizeof(buf), "%s [%s]", getLogTime().c_str(),g_tLogConfig.m_strName.c_str());

            va_list arg;
            va_start(arg, format);
            vsnprintf(buf+strlen(buf), sizeof(buf) - strlen(buf), format, arg);
            va_end(arg);

			switch (g_tLogConfig.m_eType)
			{
			case MX_LOG_CONSOLE:
			{
				// printf("%s [%s] %s\n", getLogTime().c_str(), 
				// 	g_tLogConfig.m_strName.c_str(), buf);
                printf("%s \n",buf);
				fflush(stdout);

                // fileWrite(buf, strlen(buf));
				break;
			}
			case MX_LOG_LOCAL:
			{
				fileWrite(buf, strlen(buf));
				break;
			}
			case MX_LOG_REMOTE:
			{
				remoteWrite(buf, strlen(buf));
				break;
			}
			case MX_LOG_CONSOLE_AND_LOCAL:
			{
				// printf("%s [%s] %s\n", getLogTime().c_str(), 
				// 	g_tLogConfig.m_strName.c_str(), buf);
                printf("%s \n",buf);
				fflush(stdout);

                fileWrite(buf, strlen(buf));
				break;
			}
			default:
				break;
			}
		}
	}

    void mcu_logPrint(const char *format, ...)
    {
        char buf[1024] = { 0 };

        snprintf(buf, sizeof(buf), "%s", MCU_LOG_PRINT_FLAG);

        va_list arg;
        va_start(arg, format);
        vsnprintf(buf+strlen(buf), sizeof(buf) - strlen(buf), format, arg);
        va_end(arg);

        fileWrite(buf, strlen(buf));
    }

}