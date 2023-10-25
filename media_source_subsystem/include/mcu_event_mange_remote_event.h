#ifndef __MCU_MANAGE_REMOTE_EVENT_H__
#define __MCU_MANAGE_REMOTE_EVENT_H__
#include "module.h"
#include "b_queue.h"
#include "cJSON.h"

#define OUT_PUT_EVENT_MANAGE_EVENT                  "MCUEvent"  //mcu向其他模块发送事件
#define TYPE_EVENT_MANAGE_PIR_EVENT                 "PirEvent"  //mcu向event模块发送pir事件
#define TYPE_EVENT_MANAGE_NET_EVENT					"NetEvent"  //mcu向event模块发送net唤醒事件
#define TYPE_DEV_MANAGE_KEY_RESET_EVEMT             "KeyReset"  //mcu向dev模块发送key重置事件

#define OUT_PUT_DEV_MANAGE_PLAY_FILE_EVENT          "PlayFile"  		//mcu播放文件事件
#define PLAY_FILE_SHUT_DOWN                         "shut_down" 		//mcu播放关机音效
#define PLAY_FILE_RESET_SUCCESS						"reset_success"		//mcu播放重置音乐
#define PLAY_FILE_WARMING_ALARM						"warming_alarm"		//mcu播放循环报警音频
#define PLAY_FILE_XXX                               "xxx"       		//mcu播放xxx音乐

#define OUT_PUT_DEV_MANAGE_STOP_PLAY_FILE_EVENT     "PlayFileStop"      //mcu去除播放文件事件(去除某个fileid的播放)

namespace maix {

    typedef struct {
        std::string strGUid;
        std::string strServer;
        std::string strEvent;
        std::string strJsonParam;
    }T_MCU_MANAGE_RENOTE_OUTPUT;

	class MAIX_EXPORT CMcuManageRemoteEvent
	{
	public:
		CMcuManageRemoteEvent(CModule * module);
		~CMcuManageRemoteEvent();

		mxbool init();
		mxbool unInit();

		void run();

		mxbool pushRecvData(std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT> &outPutData);
        void popRecvData(std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT> &outPutData);

	private:

		CBQueue<std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT>> m_objMcuQueue;
		CModule * m_module;
        std::thread m_OutPutThread;

	};
}
#endif //__MCU_MANAGE_REMOTE_EVENT_H__
