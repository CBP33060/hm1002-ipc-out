#ifndef __SPEAKER_SOURCE_INPUT_SERVER_H__
#define __SPEAKER_SOURCE_INPUT_SERVER_H__
#include "module.h"
#include "com_proxy_base.h"
#include "media_frame_packet.h"
#include "b_queue.h"
#include "media_interface.h"

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
		mxbool addInterface(std::shared_ptr<CMediaInterface> objInterface);
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

        unsigned long long getCurrentTime();
        void runAsSpeakerControl();

		CModule * m_module;
		int m_iLostFrameNum;
		CBQueue<std::shared_ptr<CMediaFramePacket>> m_objPacketQueue;
		std::shared_ptr<CMediaInterface> m_objInterface;

		std::string m_strModuleGUID;
		std::string m_strModuleName;

		std::string m_strChannelName;
		int m_iType;
		std::string m_strIP;
		int m_iPort;
		std::string m_strUnix;
		int m_iLen;

        E_VOICE_SOURCE m_objAudioRes;
        E_VOICE_PLAY_LEVEL m_objAudioLevel;
        int64_t m_llAudioLastTime;       //记录本次播报的时间点，用于下帧数据来的时候判断是否超过200ms，超过200ms，audiores和audiolevel重新计算

        std::thread m_threadSpeakerControl;
        std::mutex m_mutexSpeakerControl;
        std::condition_variable m_conditionSpeakerControl;
        mxbool m_bSpeakerInit;
	};
}
#endif //__SPEAKER_SOURCE_INPUT_SERVER_H__
