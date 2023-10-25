#ifndef __SPEAKER_SOURCE_INPUT_SERVER_H__
#define __SPEAKER_SOURCE_INPUT_SERVER_H__
#include "module.h"
#include "com_proxy_base.h"
#include "media_frame_packet.h"
#include "b_queue.h"

namespace maix {
	class CSpeakerSourceInputServer : public CComProxyBase
	{
	public:
		CSpeakerSourceInputServer(CModule * module);
		~CSpeakerSourceInputServer();
		mxbool init(std::string strChannelName,
			int iType,
			std::string strIP,
			int iPort,
			std::string strUnix,
			int iLen);
		mxbool unInit();
		void frameProc(RCF::ByteBuffer byteBuffer);

		bool pushFrameData(std::shared_ptr<CMediaFramePacket> &packet);
		void popFrameData(std::shared_ptr<CMediaFramePacket> &packet);

		std::string open(std::string strParam);
		std::string close(std::string strParam);
		std::string config(std::string strParam);
		std::string reset(std::string strParam);

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

	private:
		CModule * m_module;
		int m_iLostFrameNum;
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;

		std::string m_strModuleGUID;
		std::string m_strModuleName;

		std::string m_strChannelName;
		int m_iType;
		std::string m_strIP;
		int m_iPort;
		std::string m_strUnix;
		int m_iLen;

		std::string m_strKey;
		unsigned char* m_pcDecryptData;
		int m_iDecryptDataLen;
	};
}
#endif //__SPEAKER_SOURCE_INPUT_SERVER_H__
