#ifndef __I_LOCK_FILE_STREAM_H__
#define __I_LOCK_FILE_STREAM_H__

#include <string>
#include <fstream>
#include<sys/file.h>

#include "ioLockFileStream.h"

namespace maix {

	class iLockFileStream : public ioLockFileStream
	{
	public:
		iLockFileStream(const std::string& filename);
		~iLockFileStream() = default;
		int write(const void *buf, int size) = delete;
	};
}


#endif // iLockFileStream_H
