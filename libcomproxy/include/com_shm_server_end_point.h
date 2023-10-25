#ifndef __COM_SHM_SERVER_END_POINT_H__
#define __COM_SHM_SERVER_END_POINT_H__
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
#include <pthread.h>
#include <fcntl.h>          
#include <sys/stat.h>    
#include <semaphore.h>
#endif
#include "com_proxy_base.h"
#include "com_proxy_server_end_point.h"

namespace maix {
	class MAIX_EXPORT CComSHMSeverEndPoint
	{
	public:
		CComSHMSeverEndPoint();
		virtual ~CComSHMSeverEndPoint();

		bool init(T_COM_PROXY_SERVER_CONFIG &config, CComProxyBase * handle);
		bool start();
		bool stop();

	public:
		bool m_bRun;
		T_SHMRingBuf *m_tSHMRingBuf;
		CComProxyBase *m_comProxyHandle;
#ifdef _WIN32
		HANDLE m_hEventRead;
		HANDLE m_hEventWrite;
#else
		sem_t *m_hEventRead;
		sem_t *m_hEventWrite;
#endif
	private:
#ifdef _WIN32
		HANDLE m_hMapFile;
		LPVOID m_lpBase;
		HANDLE m_threadHandle;
#else
		int m_shmID;
		key_t m_shmKey;
		pthread_t m_threadHandle;
#endif
		
	};
}
#endif //__COM_SHM_SERVER_END_POINT_H__
