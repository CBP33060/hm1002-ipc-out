#ifndef __MCU_DEV_SPEC_ADAPTER_H__
#define __MCU_DEV_SPEC_ADAPTER_H__
#include "com_proxy_base.h"
#include "mcu_serial_port_module.h"
#include "mx_proto_ipc.h"

namespace maix {
	#define EVENT_SET_WDR                       "SetWdr"                       /// wdr开关       
	#define EVENT_SET_WATER_MARK                "SetWatermark"                 /// 水印开关
	#define EVENT_SET_NIGHT_SHOT         		"SetNightShot"          	   /// 夜视
	#define EVENT_SET_CAMERA_CORRECT            "SetCameraCorrect"             /// 图像矫正
	#define EVENT_SET_RECORD_DURATION           "SetRecordDuration"            /// 录像时长
	#define EVENT_SET_PEOPLE_DETECTION          "SetPeopleDetection"           /// 人形检测开关
	#define EVENT_SET_PACKAGE_DETECTION         "SetPackageDetection"          /// 包裹识别开关
	#define EVENT_SET_ANIMAL_DETECTION          "SetAnimalDetection"           /// 动物识别开关
	#define EVENT_SET_VEHICLE_DETECTION         "SetVehicleDetection"          /// 车辆检测开关
	#define EVENT_SET_FACE_RECOGNITION          "SetFaceRecognition"           /// 人脸识别开关
	#define EVENT_SET_PIR_DETECTION             "SetPirDetection"              /// pir检测开关
	#define EVENT_SET_PIR_SENSITIVITY           "SetPirSensitivity"            /// pir灵敏度设置
	#define EVENT_SET_PIR_INTERVAL         		"SetPirInterval"         	  /// pir触发间隔
	#define EVENT_SET_AUTO_ALARM_SWITCH 		"SetAutoAudibleVisualAlarm"    /// 自动声光报警开关
	#define EVENT_SET_MANUAL_ALARM_SWITCH    	"SetAudibleVisualAlarm"        /// 手动声光报警开关
	#define EVENT_SET_HOUSE_KEEPER_SWITCH       "SetHouseKeeper"               /// 看家助手开关
	#define EVENT_SET_HOUSE_KEEPER_START_TIME   "SetHouseKeeperStartTime"      /// 看家助手时间段
	#define EVENT_SET_HOUSE_KEEPER_END_TIME   	"SetHouseKeeperEndTime"        /// 看家助手时间段
	#define EVENT_SET_AREA_DETECT_COORD         "SetAreaDetectCooord"          /// 区域检测坐标
	#define EVENT_SET_ALERT_INTERVAL            "SetAlertInterval"             /// 报警时间间隔
	#define EVENT_SET_ALERT_MESSAGE_PUSH        "SetAlertMessagePush"          /// 报警消息推送开关
	#define EVENT_SET_CAMERA                    "SetCamera"                    /// 摄像头开关
	#define EVENT_SET_INDICATE_LIGHT            "SetIndicateLight"             /// 指示灯开关
	#define EVENT_SET_WHITE_LIGHT               "SetWhiteLight"                /// 白光灯开关
	#define EVENT_SET_BATTERY_LEVEL             "SetBatteryLevel"              /// 电池电量
	#define EVENT_SET_BATTERY_STATE             "SetBatteryState"              /// 电池状态
	#define EVENT_SET_TIMEZONE                  "SetTimeZone"                  /// 时区设置
	#define EVENT_SET_PEOPLE_STAY               "SetPeopleStayTime"            /// 有人逗留时间
	#define EVENT_SET_SD_CARD_STATUS            "SetSdCardStatus"              /// sd卡状态
	#define EVENT_SET_SD_CARD_TOTAL             "SetSdCardTotal"               /// sd卡总容量
	#define EVENT_SET_SD_CARD_FREE              "SetSdCardFree"                /// sd卡剩余容量
	#define EVENT_SET_SD_CARD_USED              "SetSdCardUsed"                /// sd卡使用容量
	#define EVENT_SET_FACE_RECOGNITION_PUSH     "SetFaceRecognitionPush"       /// 人脸检测推送
	#define EVENT_SET_PEOPLE_DETECTION_PUSH     "SetPersonDetectionPush"       ///人行检测推送
	#define EVENT_SET_VEHICLE_DETECTION_PUSH    "SetVehicleDetectionPush"      /// 车辆检测推送
	#define EVENT_SET_ANIMAL_DETECTION_PUSH     "SetAnimalDetectionPush"       /// 动物检测推送
	#define EVENT_SET_PACKAGE_DETECTION_PUSH    "SetPackageDetectionPush"      /// 包裹检测推送
	#define EVENT_SET_WHITE_LIGHT_BRIGHTNESS 	"SetWhiteLightBrightness"	   /// 白光灯亮度
	#define EVENT_SET_WIFI_SIGNAL				"SetWifiSignal"				///wifi信号强度
	#define EVENT_SET_TIMING_DECTION			"SetTimingDection"				///定时侦测


	#define EVENT_GET_WDR                       "GetWdr"                       /// wdr开关       
	#define EVENT_GET_WATER_MARK                "GetWatermark"                 /// 水印开关
	#define EVENT_GET_NIGHT_SHOT		        "GetNightShot"                 /// 夜视
	#define EVENT_GET_CAMERA_CORRECT            "GetCameraCorrect"            /// 图像矫正
	#define EVENT_GET_RECORD_DURATION           "GetRecordDuration"           /// 录像时长
	#define EVENT_GET_PEOPLE_DETECTION          "GetPeopleDetection"          /// 人形检测开关
	#define EVENT_GET_PACKAGE_DETECTION         "GetPackageDetection"         /// 包裹识别开关
	#define EVENT_GET_ANIMAL_DETECTION          "GetAnimalDetection"          /// 动物识别开关
	#define EVENT_GET_VEHICLE_DETECTION         "GetVehicleDetection"         /// 车辆检测开关
	#define EVENT_GET_FACE_RECOGNITION          "GetFaceRecognition"          /// 人脸识别开关
	#define EVENT_GET_PIR_DETECTION             "GetPirDetection"             /// pir检测开关
	#define EVENT_GET_PIR_SENSITIVITY           "GetPirSensitivity"           /// pir灵敏度设置
	#define EVENT_GET_PIR_INTERVAL         		"GetPirInterval"         	  /// pir触发间隔
	#define EVENT_GET_AUTO_ALARM_SWITCH 		"GetAutoAudibleVisualAlarm"   /// 自动声光报警开关
	#define EVENT_GET_MANUAL_ALARM_SWITCH    	"GetAudibleVisualAlarm"       /// 手动声光报警开关
	#define EVENT_GET_HOUSE_KEEPER_SWITCH       "GetHouseKeeper"              /// 看家助手开关
	#define EVENT_GET_HOUSE_KEEPER_START_TIME   "GetHouseKeeperStartTime"     /// 看家助手开始时间段
	#define EVENT_GET_HOUSE_KEEPER_END_TIME     "GetHouseKeeperEndTime"       /// 看家助手停止时间段
	#define EVENT_GET_AREA_DETECT_COORD         "GetAreaDetectCooord"        /// 区域检测坐标
	#define EVENT_GET_ALERT_INTERVAL            "GetAlertInterval"            /// 报警时间间隔
	#define EVENT_GET_ALERT_MESSAGE_PUSH        "GetAlertMessagePush"        /// 报警消息推送开关
	#define EVENT_GET_CAMERA                    "GetCamera"                    /// 摄像头开关
	#define EVENT_GET_INDICATE_LIGHT            "GetIndicateLight"            /// 指示灯开关
	#define EVENT_GET_WHITE_LIGHT               "GetWhiteLight"               /// 白光灯开关
	#define EVENT_GET_BATTERY_LEVEL             "GetBatteryLevel"             /// 电池电量
	#define EVENT_GET_BATTERY_STATE             "GetBatteryState"             /// 电池状态
	#define EVENT_GET_TIMEZONE                  "GetTimeZone"                 /// 时区设置
	#define EVENT_GET_PEOPLE_STAY               "GetPeopleStayTime"          /// 有人逗留时间
	#define EVENT_GET_SD_CARD_STATUS            "GetSdCardStatus"            /// sd卡状态
	#define EVENT_GET_SD_CARD_TOTAL             "GetSdCardTotal"             /// sd卡总容量
	#define EVENT_GET_SD_CARD_FREE              "GetSdCardFree"              /// sd卡剩余容量
	#define EVENT_GET_SD_CARD_USED              "GetSdCardUsed"              /// sd卡使用容量
	#define EVENT_GET_FACE_RECOGNITION_PUSH     "GetFaceRecognitionPush"     /// 人脸检测推送
	#define EVENT_GET_PEOPLE_DETECTION_PUSH     "GetPersonDetectionPush"     ///人行检测推送
	#define EVENT_GET_VEHICLE_DETECTION_PUSH    "GetVehicleDetectionPush"    /// 车辆检测推送
	#define EVENT_GET_ANIMAL_DETECTION_PUSH     "GetAnimalDetectionPush"     /// 动物检测推送
	#define EVENT_GET_PACKAGE_DETECTION_PUSH    "GetPackageDetectionPush"    /// 包裹检测推送
	#define EVENT_GET_WHITE_LIGHT_BRIGHTNESS 	"GetWhiteLightBrightness"	   ///设置白光灯亮度
	#define EVENT_GET_WIFI_SIGNAL				"GetWifiSignal"				///wifi信号强度
	#define EVENT_GET_TIMING_DECTION			"GetTimingDection"				///定时侦测

	#define MAX_BUFFER_SIZE  1024

	typedef enum __DATA_TYPE
	{
		DATA_TYPE_UINT8 = 0,
		DATA_TYPE_UINT32 = 1,
		DATA_TYPE_ARRAY = 2,
	} E_DATA_TYPE;

	typedef struct __SPEC_ADAPTER
	{
		std::string strEventSet;
		std::string strEventGet;
		int offset;
		E_DATA_TYPE dataType;
	} T_SPEC_ADAPTER;

	class CMcuDecSpecAdapter : public CComProxyBase
	{
	public:
		CMcuDecSpecAdapter(CModule *module);
		~CMcuDecSpecAdapter();

		mxbool init();
		mxbool unInit();


		void run();

		void handleDevConfig(Ipc *pIpc);
		void handleSingleConfig(int iOffset, DevConfig *pConfig, cJSON *pJsonArray);
		mxbool syncAllIpcDevConfig(const cJSON *pJsonCmd);
		mxbool syncSingleDevConfig(std::string strName, std::string strValue);

		std::string getAllDevInfo();


	private:
		mxbool m_bRun;
		mxbool m_bInit;
		CModule *m_module;
		std::thread m_syncThread;

		std::vector<T_SPEC_ADAPTER> m_vecSpecAdapter;

		std::mutex m_configMutex;
		DevConfig m_ipcDevConfig;
	
		mxbool composeConfigMsg(const std::string &strName, const std::string &strValue, cJSON *pJsonArray);
		mxbool sendSpecMsg(cJSON *pjsonArray);

		mxbool setDevConfigValue(DevConfig *config, T_SPEC_ADAPTER *specAdapter, const std::string &strValue);
		mxbool getDevConfigValue(DevConfig *config, T_SPEC_ADAPTER *specAdapter, std::string &strValue);
	};
}
#endif //__MCU_SERIAL_PORT_REMOTE_EVENT_SERVER_H__
