#ifndef __O_LOCK_FILE_STREAM_H__
#define __O_LOCK_FILE_STREAM_H__

#include <string>
#include <fstream>
#include<sys/file.h>

#include "ioLockFileStream.h"

namespace maix {
	class oLockFileStream : public ioLockFileStream
	{
	public:
		oLockFileStream(const std::string& filename);
		~oLockFileStream() = default;
		int read(void *buf, int size) = delete;
	};
}

#endif // oLockFileStream_H
