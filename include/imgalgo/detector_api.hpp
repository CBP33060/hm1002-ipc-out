/**  
 * All rights Reserved, Designed By MI
 * @projectName miot_alg_t40_sdk
 * @title     detetcor_api  
 * @package    ${PACKAGE_NAME}  
 * @description    ${TODO}  
 * @author wangdong     
 * @date   2021/7/9 下午5:25  
 * @version V0.0.0
 * @copyright 2021 wangdong21@xiaomi.com
 */
//

#ifndef MIOT_ALG_T40_MODEL_SDK_DETETCOR_APT_HPP
#define MIOT_ALG_T40_MODEL_SDK_DETETCOR_APT_HPP

#include "algorithm_sdk_error_code.h"
#include "common_export.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>


namespace miot_ipc_algorithm {
    /// callback func
//	typedef std::function<void(void *,error_code_t)> callback_function;

    class miot_alg_detector_class {
    public:
        miot_alg_detector_class();

        ~miot_alg_detector_class();

        /**
         * 算法初始化,根据配置文件(json格式)里的参数进行算法初始化
         * @param configPath
         * @return 错误码
         */
        error_code_t load_model(
                const char *config_json_path);


        error_code_t set_input_params(int fps);

        error_code_t set_input_params(mi_alg_detect_param_t roi);
        /**
         * 算法结果create，由于涉及到内存和显存的释放，所以将内存管理由算法类处理
         * 使用样例：
            void *m_image;
            detector.input_preprocess_malloc(&m_image);
            记得调用释放函数input_preprocess_free以实现内存的释放
         * @param malloc_input_ptr　图像数据句柄
         * @return 错误码
         */
        error_code_t alg_output_create(void **alg_outputs_ptr);

        /**
         * 主动析构函数，预处理数据和
         * @param malloc_input_ptr
         * @param alg_outputs
         * @return
         */
        error_code_t alg_output_destroy(void **alg_outputs_ptr);

        /**
         * 主动析构函数，预处理数据和
         * @param malloc_input_ptr
         * @param alg_outputs
         * @return
         */
        error_code_t destroy();



        /**
         * 设置回调函数，仅异步模式时生效
         * Set callback func, only support async mode
         * Note: miot_alg_base_class should be inited(Init()) before set_callback()
         * @param callback [in], callback func
         * @return 错误码
         */
        error_code_t set_callback(mi_alg_callback_t p_callback);

        /**
         * 前向预测，异步使用回调函数获取结果
         * @param malloc_input_ptr　图像数据
         * @param confidence_thresh　置信阈值
         * @param verbose　是否打印相关调试信息
         * @return 错误码
         */

        error_code_t forward_async(void *p_alg_handle, mi_alg_input_param_t *input, bool verbose = false);

        /**
         * 当算法库初始化成功后获取模型的输入宽 ,高，通道数
         * @param width 模型的输入宽
         * @param height 模型的输入高
         * @param channels 模型的输入通道数
         * @return 错误码
         */
        error_code_t get_input_shape(int &width, int &height, int &channels);

        /**
         * 获取模型算法库是否初始化成功
         * @return false 未初始化， true 初始化成功
         */
        bool is_initialized() const;

        /**
         * 获取模型版本号，需要在init函数完成后调用
         * @return 版本号
         */
        const char *get_version() const;

    private:
        class impl; /// 用于隐藏算法实现的私有类
        std::unique_ptr<impl> impl_;
    };
}
#endif //MIOT_ALG_T40_MODEL_SDK_DETETCOR_APT_HPP
