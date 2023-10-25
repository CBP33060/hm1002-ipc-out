#ifndef __IO_LOCK_FILE_STREAM_H__
#define __IO_LOCK_FILE_STREAM_H__

#include <string>
#include <fstream>
#include <sys/file.h>
#include <stdint.h>

#define ERR_FD -1

namespace maix {
	class ioLockFileStream
	{
	public:
		explicit ioLockFileStream(short int l_type);
		virtual ~ioLockFileStream();

		bool is_open() const;
		int seekg(int begin, int offet);
		int tellg();
		bool good() const;
		int read(void *buf, int size);
		int write(const void *buf, int size);
		void close();
		void flush();
		void lock();
		void unlock();

	public:
		const int end = SEEK_END;
		const int beg = SEEK_SET;

	protected:
		int m_iFd;
		bool m_status;
		bool m_bLock;
		short int m_ltype;
	};
}


#endif // iLockFileStream_H
