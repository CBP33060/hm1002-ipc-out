#ifndef PACKAGE_EVENT_H
#define PACKAGE_EVENT_H

#include <vector>

#include <Eigen/Dense>

#include "kalman_filter.h"
#include "post_process_utils.h"
#include "post_process.hpp"

namespace post_process_event {

class PackageTracker
{
public:
    PackageTracker() = default;
    ~PackageTracker() = default;

    void process(const std::vector<BoundingBox>& bbox_list);

    int getEventState() const;

    bool serialize() const;
    bool deserialize();

    // 测试接口
    std::vector<float> getTrackerResults() const;
    std::vector<BoundingBox> getBboxList() const;

private:
    void insertBboxList(const std::vector<BoundingBox>& bbox_list);
    void removeBboxList(const std::vector<BoundingBox>& bbox_list); // 未使用

    void updateOne(const Eigen::VectorXf& detection, int index);
    void updateMissingBboxList(const std::vector<BoundingBox>& bbox_list);
    void updateNormalBboxList(const std::vector<BoundingBox>& bbox_list);

    // 有丢失的bbox
    bool hasLostBbox() const;
    bool hasNewBbox() const;

    bool fileExists() const;
    // 更新额外分数，更新范围为[left, right)
    static void updateScores(const std::vector<float>& scores, std::vector<float>& extra_scores, int left, int right);
    static void updateScore(float score, float& extra_score);
    // 统一更新内部bbox_list_的状态
    void updateBboxState(const std::vector<std::pair<BoundingBox, BoundingBox>>& matches);
    void updateMatchedBbox(BoundingBox& bbox, const BoundingBox& current_bbox);
    void updateUnmatchedBbox(BoundingBox& bbox);
    bool handleBboxStateSwitch(BoundingBox& bbox);

    void resetEventState();
    void deleteBbox();

    /// 事件判定算法
    static bool isEnter(const BoundingBox& bbox);  // 包裹入场
    static bool isMove(const BoundingBox& bbox);  // 包裹移动
    // bool isLeave();  // 包裹出场 暂不使用

    std::vector<KalmanFilter> trackers_;
    std::vector<BoundingBox> bbox_list_;
    std::vector<BoundingBox> lost_bbox_list_;
    std::vector<BoundingBox> new_bbox_list_;
    std::vector<float> extra_score_;  // 额外分数，引入时间因素
    unsigned int current_frame_ = 0;
    int event_state_ = 0;

    static constexpr float baseline_ = 0.4F;
    static constexpr float trigger_ = 0.5F;

    const std::string kSerializePath = "/userfs/event.dat";

#ifdef MI_ALG_DETECT
    static constexpr bool is_mi_ = true;  // 小米算法部分取消卡尔曼滤波
#else
    static constexpr bool is_mi_ = false;  // 70MAI算法
#endif

};

} // namespace post_process_event

#endif // PACKAGE_EVENT_H