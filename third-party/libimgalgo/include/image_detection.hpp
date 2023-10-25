#ifndef __IMG_DETECTION_HPP__
#define __IMG_DETECTION_HPP__
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include "dirent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>//ifstream读文件，ofstream写文件，fstream读写文件
#include <sys/time.h>
#include <chrono>
#include <math.h>
#include <iostream>
#include <memory.h>
#include <memory>
#include <mutex>
#include <functional>

#include "mi_alg_sdk.h"
#include "common_export.h"
#include "venus.h"
#include "post_process.hpp"

#ifdef MI_ALG_DETECT
#define AI_MODEL_PATH_A  "/etc/70mai/model/detectorA.json"    ///< 三物+包裹模型flash路径 A
#define AI_MODEL_PATH_B  "/etc/70mai/model/detectorB.json"    ///< 三物+包裹模型flash路径 B
#else
#define AI_MODEL_PATH_A  "/dev/mtd7"    ///< 三物+包裹模型flash路径 A
#define AI_MODEL_PATH_B  "/dev/mtd10"    ///< 三物+包裹模型flash路径 B
#endif

struct PixelOffset {
    int top;
    int bottom;
    int left;
    int right;
};

typedef enum 
{
	E_EVENT_TYPE_PERSON = 0,            ///<人形事件
	E_EVENT_TYPE_CAR,                   ///< 车辆事件-car
	E_EVENT_TYPE_BUS,                   ///< 车辆事件-bus
	E_EVENT_TYPE_CAT,                   ///< 宠物事件 -cat
    E_EVENT_TYPE_DOG,                   ///< 宠物事件 -dog
    E_EVENT_TYPE_PACKAGE,               ///< 包裹检测事件
    E_EVENT_TYPE_MAX
} E_EVENT_TYPE;

enum E_EVENT_DETAIL_TYPE
{
    E_EVENT_DETAIL_TYPE_PERSON_APPEAR = 0,  ///<人形出现
    E_EVENT_DETAIL_TYPE_PERSON_STAY,        ///<人形逗留
    E_EVENT_DETAIL_TYPE_ANIMAL_APPEAR,      ///<宠物出现
    // E_EVENT_DETAIL_TYPE_ANIMAL_STAY,        ///<宠物逗留
    E_EVENT_DETAIL_TYPE_CAR_APPEAR,         ///<车辆出现
    E_EVENT_DETAIL_TYPE_CAR_STAY,           ///<车辆逗留
    E_EVENT_DETAIL_TYPE_PACKAGE_ENTER,      ///<包裹进入
    E_EVENT_DETAIL_TYPE_PACKAGE_MOVE,       ///<包裹移动
    E_EVENT_DETAIL_TYPE_PACKAGE_LEAVE,      ///<包裹离开
    E_EVENT_DETAIL_TYPE_MAX
};

class CImgDetection
{
public:
    CImgDetection();
    ~CImgDetection();

#ifdef MI_ALG_DETECT
    bool init(const char *model_path);
#else
    bool init(void);
    bool loadBinModel(std::string strPath);
#endif

    bool unInit(void);
    void set_cpu(int id);
    bool setStayTime(int iStayTime);
    bool setRecognitionArea(std::vector<std::vector<float>> bounds);
    int procFrameData(unsigned char *nv12_data, int iDataSize, int iType);
    int procFilterResult(std::vector<ObjectBox> &vecPersonList, int type);
    void setObj(void* obj);

private:
    bool m_Cvtbgra;

#ifdef MI_ALG_DETECT
    void *m_pModelHandle;
    void *m_pAlgHandle;
    int  m_iFrameID;
#else
    void writeOutputBin(const float* out_ptr, int size);
    void transCoords(std::vector<ObjectBox> &in_boxes, PixelOffset &pixel_offset,float scale);
    void checkPixelOffset(PixelOffset &pixel_offset);
    std::unique_ptr<venus::BaseNet> m_ODNet;
    // std::unique_ptr<venus::BaseNet> m_ParcelNet;
#endif

};

#endif //__IMG_DETECTION_HPP__