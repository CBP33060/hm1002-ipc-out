#include "ING_Stream_Mgr.h"
#include "ING_Stream_Vdo.h"
#include "ING_Rtsp_Session.h"

#define SAVE_STREAM

#ifndef WIN32
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define  TAG "ING_RTSP_SERVER_STREAM"
#define	 PT_H264         96			//负载类型

extern bool GetFWParaConfig(std::string strKey, std::string &strValue);

ING_Stream_Vdo::ING_Stream_Vdo(unsigned int dlg_handle, unsigned short port, const char *remote_ip, unsigned short remote_port,
	int stream_type, std::string type, int payload)
	:dlg_handle_(dlg_handle)
	, port_(port)
	, remote_ip_(remote_ip)
	, remote_port_(remote_port)
	, type_(type)
	, payload_(payload)
	, rtp_pack_handle_(NULL)
// 	, jrtp_handle_(jrtp_handle)
	, is_remove_flag_(false)
	, time_stamp_(0)
	, wait_key_frame_(true)
	, stream_type_(stream_type)
	, first_time_stamp_(0)
	, cur_time_stamp_(0)
	, last_send_status_(true)
	, last_send_frame_(0)
	, pause_flag_(false)
	, frame_tm_(0)
	, LastPoolTmOutPrint_(0)
	, sendCount_(0)
	, trans_mode_(RTP_TRANSMODE_UDP)
{
	printf("ING_Stream_Vdo constructor in UDP Mode. port=%d, remote_ip=%s, remote_port=%d payload=%d\n",
		port_, remote_ip_.c_str(), remote_port_, payload_);
	memset(szRtpBuf_, 0, sizeof(szRtpBuf_));
	socket_rtp_ = RTSP_UDPBind(0, port_);
	if (socket_rtp_ == INVALID_SOCKET)
		printf("================================>ING_Stream_Vdo constructor RTSP_UDPBind Error\n");
	else
		printf("================================>ING_Stream_Vdo constructor RTSP_UDPBind Success port=%d\n", port_);

#ifdef SAVE_STREAM
	std::string str;

	save_stream = 0;
	if(GetFWParaConfig("save_stream",str) == true)
	{
		save_stream = atoi(str.c_str());
	}

	if(save_stream == 1)
	{
		std::string name;
		std::string wdr;
	
		int index = 0;
		int wdr_mode = 0;

		if(GetFWParaConfig("wdr_mode",wdr) == true)
		{
			wdr_mode = atoi(wdr.c_str());
		}

		if(GetFWParaConfig("video_file_name",name) == true)
		{
			index = atoi(name.c_str());
		}

		sprintf(video_file_name,"/tmp/mnt/sdcard/chn0_%s_%d.h265",wdr_mode?"wdr":"line",index);

		index++;
		char cmd[256];
		sprintf(cmd,"fw_setenv user video_file_name %d",index);
		system(cmd);
	}

#endif
}

ING_Stream_Vdo::ING_Stream_Vdo(unsigned int dlg_handle, int stream_type, std::string type, int payload, SOCKET tcpFd)
	:dlg_handle_(dlg_handle)
	, port_(0)
//	, remote_ip_(0)
	, remote_port_(0)
	, type_(type)
	, payload_(payload)
	, rtp_pack_handle_(NULL)
	// 	, jrtp_handle_(jrtp_handle)
	, is_remove_flag_(false)
	, time_stamp_(0)
	, wait_key_frame_(true)
	, stream_type_(stream_type)
	, first_time_stamp_(0)
	, cur_time_stamp_(0)
	, last_send_status_(true)
	, last_send_frame_(0)
	, pause_flag_(false)
	, frame_tm_(0)
	, LastPoolTmOutPrint_(0)
	, sendCount_(0)
	, trans_mode_(RTP_TRANSMODE_TCP)
{
	printf("ING_Stream_Vdo constructor in TCP Mode. payload=%d\n", payload_);
	memset(szRtpBuf_, 0, sizeof(szRtpBuf_));
	socket_rtp_ = tcpFd;
}

ING_Stream_Vdo::~ING_Stream_Vdo(void)
{
	printf("ING_Stream_Vdo destructor. port=%d, remote_ip=%s, remote_port=%d payload=%d\n",
		port_, remote_ip_.c_str(), remote_port_, payload_);
	if(rtp_pack_handle_)
		DestroyH264PackHandle(rtp_pack_handle_);
}

int ING_Stream_Vdo::open()
{
	// 	INGJRTP_Startup();
	ING_Guard guard(mutex_);
	if (type_ == "video")
	{
		// 视频RTP模块创建
		// H264码流负载类型为96
// 		rtp_pack_handle_ = CreateH264PacketHandle(uiSSRC, 96, RTP_PKT_LEN_MAX, 25);   //帧率25 负载类型96
		rtp_pack_handle_ = CreateH264PacketHandle(rand(), 96, RTP_PKT_LEN_MAX, 15);   //帧率25 负载类型96
		if (rtp_pack_handle_ == NULL)
		{
			printf("ING_Stream_Vdo::open CreateH264PacketHandle Error!!!\n");
			return -1;
		}
	}
	else if (type_ == "audio")
	{
		// 音频RTP模块创建
		// G711A码流负载类型为8
	}
	return 0;
}

int ING_Stream_Vdo::close()
{
	ING_Guard guard(mutex_);
// 	if (jrtp_handle_ != NULL)
// 	{
// 		printf("close--------------------------------------------------------------------  1\n");
// 		INGJRTP_Delete(jrtp_handle_);
// 		jrtp_handle_ = NULL;
// 	}

	printf("ING_Stream_Vdo Close Success\n");

// 	INGJRTP_Cleanup();
	return 0;
}

extern volatile int rest_sensor_flag;

int ING_Stream_Vdo::run()
{
	ING_Guard guard(mutex_);      //在后面发送时加锁
	if (is_remove_flag_)
	{
		return -1;
	}
	int ret = 0, chn_i = 0;
// 	const char *fileName = "/tmp/share/stream5.h264";
// 	const char *packfileName = "/tmp/share/stream5pack.h264";
// 	int fp = 0, fppack = 0;
// 	if (chn_i == 0)
// 	{
// 		fp = ::open(fileName, O_RDWR | O_CREAT | O_APPEND, 0777);
// 		fppack = ::open(packfileName, O_RDWR | O_CREAT | O_APPEND, 0777);
// 	}
	unsigned char *szSendBuf = new unsigned char[VIDEO_BUFFER_SIZE];
	static int _times = 0,__times = 0;
	IMPEncoderStream stream;
	while (!is_remove_flag_) {                                //一次将缓冲区读完，连接断开时退出
		ret = IMP_Encoder_PollingStream(chn_i, 1);  //非阻塞等待视频流
		if (ret < 0) {
			if(_times++ > 50)
			{
				_times = 0;
				rest_sensor_flag = 1;
				printf("--------------------------------------ING_Stream_Vdo::run IMP_Encoder_PollingStream return %d------------------\n",ret);
			}
			// printf("--------------------------------------ING_Stream_Vdo::run IMP_Encoder_PollingStream return %d------------------\n",ret);
			break;                  //当前缓冲区没有视频流, 下次再获取
		}
		
		if(__times++ % 50 == 0)
		{
			printf("--------------------------------------ING_Stream_Vdo::run IMP_Encoder_PollingStream return %d _times:%d------------------\n",ret,_times);
		}

		_times = 0;
		memset(&stream, 0, sizeof(IMPEncoderStream));
		/* Get H264 Stream */
		ret = IMP_Encoder_GetStream(chn_i, &stream, 1);
		if (ret < 0) {
			printf("ING_Stream_Vdo::run IMP_Encoder_GetStream() failed return %d\n", ret);
			break;
		}

		sendCount_++;
		int nr_pack = stream.packCount;
		int nr_i = 0;
		int offset = 0;
		//组装一帧h264码流
		IMPEncoderPack *pack = NULL;
		for (nr_i = 0; nr_i < nr_pack; nr_i++) {
			pack = &stream.pack[nr_i];
			if (pack->length) {
				uint32_t remSize = stream.streamSize - pack->offset;
				if (remSize < pack->length) {
					memcpy(szSendBuf + offset, (void *)(stream.virAddr + pack->offset), remSize);
					offset += remSize;
					memcpy(szSendBuf + offset, (void *)stream.virAddr, pack->length - remSize);
					offset += pack->length - remSize;
					printf("remSize = stream.streamSize - pack->offset******************\n*********************\n");
				}
				else {
// 					printf("get stream [%d]: timestamp=%lld,framesize=%6u\n", nr_i, pack->timestamp, pack->length);
					memcpy(szSendBuf + offset, (void *)(stream.virAddr + pack->offset), pack->length);
					offset += pack->length;
				}
			}
		}
		int frameType;
		if(stream.pack[stream.packCount-1].sliceType == IMP_ENC_SLICE_I)
		{
			frameType = 1;//I帧
			printf("IMP_Get_Video_Thread ZvmGetFrame currentReadSize=[%d],frameType[%d],stream.seq[%d] count=%d\n", offset,frameType,stream.seq, nr_pack);
		}else{
			frameType = 0;//P帧
		}

		
//		printf("get stream currentReadSize=%d, count=%d\n", offset, nr_pack);
		//打包h264码流为rtp包，发送给rtsp client
		PushH265ES(rtp_pack_handle_, szSendBuf, offset);

#ifdef SAVE_STREAM
		if(chn_i == 0 && save_stream == 1){
			int _fd = ::open(video_file_name, O_RDWR | O_CREAT | O_APPEND, 0x644);
        	::write(_fd, szSendBuf, offset);
        	::close(_fd);
		}
#endif

// 		if (chn_i == 0 && fp != 0)
// 		{
// 			::write(fp, szSendBuf, offset);
// 		}
		//Release H264 Stream
		int bufferLen = MAX_MTU_LEN;
		while (1) {
			memset(szRtpBuf_, 0, sizeof(szRtpBuf_));
			bufferLen = MAX_MTU_LEN;
			ret = PopRTPPacket(rtp_pack_handle_, szRtpBuf_, &bufferLen);
			if (ret == RTPPACKET_ERROR_NOPACKET || ret == RTPPACKET_ERROR_MEM_OVERLOAD || ret == RTPPACKET_ERROR_UNKWON) {
				break;
			}
			if (ret == RTPPACKET_ERROR_CONTINUE || ret == RTPPACKET_ERROR_OK) {
// 				if (chn_i == 0 && fppack != 0)
// 				{
// 					::write(fppack, szRtpBuf_, bufferLen);
// 				}
// 				ret = INGJRTP_SendPacket(jrtp_handle_, (char *)szRtpBuf_, bufferLen);
				if (trans_mode_ == RTP_TRANSMODE_UDP)
				{
//					printf("send stream UDP\n");
					unsigned int uiRemoteIP = RTSP_AddrToU32(remote_ip_.c_str());
					ret = RTSP_UDPSendToNB(socket_rtp_, (char *)szRtpBuf_, bufferLen, uiRemoteIP, remote_port_);
				}
				//int RTSP_TCPSendDataNB(SOCKET sock, char *pSendBuf, int *piSendLen, int iDataLen);
				else
				{
//					printf("send stream tcp\n");
					int offset = 0;
					char* rtpPktPtr = new char[bufferLen + 4];
					rtpPktPtr[0] = '$';
					rtpPktPtr[1] = (unsigned char)0;
					rtpPktPtr[2] = (unsigned char)((bufferLen & 0xFF00) >> 8);
					rtpPktPtr[3] = (unsigned char)(bufferLen & 0xFF);
					memcpy(rtpPktPtr + 4, szRtpBuf_, bufferLen);
					ret = RTSP_TCPSendDataNB(socket_rtp_, rtpPktPtr, &offset, bufferLen + 4);
					delete[] rtpPktPtr;
				}
				if(ret != 0 && ret != 2)
				{
	 				printf("ING_Stream_Vdo::RTSP_UDPSendToNB send_video_frame error return %d\n", ret);
				}
			}
		}//end while

		ret = IMP_Encoder_ReleaseStream(chn_i, &stream);
		if(ret != 0)
		{
			printf("ING_Stream_Vdo::IMP_Encoder_ReleaseStream error return %d\n", ret);
		}
		memset(szSendBuf, 0, VIDEO_BUFFER_SIZE);
	}//end while
	delete[] szSendBuf;
	szSendBuf = NULL;
// 	if (chn_i == 0 && fp != 0)
// 		::close(fp);
// 	if (chn_i == 0 && fppack != 0)
// 		::close(fppack);

	//rtpsession rtcp保活
// 	ret = INGJRTP_Run(jrtp_handle_);
// 	printf("========================================>>INGJRTP_Run\n");

	return ret;
}
