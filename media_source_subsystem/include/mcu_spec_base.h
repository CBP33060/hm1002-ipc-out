#ifndef __DEV_MANAGE_SPEC_BASE_H__
#define __DEV_MANAGE_SPEC_BASE_H__

#include <string>
#include <iostream>
#include <map>
#include <functional>

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

	#define PARA_INDICATE_LIGHT_SWITCH          "indicate_light_switch"
	#define PARA_WHITE_LIGHT_SWITCH             "white_light_switch"         
	#define PARA_CAMERA_SWITCH                  "camera_switch"

	#define PARA_AI_DETECTION_MASK              "ai_detection_mask"
	#define PARA_AI_PUSH_MASK					"ai_push_mask"

	#define PARA_PERSON_DETECTION               "person_detection"
	#define PARA_ANIMAL_DETECTION               "animal_detection"
	#define PARA_VEHICLE_DETECTION              "vehicle_detection"
	#define PARA_PACKAGE_DETECTION              "package_detection"

	#define PARA_PIR_DETECTION                  "pir_detection"
	#define PARA_PIR_SENSITIVITY                "pir_sensivitive"
	#define PARA_PIR_INTERVAL                   "pir_interval"
	#define PARA_PERSON_STAY                    "people_stay_time"

	#define PARA_AREA_DETECT_COORD              "area_detect_coord"

	// #define PARA_FACE_RECOGNITION               "face_recognition"
	#define PARA_AUTO_ALARM_SWITCH      		"auto_audible_visual_alarm" 
	#define PARA_MANUAL_ALARM_SWITCH         	"m_audible_visual_alarm" 
	#define PARA_WIFI_SIGNAL					"wifi_signal"
	#define PARA_WHITE_LIGHT_BRIGHTNESS			"white_light_brightness"
	#define PARA_WIFI_SIGNAL					"wifi_signal"
}

#endif /*__DEV_MANAGE_SPEC_BASE_H__*/
