#ifndef __MEDIA_INTERFACE_H__
#define __MEDIA_INTERFACE_H__
#include "i_media_interface.h"
#ifdef _INI_CONFIG
#include "ini_config.h"
#endif
#include <string>
#include <mutex>
#include <future>
#include "media_frame_packet.h"

namespace maix {
	typedef struct _T_ServerConfig
	{
		std::string strGUID;
		std::string strServer;
		std::string strKey;
	}T_ServerConfig;

	class MAIX_EXPORT CMediaInterface : public CIMediaInterface
	{
	public:
		CMediaInterface(std::string strName);
		virtual ~CMediaInterface();

		virtual mxbool init();
		virtual mxbool unInit();
		virtual mxbool initChannel(int chnNum);
		virtual mxbool unInitChannel(int chnNum);
		virtual mxbool config(std::string strConfig);
		virtual mxbool startRcvFrame(int chnNum);
		virtual mxbool getIDRFrame(int chnNum);
		virtual unsigned char *readFrame(int chnNum, int *size);
		virtual unsigned char *readFrame(int chnNum, int *size, int *frameType, int64_t *timestamp, int *frameSeq);
		mxbool writeFrame(int chnNum,
			unsigned char* data, int size);
		virtual int getChnNum();
		virtual std::string getChnName(int iNum);
		virtual int getChnSN(int iNum);
		virtual E_P_TYPE getPacketType(int iNum);
		virtual mxbool loadConfig(std::string strPath);
		template<class T>
		mxbool getConfig(std::string strSection,
			std::string strKey, T& value);

		template<class T>
		mxbool setConfig(std::string strSection,
			std::string strKey, T& value);

		virtual mxbool saveConfig();

		virtual std::shared_ptr<CMediaInterface> getInterface(
			const std::string className);

	private:
		std::string m_strName;
	};
}
#endif //__MEDIA_INTERFACE_H__