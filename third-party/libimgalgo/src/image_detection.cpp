#include "image_detection.hpp"
#include "post_process_event.h"
#include "detector_result.h"

#define TIME
#ifdef TIME
#include <sys/time.h>
#endif

#define YUV_FRAME_WIDTH     832
#define YUV_FRAME_HEIGHT    480

#define NET_IN_WIDTH        832
#define NET_IN_HEIGHT       480

using namespace std;
using namespace post_process_event;

PostProcessEvent g_post_process_event;
CAIDetect *objAIDetect;


#ifdef MI_ALG_DETECT ////////>> XIAOMI

#define MI_ALG_SDK       "libmi_alg_t41_detector_sdk.so"
// std::ofstream outf;
#define ROI_POINTS 2

CImgDetection::CImgDetection()
{
    m_Cvtbgra = true;
	m_pModelHandle = NULL;
	m_pAlgHandle = NULL;
	m_iFrameID = 0;
}

CImgDetection::~CImgDetection()
{

}

/*
回调结果里面:
obj.type表示类别，0表示人形，1表示包裹，2表示宠物，3表示车辆
obj.score表示置信度分数
obj.box.x1, obj.box.y1, obj.box.x2, obj.box.y2表示框坐标
调用分辨率：640*384    帧率：5
*/
int mi_alg_callback(void *m_pAlgHandle, mi_alg_output *p_output) 
{
	if ((NULL == m_pAlgHandle) || (NULL == p_output)) 
	{
		printf("not correct alg result, m_pAlgHandle=%p, p_output=%p\n",
		          m_pAlgHandle, p_output);
		return MI_ALG_FAILURE;
	}

	mi_alg_obj_info_t detect_result = p_output->alg_result.detect_result;
	//printf("get alg result\n");
	//printf("alg_input: chan=%d, frame_id=%d, time_stamp=%llu\n",
	//         p_output->alg_input.chan, p_output->alg_input.frame_id, p_output->alg_input.time_stamp);
	// outf << "image_id=" << p_output->alg_input.frame_id << std::endl;
	//printf("alg_output:  frame_id=%d, num_objs=%d \n", p_output->alg_input.frame_id, detect_result.num);
	
	struct timeval now_time_stamp;
	gettimeofday(&now_time_stamp, NULL);
	u_int64_t end = 1000000 * now_time_stamp.tv_sec + now_time_stamp.tv_usec;
	u_int64_t duration = end - p_output->alg_input.time_stamp;
	//printf("all duration: %dms\n", int(duration/1000));

	vector<ObjectBox> vecFilterList;
    vecFilterList.clear();

	for (int i = 0; i < detect_result.num; ++i) 
	{
		mi_alg_obj_t obj = detect_result.p_objs[i];
		//printf("obj:id=%d type=%d ,score=%3f ,x1=%3f ,x2=%3f ,y1=%3f ,y2=%3f \n", obj.id, obj.type, obj.box.score,
		//         obj.box.x1, obj.box.x2, obj.box.y1, obj.box.y2);
        // outf << p_output->alg_input.time_stamp<< ":"<< obj.id << "," << obj.type << "," << obj.score  << "," << obj.box.x1 << "," << obj.box.y1 << ","
		//      << obj.box.x2 << "," << obj.box.y2 << std::endl;

		ObjectBox person;
		person.x1 = obj.box.x1 / YUV_FRAME_WIDTH;
		person.y1 = obj.box.y1 / YUV_FRAME_HEIGHT;
		person.x2 = obj.box.x2 / YUV_FRAME_WIDTH;
		person.y2 = obj.box.y2 / YUV_FRAME_HEIGHT;
		person.score = obj.box.score;
		person.classid = obj.type;

		vecFilterList.push_back(person); 
	}

    //printf("vecFilterList.size=[%d]\n",vecFilterList.size());
    int iDetectResult = g_post_process_event.detectEvent(vecFilterList);
	//printf("detectEvent iDetectResult=[%d]\n",iDetectResult);

	objAIDetect->returnDetectResult(iDetectResult);
	return MI_ALG_SUCCESS;
}

void CImgDetection::setObj(void* obj) {
    objAIDetect = static_cast<CAIDetect*>(obj);
}

bool CImgDetection::init(const char *model_path)
{
    int ret = MI_ALG_SUCCESS;
	
	const char *p_version = NULL;
	mi_alg_model_param model_param = {0};
	mi_alg_init_param_t init_param;
	mi_alg_detect_param_t detect_param;
    mi_alg_point_F_t points[ROI_POINTS];

    set_cpu(0);

#ifdef TIME
    struct timeval tv; 
    uint64_t time_last;
    double time_ms;
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec;
#endif

	/*1.模型加载*/
	memset(&model_param, 0, sizeof(model_param));
	strncpy((char *) model_param.alg_id, MI_ALG_SDK, MI_ALG_ID_MAX_LEN);
	model_param.model_num = 1;

	model_param.p_model_path[0] = (unsigned char *) model_path;
	printf("p_model_path [0]:%s \n", (const char *) model_param.p_model_path[0]);

	ret = mi_alg_model_load(&m_pModelHandle, &model_param);
	if (MI_ALG_SUCCESS != ret) {
		printf("load model failure ret:%d, alg_id:%s model_num=%d\n",
		          ret, model_param.alg_id, model_param.model_num);
		return false;
	}
	printf("1. load model success, m_pModelHandle=%p\n", m_pModelHandle);

#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
    time_ms = time_last*1.0/1000;
    printf("load bin model time_ms: %fms\n", time_ms);
#endif

	/*2.获取模型版本信息*/
	p_version = mi_alg_model_version_get(m_pModelHandle);
	if (NULL == p_version) {
		printf("get model version failure, m_pModelHandle=%p\n", m_pModelHandle);
		return false;
	}
	printf("2. model version:%s\n", p_version);

	/*3.算法句柄创建*/
	memset(&init_param, 0, sizeof(init_param));
	init_param.format = MI_ALG_DATA_FORMAT_NV12;
	init_param.video.fps = 15;
	init_param.video.width = YUV_FRAME_WIDTH;
	init_param.video.height = YUV_FRAME_HEIGHT;
	init_param.p_model_handle = m_pModelHandle;
	ret = mi_alg_handle_create(&m_pAlgHandle, &init_param);
	if (MI_ALG_SUCCESS != ret) {
		printf("create alg handle failure ret:%d, format=%d fps=%d width=%d, height=%d, m_pModelHandle=%p\n",
		          ret, init_param.format, init_param.video.fps, init_param.video.width,
		          init_param.video.height, init_param.p_model_handle);
		return false;
	}
	printf("3. create alg handle success, m_pAlgHandle=%p\n", m_pAlgHandle);

	/*4.获取算法版本信息*/
	p_version = mi_alg_version_get(m_pAlgHandle);
	if (NULL == p_version) {
		printf("get alg version failure, m_pAlgHandle=%p\n", m_pAlgHandle);
		return MI_ALG_FAILURE;
	}
	printf("4. alg version:%s\n", p_version);

	/*5.1 回调函数配置 */
	ret = mi_alg_param_set(m_pAlgHandle, MI_ALG_SET_CALLBACK, (void *) mi_alg_callback);
	if (MI_ALG_SUCCESS != ret) {
		printf("set alg callback function failure ret:%d, m_pAlgHandle=%p\n", ret, m_pAlgHandle);
		return false;
	}
	printf("5.1 set alg callback function success\n");

    /*5.2 检测区域配置，不设置时，则为全图，当前只支持矩形框输入（左上坐标点+右下坐标点）*/  /*现在不支持，外部处理*/
	memset(&detect_param, 0, sizeof(detect_param));
    detect_param.roi.num = 2;

    points[0].x = 0;
    points[0].y = 0;
    points[1].x = init_param.video.width;
    points[1].y = init_param.video.height;

    detect_param.roi.p_points = points;

    ret = mi_alg_param_set(m_pAlgHandle, MI_ALG_SET_DETECT, (void *) &detect_param);
    if (MI_ALG_SUCCESS != ret) {
        printf("set detect roi success failure ret:%d, m_pAlgHandle=%p\n", ret, m_pAlgHandle);
        return false;
    }
    printf("5.2 set detect roi success\n");

    g_post_process_event.init();
    return true;
}

// setup the cpu set of this program to run on
void CImgDetection::set_cpu(int id) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(id, &mask);
	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		fprintf(stderr, "warning: could not set CPU affinity/n");
	}
}

bool CImgDetection::setStayTime(int iStayTime)
{
    g_post_process_event.setPersonStayTime(iStayTime);
    g_post_process_event.setCarStayTime(iStayTime);
    return true;
}

bool CImgDetection::setRecognitionArea(std::vector<std::vector<float>> bounds)
{
	g_post_process_event.setRecognitionBounds(bounds);
    return true;
}

bool CImgDetection::unInit(void)
{
    g_post_process_event.uninit();

    /*7.销毁算法句柄*/
	int ret = mi_alg_handle_destroy(m_pAlgHandle);
	if (MI_ALG_SUCCESS != ret) {
		printf("destroy alg handle failure, m_pAlgHandle%p\n", m_pAlgHandle);
		return false;
	}

	m_pAlgHandle = NULL;
	printf("7. destroy alg handle success, m_pAlgHandle=%p\n", m_pAlgHandle);

	/*8.销毁模型句柄*/
	ret = mi_alg_model_unload(m_pModelHandle);
	if (MI_ALG_SUCCESS != ret) {
		printf("destroy model handle failure, m_pModelHandle%p\n", m_pModelHandle);
		return false;
	}

	m_pModelHandle = NULL;
	printf("8. destroy model handle success, m_pModelHandle=%p\n", m_pModelHandle);
    return true;
}

int CImgDetection::procFrameData(unsigned char *nv12_data, int iDataSize, int iType)
{
    /*6.输入数据*/
	//printf("-----------load nv12_data--------------------\n");
	// auto start = std::chrono::system_clock::now();
	
	int image_width = YUV_FRAME_WIDTH;
    int image_height = YUV_FRAME_HEIGHT;
	
	mi_alg_input_param_t input_param;
	memset(&input_param, 0, sizeof(mi_alg_input_param_t));
	/*输入信息需要跟初始化一致*/
	input_param.chan = 1;
	input_param.format = MI_ALG_DATA_FORMAT_NV12;
	// input_param.frame_id = (m_iFrameID++) * 2 + 1;
	input_param.frame_id = ++m_iFrameID;

	struct timeval time_now;
	gettimeofday(&time_now, NULL);
	u_int64_t time_stamp = 1000000 * time_now.tv_sec + time_now.tv_usec;

	input_param.time_stamp = time_stamp; // 时间戳格式
	input_param.data_info.yuv.width = image_width;
	input_param.data_info.yuv.height = image_height;

	for (int j = 0; j <= 2; ++j) {
		input_param.data_info.yuv.stride[j] = image_width;

		input_param.data_info.yuv.p_phy[j] = NULL;
	}
	input_param.data_info.yuv.p_vir[0] = (unsigned char *) nv12_data;

	input_param.data_info.yuv.p_vir[1] = (unsigned char *) (nv12_data + image_width * image_height);

	input_param.data_info.yuv.p_vir[2] = (unsigned char *) ((nv12_data + image_width * image_height) +
															image_width * image_width / 4);


	int ret = mi_alg_input(m_pAlgHandle, &input_param);
	usleep(100*1000);
	if (MI_ALG_SUCCESS != ret) 
	{
		printf("input data to alg failure, m_pAlgHandle=%p, "
					"chan=%d, format=%d, frame_id=%d, time_stamp=%llu, width=%d, height=%d\n",
					m_pAlgHandle, input_param.chan, input_param.format,
					input_param.frame_id, input_param.time_stamp, input_param.data_info.yuv.width,
					input_param.data_info.yuv.height);
		return ret;
	}

	// printf("6. input data to alg success, m_pAlgHandle=%p, "
	// 			"chan=%d, format=%d, frame_id=%d, time_stamp=%llu, width=%d, height=%d\n",
	// 			m_pAlgHandle, input_param.chan, input_param.format,
	// 			input_param.frame_id, input_param.time_stamp, input_param.data_info.yuv.width,
	// 			input_param.data_info.yuv.height);

	
	// auto end = std::chrono::system_clock::now();
	// auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	// printf("procFrameData time:%ld ms\n", elapsed.count());
	return 0;
}

#else	////////>> 70MAI

using namespace magik::venus;

CImgDetection::CImgDetection()
{
    m_Cvtbgra = true;
}

CImgDetection::~CImgDetection()
{

}

bool CImgDetection::init(void)
{
    int iRet = 0;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        printf("warning: could not set CPU affinity, continuing...\n");
    }

    iRet = venus::venus_init();
    if (0 != iRet) {
        printf("venus init failed!\n");
        return false;
    }

    if (m_Cvtbgra){
        //m_ODNet = venus::net_create(TensorFormat::NHWC);
        m_ODNet = venus::net_create(TensorFormat::NHWC, venus::ShareMemoryMode::ALL_SEPARABLE_MEM);
        // m_ParcelNet = venus::net_create(TensorFormat::NHWC, venus::ShareMemoryMode::ALL_SEPARABLE_MEM);
    }
    else { 
        //m_ODNet = venus::net_create(TensorFormat::NV12);
        m_ODNet = venus::net_create(TensorFormat::NV12, venus::ShareMemoryMode::ALL_SEPARABLE_MEM);
        // m_ParcelNet = venus::net_create(TensorFormat::NV12, venus::ShareMemoryMode::ALL_SEPARABLE_MEM);
    }
    
    g_post_process_event.init();
    return true;
}

bool CImgDetection::setStayTime(int iStayTime)
{
    g_post_process_event.setPersonStayTime(iStayTime);
    g_post_process_event.setCarStayTime(iStayTime);
    return true;
}

bool CImgDetection::setRecognitionArea(std::vector<std::vector<float>> bounds)
{
	g_post_process_event.setRecognitionBounds(bounds);
    return true;
}

void CImgDetection::setObj(void* obj) {
    objAIDetect = static_cast<CAIDetect*>(obj);
}

bool CImgDetection::loadBinModel(std::string strPath)
{
#ifdef TIME
    struct timeval tv; 
    uint64_t time_last;
    double time_ms;
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec;
#endif
    int iRet = 0;

    iRet = m_ODNet->load_model(strPath.c_str());
    if (0 != iRet) {
        printf("load bin model:%s failed,iRet=%d!\n",strPath.c_str(),iRet);
        return false;
    }

#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
    time_ms = time_last*1.0/1000;
    printf("load bin model: %s, time_ms: %fms\n", strPath.c_str(), time_ms);
#endif

    return true;
}

bool CImgDetection::unInit(void)
{
    g_post_process_event.uninit();

    int iRet = venus::venus_deinit();
    if (0 != iRet) {
        printf("venus deinit failed!\n");
        return false;
    }
    return true;
}

void CImgDetection::transCoords(vector<ObjectBox> &in_boxes, PixelOffset &pixel_offset,float scale)
{
    //printf("pad_x:%d pad_y:%d scale:%f \n",pixel_offset.left,pixel_offset.top,scale);
    for(int i = 0; i < (int)in_boxes.size(); i++) {
        in_boxes[i].x1 = (in_boxes[i].x1 - pixel_offset.left) / scale;
        in_boxes[i].x2 = (in_boxes[i].x2 - pixel_offset.left) / scale;
        in_boxes[i].y1 = (in_boxes[i].y1 - pixel_offset.top) / scale;
        in_boxes[i].y2 = (in_boxes[i].y2 - pixel_offset.top) / scale;
    }
}

int CImgDetection::procFrameData(unsigned char *pData, int iDataSize, int iType)
{
    int iRet = 0;
    int iOriImgW = YUV_FRAME_WIDTH;
    int iOriImgH = YUV_FRAME_HEIGHT;
    
    int iNetInW = NET_IN_WIDTH;
    int iNetInH = NET_IN_HEIGHT;

    float fScale;
    PixelOffset pixel_offset;

    std::unique_ptr<venus::Tensor> m_Input;
    //m_Input = (iType == 0) ? m_ODNet->get_input(0) : m_ParcelNet->get_input(0);
    m_Input = m_ODNet->get_input(0);

    magik::venus::shape_t RgbaInputShape = m_Input->shape();
    //printf("model-->%d ,%d %d \n",RgbaInputShape[1], RgbaInputShape[2], RgbaInputShape[3]);
    if (m_Cvtbgra)
    {
        m_Input->reshape({1, iNetInH, iNetInW , 4});
    }
    else
    {
        m_Input->reshape({1, iNetInH, iNetInW, 1});
    }

	//resize and padding
	magik::venus::Tensor temp_ori_input({1, iOriImgH, iOriImgW, 1}, TensorFormat::NV12);
	uint8_t *tensor_data = temp_ori_input.mudata<uint8_t>();
	//int src_size = int(iOriImgH * iOriImgW * 1.5);
	magik::venus::memcopy((void*)tensor_data, (void*)pData, iDataSize * sizeof(uint8_t));

	float fScale_x = (float)iNetInW/(float)iOriImgW;
	float fScale_y = (float)iNetInH/(float)iOriImgH;
	fScale = fScale_x < fScale_y ? fScale_x:fScale_y;  //min scale
	//printf("scale---> %f\n",scale);

	int iValidDstW = (int)(fScale*iOriImgW);
	if (iValidDstW % 2 == 1)
		iValidDstW = iValidDstW + 1;

	int iValidDstH = (int)(fScale*iOriImgH);
	if (iValidDstH % 2 == 1)
		iValidDstH = iValidDstH + 1;

	int iDW = iNetInW - iValidDstW;
	int iDH = iNetInH - iValidDstH;
	pixel_offset.top = int(round(float(iDH)/2 - 0.1));
	pixel_offset.bottom = int(round(float(iDH)/2 + 0.1));
	pixel_offset.left = int(round(float(iDW)/2 - 0.1));
	pixel_offset.right = int(round(float(iDW)/2 + 0.1));
	//check_pixel_offset(pixel_offset);

	magik::venus::BsCommonParam Param;
	Param.pad_val = 114;
	Param.pad_type = magik::venus::BsPadType::SYMMETRY;
	Param.input_height = iOriImgH;
	Param.input_width = iOriImgW;
	Param.input_line_stride = iOriImgW;
	Param.in_layout = magik::venus::ChannelLayout::NV12;
	Param.out_layout = magik::venus::ChannelLayout::RGBA;
	magik::venus::common_resize((const void*)tensor_data, *m_Input.get(), magik::venus::AddressLocate::NMEM_VIRTUAL, &Param);
	//printf("resize padding over: \n");
	//printf("resize valid_dst, w:%d h %d\n",iValidDstW,iValidDstH);
	//printf("padding info top :%d bottom %d left:%d right:%d \n",pixel_offset.top,pixel_offset.bottom,pixel_offset.left,pixel_offset.right);

#ifdef TIME
    struct timeval tv; 
    uint64_t time_last;
    double time_ms;
#endif

#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec;
#endif

    // iRet = (iType == 0) ? m_ODNet->run() : m_ParcelNet->run();
    iRet = m_ODNet->run();
		
#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
    time_ms = time_last*1.0/1000;
    //printf("net run time_ms:%fms\n", time_ms);
#endif

    unique_ptr<const venus::Tensor> Out0;
    unique_ptr<const venus::Tensor> Out1;
    unique_ptr<const venus::Tensor> Out2;

    Out0 = m_ODNet->get_output(0);
    Out1 = m_ODNet->get_output(1);
    Out2 = m_ODNet->get_output(2);

	auto Shape0 = Out0->shape();
	auto Shape1 = Out1->shape();
	auto Shape2 = Out2->shape();

	int iShapeSize0 = Shape0[0] * Shape0[1] * Shape0[2] * Shape0[3];
	int iShapeSize1 = Shape1[0] * Shape1[1] * Shape1[2] * Shape1[3];
	int iShapeSize2 = Shape2[0] * Shape2[1] * Shape2[2] * Shape2[3];

	float* fP0 = Out0->mudata<float>();
	float* fP1 = Out1->mudata<float>();
	float* fP2 = Out2->mudata<float>();

	vector<float> Out;
	for(int i = 0; i < iShapeSize0; i ++)
		Out.push_back(fP0[i]);
	for(int i = 0; i < iShapeSize1; i ++)
		Out.push_back(fP1[i]);
	for(int i = 0; i < iShapeSize2; i ++)
		Out.push_back(fP2[i]);

	/*post process*/
	vector<ObjectBox> vecPersonList;
	float *pOutData = Out.data();

	vector<float> threshold;

#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec;
#endif

	postprocess(pOutData, vecPersonList, iNetInW, iNetInH, iType, threshold);// w h
	transCoords(vecPersonList, pixel_offset, fScale);

#ifdef TIME
    gettimeofday(&tv, NULL);
    time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
    time_ms = time_last*1.0/1000;
    //printf("postprocess run time_ms:%fms\n", time_ms);
#endif

    int iBoxSize = int(vecPersonList.size());
    //printf("box_list.size=[%d]\n",iBoxSize);

    vector<ObjectBox> vecFilterList;
    vecFilterList.clear();

	for (int i = 0; i < iBoxSize; i++) 
	{
		auto person = vecPersonList[i];
		//printf("box:%d %f %f %f %f %f %d\n", i, person.x1, person.y1, person.x2, person.y2, person.score, person.classid);

		if( (person.classid == 0 && person.score >= threshold[0]) || //0: person
			(person.classid == 1 && person.score >= threshold[1]) || //1: car
			(person.classid == 2 && person.score >= threshold[2]) || //2: bus
			(person.classid == 3 && person.score >= threshold[3]) || //3: cat
			(person.classid == 4 && person.score >= threshold[4]) || //4: dog
			(person.classid == 5 && person.score >= threshold[5])    //5: parcel
			)
		{
			person.x1 = person.x1 / YUV_FRAME_WIDTH;
			person.y1 = person.y1 / YUV_FRAME_HEIGHT;
			person.x2 = person.x2 / YUV_FRAME_WIDTH;
			person.y2 = person.y2 / YUV_FRAME_HEIGHT;

			vecFilterList.push_back(person); 
		}
	}
	//printf("vecFilterList.size=[%d]\n",vecFilterList.size());
	int iDetectResult = g_post_process_event.detectEvent(vecFilterList);
	objAIDetect->returnDetectResult(iDetectResult);
    
    //printf("detection result=[%d]\n",iDetectResult);
    return iDetectResult;
}

#endif
