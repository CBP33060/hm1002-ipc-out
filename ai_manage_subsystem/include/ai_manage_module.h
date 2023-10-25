#ifndef __AI_MANAGE_MODULE_H__
#define __AI_MANAGE_MODULE_H__
#include "module.h"
#include "video_source_input_server.h"
#include "video_manage_channel.h"
#include "video_manage_channel_session.h"
#include "event_mange_remote_event.h"
#include <vector>

#define YUV_FRAME_WIDTH     832
#define YUV_FRAME_HEIGHT    480

namespace maix {
	#define EVENT_SET_PEOPLE_DETECTION          "SetPeopleDetection"           /// 人形检测开关
	#define EVENT_SET_PACKAGE_DETECTION         "SetPackageDetection"          /// 包裹识别开关
	#define EVENT_SET_ANIMAL_DETECTION          "SetAnimalDetection"           /// 动物识别开关
	#define EVENT_SET_VEHICLE_DETECTION         "SetVehicleDetection"          /// 车辆检测开关
	#define EVENT_SET_PEOPLE_STAY_TIME			"SetPeopleStayTime"			   /// 逗留时间
	#define EVENT_SET_AREA_DETECT_COORD			"SetAreaDetectCooord"			///区域侦测

	#define PARA_TIMING_DETECTION               "timing_detection"
	#define PARA_AI_DETECTION_MASK              "ai_detection_mask"
	#define PARA_AI_PUSH_MASK					"ai_push_mask"
	#define PARA_AREA_DETECT_COORD				"area_detect_coord"
	#define PARA_PERSON_STAY_TIME				"people_stay_time"
	typedef enum
	{
		E_AI_PERSON_APPEAR,
		E_AI_PERSON_STAY,		E_AI_ANIMAL_PET_APPEAR,
		//E_AI_ANIMAL_PET_STAY,
		E_AI_CAR_APPEAR,
		E_AI_CAR_STAY,
		E_AI_PACKAGE_ENTER,
		E_AI_PACKAGE_MOVE,
		E_AI_PACKAGE_LEAVE,
		E_EVENT_MAX
	}E_AI_DETECTION_TYPE;

	class MAIX_EXPORT CAIManageModule : public CModule
	{
	public:
		CAIManageModule(std::string strGUID, std::string strName);
		~CAIManageModule();

		mxbool init();
		mxbool unInit();

		mxbool initServer();
		mxbool initConnectModule();
	
		mxbool addVideoChannel(std::string strName,
			CVideoSourceInputServer * objVideoSourceInputServer);
		mxbool addEventManageRemoteEvent();

		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);

		std::string procResult(std::string code, std::string strMsg);

		int getStaytime();
		int getDetectMask();
		std::vector<std::vector<float>> getAreaDetecMask();

		void syncDetectSwitch(E_AI_DETECTION_TYPE eValue, std::string strParam);
		void syncDetectArea(std::string strParam);
		void syncDetectStayTime(std::string strParam);

		std::string closeAIDetect();
		std::string openAIDetect();

		mxbool initDevEventServer();
		void sendToDevAiEvent();

	private:
		std::mutex m_mutexVideoChn;
		std::map<std::string, std::thread> m_mapVideoChnProc;
		std::map<std::string, std::thread> m_mapVideoChnSessionProc;
		std::map<std::string, std::shared_ptr<CVideoManageChannelSession>> m_mapVideoChnSession;
		std::map<std::string, std::shared_ptr<CVideoManageChannel>> m_mapVideoChn;

		std::shared_ptr<CEventManageRemoteEvent> m_eventManageRemoteEvent;
		std::thread m_eventManageRemoteEventThread;

		std::vector<std::vector<float>> m_vecAreaDetecMask;
		std::mutex m_mutexArea;
		
		std::mutex m_mutexDete;
		int m_iDetectMask;

		std::mutex m_mutexStayTime;
		int m_iStaytime;

		std::string m_strDevManageGUID;
		std::string m_strDevManageServer;

	};
}
#endif //__AI_MANAGE_MODULE_H__
