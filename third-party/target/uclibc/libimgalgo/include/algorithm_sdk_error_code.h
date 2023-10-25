/*
 *
 */
#ifndef ALGORITHM_SDK_ERROR_CODE_H
#define ALGORITHM_SDK_ERROR_CODE_H
namespace miot_ipc_algorithm {
	/*error code*/
	typedef enum {
		ALGORITHM_OPERATION_SUCCESS = 0,
		/******************init*****************************/
		ALGORITHM_INIT_ERR = (01),
		ALGORITHM_INIT_REPETITION_ERR = (02),   /*重复初始化模型*/
		ALGORITHM_INIT_MODEL_ERR = (03),   /*模型加载出错*/
		ALGORITHM_WRONG_IMAGE_ERR = (04),   /*not support this net*/
		ALGORITHM_MODEL_PATH_ERR = (05),   /*模型路径错误 */
		ALGORITHM_MODEL_ERR = (06),   /*模型错误*/
		ALGORITHM_NOT_IMPLEMENTED = (07),   /*模型错误*/
		ALGORITHM_PARAM_ERR = (8),   /*模型错误*/
		ALGORITHM_CREATE_ERR = (9),   /*模型错误*/
		ALGORITHM_DROP_INPUT_IMAGE = (10),   /*模型错误*/
		ALGORITHM_IMAGE_EMPTY = (11),   /*模型错误*/
		ALGORITHM_DESTROY_ERR = (12),   /*模型错误*/
		ALGORITHM_ERR_OTHER = 999
	} error_code_t;

}
#endif // ALGORITHM_SDK_ERROR_CODE_H
