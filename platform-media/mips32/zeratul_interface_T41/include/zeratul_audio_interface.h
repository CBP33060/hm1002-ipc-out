#ifndef __ZERATUL_AUDIO_INTERFACE_H__
#define __ZERATUL_AUDIO_INTERFACE_H__

#include "media_interface.h"
#include <imp/imp_audio.h>

#define TAG "mxZeratulAudioInterface"

using namespace maix;
class CZeratulAudioInterface : public CMediaInterface
{
public:
	CZeratulAudioInterface(std::string strName);
	~CZeratulAudioInterface();
	mxbool init();
	mxbool unInit();
	unsigned char* readFrame(int chnNum, int *size);
	unsigned char* readFrame(int chnNum, int *size, int *frameType, int64_t *timestamp, int *frameSeq);
	int getChnNum();

	mxbool loadConfig(std::string strPath);
	template<class T>
	mxbool getConfig(std::string strSection,
		std::string strKey, T& value);

	template<class T>
	mxbool setConfig(std::string strSection,
		std::string strKey, T& value);

	mxbool saveConfig();
	int getChnNum(int iNum);
	std::string getChnName(int iNum);
	int getChnSN(int iNum);
	E_P_TYPE getPacketType(int iNum);

	int64_t getCurrentTime();

private:
	std::string m_strConfigPath;
	std::mutex m_mutexConfig;
#ifdef _CRYPTO_ENABLE
	unsigned char m_key[32];
#endif
#ifdef _INI_CONFIG
	INI::CINIFile m_configs;
#endif
    mxbool m_bInit;
    int m_iDevID = 1;
    int m_iChnID = 0;
    int m_iChnVol = 70;
    int m_iAogain = 10;
	uint8_t m_frameBuff[2048];
};

#endif //__ZERATUL_AUDIO_INTERFACE_H__

template<class T>
inline mxbool CZeratulAudioInterface::getConfig(std::string strSection, std::string strKey, T & value)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
	try
	{
		value = m_configs[strSection][strKey].as<T>();
	}
	catch (std::invalid_argument &ia)
	{
		return mxfalse;
	}
#endif
	return mxtrue;
}

template<class T>
inline mxbool CZeratulAudioInterface::setConfig(std::string strSection, std::string strKey, T & value)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _INI_CONFIG
	try
	{
		m_configs[strSection][strKey] = value;
	}
	catch (std::invalid_argument &ia)
	{
		return mxfalse;
	}
#endif
	return mxtrue;
}
