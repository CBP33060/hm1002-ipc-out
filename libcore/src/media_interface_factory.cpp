#include "media_interface_factory.h"
#include "common.h"
#include "log_mx.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace maix {
	std::map<std::string, MediaInterfaceInfo> CMediaInterfaceFactory::m_mediaInterfaces;

	CMediaInterfaceFactory::CMediaInterfaceFactory()
	{
	}

	CMediaInterfaceFactory::~CMediaInterfaceFactory()
	{
	}

	void CMediaInterfaceFactory::loadMediaSourceInterface(
		std::string strMediaSourcePath)
	{
		std::vector<std::string> files;
		mxbool bRet = mxfalse;
#ifdef WIN32
		bRet = getFiles(strMediaSourcePath, files, ".dll");
#else
		bRet = getFiles(strMediaSourcePath, files, ".so");
#endif
		if (bRet)
		{
			for (int i = 0; i < (int)files.size(); ++i)
			{
				logPrint(MX_LOG_INFOR, "%s", files[i].c_str());
#if defined(WIN32)
				bRet = LoadLibrary(files[i].c_str());
#else
				bRet = dlopen(files[i].c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
			}
		}

	}

	LP_MEDIA_INTERFACE CMediaInterfaceFactory::create(std::string strClassName, 
		std::string strName)
	{
		if (m_mediaInterfaces.count(strClassName))
		{
			const MediaInterfaceInfo& mInfo = m_mediaInterfaces[strClassName];
			if (mInfo.m_creater)
			{
				LP_MEDIA_INTERFACE lpMode = mInfo.m_creater(strName);
				return lpMode;
			}
		}
		return NULL;
	}

	MediaInterfaceInfo 
		CMediaInterfaceFactory::getInfo(const std::string & strClassName)
	{
		if (m_mediaInterfaces.count(strClassName))
			return m_mediaInterfaces[strClassName];
		return MediaInterfaceInfo();
	}

	std::list<std::string> CMediaInterfaceFactory::interfaceList()
	{
		std::list<std::string> keys;
		std::map<std::string, 
			MediaInterfaceInfo>::iterator iter;
		iter = m_mediaInterfaces.begin();
		while (iter != m_mediaInterfaces.end()) {
			std::string key = iter->first;
			keys.push_back(key);
			iter++;
		}
		return  keys;
	}
	void CMediaInterfaceFactory::regInterface(MediaInterfaceInfo info)
	{
		m_mediaInterfaces[info.m_strClassName] = info;
	}
}