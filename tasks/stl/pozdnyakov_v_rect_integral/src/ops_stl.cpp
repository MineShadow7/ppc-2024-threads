// Copyright 2024 Pozdnyakov Vasya
#include "stl/pozdnyakov_v_rect_integral/include/ops_stl.hpp"

double pozdnyakov_stl::pozdnyakov_flin(double x, double y) { return x - y; }
double pozdnyakov_stl::pozdnyakov_fxy(double x, double y) { return x * y; }
double pozdnyakov_stl::pozdnyakov_fysinx(double x, double y) { return y * std::sin(x); }
double pozdnyakov_stl::pozdnyakov_fxexpy(double x, double y) { return x * std::exp(y); }

bool pozdnyakov_stl::PozdnyakovTaskSTL::pre_processing() {
  internal_order_test();
  try {
    auto* tmp = reinterpret_cast<double*>(taskData->inputs[0]);
    x1 = tmp[0], x2 = tmp[1], y1 = tmp[2], y2 = tmp[3];
    res = 0.0;
    f = reinterpret_cast<Func>(taskData->inputs[1]);
    n = reinterpret_cast<int*>(taskData->inputs[2])[0];
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool pozdnyakov_stl::PozdnyakovTaskSTL::validation() {
  internal_order_test();
  return taskData->inputs_count[0] == 4 && taskData->outputs_count[0] == 1;
}

bool pozdnyakov_stl::PozdnyakovTaskSTL::run() {
  internal_order_test();
  try {
    double x_i = std::abs(x2 - x1) / n;
    double y_i = std::abs(y2 - y1) / n;

    int threadsCount = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::mutex mutex;

    auto parallelFunc = [&](int begin, int end) {
      double local_res = 0.0;
      for (int i = begin; i < end; ++i) {
        for (int j = 0; j < n; ++j) {
          local_res += f(x1 + i * x_i, y1 + j * y_i);
        }
      }

      mutex.lock();
      res += local_res;
      mutex.unlock();
    };

    int threadSteps = n / threadsCount;
    for (int t = 0; t < threadsCount; ++t) {
      int begin = t * threadSteps;
      int end = (t == threadsCount - 1) ? n : begin + threadSteps;
      threads.emplace_back(parallelFunc, begin, end);
    }

    for (auto& thread : threads) {
      thread.join();
    }

    res *= x_i * y_i;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool pozdnyakov_stl::PozdnyakovTaskSTL::post_processing() {
  internal_order_test();
  try {
    reinterpret_cast<double*>(taskData->outputs[0])[0] = res;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool pozdnyakov_stl::PozdnyakovTaskSequential::pre_processing() {
  internal_order_test();
  try {
    auto* tmp = reinterpret_cast<double*>(taskData->inputs[0]);
    x1 = tmp[0], x2 = tmp[1], y1 = tmp[2], y2 = tmp[3];
    res = 0.0;
    f = reinterpret_cast<Func>(taskData->inputs[1]);
    n = reinterpret_cast<int*>(taskData->inputs[2])[0];
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool pozdnyakov_stl::PozdnyakovTaskSequential::validation() {
  internal_order_test();
  return taskData->inputs_count[0] == 4 && taskData->outputs_count[0] == 1;
}

bool pozdnyakov_stl::PozdnyakovTaskSequential::run() {
  internal_order_test();
  try {
    double x_i = std::abs(x2 - x1) / n;
    double y_i = std::abs(y2 - y1) / n;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        res += f(x1 + i * x_i, y1 + j * y_i);
      }
    }
    res *= x_i * y_i;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool pozdnyakov_stl::PozdnyakovTaskSequential::post_processing() {
  internal_order_test();
  try {
    reinterpret_cast<double*>(taskData->outputs[0])[0] = res;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}
