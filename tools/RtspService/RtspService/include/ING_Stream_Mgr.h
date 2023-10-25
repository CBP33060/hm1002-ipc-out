#pragma once
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include "ING_Stream_Vdo.h"

extern chn_conf chn[FS_CHN_NUM];

class ING_Stream_Mgr
{
public:

	ING_Stream_Mgr(void);
	virtual ~ING_Stream_Mgr(void);
		// 打开关闭
	virtual int open();
	virtual int close();
	virtual int run();

	int add_stream(ING_Stream_Vdo *stream);
	int remove_stream(unsigned int dlg_handle);

// 	int pause_stream(unsigned int dlg_handle);
// 	int resume_stream(unsigned int dlg_handle);
	ING_Stream_Vdo* find_stream(unsigned int dlg_handle);
	int framesource_init();
	int framesource_exit();
	int encoder_init();
	int encoder_exit();
	int system_init();
	int system_exit();
	int system_bind();
	int system_unbind();
	int framesource_enable();
	int framesource_disable();

// 	int is_stream_pause(unsigned int dlg_handle, bool& pause);
	// 设置播放速度
// 	int set_scale(unsigned int dlg_handle, double scale);

	// 当前流数量
	int count() {
		ING_Guard guard(StreamMgrMutex_);
		return stream_arr.size();
	}

private:
	ING_Mutex StreamMgrMutex_;
// 	std::thread stream_thread_;
	bool stop_;									//线程结束标记
	IMPSensorInfo sensor_info_;
	STREAM_ARR stream_arr;						// 流队列
};
