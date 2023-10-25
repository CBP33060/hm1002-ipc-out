#include "media_interface.h"
namespace maix {

	CMediaInterface::CMediaInterface(std::string strName)
		: m_strName(strName)
	{
	}

	CMediaInterface::~CMediaInterface()
	{
	}

	mxbool CMediaInterface::init()
	{
		return mxbool();
	}

	mxbool CMediaInterface::unInit()
	{
			return mxbool();
	}

	mxbool CMediaInterface::initChannel(int chnNum)
	{
		return mxtrue;
	}

	mxbool CMediaInterface::unInitChannel(int chnNum)
	{
		return mxtrue;
	}


	mxbool CMediaInterface::config(std::string strConfig)
	{
		return mxtrue;
	}

	mxbool CMediaInterface::startRcvFrame(int chnNum)
	{
		return mxtrue;
	}

	mxbool CMediaInterface::getIDRFrame(int chnNum)
	{
		return mxtrue;
	}

	unsigned char *CMediaInterface::readFrame(int chnNum, int * size)
	{
		return NULL;
	}

	unsigned char *CMediaInterface::readFrame(int chnNum, int * size, int *frameType, int64_t *timestamp, int *frameSeq)
	{
		return NULL;
	}

	mxbool CMediaInterface::writeFrame(int chnNum, 
		unsigned char* data, int size)
	{
		return mxfalse;
	}

	int CMediaInterface::getChnNum()
	{
		return 0;
	}

	std::string CMediaInterface::getChnName(int iNum)
	{
		return std::string();
	}

	int CMediaInterface::getChnSN(int iNum)
	{
		return 0;
	}

	E_P_TYPE CMediaInterface::getPacketType(int iNum)
	{
		return E_P_NULL;
	}

	mxbool CMediaInterface::loadConfig(std::string strPath)
	{
		return mxtrue;
	}

	mxbool CMediaInterface::saveConfig()
	{
		return mxtrue;		
	}

	std::shared_ptr<CMediaInterface> CMediaInterface::getInterface(
		const std::string className)
	{
		return std::shared_ptr<CMediaInterface>();
	}

	
}