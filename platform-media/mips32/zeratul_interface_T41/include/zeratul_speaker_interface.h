#ifndef __ZERATUL_SPEAKER_INTERFACE_H__
#define __ZERATUL_SPEAKER_INTERFACE_H__

#include "media_interface.h"

using namespace maix;
class CZeratulSpeakerInterface : public CMediaInterface
{
public:
    CZeratulSpeakerInterface(std::string strName);
    ~CZeratulSpeakerInterface();
    mxbool init();
    mxbool unInit();
    mxbool writeFrame(int chnNum,
        unsigned char* data, int size);
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
    int getChnSn(int iNum);
    E_P_TYPE getPacketType(int iNum);

    int64_t getCurrentTime();

    int setVol(int audioVol);
    int getVol(int *audioVol);
    int sendFrame(int size,char *dataBuf);
    int queryChnStatus();
    int pauseChn();
    int resumeChn();
    int clearChnBuf();
    int disableVolume();
    mxbool enableState();

private:
    std::string m_strConfigPath;
    std::mutex m_mutexConfig;
#ifdef _CRYPTO_ENABLE
	unsigned char m_key[32];
#endif
#ifdef _INI_CONFIG
    INI::CINIFile m_configs;
#endif

    mutable std::mutex m_initMutex;
    mxbool m_bInit;
    int m_iDevID = 0;
    int m_iChnID = 0;
    int m_iChnVol = 60;
    int m_iAogain = 25;

};

#endif //__ZERATUL_SPEAKER_INTERFACE_H__

template<class T>
inline mxbool CZeratulSpeakerInterface::getConfig(std::string strSection, std::string strKey, T & value)
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
inline mxbool CZeratulSpeakerInterface::setConfig(std::string strSection, std::string strKey, T & value)
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
