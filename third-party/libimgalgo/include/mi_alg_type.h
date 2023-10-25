/*******************************************************************************
 *
 *
 *file     alg_type.h
 *brief    算法SDK类型定义
 *
 *author   wangningbo
 *version  1.0.0
 *date     05/12/2021
 *
 *history  1.0.0 05/12/2021, wangningbo, Create file.
 *
 *Copyright (c) 2020 Beijing Xiaomi Mobile Software Co., Ltd. All rights reserved.
 *
 *******************************************************************************/
#ifndef _MI_ALG_TYPE_H_
#define _MI_ALG_TYPE_H_ 

#include <stdint.h>
#include "mi_alg_error.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define MI_ALG_ID_MAX_LEN           64                  /*算法ID最大长度*/
#define MI_ALG_MODEL_MAX_NUM        6                   /*模型最大个数*/




/**********************************算法SDK输入***********************************/

/*数据格式*/
typedef enum{
    MI_ALG_DATA_FORMAT_START = 0,
    MI_ALG_DATA_FORMAT_NV12 = 1,                        /*YYYYYYYY UVUV*/
    MI_ALG_DATA_FORMAT_NV21 = 2,                        /*YYYYYYYY VUVU*/
    MI_ALG_DATA_FORMAT_YV12 = 3,                        /*YYYYYYYY VV UU*/
    MI_ALG_DATA_FORMAT_I420 = 4,                        /*YYYYYYYY UU VV*/
    MI_ALG_DATA_FORMAT_JPEG = 5,                        /*JPEG*/
    MI_ALG_DATA_FORMAT_H264 = 6,                        /*H.264裸流*/
    MI_ALG_DATA_FORMAT_H265 = 7,                        /*H.265裸流*/
    MI_ALG_DATA_FORMAT_RGB = 8,                         /*RGB 默认RGB888*/
    MI_ALG_DATA_FORMAT_RGB565 = 9,                      /*RGB565*/
    MI_ALG_DATA_FORMAT_BGR = 10,                        /*BGR 默认BGR888*/
    MI_ALG_DATA_FORMAT_PCM = 11,                        /*音频-PCM*/
    MI_ALG_DATA_FORMAT_ADPCM = 12,                      /*音频-ADPCM*/
    MI_ALG_DATA_FORMAT_G711U = 13,                      /*音频-G711U*/
    MI_ALG_DATA_FORMAT_G711A = 14,                      /*音频-G711A*/
    MI_ALG_DATA_FORMAT_G726 = 15,                       /*音频-G726*/
    MI_ALG_DATA_FORMAT_MP3 = 16,                        /*音频-MP3*/
    MI_ALG_DATA_FORMAT_AAC = 17,                        /*音频-AAC*/
    MI_ALG_DATA_FORMAT_SPEEX = 18,                      /*音频-SPEEX*/
    MI_ALG_DATA_FORMAT_OPUS = 19,                       /*音频-OPUS*/
    MI_ALG_DATA_FORMAT_END,
}mi_alg_data_format_t;

/*YUV数据*/
typedef struct{
    int32_t height;                                     /*宽度*/
    int32_t width;                                      /*高度*/
    int32_t stride[3];                                  /*跨距 0->Y分量 1->U分量 2->V分量*/
    uint8_t *p_vir[3];                                  /*虚拟地址 0->Y分量 1->U分量 2->V分量*/
    uint8_t *p_phy[3];                                  /*物理地址 0->Y分量 1->U分量 2->V分量*/
}mi_alg_yuv_info_t;

/*JPEG/裸流数据*/
typedef struct{
    int32_t height;                                     /*宽度*/
    int32_t width;                                      /*高度*/
    int32_t data_len;                                   /*数据长度*/
    uint8_t *p_data;                                    /*数据地址*/
    uint8_t res[8];
}mi_alg_jpeg_info_t;

/*音频信息*/
typedef enum{
    MI_ALG_AUDIO_TRACK_START = 0,
    MI_ALG_AUDIO_TRACK_MONO = 1,                        /*单声道*/
    MI_ALG_AUDIO_TRACK_STEREO = 2,                      /*双声道*/
    MI_ALG_AUDIO_TRACK_2_1 = 3,                         /*2.1声道*/
    MI_ALG_AUDIO_TRACK_5_1 = 4,                         /*5.1声道*/
    MI_ALG_AUDIO_TRACK_7_1 = 5,                         /*7.1声道*/
    MI_ALG_AUDIO_TRACK_END,
}mi_alg_audio_track_t;

typedef struct {
    mi_alg_audio_track_t audio_track;                   /*音频声道类型*/
    uint32_t samplerate;                                /*音频采样率*/
    uint32_t samplebit;                                 /*音频采样位数*/
    uint32_t frmNum;                                    /*音频帧时长ms*/
    uint32_t numPerFrm;                                 /*音频帧长，每帧采样点数*/
    uint8_t *p_data;                                    /*数据地址*/
    int32_t data_len;                                   /*数据长度*/
    uint8_t res[16];
}mi_alg_audio_info_t;


/*数据信息*/
typedef union{
    mi_alg_yuv_info_t yuv;
    mi_alg_jpeg_info_t jpeg;
    mi_alg_jpeg_info_t stream;
    mi_alg_audio_info_t audio;
}mi_alg_data_info_t;


/**********************************点 框等通用属性***********************************/
/*点 2D 浮点型*/
typedef struct{
    float x;
    float y;
    float score;                                        /*置信度*/
}mi_alg_point_F_t;

/*点 2D 整型*/
typedef struct{
    int32_t x;
    int32_t y;
    float score;                                        /*置信度*/
}mi_alg_point_I_t;

/*点 3D 浮点型*/
typedef struct{
    float x;
    float y;
    float z;
    float score;                                        /*置信度*/
}mi_alg_point_3D_F_t;

/*点 3D 整型*/
typedef struct{
    int32_t x;
    int32_t y;
    int32_t z;
    float score;                                        /*置信度*/
}mi_alg_point_3D_I_t;

/*矩形框 浮点型*/
typedef struct{
    float x1;                                           /*左上角*/
    float y1;                                           /*左上角*/
    float x2;                                           /*右下角*/
    float y2;                                           /*右下角*/
    float score;                                        /*置信度*/
}mi_alg_rectangle_F_t;

/*矩形框 整型*/
typedef struct{
    int32_t x1;                                         /*左上角*/
    int32_t y1;                                         /*左上角*/
    int32_t x2;                                         /*右下角*/
    int32_t y2;                                         /*右下角*/
    float score;                                        /*置信度*/
}mi_alg_rectangle_I_t;

/*多边形 浮点型*/
typedef struct{    
    int32_t num;                                        /*关键点数量*/
    mi_alg_point_F_t *p_points;                         /*关键点坐标 指向浮点坐标数组的指针*/
    float score;                                        /*置信度*/
}mi_alg_polygon_F_t;

/*多边形 整型*/
typedef struct{
    int32_t num;                                        /*关键点数量*/
    mi_alg_point_I_t *p_points;                         /*关键点坐标 指向浮点坐标数组的指针*/
    float score;                                        /*置信度*/
}mi_alg_polygon_I_t;

/*通用属性结果*/
typedef struct{
    int32_t value;                                      /*值*/
    float score;                                        /*置信度*/
}mi_alg_attribute_t;

/*通用数组 浮点型*/
typedef struct{
    int32_t num;                                        /*数量*/
    float *p_values;                                    /*数组 指向浮点数组的指针*/
}mi_alg_array_F_t;

/*光照模式*/
typedef enum{
    MI_ALG_LIGHT_MODE_START = 0,
    MI_ALG_LIGHT_MODE_DAY = 1,                          /*白天*/
    MI_ALG_LIGHT_MODE_SUNSET = 2,                       /*傍晚*/
    MI_ALG_LIGHT_MODE_NIGHT = 3,                        /*夜晚*/
    MI_ALG_LIGHT_MODE_END,
} mi_alg_light_mode_t;

/*灵敏度模式*/
typedef enum{
    MI_ALG_SENSITIVITY_LEVEL_START = 0,
    MI_ALG_SENSITIVITY_LEVEL_LOW = 1,                   /*灵敏度-低*/
    MI_ALG_SENSITIVITY_LEVEL_MEDIUM = 2,                /*灵敏度-中*/
    MI_ALG_SENSITIVITY_LEVEL_HIGH = 3,                  /*灵敏度-高*/
    MI_ALG_SENSITIVITY_LEVEL_END,
} mi_alg_sensitivity_level_t;


/*转动角度（度）*/
typedef struct{
    float h_degree;                                     /*水平角度*/
    float v_degree;                                     /*垂直角度*/
}mi_alg_rotation_angle_F_t;


/*通用区域属性*/
typedef struct{
    int32_t w;                                          /*宽/列数*/
    int32_t h;                                          /*高/行数*/
    float *p_values;                                    /*值,w*h个,Z字型对应*/
}mi_alg_area_attr_t;


/**********************************移动侦测+移动追踪结果*************************************/
typedef struct{    
    int valid;                                          /*是否有效，0 无效，其他有效*/
    mi_alg_rectangle_F_t track_box;                     /*跟踪目标框*/ 
    mi_alg_rotation_angle_F_t motor_angle;              /*电机转动角*/
}mi_alg_md_track_info_t;

typedef struct{    
    int32_t motion;                                     /*非0表示画面变动，按位从低到高，分别对应32个区域（8*4=32块区域，Z字型0-31）的变动情况*/
    mi_alg_area_attr_t area_motion;                     /*区域变动结果，原图划分成w*h块区域*/
    mi_alg_md_track_info_t track_info;                  /*跟踪信息 注意判断valid*/    
}mi_alg_md_result_t;



/**********************************目标检测结果(包含音视频)***********************************/

/*目标类别*/
typedef enum{
    MI_ALG_OBJ_START = 0,
    MI_ALG_OBJ_PERSON = 1,                              /*人形*/
    MI_ALG_OBJ_FACE = 2,                                /*人脸*/
    MI_ALG_OBJ_DOG = 3,                                 /*狗*/
    MI_ALG_OBJ_CAT = 4,                                 /*猫*/
    MI_ALG_OBJ_CHAIR = 5,                               /*椅子*/    
    MI_ALG_OBJ_TV = 6,                                  /*电视机*/
    MI_ALG_OBJ_POTTING = 7,                             /*盆栽植物*/
    MI_ALG_OBJ_GESTURE_OK = 8,                          /*手势类型-OK*/
    MI_ALG_OBJ_GESTURE_PALM = 9,                        /*手势类型-手掌*/
    MI_ALG_OBJ_GESTURE_YEAH = 10,                       /*手势类型-剪刀*/
    MI_ALG_OBJ_GESTURE_CALL = 11,                       /*手势类型-call*/
    MI_ALG_OBJ_GESTURE_FIST = 12,                       /*手势类型-拳头*/
    MI_ALG_OBJ_GESTURE_L_GUN = 13,                      /*手势类型-左手枪*/
    MI_ALG_OBJ_GESTURE_R_GUN = 14,                      /*手势类型-右手枪*/
    MI_ALG_OBJ_CRY = 15,                                /*声音识别-哭声*/
    MI_ALG_OBJ_BARK = 16,                               /*声音识别-犬吠*/
    MI_ALG_OBJ_MEOW = 17,                               /*声音识别-猫叫*/
    MI_ALG_OBJ_CAR = 18,                                /*小汽车*/
    MI_ALG_OBJ_BUS = 19,                                /*大巴车*/
    MI_ALG_OBJ_TRUCK = 20,                              /*卡车*/
    MI_ALG_OBJ_BICYCLE = 21,                            /*自行车*/
    MI_ALG_OBJ_ELECTRIC_BICYCLE = 22,                   /*电动自行车*/
    MI_ALG_OBJ_MOTORBIKE = 23,                          /*摩托车*/
    MI_ALG_OBJ_END,
}mi_alg_obj_type_t;

/*目标信息*/
typedef struct{    
    int32_t id;                                         /*目标id*/
    float score;                                        /*目标置信度*/
    mi_alg_obj_type_t type;                             /*目标类别*/
    mi_alg_rectangle_F_t box;                           /*目标框*/
    mi_alg_polygon_F_t *p_keypoints;                    /*关键点*/    
}mi_alg_obj_t;

/*目标信息，适用于图像、音频识别/检测*/
typedef struct{
    int32_t num;                                        /*目标数*/
    mi_alg_obj_t *p_objs;                               /*目标 指向目标信息数组的指针*/
}mi_alg_obj_info_t;



/**********************************人脸识别结果***********************************/

/*3D 人脸姿态*/
typedef struct{
    float pitch;                                        /*上下 俯仰角*/
    float yaw;                                          /*左右 偏航角*/
    float roll;                                         /*翻滚角*/
    float score;                                        /*置信度*/
}mi_alg_pose_3D_t;

/*年龄*/
typedef struct{
    int32_t value;                                      /*年龄*/
    int32_t min;                                        /*年龄段下限*/
    int32_t max;                                        /*年龄段上限*/
    float score;                                        /*置信度*/
}mi_alg_age_t;

/*人脸质量*/
typedef struct{
    mi_alg_attribute_t blur;                            /*人脸清晰度*/
    mi_alg_attribute_t brightness;                      /*人脸亮度*/
    mi_alg_attribute_t eye_abnormalities;               /*眼睛*/
    mi_alg_attribute_t mouth_abnormal;                  /*嘴巴*/
    mi_alg_attribute_t left_eye;                        /*人脸清晰度*/
    mi_alg_attribute_t right_eye;                       /*人脸清晰度*/
    mi_alg_attribute_t left_brow;                       /*人脸清晰度*/
    mi_alg_attribute_t right_brow;                      /*人脸清晰度*/
    mi_alg_attribute_t forehead;                        /*人脸清晰度*/
    mi_alg_attribute_t left_cheek;                      /*人脸清晰度*/
    mi_alg_attribute_t right_cheek;                     /*人脸清晰度*/
    mi_alg_attribute_t nose;                            /*人脸清晰度*/
    mi_alg_attribute_t mouth;                           /*人脸清晰度*/
    mi_alg_attribute_t jaw;                             /*人脸清晰度*/
}mi_alg_face_quality_t;

/*人脸信息*/
typedef struct{
    int32_t id;                                         /*人脸id*/
    mi_alg_rectangle_F_t face_rect;                     /*人脸框*/
    mi_alg_rectangle_F_t head_rect;                     /*人头框*/
    mi_alg_pose_3D_t pose3d;                            /*人脸姿态*/
    mi_alg_array_F_t landmarks;                         /*人脸关键点*/
    mi_alg_age_t age;                                   /*年龄*/
    mi_alg_attribute_t gender;                          /*性别*/
    mi_alg_attribute_t glass;                           /*眼镜*/
    mi_alg_attribute_t mask;                            /*口罩*/
    mi_alg_attribute_t anti_spoofing;                   /*活体信息*/
    mi_alg_face_quality_t quality;                      /*人脸质量*/
    mi_alg_array_F_t feature;                           /*人脸特征*/
}mi_alg_face_info_t;


/*人脸比对结果*/
typedef struct{
    uint64_t face_id;                                   /*人脸比对id 默认0，未匹配到*/
    uint64_t image_id;                                  /*图片索引 一人多图*/
    float score;                                        /*置信度*/
}mi_alg_face_cmp_result_t;

/*人脸注册结果*/
typedef struct{
    uint64_t face_id;                                   /*人脸id 透传外部传入*/
    uint64_t image_id;                                  /*图片id，透传外部传入*/
    mi_alg_array_F_t feature;                           /*注册图片提取的人脸特征*/
}mi_alg_face_register_result_t, mi_alg_face_cmp_info_t;


/*抓拍类型*/
typedef enum{
    MI_ALG_SNAPSHOT_START = 0,
    MI_ALG_SNAPSHOT_DISAPPEAR = 1,                      /*目标消失触发抓拍*/
    MI_ALG_SNAPSHOT_BEST = 2,                           /*最优目标触发抓拍*/
    MI_ALG_SNAPSHOT_END,
}mi_alg_snapshot_type_t;

/*抓拍图信息*/
typedef struct{
    uint64_t time_stamp;                                /*时间戳 跟输入一致*/
    mi_alg_data_format_t format;                        /*数据类型 一般要求NV12*/
    mi_alg_data_info_t img;                             /*数据信息*/
}mi_alg_snapshot_img_t;

/*抓拍信息*/
typedef struct{    
    int32_t id;                                         /*目标id*/
    int32_t snapshot_num;                               /*抓拍图张数*/   
    mi_alg_snapshot_img_t *p_imgs;                      /*抓拍图信息*/
    mi_alg_snapshot_type_t type;                        /*抓拍类型*/
    mi_alg_face_info_t face_info;                       /*人脸信息*/
}mi_alg_snapshot_info_t;

/*人脸识别结果*/
typedef struct{
    int32_t face_num;                                   /*人脸信息数量*/        
    mi_alg_face_info_t *p_face_info;                    /*人脸信息 一般情况下，每帧均有，帧信息*/
    int32_t snapshot_num;                               /*人脸抓拍数*/
    mi_alg_snapshot_info_t *p_snapshot_result;          /*人脸抓拍结果*/
    int32_t cmp_num;                                    /*人脸比对结果数*/
    mi_alg_face_cmp_result_t *p_cmp_result;             /*人脸比对结果*/
    int32_t reg_num;                                    /*人脸注册结果数*/
    mi_alg_face_register_result_t *p_reg_result;        /*人脸注册结果*/
}mi_alg_face_result_t;


/**********************************算法SDK输出***********************************/

/*算法结果类型 可按位与*/
typedef enum{
    MI_ALG_EVENT_START = 0,
    MI_ALG_EVENT_OBJ_DETECT = 0x1 << 0,                 /*目标检测结果*/
    MI_ALG_EVENT_FACE_RECOG = 0x1 << 1,                 /*人脸特征结果*/
    MI_ALG_EVENT_FACE_REG = 0x1 << 2,                   /*人脸注册结果*/
    MI_ALG_EVENT_FACE_CMP = 0x1 << 3,                   /*人脸比对结果*/
    MI_ALG_EVENT_MOTION_DETECT = 0x1 << 4,              /*移动侦测结果*/
    MI_ALG_EVENT_GESTURE_REG = 0x1 << 5,                /*手势识别结果*/
    MI_ALG_EVENT_AUDIO_DETECT = 0x1 << 6,               /*声音识别结果*/
    MI_ALG_EVENT_END,
}mi_alg_event_type_t;

/*算法分析结果*/
typedef struct{
    mi_alg_event_type_t type;                           /*算法结果类型*/
    mi_alg_obj_info_t detect_result;                    /*目标检测结果*/
    mi_alg_face_result_t face_result;                   /*人脸识别结果（每帧+抓拍）*/
    mi_alg_md_result_t motion_result;                   /*移动侦测结果（包含移动追踪）*/
    mi_alg_obj_info_t gesture_result;                   /*手势检测结果*/
    mi_alg_obj_info_t audio_detect_result;              /*声音检测结果*/
}mi_alg_result_t;

/*算法输入*/
typedef struct{
    int32_t chan;                                       /*通道号*/
    int32_t frame_id;                                   /*帧号*/
    uint64_t time_stamp;                                /*时间戳*/
    mi_alg_data_format_t format;                        /*输入数据类型*/
    mi_alg_data_info_t data_info;                       /*数据信息*/
    mi_alg_result_t alg_info;                           /*其他算法分析结果，用于算法串联使用*/
}mi_alg_input_param_t;


/*算法输出*/
typedef struct {    
    mi_alg_input_param_t alg_input;                     /*原始输入信息*/
    mi_alg_result_t alg_result;                         /*算法分析结果*/
}mi_alg_output;

/**********************************算法SDK配置***********************************/

/*算法模型参数*/
typedef struct{
    const uint8_t alg_id[MI_ALG_ID_MAX_LEN];            /*算法唯一标识，采用动态库名称*/
    int32_t model_num;                                  /*模型个数*/
    uint8_t *p_model_path[MI_ALG_MODEL_MAX_NUM];        /*模型路径*/
    int32_t path_len[MI_ALG_MODEL_MAX_NUM];             /*模型路径长度*/    
}mi_alg_model_param;

/*算法初始化参数*/
typedef struct {
    void *p_model_handle;                               /*算法模型句柄*/
    mi_alg_data_format_t format;                        /*算法分析数据类型*/
    union{
        struct{
            int32_t width;                              /*图像宽度*/
            int32_t height;                             /*图像高度*/
            int32_t fps;                                /*帧率*/
            int32_t res[3];
        }video;
        struct{
            mi_alg_audio_track_t audio_track;           /*音频声道类型*/
            uint32_t samplerate;                        /*音频采样率*/
            uint32_t samplebit;                         /*音频采样位数*/
            uint32_t frmNum;                            /*音频帧时长(ms)*/
            uint32_t numPerFrm;                         /*音频帧长(采样点数)*/
            int32_t step;                               /*音频滑动步长，单位:秒(s)*/
            int32_t skip_num;                           /*音频跳帧数*/
            int32_t res[3];
        }audio;
    };
}mi_alg_init_param_t;

/*算法配置类型*/
typedef enum{
    MI_ALG_SET_START = 0,
    MI_ALG_SET_CALLBACK = 1,                            /*设置回调*/
    MI_ALG_REGISTER_FACE = 2,                           /*注册人脸*/
    MI_ALG_UNREGISTER_FACE = 3,                         /*反注册人脸*/
    MI_ALG_SET_DETECT = 4,                              /*设置目标检测*/
    MI_ALG_SET_MOTION_DETECT = 5,                       /*设置移动侦测*/
    MI_ALG_SET_GESTURE = 6,                             /*设置手势识别*/
    MI_ALG_SET_AUDIO_DETECT = 7,                        /*设置声音识别*/
    MI_ALG_SET_END,
}mi_alg_set_mode_t;

/*算法回调函数*/
typedef int (* mi_alg_callback_t)(void *p_alg_handle, mi_alg_output *p_output);

/*移动侦测参数*/
typedef struct {
     mi_alg_polygon_F_t roi;                            /*算法分析区域*/
     mi_alg_area_attr_t threshold;                      /*区域阈值，范围0-255，值越小越灵敏*/
}mi_alg_MD_param_t;

/*目标检测参数-视觉*/
typedef struct {
    mi_alg_light_mode_t mode;                           /*光照模式*/
    mi_alg_polygon_F_t roi;                             /*算法分析区域*/ 
}mi_alg_detect_param_t;

/*声音识别参数*/
typedef struct {
    mi_alg_sensitivity_level_t level;                   /*灵敏度等级*/
    uint8_t res[16];
}mi_alg_audio_detect_param_t;

/*人脸注册信息参数*/
typedef struct {
    uint64_t face_id;                                   /*人脸ID*/
    uint64_t image_id;                                  /*图片索引 一人多图*/
    mi_alg_data_format_t format;                        /*输入数据类型*/
    mi_alg_data_info_t img_info;                        /*人脸图片*/
    mi_alg_array_F_t feature;                           /*人脸特征，外部传入，无人脸图片时使用*/
}mi_alg_face_reg_info_t;

typedef struct {
    mi_alg_face_reg_info_t register_info;               /*人脸注册信息*/
}mi_alg_face_param_t;



/*适配层 算法接口类型*/
typedef mi_alg_error_t (* mi_alg_model_load_t)(void **p_alg_model_handle, mi_alg_model_param *p_param);
typedef mi_alg_error_t (* mi_alg_model_unload_t)(void *p_alg_model_handle);
typedef const char *(* mi_alg_model_version_get_t)(void *p_model_handle);
typedef mi_alg_error_t (* mi_alg_handle_create_t)(void **p_alg_handle, mi_alg_init_param_t *p_alg_param);
typedef mi_alg_error_t (* mi_alg_handle_destroy_t)(void *p_alg_handle);
typedef mi_alg_error_t (* mi_alg_param_set_t)(void *p_alg_handle, mi_alg_set_mode_t mode, void *p_param);
typedef mi_alg_error_t (* mi_alg_param_get_t)(void *p_alg_handle, mi_alg_set_mode_t mode, void *p_param);
typedef mi_alg_error_t (* mi_alg_input_t)(void *p_alg_handle, mi_alg_input_param_t *input);
typedef const char *(* mi_alg_version_get_t)(void *p_alg_handle);

/*此结构体用于扫描动态库中必备接口使用*/
typedef struct{
    mi_alg_model_load_t p_fun_model_load;
    mi_alg_model_unload_t p_fun_model_unload;
    mi_alg_model_version_get_t p_fun_model_version_get;
    mi_alg_handle_create_t p_fun_handle_create;
    mi_alg_handle_destroy_t p_fun_handle_destroy;
    mi_alg_param_set_t p_fun_param_set;
    mi_alg_param_get_t p_fun_param_get;
    mi_alg_input_t p_fun_input;
    mi_alg_version_get_t p_fun_version_get;
}mi_alg_essential_interface_info_t;

   
#ifdef __cplusplus
    }
#endif
#endif





