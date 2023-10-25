#ifndef POST_PROCESS_UTILS_H
#define POST_PROCESS_UTILS_H

#include <algorithm>
#include <array>
#include <vector>

// #include "test_class.h"
#include "post_process.hpp"

namespace post_process_event {

enum class FrameState
{
    kFrameTypeNormal = 0,
    kFrameTypeNew = 1,
    kFrameTypeLost = 2,
    // E_FRAME_TYPE_RECOVER,
    kFrameTypeCover = 3,
    kFrameTypeMove = 4,
    kFrameTypeStatic = 5,
    // E_FRAME_TYPE_MAX
};

// TODO 每次测试都需要确认
/**
* 自测数据1&2 854*480
* 主观数据1&2 832*480
* 后期数据 832*480(待确认)
*/
constexpr float kPictureSizeWidth = 832.F;
constexpr float kPictureSizeHeight = 480.F;
constexpr unsigned int kFrameWindowSize = 15U;

struct BoundingBox
{
    // x1, y1, x2, y2, 范围[0, 1]
    float x1, y1, x2, y2;
    float score = 0.F;
    // int time = 0;   // TODO 考虑把time向上提升到tracker中
    int id = -1;    // 目前只有在插入和删除的时候才会更新id
    /**
     * 上次的状态
     *
     * 包裹入场：normal→move→static 特征：move→static
     * 包裹出场：static→move→lost 特征：static→move
     *
     */
    FrameState last_state = FrameState::kFrameTypeNew;
    std::array<FrameState, kFrameWindowSize> state_history{};  // 用于记录目标的状态历史，最多记录15帧
    std::vector<float> static_position; // 用于存放本包围盒静止状态下的位置。放在这里其实不太合适

    BoundingBox() = default;

    BoundingBox(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}

    BoundingBox(float x1, float y1, float x2, float y2, float score) : x1(x1), y1(y1), x2(x2), y2(y2), score(score) {}

    BoundingBox(std::vector<float> bbox) { 
        //assert(bbox.size() == 4);
        x1 = bbox[0], y1 = bbox[1];
        x2 = bbox[2], y2 = bbox[3];
    }
};

class PostProcessUtils
{
public:
    // 像素转[0,1]
    static BoundingBox pixel2ratio(float x1, float y1, float x2, float y2);
    static BoundingBox pixel2ratio(ObjectBox object_box);
    static float iou(const BoundingBox& bbox1, const BoundingBox& bbox2);
    // 计算两个包围盒之间的距离（欧氏距离）
    static float computeDistance(const BoundingBox& bbox1, const BoundingBox& bbox2);
    //static std::vector<BoundingBox> detectLostTargets(const std::vector<BoundingBox>& frame1, const std::vector<BoundingBox>& frame2);
    // 匹配相邻帧上的包围盒，并判断目标是否发生了丢失
    static std::vector<std::pair<BoundingBox, BoundingBox>>
        matchBoundingBoxes(const std::vector<BoundingBox>& last_frame, const std::vector<BoundingBox>& current_frame, std::vector<BoundingBox>& lost_targets, std::vector<BoundingBox>& new_targets);
    // 判断包围盒是否静止
    static bool isBoundingBoxStatic(const BoundingBox& bbox1, const BoundingBox& bbox2, float threshold = 0.5F);
};

}  // namespace post_process_event

#endif  // POST_PROCESS_UTILS_H
