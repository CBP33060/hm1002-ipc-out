#ifndef __I_MEDIA_INTERFACE_H__
#define __I_MEDIA_INTERFACE_H__
#include "global_export.h"
#include "type_def.h"

namespace maix {
	class CIMediaInterface
	{
	public:
		virtual mxbool init() = 0;
		virtual mxbool unInit() = 0;
		virtual mxbool initChannel(int chnNum) = 0;
		virtual mxbool unInitChannel(int chnNum) = 0;
		virtual mxbool getIDRFrame(int chnNum) = 0;
		virtual mxbool startRcvFrame(int chnNum) = 0;
		virtual unsigned char *readFrame(int chnNum, int *size) = 0;
		virtual unsigned char *readFrame(int chnNum, int *size, int *frameType, long long *timestamp, int *frameSeq) = 0;
		virtual mxbool writeFrame(int chnNum, unsigned char* data, int size) = 0;
	};
}
#endif //__I_MEDIA_INTERFACE_H__
