// Copyright 2024 Bonyuk Peter
#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "core/task/include/task.hpp"

std::vector<int> getImage(int n, int m, uint8_t min, uint8_t max);

class LinearGaussianFiltering : public ppc::core::Task {
 public:
  explicit LinearGaussianFiltering(std::shared_ptr<ppc::core::TaskData> taskData_) : Task(std::move(taskData_)) {}
  bool pre_processing() override;
  bool validation() override;
  bool run() override;
  bool post_processing() override;
  int getPixel(int, int);
  void setPixel(int, int, int);

 private:
  int height{}, width{}, min{}, max{};
  std::vector<int> input = {};
  std::vector<int> res = {};
};