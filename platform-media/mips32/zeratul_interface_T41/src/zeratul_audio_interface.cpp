#include "zeratul_audio_interface.h"
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

CZeratulAudioInterface::CZeratulAudioInterface(std::string strName)
	: CMediaInterface(strName)
{
	m_bInit = false;
}

CZeratulAudioInterface::~CZeratulAudioInterface()
{
}

mxbool CZeratulAudioInterface::init()
{
    int ret = 0;

    if(m_bInit)
    {
        ret = -1;
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface CAudioInputEntity init err : has been init !\n");
        return mxfalse;
    }

	/* Step 1: set public attribute of AI device. */
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_16000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 640;
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(m_iDevID, &attr);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface set ai %d attr err: %d !\n", m_iDevID, ret);
		return mxfalse;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AI_GetPubAttr(m_iDevID, &attr);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface get ai %d attr err: %d !\n", m_iDevID, ret);
		return mxfalse;
	}

	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr samplerate : %d !\n", attr.samplerate);
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr   bitwidth : %d !\n", attr.bitwidth);
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr  soundmode : %d !\n", attr.soundmode);
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr     frmNum : %d !\n", attr.frmNum);
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr  numPerFrm : %d !\n", attr.numPerFrm);
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetPubAttr     chnCnt : %d !\n", attr.chnCnt);
	
	/* Step 2: enable AI device. */
	ret = IMP_AI_Enable(m_iDevID);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface enable ai %d err !\n", m_iDevID);
		return mxfalse;
	}

	/* Step 3: set audio channel attribute of AI device. */
	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 20;
	chnParam.aecChn = (IMPAudioAecChn)0;
	ret = IMP_AI_SetChnParam(m_iDevID, m_iChnID, &chnParam);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface set ai %d channel %d attr err: %d !\n", m_iDevID, m_iChnID, ret);
		return mxfalse;
	}

	memset(&chnParam, 0x0, sizeof(chnParam));
	ret = IMP_AI_GetChnParam(m_iDevID, m_iChnID, &chnParam);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface get ai %d channel %d attr err: %d !\n", m_iDevID, m_iChnID, ret);
		return mxfalse;
	}

	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetChnParam usrFrmDepth : %d !\n",  chnParam.usrFrmDepth);

	/* Step 4: enable AI channel. */
	ret = IMP_AI_EnableChn(m_iDevID, m_iChnID);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record enable channel failed !\n");
		return mxfalse;
	}

	ret = IMP_AI_EnableAec(m_iDevID, m_iChnID, 0, 0);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record enable aec failed !\n");
		return -1;
	}

	// ret = IMP_AI_EnableHs();
	// if(ret != 0) {
	// 	logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record EnableHs failed !\n");
	// 	return -1;
	// }

	/* Step 5: Set audio channel volume. */
	// int chnVol = 60;
	ret = IMP_AI_SetVol(m_iDevID, m_iChnID, m_iChnVol);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record set volume failed !\n");
		return mxfalse;
	}

	ret = IMP_AI_GetVol(m_iDevID, m_iChnID, &m_iChnVol);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record get volume failed !\n");
		return mxfalse;
	}

	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetVol  vol : %d !\n", m_iChnVol);

	// int aigain = 23;
	ret = IMP_AI_SetGain(m_iDevID, m_iChnID, m_iAogain);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record Set Gain failed !\n");
		return mxfalse;
	}

	ret = IMP_AI_GetGain(m_iDevID, m_iChnID, &m_iChnVol);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Record Get Gain failed !\n");
		return mxfalse;
	}
	logPrint(MX_LOG_INFOR, "mxZeratulAudioInterface Audio In GetGain gain : %d !\n", m_iChnVol);
	/* Step 6: enable AI algorithm. */
	// ret = IMP_AI_EnableAlgo(m_iDevID, m_iChnID);
	// if(ret != 0) {
	// 	IMP_LOG_ERR(TAG, "IMP_AI_EnableAlgo failed\n");
	// 	return mxfalse;
	// }

	m_bInit = true;
    return mxtrue;
}

mxbool CZeratulAudioInterface::unInit()
{
    int ret = 0;

    if(!m_bInit)
    {
        ret = -1;
        return mxfalse;
    }

	/* Step 10: disable AI algorithm. */
	ret = IMP_AI_DisableAlgo(m_iDevID, m_iChnID);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface IMP_AI_DisableAlgo failed !\n");
		return mxfalse;
	}
	/* Step 11: disable the audio channel. */
	ret = IMP_AI_DisableChn(m_iDevID, m_iChnID);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio channel disable error !\n");
		return mxfalse;
	}

	/* Step 12: disable the audio devices. */
	ret = IMP_AI_Disable(m_iDevID);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio device disable error !\n");
		return mxfalse;
	}
	
	m_bInit = false;
    return mxtrue;
}

unsigned char * CZeratulAudioInterface::readFrame(int chnNum, int *size)
{
    int ret = 0;
    if(!m_bInit)
    {
        ret = -1;
		*size = 0;
        return NULL;
    }
	/* Step 7: get audio record frame. */
	ret = IMP_AI_PollingFrame(m_iDevID, chnNum, 1000);
	if (ret != 0 ) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Polling Frame Data error !\n");
	}
	IMPAudioFrame frame;
	ret = IMP_AI_GetFrame(m_iDevID, chnNum, &frame, BLOCK);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Get Frame Data error !\n");
		return NULL;
	}
	memcpy(m_frameBuff, frame.virAddr, frame.len);
	*size = frame.len;
	// logPrint(MX_LOG_DEBUG, "mxZeratulAudioInterface audio pcm size is ---------------frame.len[%d]---- !\n", frame.len);
	ret = IMP_AI_ReleaseFrame(m_iDevID, chnNum, &frame);
	if(ret != 0) 
	{
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio release frame data error !\n");
	}

	return m_frameBuff;
}

unsigned char* CZeratulAudioInterface::readFrame(int chnNum, int *size, int *frameType, int64_t *timestamp, int *frameSeq)
{
    int ret = 0;
    if(!m_bInit)
    {
        ret = -1;
		*size = 0;
        return NULL;
    }
	/* Step 7: get audio record frame. */
	ret = IMP_AI_PollingFrame(m_iDevID, chnNum, 1000);
	if (ret != 0 ) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Polling Frame Data error !\n");
	}
	IMPAudioFrame frame;
	ret = IMP_AI_GetFrame(m_iDevID, chnNum, &frame, BLOCK);
	if(ret != 0) {
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio Get Frame Data error !\n");
		return NULL;
	}
	memcpy(m_frameBuff, frame.virAddr, frame.len);
	*size = frame.len;

	*timestamp = frame.timeStamp;
	*frameSeq = frame.seq;

	// logPrint(MX_LOG_DEBUG, "mxZeratulAudioInterface audio pcm size is ---------------frame.len[%d]---- !\n", frame.len);
	ret = IMP_AI_ReleaseFrame(m_iDevID, chnNum, &frame);
	if(ret != 0) 
	{
		logPrint(MX_LOG_ERROR, "mxZeratulAudioInterface Audio release frame data error !\n");
	}

	return m_frameBuff;
}

mxbool CZeratulAudioInterface::loadConfig(std::string strPath)
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

mxbool CZeratulAudioInterface::saveConfig()
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

int CZeratulAudioInterface::getChnNum()
{
	int iChnNum = 0;

	if (!getConfig("INTERFACE", "CHN_NUM", iChnNum))
	{
		return 0;
	}

	return iChnNum;
}

std::string CZeratulAudioInterface::getChnName(int iNum)
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

int CZeratulAudioInterface::getChnSN(int iNum)
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

E_P_TYPE CZeratulAudioInterface::getPacketType(int iNum)
{
	std::string strType;
	E_P_TYPE eType = E_P_NULL;
	if (iNum == 0)
	{
		if (getConfig("CHN_NUM_1", "TYPE", strType))
		{
			if (strType.compare("PCM") == 0)
			{
				eType = E_P_AUDIO_PCM;
			}
		}
	}
	return eType;
}


int64_t CZeratulAudioInterface::getCurrentTime()
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
