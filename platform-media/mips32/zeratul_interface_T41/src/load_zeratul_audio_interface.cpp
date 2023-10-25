#include "load_zeratul_audio_interface.h"
#include "zeratul_audio_interface.h"

REG_MEDIA_INTERFACE_BEGIN(CLoadZeratulAudioInterface, "audio")
REG_MEDIA_INTERFACE_END(CLoadZeratulAudioInterface)

CLoadZeratulAudioInterface::CLoadZeratulAudioInterface(std::string strName)
	: CMediaInterface(strName)
	, m_strName(strName)
{
}

CLoadZeratulAudioInterface::~CLoadZeratulAudioInterface()
{
}

std::shared_ptr<CMediaInterface> CLoadZeratulAudioInterface::getInterface(
	const std::string className)
{
	if (className == "CLoadZeratulAudioInterface")
	{
		std::shared_ptr<CMediaInterface> objAudioInterface(
			new CZeratulAudioInterface(m_strName));
		return objAudioInterface;
	}

	return std::shared_ptr<CMediaInterface>();
}
