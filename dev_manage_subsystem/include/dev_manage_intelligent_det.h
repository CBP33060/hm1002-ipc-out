#ifndef __DEV_MANAGE_INTELLIGENT_DET_H__
#define __DEV_MANAGE_INTELLIGENT_DET_H__
#include <string>
#include "fw_env_para.h"
#include "cJSON.h"
#include "dev_manage_module.h"

namespace maix {
	class CDevManageModule;
	class CDevIntelligentDet
	{
	public:
		CDevIntelligentDet(CDevManageModule* DevManageModule);
		~CDevIntelligentDet();

		mxbool handleSetPeopleDetection(std::string &strValue);
		mxbool handleSetPackageDetection(std::string &strValue);
		mxbool handleSetVehicleDetection(std::string &strValue);
		mxbool handleSetAnimalDetection(std::string &strValue);
		mxbool handleSetPeopleStayTime(std::string &strValue);
		mxbool handleSetFaceRecognition(std::string &strValue);
		mxbool handleSetAreaDetectCoord(std::string &strValue);
		mxbool handleSetTimingDetection(std::string &strValue);
		std::string spliceMsg(std::string strParam,std::string strEvent);
	private:
		CDevManageModule* m_objDevModule;


	};
}

#endif /*__DEV_MANAGE_INTELLIGENT_DET_H__*/
