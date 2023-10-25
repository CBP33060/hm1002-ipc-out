#include "load_zeratul_speaker_interface.h"
#include "media_interface_factory.h"
#include "zeratul_speaker_interface.h"

REG_MEDIA_INTERFACE_BEGIN(CLoadZeratulSpeakerInterface, "speaker")
REG_MEDIA_INTERFACE_END(CLoadZeratulSpeakerInterface)

CLoadZeratulSpeakerInterface::CLoadZeratulSpeakerInterface(std::string strName)
	: CMediaInterface(strName)
	, m_strName(strName)
{
}

CLoadZeratulSpeakerInterface::~CLoadZeratulSpeakerInterface()
{
}

std::shared_ptr<CMediaInterface> CLoadZeratulSpeakerInterface::getInterface(
	const std::string className)
{
	if (className == "CLoadZeratulSpeakerInterface")
	{
		std::shared_ptr<CMediaInterface> objSpeakerInterface(
			new CZeratulSpeakerInterface(m_strName));
		return objSpeakerInterface;
	}

	return std::shared_ptr<CMediaInterface>();
}