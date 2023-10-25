#include "ai_object_detection.h"
#include "ai_manage_module.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log_mx.h"
#include "common.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace cv;

#define FRAME_NUM_OPENCV 	3

#define _RINGBUF_LEN 4
typedef struct _ringbuf_t_
{
    volatile unsigned int   ringbuf_head;
    volatile unsigned int   ringbuf_tail;
    volatile unsigned char  p_ringbuf[_RINGBUF_LEN][599040];
}ringbuf;

namespace maix {
	CAIObjectDetection::CAIObjectDetection(CModule * module, std::string strName,
		std::shared_ptr<CEventManageRemoteEvent> eventManageRemoteEvent)
		: m_module(module)
		, m_strName(strName)
		, m_eventManageRemoteEvent(eventManageRemoteEvent)
	{
		m_bDetect = mxtrue;
		m_bOpencvResult = mxfalse;
		m_imgDetection = NULL;
		m_bFrame1 = mxfalse;
		m_bFrame2 = mxfalse;
		m_iConsecutivePositiveDiffCount = 0;
		m_iCountframe = 2;
		m_iRestFlag = 0;
	}

	CAIObjectDetection::~CAIObjectDetection()
	{
	}

	mxbool CAIObjectDetection::init()
	{
		if (!m_objPacketQueue.init(3, 1000))
			return mxfalse;
		
		return mxtrue;
	}

	mxbool CAIObjectDetection::unInit()
	{
		if(m_imgDetection)
			m_imgDetection->unInit();
		return mxtrue;
	}

	void CAIObjectDetection::returnDetectResult(int iResult)
	{
		logPrint(MX_LOG_DEBUG, "object detection data result: %d", iResult);		
		if(iResult > 0)
		{
			CAIManageModule *module = dynamic_cast<CAIManageModule *>(m_module);
			int maskdetect = module->getDetectMask();
			module->sendToDevAiEvent();
			for (int i = 0; i < E_EVENT_DETAIL_TYPE_MAX; i++)
			{
				if ((iResult & (1 << i)) &maskdetect)
				{
					//event
					m_eventManageRemoteEvent->pushFrameData(std::to_string(i));	
				}
			}
		}
	}

	void CAIObjectDetection::run()
	{
		int iFrameThreshold = 0;
		mxbool bLoadModel = mxfalse;
		CAIManageModule *module = dynamic_cast<CAIManageModule *>(m_module);

	    ringbuf *rb;

		int shm_id;
		key_t shm_key;

		if ((shm_key = ftok("/etc/passwd", 12345)) == -1) {
			perror("ftok()");
			exit(1);
		}

		shm_id = shmget(shm_key, sizeof(ringbuf), 0);
		if (shm_id < 0) {
			perror("shmget()");
			exit(1);
		}

		rb = (ringbuf *)shmat(shm_id, NULL, 0);
		if ((void *)rb == (void *)-1) {
			perror("shmat()");
			exit(1);
		}

		while (1)
		{
				if (!m_bDetect)
				{
	#ifdef	WIN32
					Sleep(500);
	#else
					usleep(1000*500);
	#endif
					continue;
				}

				//logPrint(MX_LOG_DEBUG, "CAIObjectDetection rb->ringbuf_tail:%d, rb->ringbuf_head:%d\n",rb->ringbuf_tail,rb->ringbuf_head);
				while(rb->ringbuf_tail == rb->ringbuf_head)
				{
					usleep(100*1000);
				}

				iFrameThreshold++;

				if(!m_bOpencvResult)
				{
					opencvIdentify((unsigned char*)rb->p_ringbuf[rb->ringbuf_head], YUV_FRAME_SIZE, m_bOpencvResult);

					if(iFrameThreshold >= FRAME_NUM_OPENCV && !m_bOpencvResult) 
					{
						//小算法在使用前3帧未识别到画面变动的情况下，给center模块发消息进低功耗
						logPrint(MX_LOG_DEBUG, "opencvIdentify not recognize changes, send CenterNoEvent...");
						m_eventManageRemoteEvent->sendAlarmEvent("0", "CenterNoEvent");	
						return;
					}
				}
				else
				{
					int staytime = module->getStaytime();  
					logPrint(MX_LOG_DEBUG, "obj detection staytime:%d, iFrameThreshold:%d", staytime, iFrameThreshold);

					if( (staytime == OBJECT_STAY_TIME_0) || 
						(staytime == OBJECT_STAY_TIME_1 && iFrameThreshold >= STAYTIME_FRAME_NUM_1) || 
						(staytime == OBJECT_STAY_TIME_2 && iFrameThreshold >= STAYTIME_FRAME_NUM_2)
					  ) 
					{  
						if(!bLoadModel)
						{
							if(!initImgDetection())
							{
								m_eventManageRemoteEvent->sendAlarmEvent("0", "CenterNoEvent");	
								return;
							}

							bLoadModel = mxtrue;
						}

						if(m_imgDetection) {  
							m_imgDetection->procFrameData((unsigned char*)rb->p_ringbuf[rb->ringbuf_head], YUV_FRAME_SIZE, 0);  
						}  
					}
					
				}

				rb->ringbuf_head = (rb->ringbuf_head + 1) % _RINGBUF_LEN;
		}
	}

	bool CAIObjectDetection::pushFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CAIObjectDetection::popFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

	void CAIObjectDetection::openDetect()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_bDetect = mxtrue;

		logPrint(MX_LOG_DEBUG, "open ai obj detection");
	}

	void CAIObjectDetection::closeDetect()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_bDetect = mxfalse;

		logPrint(MX_LOG_DEBUG, "close ai obj detection");
	}

	mxbool CAIObjectDetection::setStayTime(int iStaytime)
	{
		// if(m_imgDetection)
		// 	m_imgDetection->setStayTime(iStaytime);

		return mxtrue;
	}

	mxbool CAIObjectDetection::initImgDetection()
	{
		//取tag 判定使用 AI_MODEL_PATH_A 还是 AI_MODEL_PATH_B
		std::string strValue;
		linuxPopenExecCmd(strValue, "tag_env_info --get HW 70mai_system_partition");
		int iPos = strValue.find("Value=");
		if(-1 == iPos)
		{
			logPrint(MX_LOG_ERROR, "initImgDetection not found 70mai_system_partition");
			return mxfalse;
		}

		strValue = strValue.substr(iPos + strlen("Value="), strValue.length() - strlen("Value="));
		int iPartition = atoi(strValue.c_str());
		logPrint(MX_LOG_DEBUG, "initImgDetection partition:%d", iPartition);

		std::shared_ptr<CImgDetection> objectImgDetection(new CImgDetection());
	
#ifdef MI_ALG_DETECT
		const char *model_path = (iPartition == 0 ? AI_MODEL_PATH_A : AI_MODEL_PATH_B);
		if (!objectImgDetection->init(model_path))
		{
			logPrint(MX_LOG_ERROR, "initImgDetection init error");
			return mxfalse;
		}
#else
		if (!objectImgDetection->init())
		{
			logPrint(MX_LOG_ERROR, "objectImgDetection init error");
			return mxfalse;
		}

		if (!objectImgDetection->loadBinModel(iPartition == 0 ? AI_MODEL_PATH_A : AI_MODEL_PATH_B))
		{
			logPrint(MX_LOG_ERROR, "initImgDetection loadBinModel error");
			return mxfalse;
		}
#endif

		CAIManageModule *module = dynamic_cast<CAIManageModule *>(m_module);
		// objectImgDetection->setStayTime(module->getStaytime());
		objectImgDetection->setStayTime(0);
		objectImgDetection->setRecognitionArea(module->getAreaDetecMask());
		objectImgDetection->setObj(this);
		m_imgDetection = objectImgDetection;

		return mxtrue;
	}

	mxbool CAIObjectDetection::opencvIdentify(unsigned char* frameBuff, int frameSize, mxbool &bResult)
	{
		// 获取图像宽度和高度
		int width = 832; // 图像宽度
		int height = 480; // 图像高度
		int currentframeSize = width * height * 3 / 2; // YUV NV12格式的帧大小
		if(currentframeSize != frameSize)
		{
			logPrint(MX_LOG_ERROR, "opencvIdentify frameSize error, currentframeSize %d != frameSize %d", currentframeSize, frameSize);
			return mxfalse;
		}

		if(frameBuff == NULL)
		{
			logPrint(MX_LOG_ERROR, "opencvIdentify frameBuff is NULL");
			return mxfalse;
		}

		if( !m_bFrame1 )
		{
			logPrint(MX_LOG_DEBUG, "opencvIdentify set first frame...");

			Mat yMat(height, width, CV_8UC1, (void*)frameBuff);
			Mat uMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height));
			Mat vMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height + width * height / 4));

			// Resize U and V components to match Y component's dimensions
			resize(uMat, uMat, Size(width, height));
			resize(vMat, vMat, Size(width, height));

			// Merge YUV components to create a full YUV image
			Mat yuvImg;
			vector<Mat> yuvPlanes = { yMat, uMat, vMat };
			merge(yuvPlanes, yuvImg);

			// Convert YUV image to BGR format
			Mat bgrImg;
			cvtColor(yuvImg, bgrImg, COLOR_YUV2BGR);

			// 将第一帧YUV NV12转换为灰度图像
			cvtColor(bgrImg, m_matFrameGray1, COLOR_BGR2GRAY); // cv::COLOR_YUV2GRAY_NV12);

			m_bFrame1 = mxtrue;
			return mxtrue;
		}

		if( !m_bFrame2 )
		{
			logPrint(MX_LOG_DEBUG, "opencvIdentify set second frame...");

			Mat yMat(height, width, CV_8UC1, (void*)frameBuff);
			Mat uMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height));
			Mat vMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height + width * height / 4));

			// Resize U and V components to match Y component's dimensions
			resize(uMat, uMat, Size(width, height));
			resize(vMat, vMat, Size(width, height));

			// Merge YUV components to create a full YUV image
			Mat yuvImg;
			vector<Mat> yuvPlanes = { yMat, uMat, vMat };
			merge(yuvPlanes, yuvImg);

			// Convert YUV image to BGR format
			Mat bgrImg;
			cvtColor(yuvImg, bgrImg, COLOR_YUV2BGR);

			// 将第二帧YUV NV12转换为灰度图像
			cvtColor(bgrImg, m_matFrameGray2, COLOR_BGR2GRAY); // cv::COLOR_YUV2GRAY_NV12);

			m_bFrame2 = mxtrue;
			return mxtrue;
		}

		// 循环剩余帧并进行帧差计算
		// 打开当前帧YUV文件
		logPrint(MX_LOG_DEBUG, "opencvIdentify set current frame...");

		// 读取当前帧数据
		Mat yMat(height, width, CV_8UC1, (void*)frameBuff);
		Mat uMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height));
		Mat vMat(height / 2, width / 2, CV_8UC1, (void*)(frameBuff + width * height + width * height / 4));

		// Resize U and V components to match Y component's dimensions
		resize(uMat, uMat, Size(width, height));
		resize(vMat, vMat, Size(width, height));

		// Merge YUV components to create a full YUV image
		Mat yuvImg;
		vector<Mat> yuvPlanes = { yMat, uMat, vMat };
		merge(yuvPlanes, yuvImg);

		// Convert YUV image to BGR format
		Mat bgrImg;
		cvtColor(yuvImg, bgrImg, COLOR_YUV2BGR);

		// 将当前帧YUV NV12转换为灰度图像
		Mat matFrameGray;
		cvtColor(bgrImg, matFrameGray, COLOR_BGR2GRAY); // cv::COLOR_YUV2GRAY_NV12);

		// 计算三帧差
		Mat matFrameDiff, matFrameDiff1, matFrameDiff2;
		absdiff(m_matFrameGray2, m_matFrameGray1, matFrameDiff1);
		absdiff(matFrameGray, m_matFrameGray2, matFrameDiff2);
		matFrameDiff = matFrameDiff1 | matFrameDiff2;

		//计算
		Mat filteredFrameDiff;
		Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3)); // 使用 5x5 的矩形结构元素
		morphologyEx(matFrameDiff, filteredFrameDiff, MORPH_OPEN, kernel);

		int pixelCount = countNonZero(filteredFrameDiff > 50); // 统计像素个数，可调
		logPrint(MX_LOG_DEBUG, "opencvIdentify pixelCount: %d", pixelCount);
		if (pixelCount > 0)
		{
			m_iConsecutivePositiveDiffCount++;

			// 判断连续帧差是否都大于0
			if (m_iConsecutivePositiveDiffCount >= 1)	//此处改为1需要算法再确认
			{
				logPrint(MX_LOG_INFOR, "opencvIdentify Recognize changes in the screen!");
				bResult = mxtrue;
				return mxtrue;
			}
		}
		else
		{
			m_iConsecutivePositiveDiffCount = 0;
		}
		logPrint(MX_LOG_DEBUG, "opencvIdentify m_iConsecutivePositiveDiffCount :%d",m_iConsecutivePositiveDiffCount);

		// 更新前两帧
		m_matFrameGray1 = m_matFrameGray2;

		// 更新第二帧
		m_matFrameGray2 = matFrameGray;
		
		m_iCountframe++;
		
		return mxtrue;
	}
}
