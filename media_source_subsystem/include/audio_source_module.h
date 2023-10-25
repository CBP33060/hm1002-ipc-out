#ifndef __AUDIO_SOURCE_MODULE_H__
#define __AUDIO_SOURCE_MODULE_H__
#include "module.h"
#include "audio_source_input.h"
#include "audio_source_channel.h"
#include "media_interface.h"

namespace maix {

	class MAIX_EXPORT CAudioSourceModule : public CModule
	{
	public:
		CAudioSourceModule(std::string strGUID, std::string strName);
		~CAudioSourceModule();

		mxbool init();
		mxbool unInit();
		mxbool addInterface(std::string strName,
			std::shared_ptr<CMediaInterface> mediaInterface);
		mxbool delInterface(std::string strName);

		mxbool addInterfaceChannel(std::string strName,
			std::shared_ptr<CAudioSourceInput> input);
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
	private:
		std::mutex m_mutexMediaInterfaces;
		std::map<std::string, std::shared_ptr<CMediaInterface>> m_mapMediaInterfaces;

		std::mutex m_mutexInterfaceChannel;
		std::map<std::string, std::thread> m_mapInterfaceInputProc;
		std::map<std::string, std::shared_ptr<CAudioSourceInput>> m_mapInterfaceInput;
		std::map<std::string, std::thread> m_mapInterfaceChannelProc;
		std::map<std::string, std::shared_ptr<CAudioSourceChannel>> m_mapInterfaceChannel;
	};
}
#endif //__AUDIO_SOURCE_MODULE_H__
