#ifndef __SPEAKER_SOURCE_MODULE_H__
#define __SPEAKER_SOURCE_MODULE_H__
#include "module.h"
#include "speaker_source_channel.h"
#include "media_interface.h"
#include "speaker_source_input_server.h"

namespace maix {

	class MAIX_EXPORT CSpeakerSourceModule : public CModule
	{
	public:
		CSpeakerSourceModule(std::string strGUID, std::string strName);
		~CSpeakerSourceModule();

		mxbool init();
		mxbool unInit();
		mxbool addInterface(std::string strName,
			std::shared_ptr<CMediaInterface> mediaInterface);
		mxbool delInterface(std::string strName);

		mxbool addInterfaceChannel(std::string strName,
			std::shared_ptr<CSpeakerSourceChannel> channel);
		mxbool delInterfaceChannel(std::string strName);

		mxbool initServer();
		mxbool addSpeakerChannel(std::string strName,
			CSpeakerSourceInputServer* objSpeakerSourceInputServer);
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		std::string open(std::string strParam);
		std::string close(std::string strParam);
		std::string config(std::string strParam);
		std::string reset(std::string strParam);

		std::string procResult(std::string code, 
			std::string strMsg, std::string strErr);
        CSpeakerSourceInputServer* getSpeakerSourceInputServer();
	private:

        std::string speakerSourceUninit();

		std::mutex m_mutexInterfaces;
		std::map<std::string, std::shared_ptr<CMediaInterface>> m_mapInterfaces;

		std::mutex m_mutexMediaInterfaceChn;
		std::map<std::string, std::thread> m_mapSpeakerInterfacesChnProc;
		std::map<std::string, std::shared_ptr<CSpeakerSourceChannel>> 
			m_mapSpeakerInterfacesChn;

		std::mutex m_mutexSpeakerInputServer;
		std::map<std::string, CSpeakerSourceInputServer*> 
			m_mapSpeakerInputServer;
	};
}
#endif //__SPEAKER_SOURCE_MODULE_H__
