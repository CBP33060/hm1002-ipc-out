#include "package_event.h"
#include "image_detection.hpp"

#include <fstream>
#include <sstream>
#include <map>

namespace post_process_event {

void PackageTracker::insertBboxList(const std::vector<BoundingBox>& bbox_list)
{
    int id = bbox_list_.size();
    std::vector<BoundingBox> new_bbox_list = bbox_list;
    std::vector<float> score;
    score.reserve(bbox_list.size());
    for (auto& bbox : new_bbox_list)
    {
        bbox.id = id++;
        score.emplace_back(bbox.score);
        bbox.last_state = FrameState::kFrameTypeNew;  // last_history可以不在这里赋值
        bbox.state_history[current_frame_ % kFrameWindowSize] = bbox.last_state;
    }
    bbox_list_.insert(bbox_list_.end(), new_bbox_list.begin(), new_bbox_list.end());

    if (!is_mi_)
    {
        std::vector<float> extra_score;
        extra_score.resize(new_bbox_list.size());
        updateScores(score, extra_score, 0, extra_score.size());
        extra_score_.insert(extra_score_.end(), extra_score.begin(), extra_score.end());

        for (const auto& bbox : new_bbox_list)
        {
            trackers_.emplace_back(KalmanFilter(Eigen::VectorXf::Zero(1), Eigen::MatrixXf::Identity(1, 1) * 0.7));

            Eigen::VectorXf detection(1);
            detection << bbox.score + extra_score_[bbox.id];
            updateOne(detection, bbox.id);
        }
    }
}

void PackageTracker::removeBboxList(const std::vector<BoundingBox>& bbox_list)
{
    for (int i = bbox_list.size() - 1; i >= 0; --i)
    {
        for (int j = bbox_list_.size() - 1; j >= 0; --j)
        {
            if (bbox_list[i].id == bbox_list_[j].id)
            {
                bbox_list_.erase(bbox_list_.begin() + j);
                if (!is_mi_)
                {
                    extra_score_.erase(extra_score_.begin() + j);
                    trackers_.erase(trackers_.begin() + j);
                }
                break;
            }
        }
    }

    // 更新id
    for (int i = 0; i < bbox_list_.size(); ++i)
    {
        bbox_list_[i].id = i;
    }
}

void PackageTracker::updateOne(const Eigen::VectorXf& detection, int index)
{
    trackers_[index].predict();
    trackers_[index].update(detection);
    updateScore(detection[0] - extra_score_[index], extra_score_[index]);  // TODO 增加新的值作为滤波时这里也要修改
}

void PackageTracker::updateMissingBboxList(const std::vector<BoundingBox>& bbox_list)
{
    for (const auto& bbox : bbox_list)
    {
        for (int i = 0; i < bbox_list_.size(); ++i)
        {
            if (bbox.id == bbox_list_[i].id)
            {
                if (!is_mi_) {
                    Eigen::VectorXf detection(1);
                    detection << extra_score_[i];
                    updateOne(detection, i);
                }
                else {
                    bbox_list_[i].score = 0.0F;
                }
                
                break;
            }
        }
    }
}

void PackageTracker::updateNormalBboxList(const std::vector<BoundingBox>& bbox_list)
{
    for (const auto& bbox : bbox_list)
    {
        for (int i = 0; i < bbox_list_.size(); ++i)
        {
            if (bbox.id == bbox_list_[i].id)
            {
                if (!is_mi_) {
                    // Eigen::VectorXf detection(5);
                    // detection << bbox.x1, bbox.y1, bbox.x2, bbox.y2, bbox.score; // TODO 暂时只传入score
                    Eigen::VectorXf detection(1);
                    detection << bbox.score + extra_score_[i];
                    updateOne(detection, i);
                }
                else {
                    bbox_list_[i].score = bbox.score;
                }
                break;
            }
        }
    }
}

void PackageTracker::process(const std::vector<BoundingBox>& bbox_list)
{
    // printf("current_frame_: %d\n", current_frame_);
    lost_bbox_list_.clear();  // 上一次触发的丢失包裹列表、新出现的包裹列表需要清空
    new_bbox_list_.clear();
    event_state_ = 0;

    const std::vector<BoundingBox>& last_frame_bbox_list = bbox_list_;
    const std::vector<BoundingBox>& current_frame_bbox_list = bbox_list;

    std::vector<BoundingBox> lost_bbox_list;
    std::vector<BoundingBox> new_bbox_list;
    auto matches = PostProcessUtils::matchBoundingBoxes(last_frame_bbox_list, current_frame_bbox_list, lost_bbox_list, new_bbox_list);
    std::vector<BoundingBox> matched_current_frame_bbox_list;
    matched_current_frame_bbox_list.reserve(matches.size());
    for (const auto& match : matches)
    {
        matched_current_frame_bbox_list.emplace_back(match.second);
    }

    // 1. 更新丢失的包裹
    updateMissingBboxList(lost_bbox_list);
    // 2. 更新匹配的包裹
    updateNormalBboxList(matched_current_frame_bbox_list);
    // 3. 插入新出现的包裹
    insertBboxList(new_bbox_list);

    // 4. 检查各包裹状态、触发事件类型
    updateBboxState(matches);
    // 5. 删除已失效数据
    deleteBbox();
    ++current_frame_;
}

std::vector<float> PackageTracker::getTrackerResults() const
{
    std::vector<float> result;
    for (const auto& tracker : trackers_)
    {
        result.emplace_back(tracker.state()[0]);
    }
    return result;
}

std::vector<BoundingBox> PackageTracker::getBboxList() const
{
    return bbox_list_;
}

bool PackageTracker::hasLostBbox() const
{
    return !lost_bbox_list_.empty();
}

bool PackageTracker::hasNewBbox() const
{
    return !new_bbox_list_.empty();
}

int PackageTracker::getEventState() const
{
    return event_state_;
}

bool PackageTracker::serialize() const
{
    std::ofstream ofs(kSerializePath);
    if (ofs.is_open())
    {
        for (const auto& bbox : bbox_list_)
        {
            if (bbox.last_state == FrameState::kFrameTypeStatic)
            {
                ofs << bbox.static_position[0] << " " << bbox.static_position[1] << " " << bbox.static_position[2]
                    << " " << bbox.static_position[3] << " " << bbox.score << std::endl;
            }
        }
        ofs.close();
        return true;
    }
    return false;
}

bool PackageTracker::deserialize()
{
    bbox_list_.clear();
    if (!fileExists())
    {
        // 测试，在这里直接生成对应的文件
        // std::ofstream file(kSerializePath);
        return true;
    }
    std::ifstream ifs(kSerializePath);
    if (ifs.is_open())
    {
        std::string line;
        int bbox_id = 1;
        std::vector<BoundingBox> bbox_list;
        while (std::getline(ifs, line))
        {
            std::istringstream iss(line);
            BoundingBox bbox;
            iss >> bbox.x1 >> bbox.y1 >> bbox.x2 >> bbox.y2 >> bbox.score;
            bbox.id = bbox_id++;
            bbox_list.emplace_back(bbox);
        }
        insertBboxList(bbox_list);
        // 从磁盘读取的包裹直接切换为静止状态
        for (auto& bbox : bbox_list_)
        {
            bbox.last_state = FrameState::kFrameTypeStatic;
            bbox.state_history.fill(FrameState::kFrameTypeStatic);
            bbox.static_position = std::vector<float> {bbox.x1, bbox.y1, bbox.x2, bbox.y2};
        }
        ifs.close();
        return true;
    }
    return false;
}

bool PackageTracker::fileExists() const
{
    std::ifstream file(kSerializePath);
    return file.good();
}

void PackageTracker::updateBboxState(const std::vector<std::pair<BoundingBox, BoundingBox>>& matches)
{
    std::map<int, int> id_map;  // id -> index
    for (int i = 0; i < matches.size(); ++i)
    {
        id_map[matches[i].first.id] = i;
    }

    bool status_switch = false;
    for (auto& bbox : bbox_list_)
    {
        float score = bbox.score;
        if (!is_mi_) {
            score = trackers_[bbox.id].state()[0];
        }
        auto iter = id_map.find(bbox.id);
        if (score < trigger_)
        {
            bbox.state_history[current_frame_ % kFrameWindowSize] = FrameState::kFrameTypeLost;
        }
        else if (iter != id_map.end())
        {
            // 匹配成功状态: 静止/移动
            const auto& match = matches[iter->second];
            updateMatchedBbox(bbox, match.second);
        }
        else
        {
            // 匹配失败状态: 新目标/丢失
            updateUnmatchedBbox(bbox);
        }

        status_switch |= handleBboxStateSwitch(bbox);
    }

    if (status_switch)
    {
        serialize();
    }
}

void PackageTracker::updateMatchedBbox(BoundingBox& bbox, const BoundingBox& current_bbox)
{
    BoundingBox position = bbox.last_state == FrameState::kFrameTypeStatic ? BoundingBox(bbox.static_position) : bbox;
    FrameState frame_state = PostProcessUtils::isBoundingBoxStatic(position, current_bbox) ?
                                 FrameState::kFrameTypeStatic :
                                 FrameState::kFrameTypeMove;

    bbox.state_history[current_frame_ % kFrameWindowSize] = frame_state;
    // 更新包围盒大小为最新大小，没有放入卡尔曼进行平滑
    bbox.x1 = current_bbox.x1;
    bbox.y1 = current_bbox.y1;
    bbox.x2 = current_bbox.x2;
    bbox.y2 = current_bbox.y2;
}

void PackageTracker::updateUnmatchedBbox(BoundingBox& bbox)
{
    auto index = current_frame_ % kFrameWindowSize;
    if (bbox.last_state != FrameState::kFrameTypeNew)
    {
        bbox.state_history[index] = FrameState::kFrameTypeLost;  // 目前Lost和Move统一认为是移动
    }
    else
    {
        bbox.state_history[index] = FrameState::kFrameTypeNew;
    }
}

bool PackageTracker::handleBboxStateSwitch(BoundingBox& bbox)
{
    bool status_switch = false;

    switch (bbox.last_state)
    {
        case FrameState::kFrameTypeStatic:
            if (isMove(bbox))
            {
                event_state_ |= 1 << E_EVENT_DETAIL_TYPE_PACKAGE_MOVE;
                bbox.last_state = FrameState::kFrameTypeMove;
                status_switch = true;
            }
            break;
        // Normal状态只触发入场，通过序列化加入的包裹初始为Static
        case FrameState::kFrameTypeNormal:
        case FrameState::kFrameTypeMove:
            if (isEnter(bbox))
            {
                event_state_ |= 1 << E_EVENT_DETAIL_TYPE_PACKAGE_ENTER;
                bbox.last_state = FrameState::kFrameTypeStatic;
                bbox.static_position = std::vector<float> {bbox.x1, bbox.y1, bbox.x2, bbox.y2};
                status_switch = true;
            }
            break;
        case FrameState::kFrameTypeNew: bbox.last_state = FrameState::kFrameTypeNormal; break;
        default: break;
    }
    return status_switch;
}

void PackageTracker::resetEventState()
{
    event_state_ = 0;
}

void PackageTracker::deleteBbox()
{
    for (auto& bbox : bbox_list_)
    {
        if (std::count_if(bbox.state_history.begin(), bbox.state_history.end(),
                          [](FrameState state) { return state == FrameState::kFrameTypeLost; }) >= kFrameWindowSize / 2)
        {
            bbox.last_state = FrameState::kFrameTypeLost;
            lost_bbox_list_.emplace_back(bbox);
        }
    }
    // 从list中删除
    for (int i = bbox_list_.size() - 1; i >= 0; --i)
    {
        if (bbox_list_[i].last_state == FrameState::kFrameTypeLost)
        {
            bbox_list_.erase(bbox_list_.begin() + i);
            if (!is_mi_) {
                extra_score_.erase(extra_score_.begin() + i);
                trackers_.erase(trackers_.begin() + i);
            }
        }
    }

    // 更新id
    for (int i = 0; i < bbox_list_.size(); ++i)
    {
        bbox_list_[i].id = i;
    }
}

bool PackageTracker::isEnter(const BoundingBox& bbox)
{
    const auto& state_history = bbox.state_history;
    int static_count = std::count_if(state_history.begin(), state_history.end(),
                                     [](FrameState state) { return state == FrameState::kFrameTypeStatic; });

    return static_count > kFrameWindowSize * 0.6F;
}

bool PackageTracker::isMove(const BoundingBox& bbox)
{
    const auto& state_history = bbox.state_history;
    int move_count = std::count_if(state_history.begin(), state_history.end(),
                                   [](FrameState state) { return state == FrameState::kFrameTypeMove; });
    int lost_count = std::count_if(state_history.begin(), state_history.end(),
                                   [](FrameState state) { return state == FrameState::kFrameTypeLost; });

    return (move_count + lost_count) > kFrameWindowSize * 0.4F;
}

// [left, right)
void PackageTracker::updateScores(const std::vector<float>& scores, std::vector<float>& extra_scores, int left,
                                  int right)
{
    for (int i = left; i < right; ++i)
    {
        updateScore(scores[i], extra_scores[i]);
    }
}

void PackageTracker::updateScore(float score, float& extra_score)
{
    /* 策略3.0
        预计使用ln规整分数,更符合直觉 2.0缺陷:
       一阶导数不连续,导致分数突变;引入额外分数后,量纲发生变化,破坏了边界,不符合设计初衷 计划改进:
       使用偏移后的ln函数,曲线的x轴为分数/置信度,y轴为权重
            曲线二阶导【最大值】(暂时考虑为-1,考虑对称性;找最值需要换函数)的点x,y为分界点/锚点,即x为模型baseline,
       y为权重占比(默认为0.5,分割左右区间)
    */
    // 策略2.0
    if (score > baseline_)
    {
        extra_score += (score - baseline_) * 0.6F;
    }
    else if (score > baseline_ / 2)
    {
        extra_score += (score - baseline_ / 2) * 0.3F;
    }
    else
    {
        extra_score += (score - baseline_ / 2) * 0.9F;
    }
    extra_score = std::max(0.F, std::min(1.F, extra_score));
    // 策略1.0
    // if (score > baseline_)
    // {
    //     extra_score += (score - baseline_) * 0.6F;
    // }
    // else
    // {
    //     extra_score += (score - baseline_) * 0.9F;
    // }
    // extra_score_[index] = std::max(0.F, std::min(1.F, extra_score_[index]));
}

}  // namespace post_process_event