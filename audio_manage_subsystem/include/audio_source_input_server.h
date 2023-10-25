#ifndef __AUDIO_SOURCE_INPUT_SERVER_H__
#define __AUDIO_SOURCE_INPUT_SERVER_H__
#include "com_proxy_base.h"
#include "media_frame_packet.h"
#include "b_queue.h"
#include "module.h"

namespace maix {
	class CAudioSourceInputServer : public CComProxyBase
	{
	public:
		CAudioSourceInputServer(CModule * module);
		~CAudioSourceInputServer();
		mxbool init(std::string strChannelName,
			int iType,
			std::string strIP,
			int iPort,
			std::string strUnix,
			int iLen);
		mxbool unInit();

		mxbool initAudioSourceRemoteEventServer();

		mxbool open();
		mxbool close();
		mxbool config(std::string strConfig);
		mxbool reset();

		void updatePacketTime();
		mxbool noPacket();

		void frameProc(RCF::ByteBuffer byteBuffer);
		void frameProc(unsigned char* byteBuffer, int len);
		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

		int64_t getCurrentTime();

		mxbool parseResult(std::string &strInput,
			std::string &code, std::string &strMsg,
			std::string &strErr);

	private:
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		CModule * m_module;
		int m_iLostFrameNum;
		std::string m_strModuleGUID;
		std::string m_strModuleName;

		std::string m_strAudioSourceGUID;
		std::string m_strAudioSourceRemoteEventServer;

		int  m_iPacketReceiveTime;

		std::string m_strChannelName;
		int m_iType;
		std::string m_strIP;
		int m_iPort;
		std::string m_strUnix;
		int m_iLen;
	};
}
#endif //__AUDIO_SOURCE_INPUT_SERVER_H__
