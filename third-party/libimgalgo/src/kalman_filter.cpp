#include "kalman_filter.h"

KalmanFilter::KalmanFilter(const Eigen::VectorXf& x0, const Eigen::MatrixXf& P0) : x_(x0), P_(P0)
{
    int dim = x0.size();

    F_ = Eigen::MatrixXf::Identity(dim, dim);
    H_ = Eigen::MatrixXf::Identity(dim, dim);
    Q_ = Eigen::MatrixXf::Identity(dim, dim);   // 过程噪声的协方差矩阵，这里可调
    R_ = Eigen::MatrixXf::Identity(dim, dim)*0.5;   // 观测噪声的协方差矩阵，这里可调
}

void KalmanFilter::predict()
{
    x_ = F_ * x_;
    P_ = F_ * P_ * F_.transpose() + Q_;
}

void KalmanFilter::update(const Eigen::VectorXf& z)
{
    Eigen::VectorXf y = z - H_ * x_;
    Eigen::MatrixXf S = H_ * P_ * H_.transpose() + R_;
    Eigen::MatrixXf K = P_ * H_.transpose() * S.inverse();

    x_ = x_ + K * y;
    P_ = (Eigen::MatrixXf::Identity(x_.size(), x_.size()) - K * H_) * P_;
}
