#ifndef __VIDEO_MANAGE_CHANNEL_H__
#define __VIDEO_MANAGE_CHANNEL_H__
#include "module.h"
#include "video_source_input_server.h"
#include <list>
#include <map>

namespace maix {

	class MAIX_EXPORT CVideoManageChannel
	{
	public:
		CVideoManageChannel(CModule * module, std::string strName,
			CVideoSourceInputServer* objVideoSourceInputServer);
		~CVideoManageChannel();

		mxbool init();
		mxbool unInit();
		mxbool open(std::string strGUID, 
			std::string strServerName, std::string strKey);
		mxbool close(std::string strGUID, std::string strServerName);
		mxbool config(std::string strConfig);
		mxbool reset();

		mxbool getJpegFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		int64_t getCurrentTime();
		void run();
	private:
		CModule * m_module;
		std::string m_strName;
		CVideoSourceInputServer* m_objVideoSourceInputServer;

		std::mutex m_mutexClient;
		std::map<std::string,
			std::map<std::string, std::string>> m_mapClient;

		unsigned char* m_pcEncryptData;
		int m_iEncryptDataLen;
		mxbool m_jpeg;
	};
}
#endif //__VIDEO_MANAGE_CHANNEL_H__
