#ifndef IO_H
#define IO_H

#include <Eigen/Dense>
#include <queue>
#include <set>

void matrix_to_csv(const Eigen::MatrixXf&);

void print_queue(const std::queue<int>&);

void print_deque(const std::deque<int>&);

void print_set(const std::set<int>& s);

void print_bandwidth_comparison(const Eigen::MatrixXf&, const Eigen::MatrixXf&);

void print_peak_comparison(const Eigen::MatrixXf&, const std::vector<int>&,
                           const std::vector<int>&);

#endif
