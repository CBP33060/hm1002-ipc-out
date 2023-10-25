#include "iLockFileStream.h"
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <string.h>

#include "log_mx.h"

namespace maix {
	iLockFileStream::iLockFileStream(const std::string& filename)
	: ioLockFileStream(F_RDLCK)
	{
		if (access(filename.c_str(), F_OK) != 0)
		{
			return;
		}

		m_iFd = ::open(filename.c_str(), O_RDONLY);
		if (m_iFd < 0)
		{
			return;
		}

		try
		{
			lock();
		} catch(...)
		{
			logPrint(MX_LOG_ERROR, "[iLockFileStream] lock file failed, path: %s\n", filename.c_str());
			return;
		}

		m_status = true;
		m_bLock = true;
	}
}
