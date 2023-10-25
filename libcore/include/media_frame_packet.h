#ifndef __MEDIA_FRAME_PACKET_H__
#define __MEDIA_FRAME_PACKET_H__
#include "type_def.h"
#include "global_export.h"
typedef long long int64_t;
typedef signed int int32_t;
#define MEDIA_FRAME_PACKET_LEN 32 * 1024

namespace maix {
	typedef enum
	{
		E_P_NULL,
		E_P_VIDEO_YUV,
		E_P_VIDEO_H264,
		E_P_VIDEO_H265,
		E_P_VIDEO_JPEG,
		E_P_AUDIO_PCM,
		E_P_AUDIO_AAC,
		E_P_AUDIO_OPUS,
	}E_P_TYPE;

	typedef enum
	{
		E_F_NULL,
		E_F_IDR,
		E_F_I,
		E_F_P,
		E_F_B,
	}E_F_TYPE;

    //用于判断speaker语音资源来源,通过media_frame_packet,iReserve_1传递
    typedef enum 
    {
        E_SOURCE_FILE = 1,
        E_SOURCE_P2P,
    }E_VOICE_SOURCE;

    //用于判断speaker语音播放优先级,通过media_frame_packet,iReserve_2传递
    typedef enum 
    {
        E_LEVEL_LOWEST = 0,
        E_LEVEL_1,
        E_LEVEL_2,
        E_LEVEL_3,
        E_LEVEL_HIGHEST,
    }E_VOICE_PLAY_LEVEL;

	typedef struct _MediaFramePacketHeader
	{
		E_P_TYPE ePacketType;
		int nPacketSize;
		E_F_TYPE eFrameType;
		int64_t lTimeStamp;
		int32_t iFrameSeq;
		int32_t iMark;
        int32_t iReserve_1;
        int32_t iReserve_2;
        int32_t iReserve_3;
        int32_t iReserve_4;
        int32_t iReserve_5;
	}T_MediaFramePacketHeader;

	class MAIX_EXPORT CMediaFramePacket
	{
	public:
		CMediaFramePacket();
		~CMediaFramePacket();

		void setPacketType(E_P_TYPE ePacketType);
		E_P_TYPE getPacketType();

        void setReserve_1(int32_t iReserve_1);
        int32_t getReserve_1();     
    
        void setReserve_2(int32_t iReserve_2);
        int32_t getReserve_2();     

        void setReserve_3(int32_t iReserve_3);
        int32_t getReserve_3();     

        void setReserve_4(int32_t iReserve_4);
        int32_t getReserve_4();     

        void setReserve_5(int32_t iReserve_5);
        int32_t getReserve_5();    

		void setFrameType(E_F_TYPE eFrameType);
		E_F_TYPE getFrameType();
		bool setFrameData(unsigned char* pcPacketData,
			int nPacketSize,
			int64_t nTimeStamp,
			int32_t iFrameSeq);
		int64_t getFrameTimeStamp();
		int32_t getFrameSeq();
		unsigned char* getFrameData();
		int getFrameDataLen();
		void setPacketMark(int iMark);
		int getPacketMark();

	private:
		T_MediaFramePacketHeader m_packetHeader;
		unsigned char* m_pcPacketData;
	};
}
#endif //__MEDIA_FRAME_PACKET_H__
