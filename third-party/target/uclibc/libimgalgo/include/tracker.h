#ifndef TRACKER_H
#define TRACKER_H

#include <vector>

#include <Eigen/Dense>

#include "kalman_filter.h"

class Tracker
{
public:
    Tracker();
    Tracker(const std::vector<Eigen::VectorXf>& x0s, const std::vector<Eigen::MatrixXf>& P0s);
    ~Tracker() = default;

    void init(const std::vector<Eigen::VectorXf>& x0s, const std::vector<Eigen::MatrixXf>& P0s);

    // TODO 用于测试的接口
    void reload();

    void predict();
    void update(const std::vector<Eigen::VectorXf>& detections);

    std::vector<float> getResults();

private:
    std::vector<KalmanFilter> trackers_;
};

#endif // TRACKER_H
