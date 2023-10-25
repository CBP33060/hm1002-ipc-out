#ifndef POST_PROCESS_H
#define POST_PROCESS_H

#include <array>
#include <vector>

#include "tracker.h"
#include "package_event.h"

// #include "test_class.h"
#include "post_process.hpp"

namespace post_process_event {

class PostProcessEvent
{
public:
    void init();
    void uninit();  // 当前未使用

    // 快速启动判定，当前未使用
    int fastLaunch(const std::vector<ObjectBox>& bbox_list);

    // 检测事件
    int detectEvent(const std::vector<ObjectBox>& bbox_list);
    // 测试输出结果
    std::vector<float> getResults() {
        if (!is_mi_) {
            std::vector<float> result = tracker_.getResults();
            auto package_list = tracker_package_.getTrackerResults();
            float package_max = 0;
            for (auto& package : package_list) {
                package_max = std::max(package_max, package);
            }
            result.emplace_back(package_max);
            return result;
        }
        else {
            return { 0,0,0,0,0,0 };
        }
       
    }

    // 设置行人逗留时间
    void setPersonStayTime(int time);
    // 设置车辆逗留时间
    void setCarStayTime(int time);
    // 设置多个识别范围 支持多组四边形：ltx,lty, lbx,lby, rtx, rty, tbx, rby
    // 使用归一化后的坐标而非像素坐标
    void setRecognitionBounds(const std::vector<std::vector<float>>& bounds);

    PostProcessEvent() = default;
    ~PostProcessEvent() = default;

private:
    // 发现人员 事件暂时弃用，仅作为流程步骤
    bool findPerson(const std::vector<float>& scores);
    // 人员逗留
    bool isPersonStay();
    // 发现动物
    bool findAnimal(const std::vector<float>& scores);
    // 发现车辆 事件暂时弃用，仅作为流程步骤
    bool findCar(const std::vector<float>& scores);
    // 车辆逗留
    bool isCarStay();
    // 包裹事件处理
    int packageEvent(const std::vector<ObjectBox>& bbox_list);
    // 是否丢帧
    bool isFrameLost(const std::vector<Eigen::VectorXf>& detections);

    // 事件序列化到磁盘
    void serializeEvent();
    // 事件从磁盘反序列化
    void deserializeEvent();
    
    // 使用自研模型+后处理
    int detectEventWith70Mai(const std::vector<ObjectBox>& bbox_list);
    // 使用小米模型+自编修改的后处理
    int detectEventWithMi(const std::vector<ObjectBox>& bbox_list);
    // 包围盒范围过滤
    std::vector<ObjectBox> filterRecognitionBounds(const std::vector<ObjectBox>& bbox_list);

    static constexpr bool debug_flag_ = false;

#ifdef MI_ALG_DETECT
    static constexpr bool is_mi_ = true;  // 小米算法
#else
    static constexpr bool is_mi_ = false;  // 70MAI算法
#endif

    Tracker tracker_;
    PackageTracker tracker_package_;
    unsigned int tracker_current_frame_ = 0;
    int filter_event_ = 0;

    std::array<float, 6> extra_score_ {0};
    // static constexpr std::array<float, 6> trigger_ = {0.35F, 0.4F, 0.4F, 0.35F, 0.35F, 0.45F};  // 触发阈值旧
    //static constexpr std::array<float, 6> trigger_ = { 0.35F, 1.1F, 1.1F, 0.35F, 0.35F, 0.65F }; // 触发阈值新
    static constexpr std::array<float, 6> trigger_ = { 0.4F, 0.7F, 0.7F, 0.4F, 0.65F, 0.5F }; // 触发阈值23.7.27

    std::array<std::array<int, 3>, 5> fast_launch_pool_{ 0 };    // 0: person, 1: car, 2: bus, 3: cat, 4: dog, 5: package
    unsigned int fast_current_frame_ = 0;

    static constexpr unsigned int fps_ = 5;
    static constexpr unsigned int kTraceParamNumber = 1;  // 目前只追踪类型是否存在，输入为score，范围0~1

    int person_stay_time_ = 6;
    int car_stay_time_ = 6;
    // 多组四边形：将平铺坐标为ltx,lty, lbx,lby, rtx, rty, rbx, rby
    std::vector<std::vector<float>> recognition_bounds_;

    struct PackageInfo {
        Eigen::VectorXf box; // x1,y1,x2,y2
        int time = 0;
        float score = 0.F;
    };
    std::vector<PackageInfo> package_info_list_;    // 包裹需要更多的处理，类型单独记录
    std::vector<float> person_score_list_;
    // 两类车分开进行处理
    std::vector<float> car_score_list_;
    std::vector<float> bus_score_list_;
};

} // namespace post_process_event

#endif // POST_PROCESS_H
