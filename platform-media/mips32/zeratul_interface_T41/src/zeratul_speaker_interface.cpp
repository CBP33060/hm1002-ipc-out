#include "zeratul_speaker_interface.h"
#include <sys/time.h>
#include <iostream>
#include <imp/imp_audio.h>
#include <string.h>
#ifdef _CRYPTO_ENABLE
#include "crypt_api_mx.h"
#endif
#include "common.h"
#include "log_mx.h"
#include "iLockFileStream.h"
#include "oLockFileStream.h"


CZeratulSpeakerInterface::CZeratulSpeakerInterface(std::string strName)
	: CMediaInterface(strName)
{
    m_bInit = false;
}

CZeratulSpeakerInterface::~CZeratulSpeakerInterface()
{
}

mxbool CZeratulSpeakerInterface::init()
{
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;

    if(m_bInit)
    {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface CVoicePlayEntity init err : has been init !\n");
        return mxfalse;
    }
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface start init ====== !\n");
    /* Step 1: set public attribute of AO device. */
    
    IMPAudioIOAttr attr;
    attr.samplerate = AUDIO_SAMPLE_RATE_16000;
    attr.bitwidth = AUDIO_BIT_WIDTH_16;
    attr.soundmode = AUDIO_SOUND_MODE_MONO;
    attr.frmNum = 20;
    attr.numPerFrm = 640;
    attr.chnCnt = 1;
    ret = IMP_AO_SetPubAttr(m_iDevID, &attr);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface set ao %d attr err: %d !\n", m_iDevID, ret);
        return mxfalse;
    }

    memset(&attr, 0x0, sizeof(attr));
    ret = IMP_AO_GetPubAttr(m_iDevID, &attr);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface get ao %d attr err: %d !\n", m_iDevID, ret);
        return mxfalse;
    }

    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr samplerate:%d !\n", attr.samplerate);
	logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr   bitwidth:%d !\n", attr.bitwidth);
	logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr  soundmode:%d !\n", attr.soundmode);
	logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr     frmNum:%d !\n", attr.frmNum);
	logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr  numPerFrm:%d !\n", attr.numPerFrm);
	logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface Audio Out GetPubAttr     chnCnt:%d !\n", attr.chnCnt);

    /* Step 2: enable AO device. */
    ret = IMP_AO_Enable(m_iDevID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface enable ao %d err %d !\n",m_iDevID,ret);
        return mxfalse;
    }

    /* Step 3: enable AI channel. */
    ret = IMP_AO_EnableChn(m_iDevID, m_iChnID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio play enable channel failed err %d !\n",ret);
        return mxfalse;
    }

    //关闭cache功能，及时播报
    ret = IMP_AO_CacheSwitch(m_iDevID, m_iChnID, 0);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio play enable channel failed err %d !\n",ret);
        return mxfalse;
    }
    
    ret = IMP_AO_SetGain(m_iDevID, m_iChnID, m_iAogain);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio Record Set Gain failed err %d !\n",ret);
        return mxfalse;
    }

    ret = IMP_AO_GetGain(m_iDevID, m_iChnID, &m_iAogain);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio Record Get Gain failed err %d !\n",ret);
        return mxfalse;
    }
    
    /* Step 4: Set audio channel volume. */
    ret = IMP_AO_SetVol(m_iDevID, m_iChnID, m_iChnVol);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio Play set volume failed err %d !\n",ret);
        return mxfalse;
    }
    /* Step 5: enable AO algorithm. */
    ret = IMP_AO_EnableAlgo(m_iDevID, m_iChnID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_EnableAlgo failed err %d !\n",ret);
        return mxfalse;
    }
    
    ret = IMP_AO_ResumeChn(m_iDevID, m_iChnID);
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  IMP_AO_ResumeChn [%d]",ret);
    
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  stop init ====== !\n");
    m_bInit = true;
    return mxtrue;
}

mxbool CZeratulSpeakerInterface::unInit()
{
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  start uninit  !\n");
    pauseChn();
    clearChnBuf();
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  enter uninit !\n");

    if(!m_bInit)
    {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface CVoicePlayEntity deint err : not init !\n");
        return mxfalse;
    }

    // logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  IMP_AO_FlushChnBuf start !\n");
    // ret = IMP_AO_FlushChnBuf(m_iDevID, m_iChnID);
    // if (ret != 0) {
    //     logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_FlushChnBuf error %d !\n", ret);
    //     return mxfalse;
    // }
    /* Step 7: disable AO algorithm. */
    ret = IMP_AO_DisableAlgo(m_iDevID, m_iChnID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_DisableAlgo error %d !\n", ret);
        return mxfalse;
    }

    /* Step 8: disable the audio channel. */
    ret = IMP_AO_DisableChn(m_iDevID, m_iChnID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio channel disable error %d !\n", ret);
        return mxfalse;
    }

    /* Step 9: disable the audio devices. */
    ret = IMP_AO_Disable(m_iDevID);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface Audio device disable error %d !\n", ret);
        return mxfalse;
    }

    m_bInit = false;
    logPrint(MX_LOG_INFOR, "mxZeratulSpeakerInterface  end uninit !\n");
    return mxtrue;
}

mxbool CZeratulSpeakerInterface::writeFrame(int chnNum, 
	unsigned char* data,int size)
{
    /* Step 6: send frame data. */
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;
    if(!m_bInit)
    {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_SendFrame not init !\n");
        return false;
    }

    if(data == NULL || size <= 0)
    {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_SendFrame data err !\n");
        return false;
    }

    IMPAudioFrame frame;
    frame.virAddr = (uint32_t *)data;
    frame.len = size;
    ret = IMP_AO_SendFrame(m_iDevID, m_iChnID, &frame, BLOCK);
    if (ret != 0) {
        logPrint(MX_LOG_ERROR, "mxZeratulSpeakerInterface IMP_AO_SendFrame ret = %d !\n", ret);
        return false;
    }
    return true;
}

int CZeratulSpeakerInterface::setVol(int audioVol)
{
    int ret = 0;
    m_iChnVol = audioVol;
    return ret;
}

int CZeratulSpeakerInterface::getVol(int *audioVol)
{
    int ret = 0;
    ret = IMP_AO_GetVol(m_iDevID, m_iChnID, audioVol);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int CZeratulSpeakerInterface::queryChnStatus()
{
    int ret = 0;
    IMPAudioOChnState play_status;
    ret = IMP_AO_QueryChnStat(m_iDevID, m_iChnID, &play_status);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int CZeratulSpeakerInterface::pauseChn()
{
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;

    if(!m_bInit)
    {
        ret = -1;
        return ret;
    }

    ret = IMP_AO_PauseChn(m_iDevID, m_iChnID);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

int CZeratulSpeakerInterface::resumeChn()
{
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;

    if(!m_bInit)
    {
        ret = -1;
        return ret;
    }
    
    return IMP_AO_ResumeChn(m_iDevID, m_iChnID);
}

int CZeratulSpeakerInterface::clearChnBuf()
{
    std::lock_guard<std::mutex> lck(m_initMutex);
    int ret = 0;

    if(!m_bInit)
    {
        ret = -1;
        return ret;
    }
    ret = IMP_AO_ClearChnBuf(m_iDevID, m_iChnID);
    if (ret != 0) {
        return ret;
    }
    return ret;
}

mxbool CZeratulSpeakerInterface::loadConfig(std::string strPath)
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
	m_strConfigPath = strPath;
    std::string strTmpPath = strPath;

#ifdef _CRYPTO_ENABLE
	std::string strKey = getPSK();
	std::string strDID = getDID();
	std::string strMac = "maix";

	int ret = crypto_hmac_sha256(strKey, strDID, strMac, m_key);
	if (ret != 0)
		return mxfalse;

	std::string strCryptoPath(m_strConfigPath);
	strCryptoPath.append(std::string(".enc"));

	iLockFileStream fenc(strCryptoPath.c_str());
	if (fenc.good() == mxtrue)
	{
		fenc.seekg(0, fenc.end);
		int size = fenc.tellg();
		if (size == 0)
			return mxfalse;

		fenc.seekg(0, fenc.beg);
		unsigned char *pcEncFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcEncFileBuffer == NULL)
			return mxfalse;

		unsigned char *pcFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcFileBuffer == NULL)
		{
			free(pcEncFileBuffer);
			return mxfalse;
		}

		memset(pcEncFileBuffer, 0, size + 1);
		memset(pcFileBuffer, 0, size + 1);

		fenc.read((char*)pcEncFileBuffer, size);

		int iFileLen = 0;
		ret = crypto_aes256_decrypt_base64(m_key,
			pcEncFileBuffer, size, pcFileBuffer, &iFileLen);
		if (ret != 0)
		{
			free(pcEncFileBuffer);
			free(pcFileBuffer);
			return mxfalse;
		}

#ifdef _WIN32
		strTmpPath.append(std::string(".read.tmp"));
#else
		strTmpPath = std::string("/tmp/");
		std::string strGUID;
		getGUIDData(strGUID);
		strTmpPath.append(strGUID);
#endif
		oLockFileStream fout(strTmpPath.c_str());
		fout.write((char*)pcFileBuffer, iFileLen);
		fout.close();

		free(pcEncFileBuffer);
		free(pcFileBuffer);
	}
#else
		strTmpPath = strPath;
#endif
#ifdef _INI_CONFIG
	try
	{
		iLockFileStream f(strTmpPath.c_str());
		if (f.good() == mxtrue)
		{
			m_configs.load(strTmpPath);
#ifdef _CRYPTO_ENABLE
			if (fenc.good() != mxtrue)
			{
				lock.unlock();
				if (!saveConfig())
				{
					logPrint(MX_LOG_ERROR,
						"save app config failed: %s", strTmpPath.c_str());
				}
			}
			else
			{
				remove(strTmpPath.c_str());
			}

            f.close();
			
#endif
			return mxtrue;
		}
	}
	catch (std::exception & e)
	{
		logPrint(MX_LOG_ERROR, "loadConfig app  failed: %s", strTmpPath.c_str());
	}
#endif

#ifdef _CRYPTO_ENABLE
		remove(strTmpPath.c_str());
#endif
	return mxfalse;
}

mxbool CZeratulSpeakerInterface::saveConfig()
{
	std::unique_lock<std::mutex> lock(m_mutexConfig);
#ifdef _WIN32
	std::string strTmpPath(m_strConfigPath);
	strTmpPath.append(std::string(".write.tmp"));
#else
	std::string strTmpPath("/tmp/");
	std::string strGUID;
	getGUIDData(strGUID);
	strTmpPath.append(strGUID);
#endif
#ifdef _INI_CONFIG
#ifdef _CRYPTO_ENABLE
		m_configs.save(strTmpPath);
#else
		m_configs.save(m_strConfigPath);
#endif
#endif
#ifdef _CRYPTO_ENABLE
	iLockFileStream fin(strTmpPath.c_str());
	if (fin.good() == mxtrue)
	{
		fin.seekg(0, fin.end);
		int size = fin.tellg();
		if (size == 0)
			return mxfalse;

		fin.seekg(0, fin.beg);
		unsigned char *pcEncFileBuffer =
			(unsigned char*)malloc(size + 2048);
		if (pcEncFileBuffer == NULL)
			return mxfalse;

		unsigned char *pcFileBuffer =
			(unsigned char*)malloc(size + 1);
		if (pcFileBuffer == NULL)
		{
			free(pcEncFileBuffer);
			return mxfalse;
		}

		memset(pcEncFileBuffer, 0, size + 2048);
		memset(pcFileBuffer, 0, size + 1);

		fin.read((char*)pcFileBuffer, size);

		size_t iEncFileLen = 0;
		int ret = crypto_aes256_encrypt_base64(m_key,
			pcFileBuffer, size, pcEncFileBuffer,
			size + 2048,
			&iEncFileLen);

		if (ret != 0)
		{
			free(pcEncFileBuffer);
			free(pcFileBuffer);
			return mxfalse;
		}
		std::string strPath = m_strConfigPath;
		strPath.append(std::string(".enc"));
		oLockFileStream fout(strPath.c_str());
		fout.write((char*)pcEncFileBuffer, iEncFileLen);
		fout.close();

		free(pcEncFileBuffer);
		free(pcFileBuffer);
	}

	fin.close();
	remove(strTmpPath.c_str());
#endif
	return mxtrue;
}

int CZeratulSpeakerInterface::getChnNum()
{
	int iChnNum = 0;

	if (!getConfig("INTERFACE", "CHN_NUM", iChnNum))
	{
		return 0;
	}

	return iChnNum;
}

std::string CZeratulSpeakerInterface::getChnName(int iNum)
{
	std::string strName;
	if (iNum == 0)
	{
		if (getConfig("CHN_NUM_1", "NAME", strName))
		{
			return strName;
		}
	}

	return strName;
}

int CZeratulSpeakerInterface::getChnSn(int iNum)
{
	int sn = -1;
	if (iNum == 0)
	{
		if (getConfig("CHN_NUM_1", "SN", sn))
		{
			return sn;
		}
	}

	return sn;
}

E_P_TYPE CZeratulSpeakerInterface::getPacketType(int iNum)
{
    E_P_TYPE type = E_P_NULL;
    std::string strType;
    if (iNum == 0)
    {
        if (getConfig("CHN_NUM_1", "TYPE", strType))
        {
            if(strType.compare("PCM") == 0)
            {
                type = E_P_AUDIO_PCM;
            }
        }
    }
    return type;
}

int64_t CZeratulSpeakerInterface::getCurrentTime()
{
    // linux 下可用

    struct timeval tv;
    gettimeofday(&tv, NULL);    //linux下该函数在sys/time.h头文件中  
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;

    // 获取秒 时间戳
    /*time_t rawtime;
    return (int)time(&rawtime);*/

    // 获取毫秒 时间戳
    // struct timeb rawtime;
    // ftime(&rawtime);
    // return rawtime.time * 1000 + rawtime.millitm;
}
