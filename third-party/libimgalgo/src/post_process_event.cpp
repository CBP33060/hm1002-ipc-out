#include "post_process_event.h"
#include "image_detection.hpp"

#include <algorithm>

#include "overlap_checker.h"

namespace post_process_event {

void PostProcessEvent::init()
{
    fast_current_frame_ = 0;
    fast_launch_pool_.fill({0});
    tracker_current_frame_ = 0;
    extra_score_ = {0};
    filter_event_ = 0;

    person_stay_time_ = 6;  // 初始化为6s
    car_stay_time_ = 6;     // 初始化为6s
    //recognition_bounds_ = { {0.6F, 0.F, 0.7F, 1.F, 1.F, 0.F, 1.F, 1.F} }; // test
    //recognition_bounds_ = { {0.1F, 0.2F, 0.3F, 1.4F, 1.5F, 0.6F, 1.7F, 1.8F} };   // valid
    recognition_bounds_ = { {0.F, 0.F, 0.F, 1.F, 1.F, 0.F, 1.F, 1.F} };

    setPersonStayTime(person_stay_time_);
    setCarStayTime(car_stay_time_);

    if (!is_mi_) {
        // 初始化tracker
        std::vector<Eigen::VectorXf> x0s = std::vector<Eigen::VectorXf>();
        std::vector<Eigen::MatrixXf> P0s = std::vector<Eigen::MatrixXf>();

        // P0初始值暂定为0.7
        for (int i = 0; i < 5; ++i)
        {
            x0s.emplace_back(Eigen::VectorXf::Zero(kTraceParamNumber));
            P0s.emplace_back(Eigen::MatrixXf::Identity(kTraceParamNumber, kTraceParamNumber) * 0.7);
        }
        // 初始化
        tracker_.init(x0s, P0s);
    }
    deserializeEvent(); // TODO PC批量测试需要注释掉
}

void PostProcessEvent::uninit()
{
    serializeEvent();
}

int PostProcessEvent::fastLaunch(const std::vector<ObjectBox>& bbox_list)
{
    int result = 0;

    for (const auto& person : bbox_list) {
        fast_launch_pool_[fast_current_frame_%3][person.classid] = 1;
    }

    for (int i = 0; i < 5; ++i) {
        // 快速判定是否需要启动，使用三取二的简单策略
        if (fast_launch_pool_[0][i] + fast_launch_pool_[1][i] + fast_launch_pool_[2][i] >= 2) {
            switch (i) {
            case 0:
                result |= 1 << E_EVENT_TYPE_PERSON; break;
            case 1:
                result |= 1 << E_EVENT_TYPE_CAR; break;
            case 2:
                result |= 1 << E_EVENT_TYPE_BUS; break;
            case 3:
                result |= 1 << E_EVENT_TYPE_CAT; break;
            case 4:
                result |= 1 << E_EVENT_TYPE_DOG; break;

            default:
                break;
            }
        }
    }

    ++fast_current_frame_;
    return result;
}

int PostProcessEvent::detectEvent(const std::vector<ObjectBox>& bbox_list) {
    std::vector<ObjectBox> filter_bbox_list = filterRecognitionBounds(bbox_list);

    if (is_mi_) {
        return detectEventWithMi(filter_bbox_list);
    }
    else {
        return detectEventWith70Mai(filter_bbox_list);
    }
}

int PostProcessEvent::detectEventWith70Mai(const std::vector<ObjectBox>& bbox_list)
{
    int state = 0;
    // static float baseline[5] = {0.53F, 0.5F, 0.5F, 0.62F, 0.49F};    // 18号的阈值
	//constexpr float baseline[5] = { 0.62F, 0.5F, 0.5F, 0.62F, 0.8F };    // 10号的阈值
    //constexpr float baseline[5] = { 0.72F, 0.6F, 0.6F, 0.5F, 0.5F };    //25号的阈值
    // constexpr float baseline[5] = { 0.6F, 0.6F, 0.6F, 0.4F, 0.4F };    // 26日基于结果的修改，后续增加其他策略后需要恢复
    // constexpr float baseline[6] = { 0.39F, 0.5F, 0.5F, 0.3F, 0.3F, 0.75F };    // 6.9新模型精调阈值
    // constexpr std::array<float, 6> baseline = {0.5F, 0.35F, 0.35F, 0.45F, 0.45F, 0.61F};  // 6.12新模型精调阈值
    // constexpr std::array<float, 6> baseline = {0.4F, 0.4F, 0.4F, 0.4F, 0.4F, 0.4F};  // 7.17新新模型粗调阈值
    //constexpr std::array<float, 6> baseline = {0.4F, 0.6F, 0.6F, 0.4F, 0.4F, 0.65F};  // 7.17新新模型粗调阈值
    constexpr std::array<float, 6> baseline = { 0.3F, 0.45F, 0.6F, 0.4F, 0.4F, 0.4F };  // 7.27模型粗调阈值

    constexpr std::array<float, 6> ceiling = {1.F, 1.F, 1.F, 1.F, 1.F, 1.F}; // 封顶阈值
    //constexpr std::array<float, 6> trigger = { 0.4F, 0.7F, 0.7F, 0.4F, 0.65F, 0.5F }; // 触发阈值在类中

    tracker_.predict();
    std::vector<Eigen::VectorXf> detections = std::vector<Eigen::VectorXf>();
    ///===========算法核心(不含包裹处理)=============start=====///
    // 1.2版本：只使用每一类传入的最大置信度，未考虑多个目标、检测框的情况；引入时间信息对置信度进行调整
    std::array<float, 6> update_list {0};
    for (const auto& person : bbox_list) {
        update_list[person.classid] = std::max(update_list[person.classid], person.score);
    }
    // update_list[1] = std::max(update_list[1], update_list[2]);  // 合并两种车的数据
    update_list[3] = std::max(update_list[3], update_list[4]);  // 合并猫和狗的数据

    // 这里不处理包裹事件，包裹额外处理
    for (int i = 0; i < 5; ++i) {
        float scale = (update_list[i] - baseline[i]) / (ceiling[i] - baseline[i]);
        scale = std::max(0.F, std::min(1.F, scale));
        scale += extra_score_[i];
        detections.emplace_back(Eigen::VectorXf::Ones(kTraceParamNumber) * scale);

        // 额外置信度2.0
        if (update_list[i] > baseline[i])
        {
            // extra_score[i] += 0.1F;
            extra_score_[i] += (update_list[i] - baseline[i]) * 0.6F;
        }
        else if (update_list[i] > baseline[i] / 2)
        {
            // extra_score[i] += 0.05F;
            extra_score_[i] += (update_list[i] - baseline[i] / 2) * 0.3F;
        }
        else
        {
            // extra_score[i] -= 0.1F;
            extra_score_[i] += (update_list[i] - baseline[i] / 2) * 0.9F;
        }
        extra_score_[i] = std::max(0.F, std::min(1.F, extra_score_[i]));
    }
    tracker_.update(detections);
    ///===========算法核心(不含包裹处理)=============end=====///

    auto frame_result = tracker_.getResults();
    if (debug_flag_)
    {
        // bbox_list
        printf("bbox_list.size: %d\n", bbox_list.size());
        for (auto& bbox: bbox_list)
        {
            printf("bbox: %d, %f, %f, %f, %f, %f\n", bbox.classid, bbox.x1, bbox.y1, bbox.x2, bbox.y2, bbox.score);
        }
        printf("class result: %f, %f, %f, %f, %f\n\n", frame_result[0], frame_result[1], frame_result[2], frame_result[3], frame_result[4]);
    }
    // TODO 事件需要一定的排序
    // 优先级：人>包裹>动物 定义未明确，暂无优先级
    if (findPerson(frame_result)) {
        if (isPersonStay()) {
            state |= 1 << E_EVENT_DETAIL_TYPE_PERSON_STAY;
        }
        //else {
        //    state |= 1 << E_EVENT_DETAIL_TYPE_PERSON_APPEAR;
        //}
    }
    // TODO 重点是参与计算的extra_score是滞后一帧的，可能会有影响
    if (findCar(frame_result)) {
        if (isCarStay()) {
            state |= 1 << E_EVENT_DETAIL_TYPE_CAR_STAY;
        }
        //else {
        //    state |= 1 << E_EVENT_DETAIL_TYPE_CAR_APPEAR;
        //}
    }
    if (findAnimal(frame_result)) {
        state |= 1 << E_EVENT_DETAIL_TYPE_ANIMAL_APPEAR;
    }
    ///===========算法核心(单包裹)=============start=====///
    auto package_state = packageEvent(bbox_list);
    state |= package_state;
    ///===========算法核心(单包裹)=============end=====///

    // 目前实机不断电，设置每20s自动重置事件发送情况
    if (debug_flag_) {
        if (tracker_current_frame_ % 100 == 0) {
            printf("==========filter_event_ is %d, clear==========\n", filter_event_);
            filter_event_ = 0;
        }
    }
    int output_state = state & ~filter_event_;
    filter_event_ |= state;
    ++tracker_current_frame_;
    return state;
}

std::vector<ObjectBox> PostProcessEvent::filterRecognitionBounds(const std::vector<ObjectBox>& bbox_list)
{
	std::vector<ObjectBox> result;
	OverlapChecker::Point quad[4];
	OverlapChecker::Point rect[4];
	for (const auto& input_bbox : bbox_list) {
		quad[0] = { input_bbox.x1, input_bbox.y1 }, quad[3] = { input_bbox.x2, input_bbox.y1 };
		quad[1] = { input_bbox.x1, input_bbox.y2 }, quad[2] = { input_bbox.x2, input_bbox.y2 };

		// 使用 std::find_if 查找第一个与 input_bbox 重叠的 bounds
		auto it = std::find_if(recognition_bounds_.begin(), recognition_bounds_.end(), [&](const std::vector<float>& bounds) {
			rect[0] = { bounds[0], bounds[1] }, rect[3] = { bounds[4], bounds[5] };
			rect[1] = { bounds[2], bounds[3] }, rect[2] = { bounds[6], bounds[7] };
			return OverlapChecker::checkOverlap(quad, rect);
			});
		if (it != recognition_bounds_.end()) { // 如果找到了重叠的 bounds
			result.push_back(input_bbox);
		}
	}
	return result;
}

int PostProcessEvent::detectEventWithMi(const std::vector<ObjectBox>& bbox_list) {
    int state = 0;
    std::vector<ObjectBox> current_bbox_list = bbox_list;   // 小米的类别不同，需要修改
    ///===========算法核心(不含包裹处理)=============start=====///
    // 1.2with mi版本：传入包围盒即认为是真值，进行滑动窗口
    std::array<float, 6> update_list{ 0 };
    constexpr std::array<int, 4> kClassidMap{ 0, 5, 3, 1 };    // 模型从小米映射到70mai的定义，人0→0，包裹1→5，宠2→3， 车3→1
    for (auto& person : current_bbox_list) {
        person.score = 100.F; // 保证设置的分数自研模型无法达到，以示区分
        person.classid = kClassidMap[person.classid];
        update_list[person.classid] = std::max(update_list[person.classid], person.score);
    }
    ///===========算法核心(不含包裹处理)=============end=====///

    std::vector<float> frame_result(update_list.begin(), update_list.end());
    if (debug_flag_)
    {
        for (auto& bbox : bbox_list)
        {
            printf("bbox: %f, %f, %f, %f, %f\n", bbox.x1, bbox.y1, bbox.x2, bbox.y2, bbox.score);
        }
        printf("class result: %f, %f, %f, %f, %f\n\n", frame_result[0], frame_result[1], frame_result[2], frame_result[3], frame_result[4]);
    }
    // TODO 事件需要一定的排序
    // 优先级：人>包裹>动物 定义未明确，暂无优先级
    if (findPerson(frame_result)) {
        if (isPersonStay()) {
            state |= 1 << E_EVENT_DETAIL_TYPE_PERSON_STAY;
        }
    }
    if (findCar(frame_result)) {
        if (isCarStay()) {
            state |= 1 << E_EVENT_DETAIL_TYPE_CAR_STAY;
        }
    }
    if (findAnimal(frame_result)) {
        state |= 1 << E_EVENT_DETAIL_TYPE_ANIMAL_APPEAR;
    }
    ///===========算法核心(单包裹)=============start=====///
    auto package_state = packageEvent(current_bbox_list);
    state |= package_state;
    ///===========算法核心(单包裹)=============end=====///

    // 目前实机不断电，设置每20s自动重置事件发送情况
    if (debug_flag_) {
        if (tracker_current_frame_ % 100 == 0) {
            printf("==========mi filter_event_ is %d, clear==========\n", filter_event_);
            filter_event_ = 0;
        }
    }

    int output_state = state & ~filter_event_;
    filter_event_ |= state;
    ++tracker_current_frame_;
    return state;
}

void PostProcessEvent::setPersonStayTime(int time)
{
    if (time < 0) {
        printf("person stay time must be greater than or equal to 0, current time is %d\n", time);
        return;
    }
    if (time == 0) {
        time = 1;
    }
    //person_stay_time_ = std::ceil(time / 0.7F); // 设定时间为触发长度，窗口长度为触发长度的1.4倍
    person_stay_time_ = time; // 目前为设定时间内触发，而不是侦测到设定时间所对应的帧数再触发
    person_score_list_.clear();
    person_score_list_.resize(person_stay_time_ * fps_);
}

void PostProcessEvent::setCarStayTime(int time)
{
    if (time < 0) {
        printf("car stay time must be greater than or equal to 0, current time is %d\n", time);
        return;
    }
    if (time == 0) {
        time = 1;
    }
    //car_stay_time_ = std::ceil(time / 0.7F);
    car_stay_time_ = time;
    car_score_list_.clear();
    car_score_list_.resize(car_stay_time_ * fps_);
    bus_score_list_.clear();
    bus_score_list_.resize(car_stay_time_ * fps_);
}

void PostProcessEvent::setRecognitionBounds(const std::vector<std::vector<float>>& bounds)
{
    //recognition_bounds_ = bounds;
    std::vector<std::vector<float>> verify_bounds;
    // 简易的输入验证
    for (const auto& b : bounds)
    {
        if (b.size() == 8)
        {
            verify_bounds.push_back(b);
        }
    }

    if (!verify_bounds.empty())
    {
        recognition_bounds_ = verify_bounds;
    }
}

bool PostProcessEvent::findPerson(const std::vector<float>& scores)
{
    person_score_list_[tracker_current_frame_ % person_score_list_.size()] = scores[0];
    return scores[0] > trigger_[0];
}

bool PostProcessEvent::isPersonStay()
{
    const int person_limit_size = person_score_list_.size();
    auto total = std::count_if(person_score_list_.begin(), person_score_list_.end(), [&](float score) { return score > trigger_[0]; });
    return total > person_limit_size * 0.7F;
}

bool PostProcessEvent::findAnimal(const std::vector<float>& scores)
{
    return scores[3] > trigger_[3] || scores[4] > trigger_[4];
}

bool PostProcessEvent::findCar(const std::vector<float>& scores)
{
    car_score_list_[tracker_current_frame_ % car_score_list_.size()] = scores[1];
    bus_score_list_[tracker_current_frame_ % bus_score_list_.size()] = scores[2];
    return scores[1] > trigger_[1] || scores[2] > trigger_[2];
}

bool PostProcessEvent::isCarStay()
{
    // 驻留6s
    const int car_limit_size = car_score_list_.size();
    int total = 0;
    for (int i = 0; i < car_limit_size; ++i) {
        if (car_score_list_[i] > trigger_[1] || bus_score_list_[i] > trigger_[2])
        {
            ++total;
        }
    }
    return total > car_limit_size * 0.7F;
}

int PostProcessEvent::packageEvent(const std::vector<ObjectBox>& bbox_list)
{
    std::vector<BoundingBox> current_bbox_list;
    for (const auto& bbox : bbox_list) {
        if (bbox.classid == 5) {
            // 传入像素坐标，需要归一化处理
            //current_bbox_list.emplace_back(PostProcessUtils::pixel2ratio(bbox.x1, bbox.y1, bbox.x2, bbox.y2));
            current_bbox_list.emplace_back(BoundingBox(bbox.x1,bbox.y1,bbox.x2,bbox.y2,bbox.score));
            // current_bbox_list.emplace_back(PostProcessUtils::pixel2ratio(bbox));
        }
    }

    ////////////////////////////////////////////////////////
    tracker_package_.process(current_bbox_list);
    auto event_state = tracker_package_.getEventState();
    return event_state;
    ////////////////////////////////////////////////////////
}

bool PostProcessEvent::isFrameLost(const std::vector<Eigen::VectorXf>& detections)
{
    return false;
}

void PostProcessEvent::serializeEvent()
{
    if (tracker_package_.serialize()) {
        printf("serialize success\n");
    } else {
        printf("serialize failed\n");
    }
}

void PostProcessEvent::deserializeEvent()
{
    try {
        if (tracker_package_.deserialize()) {
            printf("deserialize success\n");
        } else {
            printf("deserialize failed\n");
        }
    }
    catch (std::exception& e)
    {
        printf("deserialize file error\n");
    }
}

}  // namespace post_process_event