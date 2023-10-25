#ifndef __UPLOAD_OFFLINE_MEDIA_FILE_H__
#define __UPLOAD_OFFLINE_MEDIA_FILE_H__
#include "module.h"
#include <list>
#include <map>
#include <mp4v2/mp4v2.h>

namespace maix {

	class MAIX_EXPORT CUploadOfflineMediaFile
	{
	public:
		CUploadOfflineMediaFile(CModule * module);
		~CUploadOfflineMediaFile();

		mxbool init();
		mxbool unInit();
		mxbool open(std::string strGUID,
			std::string strServerName, std::string strKey,
			std::string strFileName);
		mxbool close(std::string strGUID, std::string strServerName);

		void run();
	private:
		CModule * m_module;
		std::string m_strName;

		std::mutex m_mutexClient;
		std::map<std::string,
			std::map<std::string, std::string >> m_mapClient;

		unsigned char* m_pcEncryptData;
		int m_iEncryptDataLen;

		MP4FileHandle m_pMP4FileHandle;
		std::string m_strFileName;
	};
}
#endif //__UPLOAD_OFFLINE_MEDIA_FILE_H__
