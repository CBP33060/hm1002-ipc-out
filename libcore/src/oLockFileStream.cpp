#include "oLockFileStream.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <string.h>

#include "log_mx.h"

namespace maix {
	oLockFileStream::oLockFileStream(const std::string& filename)
	: ioLockFileStream(F_WRLCK)
	{
		m_iFd = ::open(filename.c_str(), O_RDWR|O_CREAT, 0666);
		if (m_iFd < 0)
		{
			return;
		}

		try
		{
			lock();
		} catch(...)
		{
			logPrint(MX_LOG_ERROR, "[oLockFileStream] lock file failed, path: %s\n", filename.c_str());
			return;
		}

		m_status = true;
		m_bLock = true;
	}
}