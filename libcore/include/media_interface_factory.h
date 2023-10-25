#ifndef __MEDIA_INTERFACE_FACTORY_H__
#define __MEDIA_INTERFACE_FACTORY_H__
#include "media_interface.h"
#include <map>
#include <list>

namespace maix {
	typedef	CMediaInterface* LP_MEDIA_INTERFACE;

	typedef LP_MEDIA_INTERFACE(*_CREATE_MEDIA_INTERFACE)(std::string strName);

	struct MediaInterfaceInfo
	{
		MediaInterfaceInfo() :m_creater(0) {}
		MediaInterfaceInfo(std::string strClassName,
			_CREATE_MEDIA_INTERFACE p,
			std::string strType)
			: m_strClassName(strClassName)
			, m_creater(p)
			, m_strType(strType) {};

		std::string m_strClassName;
		_CREATE_MEDIA_INTERFACE m_creater;
		std::string m_strType;
	};

	class MAIX_EXPORT CMediaInterfaceFactory
	{
		friend class CMediaInterfaceReg;
	public:
		CMediaInterfaceFactory();
		~CMediaInterfaceFactory();

		static void loadMediaSourceInterface(std::string strMediaSourcePath);
		static LP_MEDIA_INTERFACE create(std::string strClassName, std::string strName);
		static MediaInterfaceInfo getInfo(const std::string &strClassName);
		static std::list<std::string> interfaceList();
		static void regInterface(MediaInterfaceInfo info);
	private:
		static std::map<std::string, MediaInterfaceInfo> m_mediaInterfaces;
	};

	class  CMediaInterfaceReg
	{
	public:
		CMediaInterfaceReg(MediaInterfaceInfo info)
		{
			CMediaInterfaceFactory::regInterface(info);
		}
	};

#define _CREATE_MEDIA_INTERFACE(x)\
	LP_MEDIA_INTERFACE _createModule_##x( std::string strName)\
	{\
		LP_MEDIA_INTERFACE lpData = NULL;\
		lpData = new x( strName);\
		return (LP_MEDIA_INTERFACE) lpData;\
	}

#define DEFINE_MEDIA_INTERFACE \
    static CMediaInterfaceReg __regFactory;\
    public:\
	virtual MediaInterfaceInfo getMetaData() const { return _metaData;}\
    private:\
	static MediaInterfaceInfo _metaData;\

#define REG_MEDIA_INTERFACE_BEGIN(classname, interfaceType)\
	_CREATE_MEDIA_INTERFACE(classname)\
	std::string classname##__regClassName = #classname;\
	_CREATE_MEDIA_INTERFACE  classname##__regCreateMethod = _createModule_##classname;\
	std::string classname##__regTypeName = interfaceType;\


#define REG_MEDIA_INTERFACE_END(classname)\
	MediaInterfaceInfo classname::_metaData(classname##__regClassName, classname##__regCreateMethod, classname##__regTypeName);\
    CMediaInterfaceReg classname::__regFactory(_metaData);
}
#endif //__MEDIA_INTERFACE_FACTORY_H__
