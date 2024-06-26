// Copyright 2024 Kostanyan Arsen
#include "seq/kostanyan_a_sobel/include/ops_seq.hpp"

#include <random>
#include <thread>

using namespace std::chrono_literals;

std::vector<uint8_t> kostanyan_getRandomPicture(int n, int m, uint8_t min, uint8_t max) {
  int size = n * m;
  std::random_device dev;
  std::mt19937 gen(dev());
  std::uniform_int_distribution<int> distrib(min, max);
  std::vector<uint8_t> pict(size);
  for (int i = 0; i < size; i++) {
    pict[i] = distrib(gen);
  }
  return pict;
}

bool kostanyan_EdgeDetectionSequential::validation() {
  internal_order_test();
  return taskData->inputs_count[1] == taskData->outputs_count[0];
}

bool kostanyan_EdgeDetectionSequential::pre_processing() {
  internal_order_test();
  n = reinterpret_cast<int *>(taskData->inputs[0])[0];
  m = reinterpret_cast<int *>(taskData->inputs[0])[1];
  for (int i = 0; i < n * m; i++) {
    input_.push_back(reinterpret_cast<uint8_t *>(taskData->inputs[1])[i]);
    res.push_back(0);
  }
  return true;
}

bool kostanyan_EdgeDetectionSequential::run() {
  internal_order_test();
  int size = n * m;
  if (size == 0) {
    return false;
  }
  if (size == 1) {
    res[0] = input_[0];
    return true;
  }

  for (int i = 1; i < n - 1; i++) {
    for (int j = 1; j < m - 1; j++) {
      int gx = -input_[(i - 1) * m + j - 1] + input_[(i - 1) * m + j + 1] - 2 * input_[i * m + j - 1] +
               2 * input_[i * m + j + 1] - input_[(i + 1) * m + j - 1] + input_[(i + 1) * m + j + 1];
      int gy = input_[(i - 1) * m + j - 1] + 2 * input_[(i - 1) * m + j] + input_[(i - 1) * m + j + 1] -
               input_[(i + 1) * m + j - 1] - 2 * input_[(i + 1) * m + j] - input_[(i + 1) * m + j + 1];
      double edge_strength = sqrt(gx * gx + gy * gy);
      res[i * m + j] = edge_strength > 255 ? 255 : static_cast<uint8_t>(edge_strength);
    }
  }

  return true;
}

bool kostanyan_EdgeDetectionSequential::post_processing() {
  internal_order_test();
  for (int i = 0; i < n * m; i++) {
    reinterpret_cast<uint8_t *>(taskData->outputs[0])[i] = res[i];
  }
  return true;
}