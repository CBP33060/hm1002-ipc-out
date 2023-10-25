#ifndef __LOAD_ZERATUL_SPEAKER_INTERFACE_H__
#define __LOAD_ZERATUL_SPEAKER_INTERFACE_H__
#include "global_export.h"
#include "media_interface_factory.h"
#include "media_interface.h"
using namespace maix;

class CLoadZeratulSpeakerInterface : public CMediaInterface
{
	DEFINE_MEDIA_INTERFACE

public:
	CLoadZeratulSpeakerInterface(std::string strName);
	~CLoadZeratulSpeakerInterface();

	std::shared_ptr<CMediaInterface> getInterface(const std::string className);

private:
	std::string m_strName;
};

#endif //__LOAD_ZERATUL_SPEAKER_INTERFACE_H__
