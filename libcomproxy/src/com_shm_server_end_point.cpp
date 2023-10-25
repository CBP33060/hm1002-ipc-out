#include "com_shm_server_end_point.h"

namespace maix {	
#ifdef _WIN32
	DWORD WINAPI SHMThreadFun(LPVOID pM)
	{
		CComSHMSeverEndPoint *objComSHMSeverEndPoint = 
			(CComSHMSeverEndPoint *)pM;

		if (objComSHMSeverEndPoint == NULL || 
			objComSHMSeverEndPoint->m_tSHMRingBuf == NULL)
			return -1;

		T_SHMRingBuf *tSHMRingBuf = 
			objComSHMSeverEndPoint->m_tSHMRingBuf;

		CComProxyBase *objComProxyHandle = 
			objComSHMSeverEndPoint->m_comProxyHandle;

		while (objComSHMSeverEndPoint->m_bRun)
		{
			WaitForSingleObject(objComSHMSeverEndPoint->m_hEventRead, INFINITE);

			objComProxyHandle->frameProc((unsigned char*)tSHMRingBuf->pDataBuf,
				tSHMRingBuf->iDataLen);
			ResetEvent(objComSHMSeverEndPoint->m_hEventRead);
			SetEvent(objComSHMSeverEndPoint->m_hEventWrite);
		}
		return 0;

	}
#else
	static void * SHMThreadFun(void *arg)
	{
		CComSHMSeverEndPoint *objComSHMSeverEndPoint =
			(CComSHMSeverEndPoint *)arg;

		if (objComSHMSeverEndPoint == NULL ||
			objComSHMSeverEndPoint->m_tSHMRingBuf == NULL)
			return  NULL;

		T_SHMRingBuf *tSHMRingBuf =
			objComSHMSeverEndPoint->m_tSHMRingBuf;

		CComProxyBase *objComProxyHandle =
			objComSHMSeverEndPoint->m_comProxyHandle;

		while (objComSHMSeverEndPoint->m_bRun)
		{
			sem_wait(objComSHMSeverEndPoint->m_hEventRead);
			objComProxyHandle->frameProc((unsigned char*)tSHMRingBuf->pDataBuf,
				tSHMRingBuf->iDataLen);
			sem_post(objComSHMSeverEndPoint->m_hEventWrite);
		}
		return  NULL;
	}
#endif
	CComSHMSeverEndPoint::CComSHMSeverEndPoint()
	{
#ifdef _WIN32
		m_hMapFile = NULL;
		m_lpBase = NULL;
		m_tSHMRingBuf = NULL;
		m_threadHandle = NULL;
#else
		m_tSHMRingBuf = NULL;
		m_threadHandle = 0;
#endif
		m_comProxyHandle = NULL;
		m_bRun = false;
	}

	CComSHMSeverEndPoint::~CComSHMSeverEndPoint()
	{
#ifdef _WIN32
		CloseHandle(m_hEventRead);
		CloseHandle(m_hEventWrite);
		UnmapViewOfFile(m_lpBase);
		CloseHandle(m_hMapFile);
		m_lpBase = NULL;
		m_hMapFile = NULL;
#else
		sem_close(m_hEventRead);
		sem_close(m_hEventWrite);
		sem_destroy(m_hEventRead);
		sem_destroy(m_hEventWrite);
		shmdt(m_tSHMRingBuf);
		shmctl(m_shmID, IPC_RMID, 0);
#endif
		m_tSHMRingBuf = NULL;
		m_comProxyHandle = NULL;
		m_bRun = false;
	}

	bool CComSHMSeverEndPoint::init(T_COM_PROXY_SERVER_CONFIG &config, CComProxyBase * handle)
	{
		if (handle == NULL)
			return false;

		m_comProxyHandle = handle;
#ifdef _WIN32
		std::string::size_type pos = config.m_strUnix.find("pipe");
		std::string strShareName = config.m_strUnix.substr(pos + 5,
			config.m_strUnix.length() - pos - 5);
		HANDLE m_hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(T_SHMRingBuf) + config.m_iUNIXMsgLen,
			strShareName.c_str()
			);

		if (m_hMapFile == NULL)
			return false;

		m_lpBase = MapViewOfFile(
			m_hMapFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			sizeof(T_SHMRingBuf) + config.m_iUNIXMsgLen
			);

		if (m_lpBase == NULL)
		{
			CloseHandle(m_hMapFile);
			m_hMapFile = NULL;
			return false;
		}
			
		m_tSHMRingBuf = (T_SHMRingBuf *)m_lpBase;

		std::string strRead = strShareName + std::string("EventRead");
		m_hEventRead = CreateEvent(NULL, TRUE, FALSE, strRead.c_str());
		if (nullptr == m_hEventRead)
		{
			UnmapViewOfFile(m_lpBase);
			CloseHandle(m_hMapFile);
			m_lpBase = NULL;
			m_hMapFile = NULL;
			m_tSHMRingBuf = NULL;
			return false;
		}

		std::string strWrite = strShareName + std::string("EventWrite");
		m_hEventWrite = CreateEvent(NULL, TRUE, TRUE, strWrite.c_str());
		if (nullptr == m_hEventWrite)
		{
			CloseHandle(m_hEventRead);
			UnmapViewOfFile(m_lpBase);
			CloseHandle(m_hMapFile);
			m_lpBase = NULL;
			m_hMapFile = NULL;
			m_tSHMRingBuf = NULL;
			return false;
		}

		return true;
#else
		std::string strName;
		strName = config.m_strUnix + std::string("_shm");
		if(access(strName.c_str(),F_OK) != 0)
        {
            int fd = creat(strName.c_str(),S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
            close(fd);
        }
		
		if ((m_shmKey = ftok(strName.c_str(), 12345)) == -1)
		{
			return false;
		}
		
		m_shmID = shmget(m_shmKey, sizeof(m_tSHMRingBuf) + 
			config.m_iUNIXMsgLen, IPC_CREAT | O_EXCL | 0600);
		if (m_shmID < 0)
		{
			m_shmID = shmget(m_shmKey, sizeof(m_tSHMRingBuf) + 
			config.m_iUNIXMsgLen, 0600);
			if (m_shmID < 0)
			{
				return false;
			}

		}
		
		m_tSHMRingBuf = (T_SHMRingBuf *)shmat(m_shmID, NULL, 0);
		if ((void *)m_tSHMRingBuf == (void *)-1) 
		{
			return false;
		}
		
		std::string strRead = config.m_strUnix + std::string("_event_read");
		std::replace(strRead.begin() + 2, strRead.end(), '/', '_');
		m_hEventRead = sem_open(strRead.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666, 0);
		if (m_hEventRead == SEM_FAILED)
		{
			m_hEventRead = sem_open(strRead.c_str(), O_RDWR, 0666, 0);
			if (m_hEventRead == SEM_FAILED)
			{
				shmdt(m_tSHMRingBuf);
				m_tSHMRingBuf = NULL;
				return false;
			}
			else
			{
				int val = -1;
			  	sem_getvalue(m_hEventRead, &val);
				if(val == 1)
				{
					sem_wait(m_hEventRead);
				}
				
			}
		}
		else
		{
			int val = -1;
		  	sem_getvalue(m_hEventRead, &val);
			if(val == 1)
			{
				sem_wait(m_hEventRead);
			}
					
		}

		std::string strWrite = config.m_strUnix + std::string("_event_write");
		std::replace(strWrite.begin() + 2, strWrite.end(), '/', '_');
		m_hEventWrite = sem_open(strWrite.c_str(), O_CREAT |O_EXCL| O_RDWR, 0666, 1);
		if (m_hEventWrite == SEM_FAILED)
		{

			m_hEventWrite = sem_open(strWrite.c_str(),  O_RDWR, 0666, 1);
			if (m_hEventWrite == SEM_FAILED)
			{
				sem_close(m_hEventRead);
				shmdt(m_tSHMRingBuf);
				m_tSHMRingBuf = NULL;
				return false;
			}
			else
			{
				int val = -1;
		  		sem_getvalue(m_hEventWrite, &val);
				if(val == 0)
				{
					sem_post(m_hEventWrite);
				}
			}
		}
		else
		{
			int val = -1;
		  	sem_getvalue(m_hEventWrite, &val);
			if(val == 0)
			{
				sem_post(m_hEventWrite);
			}
		}
	
		return true;
#endif
	}

	bool CComSHMSeverEndPoint::start()
	{
		m_bRun = true;
#ifdef _WIN32
		m_threadHandle = CreateThread(NULL, 0, SHMThreadFun, this, 0, NULL);

		if (m_threadHandle == NULL)
		{
			CloseHandle(m_hEventRead);
			CloseHandle(m_hEventWrite);
			UnmapViewOfFile(m_lpBase);
			CloseHandle(m_hMapFile);
			m_lpBase = NULL;
			m_hMapFile = NULL;
			m_tSHMRingBuf = NULL;
			return false;
		}

#else
	
		int ret = 0;
		ret = pthread_create(&m_threadHandle, NULL, SHMThreadFun, this);
		if (ret != 0)
		{
			sem_close(m_hEventRead);
			sem_close(m_hEventWrite);
			shmdt(m_tSHMRingBuf);
			return false;
		}
#endif
		return true;
	}

	bool CComSHMSeverEndPoint::stop()
	{
		//stop thread todo

		return true;
	}
}
