#ifndef __COM_SHM_CLIENT_END_POINT_H__
#define __COM_SHM_CLIENT_END_POINT_H__
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>          
#include <sys/stat.h>    
#include <semaphore.h>
#endif
#include "com_proxy_base.h"
#include <string>
#include "com_proxy_client_end_point.h"

namespace maix {
	class MAIX_EXPORT CComSHMClientEndPoint
	{
	public:
		CComSHMClientEndPoint();
		virtual ~CComSHMClientEndPoint();
		bool init(T_COM_PROXY_CLIENT_CONFIG &config);
		virtual std::string  output(unsigned char * pcData, int iLen);
		int getClientType();

	private:
		T_SHMRingBuf *m_tSHMRingBuf;
#ifdef _WIN32
		HANDLE m_hMapFile;
		LPVOID m_lpBase;
		HANDLE m_hEventRead;
		HANDLE m_hEventWrite;
#else
		int m_shmID;
		key_t m_shmKey;
		sem_t *m_hEventRead;
		sem_t *m_hEventWrite;
		std::string m_strName;
#endif
		int m_bTryNum;
	};
}
#endif //__COM_PROXY_UDP_CLIENT_H__
