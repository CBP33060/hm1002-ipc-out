#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "log_mx.h"
#include "ai_rapid_detection.h"
#include "ai_manage_module.h"

using namespace cv;

namespace maix {
	CAIRapidDetection::CAIRapidDetection(CModule * module, std::string strName)
		: m_module(module)
		, m_strName(strName)
	{
	}

	CAIRapidDetection::~CAIRapidDetection()
	{
	}

	mxbool CAIRapidDetection::init()
	{
		if (!m_objPacketQueue.init(3, 1000))
			return mxfalse;

		return mxtrue;
	}

	mxbool CAIRapidDetection::unInit()
	{
		return mxtrue;
	}

	void CAIRapidDetection::run()
	{
		mxbool bResult = mxfalse;
		int iPacketHeaderLen = sizeof(T_MediaFramePacketHeader);
		CAIManageModule *module = dynamic_cast<CAIManageModule *>(m_module);

		while (1)
		{
			std::shared_ptr<CMediaFramePacket> packet = NULL;
			popFrameData(packet);

			if (!packet)
			{
#ifdef	WIN32
				Sleep(500);
#else
				usleep(1000*500);
#endif
				continue;
			}

			if (packet->getPacketType() == E_P_VIDEO_YUV)
			{
				opencvIdentify(packet->getFrameData()+iPacketHeaderLen,  packet->getFrameDataLen()-iPacketHeaderLen, bResult);
				if(bResult)
				{
					module->openAIDetect();
					return;//此线程退出,开启大模型
				}
			}
		}
	}

	mxbool CAIRapidDetection::opencvIdentify(unsigned char* frameBuff, int frameSize, mxbool &bResult)
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

			// 将第一帧YUV NV12转换为灰度图像
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

		int pixelCount = countNonZero(filteredFrameDiff > 8); // 统计像素个数，可调
		logPrint(MX_LOG_DEBUG, "opencvIdentify pixelCount: %d", pixelCount);
		if (pixelCount > 0)
		{
			m_iConsecutivePositiveDiffCount++;

			// 判断连续帧差是否都大于0
			if (m_iConsecutivePositiveDiffCount >= 4)
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

	bool CAIRapidDetection::pushFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		return  m_objPacketQueue.push(packet);
	}

	void CAIRapidDetection::popFrameData(std::shared_ptr<CMediaFramePacket>& packet)
	{
		m_objPacketQueue.pop(packet);
	}

}
