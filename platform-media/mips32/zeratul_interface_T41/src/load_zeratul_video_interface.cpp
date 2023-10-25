#include "load_zeratul_video_interface.h"
#include "media_interface_factory.h"
#include "zeratul_video_interface.h"

REG_MEDIA_INTERFACE_BEGIN(CLoadZeratulVideoInterface, "video")
REG_MEDIA_INTERFACE_END(CLoadZeratulVideoInterface)

CLoadZeratulVideoInterface::CLoadZeratulVideoInterface(std::string strName)
	: CMediaInterface(strName)
	, m_strName(strName)
{
}

CLoadZeratulVideoInterface::~CLoadZeratulVideoInterface()
{
}

std::shared_ptr<CMediaInterface> CLoadZeratulVideoInterface::getInterface(
	const std::string className)
{
	if (className == "CLoadZeratulVideoInterface")
	{
		std::shared_ptr<CMediaInterface> objVideoInterface(
			new CZeratulVideoInterface(m_strName));
		return objVideoInterface;
	}

	return std::shared_ptr<CMediaInterface>();
}
