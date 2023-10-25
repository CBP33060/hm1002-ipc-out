#include "ioLockFileStream.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <string.h>

namespace maix {
	ioLockFileStream::ioLockFileStream(short int l_type)
	: m_iFd(ERR_FD), m_status(false), m_bLock(false), m_ltype(l_type)
	{	  
	}

	ioLockFileStream::~ioLockFileStream() 
	{
		close();
	}

	bool ioLockFileStream::is_open() const
	{
		return m_iFd != ERR_FD;
	}

	bool ioLockFileStream::good() const
	{
		return m_status;
	}

	void ioLockFileStream::close()
	{
		if (m_bLock)
		{
			try
			{
				unlock();
				m_bLock = false;
			}
			catch (...){}
		}

		if (is_open())
		{
			::close(m_iFd);
			m_iFd = ERR_FD;
		}

	}

	int ioLockFileStream::seekg(int offet, int where)
	{
		if (!is_open())
		{
			return 0;
		}

		return ::lseek(m_iFd, offet, where);
	}

	int ioLockFileStream::tellg()
	{
		return seekg(0, SEEK_CUR);
	}

	void ioLockFileStream::flush()
	{
	}

	int ioLockFileStream::read(void *buf, int size)
	{
		if (!is_open())
		{
			return -1;
		}

		int ret = ::read(m_iFd, buf, size);
		if (ret == -1)
		{
			perror("Read failed");
			m_status = false;
		}

		return ret;
	}

	int ioLockFileStream::write(const void *buf, int size)
	{
		if (!is_open())
		{
			return -1;
		}

		int ret = ::write(m_iFd, buf, size);
		if (ret == -1)
		{
			perror("Write failed");
			m_status = false;
		}

		return ret;
	}

	void ioLockFileStream::lock()
	{
		if (-1 == m_iFd)
		{
			throw std::runtime_error("cannot lock");
		}

		if (m_bLock)
		{
			throw std::runtime_error("repeat lock");
		}

		/// 阻塞式等待文件锁，可能会耗时较久
		struct flock  flck;
		flck.l_type = m_ltype;
		flck.l_whence = SEEK_SET;
		flck.l_start = 0;
		flck.l_len = 0;

		if (fcntl(m_iFd, F_SETLKW, &flck) == -1) 
		{
			throw std::runtime_error("can not get file lock");
		}

		m_bLock = true;
	}

	void ioLockFileStream::unlock()
	{
		if (-1 == m_iFd)
		{
			throw std::runtime_error("cannot unlock");
		}


		if (!m_bLock)
		{
			throw std::runtime_error("repeat unlock");
		}

		struct flock fileLock;
		fileLock.l_type = F_UNLCK;
		fileLock.l_whence = SEEK_SET;
		fileLock.l_start = 0;
		fileLock.l_len = 0;

		if (fcntl(m_iFd, F_SETLK, &fileLock) < 0)
		{
			throw std::runtime_error("can not get file lock");
		}

		m_bLock = false;
	}
}
