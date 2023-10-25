#ifndef __VIDEO_SOURCE_MODULE_H__
#define __VIDEO_SOURCE_MODULE_H__
#include "module.h"
#include "video_source_input.h"
#include "video_source_channel.h"
#include "media_interface.h"

namespace maix {

    typedef struct CHANNEL_LIVE_INFO
    {
        int bLiveStream;
        int bEventStream;
    }T_CHANNEL_LIVE_INFO;

	class MAIX_EXPORT CVideoSourceModule : public CModule
	{
	public:
		CVideoSourceModule(std::string strGUID, std::string strName);
		~CVideoSourceModule();

		mxbool init();
		mxbool unInit();
		mxbool addInterface(std::string strName,
			std::shared_ptr<CMediaInterface> mediaInterface);
		mxbool delInterface(std::string strName);

		mxbool addInterfaceChannel(std::string strName, E_P_TYPE ePacketType, 
			std::shared_ptr<CVideoSourceInput> input);
		mxbool delInterfaceChannel(std::string strName);

		mxbool initServer();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		std::string open(std::string strParam);
		std::string close(std::string strParam);
		std::string config(std::string strParam);
		std::string reset(std::string strParam);

		std::string procResult(std::string code, 
			std::string strMsg, std::string strErr);

        mxbool initConnectModule();
        mxbool initLowPower();
		void   lowPowerRun();
		std::string lowPowerProc();
        std::string enterLowPower(std::string strParam);

	private:
		std::mutex m_mutexMediaInterfaces;
		std::map<std::string, std::shared_ptr<CMediaInterface>> m_mapMediaInterfaces;

		std::mutex m_mutexInterfaceChannel;
		std::map<std::string, std::thread> m_mapInterfaceInputProc;
		std::map<std::string, std::shared_ptr<CVideoSourceInput>> m_mapInterfaceInput;
		std::map<std::string, std::thread> m_mapInterfaceChannelProc;
		std::map<std::string, std::shared_ptr<CVideoSourceChannel>> m_mapInterfaceChannel;

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

		std::mutex m_mutexLiveStreamNum;
        std::map<std::string, T_CHANNEL_LIVE_INFO> m_mapChannelLiveStream;
		int m_iLowpowerPrintCount;
	};
}
#endif //__VIDEO_SOURCE_MODULE_H__