#include "post_process_utils.h"

#include <math.h>

namespace post_process_event {

BoundingBox PostProcessUtils::pixel2ratio(float x1, float y1, float x2, float y2)
{
    return BoundingBox(x1/kPictureSizeWidth, y1/kPictureSizeHeight, x2/kPictureSizeWidth, y2/kPictureSizeHeight);
}

BoundingBox PostProcessUtils::pixel2ratio(ObjectBox object_box)
{
    return BoundingBox(object_box.x1 / kPictureSizeWidth, object_box.y1 / kPictureSizeHeight,
                       object_box.x2 / kPictureSizeWidth, object_box.y2 / kPictureSizeHeight, object_box.score);
}

float PostProcessUtils::iou(const BoundingBox& bbox1, const BoundingBox& bbox2)
{
    // 计算两个包围盒相交的坐标
    float x1 = std::max(bbox1.x1, bbox2.x1);
    float y1 = std::max(bbox1.y1, bbox2.y1);
    float x2 = std::min(bbox1.x2, bbox2.x2);
    float y2 = std::min(bbox1.y2, bbox2.y2);

    // 计算相交的面积
    float w = std::max(0.F, x2 - x1 + 1);
    float h = std::max(0.F, y2 - y1 + 1);
    float inter = w * h;

    // 计算两个包围盒的面积
    float bbox1_area = (bbox1.x2 - bbox1.x1 + 1) * (bbox1.y2 - bbox1.y1 + 1);
    float bbox2_area = (bbox2.x2 - bbox2.x1 + 1) * (bbox2.y2 - bbox2.y1 + 1);

    // 计算iou
    float iou = inter / (bbox1_area + bbox2_area - inter);
    return iou;
}

float PostProcessUtils::computeDistance(const BoundingBox& bbox1, const BoundingBox& bbox2)
{
    float dx = bbox1.x1 - bbox2.x1;
    float dy = bbox1.y1 - bbox2.y1;
    float dw = bbox1.x2 - bbox1.x1 - (bbox2.x2 - bbox2.x1);
    float dh = bbox1.y2 - bbox1.y1 - (bbox2.y2 - bbox2.y1);

    return sqrtf(dx * dx + dy * dy + dw * dw + dh * dh);
}

std::vector<std::pair<BoundingBox, BoundingBox>>
    PostProcessUtils::matchBoundingBoxes(const std::vector<BoundingBox>& last_frame, const std::vector<BoundingBox>& current_frame, std::vector<BoundingBox>& lost_targets, std::vector<BoundingBox>& new_targets)
{
    constexpr float kIouThreshold = 0.5F;

    std::vector<std::pair<BoundingBox, BoundingBox>> matches;
    std::vector<bool> matched(current_frame.size(), false);

    for (const BoundingBox& bbox1 : last_frame)
    {
        float max_iou = 0.F;
        int match_index = -1;
        BoundingBox best_match;

        for (int j = 0; j < current_frame.size(); ++j)
        {
            if (matched[j])
            {
                continue;
            }
            const auto& bbox2 = current_frame[j];
            float iou_value = iou(bbox1, bbox2);

            if (iou_value > max_iou)
            {
                max_iou = iou_value;
                best_match = bbox2;
                match_index = j;
            }
        }

        // 将最佳匹配的包围盒对添加到匹配列表中
        if (max_iou >= kIouThreshold)
        {
            best_match.id = bbox1.id;
            matches.emplace_back(bbox1, best_match);
            matched[match_index] = true;
        } else {
            lost_targets.push_back(bbox1);
        }
    }

    for (int j = 0; j < current_frame.size(); ++j)
    {
        if (!matched[j])
        {
            new_targets.push_back(current_frame[j]);
        }
    }

    return matches;
}

bool PostProcessUtils::isBoundingBoxStatic(const BoundingBox& bbox1, const BoundingBox& bbox2, float threshold /*= 0.5F*/)
{
    float width = bbox1.x2 - bbox1.x1;
    float height = bbox1.y2 - bbox1.y1;

    float delta_x1 = std::abs(bbox2.x1 - bbox1.x1);
    float delta_y1 = std::abs(bbox2.y1 - bbox1.y1);
    float delta_x2 = std::abs(bbox2.x2 - bbox1.x2);
    float delta_y2 = std::abs(bbox2.y2 - bbox1.y2);

    // 测试：引入中心点变化量
    float center_x1 = (bbox1.x1 + bbox1.x2) / 2;
    float center_y1 = (bbox1.y1 + bbox1.y2) / 2;
    float center_x2 = (bbox2.x1 + bbox2.x2) / 2;
    float center_y2 = (bbox2.y1 + bbox2.y2) / 2;

    float delta_center_x = std::abs(center_x2 - center_x1);
    float delta_center_y = std::abs(center_y2 - center_y1);

    float threshold_x = threshold * width;
    float threshold_y = threshold * height;

    //float min_delta_x = std::min({ delta_x1, delta_x2, delta_center_x });
    //float min_delta_y = std::min({ delta_y1, delta_y2, delta_center_y });
    float min_delta_x = std::min({ delta_x1, delta_x2 });
    float min_delta_y = std::min({ delta_y1, delta_y2 });

    // printf("min_delta_x: %f, min_delta_y: %f, threshold_x: %f, threshold_y: %f\n", min_delta_x, min_delta_y, threshold_x, threshold_y);
    // printf("min_delta_x <= threshold_x: %d, min_delta_y <= threshold_y: %d\n\n", min_delta_x <= threshold_x, min_delta_y <= threshold_y);

    // 如果位置变化量小于阈值，则判定为静止状态
    return min_delta_x <= threshold_x && min_delta_y <= threshold_y;
}

}  // namespace post_process_event