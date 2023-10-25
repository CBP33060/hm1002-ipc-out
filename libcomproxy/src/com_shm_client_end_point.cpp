#include "com_shm_client_end_point.h"
#include <sys/time.h>

namespace maix {
	CComSHMClientEndPoint::CComSHMClientEndPoint()
	{
#ifdef _WIN32
		m_hMapFile = NULL;
		m_lpBase = NULL;	
#else
		m_shmID = -1;
		m_hEventRead = NULL;
		m_hEventWrite = NULL;
#endif
		m_tSHMRingBuf = NULL;
		m_bTryNum = 0;
	}

	CComSHMClientEndPoint::~CComSHMClientEndPoint()
	{
#ifdef _WIN32
		CloseHandle(m_hEventRead);
		CloseHandle(m_hEventWrite);
		UnmapViewOfFile(m_lpBase);
		CloseHandle(m_hMapFile);
		m_hEventRead = NULL;
		m_hEventWrite = NULL;
		m_lpBase = NULL;
		m_hMapFile = NULL;	
#else
		shmdt(m_tSHMRingBuf);
		sem_close(m_hEventRead);
		sem_close(m_hEventWrite);
		sem_destroy(m_hEventRead);
		sem_destroy(m_hEventWrite);
		m_hEventRead = NULL;
		m_hEventWrite = NULL;
#endif
		m_tSHMRingBuf = NULL;
	}

	bool CComSHMClientEndPoint::init(T_COM_PROXY_CLIENT_CONFIG &config)
	{
#ifdef _WIN32
		std::string::size_type pos = config.m_strUnix.find("pipe");
		std::string strShareName = config.m_strUnix.substr(pos + 5,
			config.m_strUnix.length() - pos - 5);

		HANDLE m_hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,
			NULL, 
			strShareName.c_str()
			);

		if (m_hMapFile == NULL)
			return false;

		m_lpBase = MapViewOfFile(
			m_hMapFile,           
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
			);
		
		if (m_lpBase == NULL)
		{
			CloseHandle(m_hMapFile);
			m_hMapFile = NULL;
			return false;
		}
			
		m_tSHMRingBuf = (T_SHMRingBuf *)m_lpBase;

		if (m_tSHMRingBuf == NULL)
			return false;

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
		m_strName =  config.m_strUnix;
		std::string strName;
		strName = config.m_strUnix + std::string("_shm");
		if(access(strName.c_str(),F_OK) != 0)
        {
            int fd;
            fd = creat(strName.c_str(),S_IRWXU | S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
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
			shmctl(m_shmID, IPC_RMID, 0);
			return false;
		}
	
		std::string strRead = config.m_strUnix + std::string("_event_read");
		std::replace(strRead.begin() + 2, strRead.end(), '/', '_');
		m_hEventRead = sem_open(strRead.c_str(),  O_RDWR, 0666, 0);
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
		
		std::string strWrite = config.m_strUnix + std::string("_event_write");
		std::replace(strWrite.begin() + 2, strWrite.end(), '/', '_');
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
		return true;
#endif
	}

	std::string CComSHMClientEndPoint::output(unsigned char * pcData, int iLen)
	{
		if (m_tSHMRingBuf && m_hEventWrite && m_hEventRead)
		{
#ifdef _WIN32
			DWORD dw = WaitForSingleObject(m_hEventWrite, 200);
			if (WAIT_TIMEOUT == dw || WAIT_FAILED == dw)
			{
				m_bTryNum++;
				if (m_bTryNum == 5)
				{
					return std::string("disconnect");
				}
			}
			else
			{
				m_bTryNum = 0;
				m_tSHMRingBuf->iDataLen = iLen;
				memcpy((void *)m_tSHMRingBuf->pDataBuf,pcData, iLen);
				SetEvent(m_hEventRead);
				ResetEvent(m_hEventWrite);
			}
#else
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_nsec += 10 * 1000 * 1000;
			
			int ret = sem_timedwait(m_hEventWrite, &ts);
			if (ret == -1)
			{
				m_bTryNum++;
				if (m_bTryNum == 5)
				{
					return std::string("disconnect");
				}
			}
			else
			{
				m_bTryNum = 0;
				m_tSHMRingBuf->iDataLen = iLen;
				memcpy((void *)m_tSHMRingBuf->pDataBuf, pcData, iLen);
				sem_post(m_hEventRead);
			}
			

			return std::string("");
			
#endif
			
		}
		else
		{
			return std::string("disconnect");
		}

		return std::string();
	}

	int CComSHMClientEndPoint::getClientType()
	{
		return 0;
	}

}
