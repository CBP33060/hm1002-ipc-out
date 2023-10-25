#ifndef __LOCAL_STORAGE_MODULE_H__
#define __LOCAL_STORAGE_MODULE_H__
#include "module.h"
#include "video_source_input_server.h"
#include "video_manage_channel.h"
#include "video_manage_channel_session.h"
#include "audio_source_input_server.h"
#include "audio_manage_channel.h"
#include "audio_manage_channel_session.h"
#include "off_line_event_proc.h"
#include "upload_offline_media_file.h"

namespace maix {
	class MAIX_EXPORT CLocalStorageModule : public CModule
	{
	public:
		CLocalStorageModule(std::string strGUID, std::string strName);
		~CLocalStorageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
		mxbool initLowPower();
		mxbool initdOffLineEvent();
		mxbool initUploadOfflineMediaFile();
		void   lowPowerRun();
		std::string lowPowerProc();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		mxbool addVideoChannel(std::string strName,
			CVideoSourceInputServer* objVideoSourceInputServer);
		mxbool addAudioChannel(std::string strName,
			CAudioSourceInputServer* objAudioSourceInputServer);

		std::string enterLowPower(std::string strParam);
		std::string eventOccurrence(std::string strParam);
		std::string getMediaList(std::string strParam);
		std::string getMediaFileStart(std::string strParam);
		std::string getMediaFileStop(std::string strParam);
		mxbool stopRecord();

		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);

		bool pushFrameDataToEvent(
			std::shared_ptr<CMediaFramePacket> &packet);
	private:
		std::mutex m_mutexVideoChn;
		std::map<std::string, std::thread> m_mapVideoChnProc;
		std::map<std::string, 
			std::shared_ptr<CVideoManageChannel>> m_mapVideoChn;
		std::map<std::string, std::thread> m_mapVideoChnSessionProc;
		std::map<std::string, 
			std::shared_ptr<CVideoManageChannelSession>> m_mapVideoChnSession;

		std::mutex m_mutexAudioChn;
		std::map<std::string, std::thread> m_mapAudioChnProc;
		std::map<std::string, std::thread> m_mapAudioChnSessionProc;
		std::map<std::string,
			std::shared_ptr<CAudioManageChannelSession>> m_mapAudioChnSession;
		std::map<std::string,
			std::shared_ptr<CAudioManageChannel>> m_mapAudioChn;

		std::shared_ptr<COffLineEventProc> m_offLineEventProc;
		std::thread m_threadOffLineEvent;

		std::thread m_threadEnterLowPower;
		std::string m_strLowPowerGUID;
		std::string m_strLowPowerServer;
		std::mutex m_mutexLowPower;
		std::condition_variable m_conditionLowPower;

		std::shared_ptr<CUploadOfflineMediaFile> m_uploadOfflineMediaFile;
		std::thread m_threadUploadOfflineMediaFile;
	};
}
#endif //__LOCAL_STORAGE_MODULE_H__
