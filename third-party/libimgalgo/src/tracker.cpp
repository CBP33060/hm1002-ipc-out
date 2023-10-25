#include "tracker.h"

Tracker::Tracker()
{
    reload();
}

Tracker::Tracker(const std::vector<Eigen::VectorXf>& x0s, const std::vector<Eigen::MatrixXf>& P0s)
{
    init(x0s, P0s);
}

void Tracker::init(const std::vector<Eigen::VectorXf>& x0s, const std::vector<Eigen::MatrixXf>& P0s)
{
    reload();
    trackers_ = std::vector<KalmanFilter>();
    // assert(x0s.size() == P0s.size(), "x0s.size() != P0s.size()");
    assert(x0s.size() == P0s.size());
    for (int i = 0; i < x0s.size(); ++i) {
        trackers_.emplace_back(KalmanFilter(x0s[i], P0s[i]));
    }
}

void Tracker::predict()
{
    for (auto& tracker : trackers_) {
        tracker.predict();
    }
}

void Tracker::update(const std::vector<Eigen::VectorXf>& detections)
{
    // assert(trackers_.size() == detections.size(), "trackers_.size() != detections.size()");
    assert(trackers_.size() == detections.size());
    for (int i = 0; i < trackers_.size(); ++i) {
        trackers_[i].update(detections[i]);
    }
}

void Tracker::reload()
{
    trackers_.clear();
    trackers_.shrink_to_fit();
}

std::vector<float> Tracker::getResults()
{
    std::vector<float> results;
    for (auto& tracker : trackers_) {
        results.emplace_back(tracker.state()[0]);
    }
    return results;
}