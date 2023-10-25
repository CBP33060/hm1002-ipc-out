#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <Eigen/Dense>

class KalmanFilter
{
public:
    KalmanFilter(const Eigen::VectorXf& x0, const Eigen::MatrixXf& P0);

    void predict();
    void update(const Eigen::VectorXf& z);

    Eigen::VectorXf state() const
    {
        return x_;
    }

    Eigen::MatrixXf covariance() const
    {
        return P_;
    }

private:
    Eigen::VectorXf x_;  // state
    Eigen::MatrixXf P_;  // covariance
    Eigen::MatrixXf F_;  // state transition matrix
    Eigen::MatrixXf H_;  // observation model
    Eigen::MatrixXf Q_;  // process noise covariance
    Eigen::MatrixXf R_;  // observation noise covariance
};

#endif  // KALMAN_FILTER_H
