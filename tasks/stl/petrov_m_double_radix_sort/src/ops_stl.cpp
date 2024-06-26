// Copyright 2024 Petrov Maksim

#include "stl/petrov_m_double_radix_sort/include/ops_stl.hpp"

using namespace std::chrono_literals;
using namespace petrov_stl;

void PetrovRadixSortDoubleSTL::PetrovCountSort(double* in, double* out, int len, int exp) {
  auto* buf = reinterpret_cast<unsigned char*>(in);
  int count[256] = {0};
  for (int i = 0; i < len; i++) {
    count[buf[8 * i + exp]]++;
  }
  int sum = 0;
  for (int i = 0; i < 256; i++) {
    int temp = count[i];
    count[i] = sum;
    sum += temp;
  }
  for (int i = 0; i < len; i++) {
    out[count[buf[8 * i + exp]]] = in[i];
    count[buf[8 * i + exp]]++;
  }
}

bool PetrovRadixSortDoubleSTL::PetrovCountSortSigns(const double* in, double* out, int len) {
  bool positiveFlag = false;
  bool negativeFlag = false;
  int firstNegativeIndex = -1;
  // int firstPositiveIndex = -1;
  for (int i = 0; i < len; i++) {
    if (positiveFlag && negativeFlag) {
      break;
    }
    if (in[i] < 0 && !negativeFlag) {
      negativeFlag = true;
      firstNegativeIndex = i;
    }
    if (in[i] > 0 && !positiveFlag) {
      positiveFlag = true;
      // firstPositiveIndex = i;
    }
  }
  if (positiveFlag && negativeFlag) {
    bool forward = false;
    int j = len - 1;
    for (int i = 0; i < len; i++) {
      out[i] = in[j];
      if (forward) {
        j++;
      } else {
        j--;
      }
      if (j == firstNegativeIndex - 1 && !forward) {
        j = 0;
        forward = true;
      }
    }
    return true;
  }
  if (!positiveFlag) {
    for (int i = len - 1, j = 0; i >= 0; i--, j++) {
      out[j] = in[i];
    }
    return true;
  }
  return false;
}

std::vector<std::vector<double>> PetrovRadixSortDoubleSTL::PetrovSplitVector(const std::vector<double>& data,
                                                                             int numParts) {
  std::vector<std::vector<double>> resultVec;
  std::vector<double> tmp;

  if (numParts < 2 || (int)data.size() < numParts) {
    resultVec.push_back(data);
    return resultVec;
  }

  int part = data.size() / numParts;
  int remainder = data.size() % numParts;

  for (int i = 0; i < numParts; ++i) {
    int start = i * part + std::min(i, remainder);
    int end = start + part + (i < remainder ? 1 : 0);
    resultVec.emplace_back(data.begin() + start, data.begin() + end);
  }

  return resultVec;
}

std::vector<double> PetrovRadixSortDoubleSTL::PetrovMerge(const std::vector<double>& arr1,
                                                          const std::vector<double>& arr2) {
  int len1 = arr1.size();
  int len2 = arr2.size();
  std::vector<double> out;
  out.reserve(len1 + len2);
  int indexFirst = 0;
  int indexSecond = 0;
  while (indexFirst < len1 && indexSecond < len2) {
    if (arr1[indexFirst] < arr2[indexSecond]) {
      out.push_back(arr1[indexFirst++]);
    } else {
      out.push_back(arr2[indexSecond++]);
    }
  }

  while (indexFirst < len1) {
    out.push_back(arr1[indexFirst++]);
  }
  while (indexSecond < len2) {
    out.push_back(arr2[indexSecond++]);
  }

  return out;
}

std::vector<double> PetrovRadixSortDoubleSTL::PetrovRadixSort(const std::vector<double>& data) {
  int len = static_cast<int>(data.size());
  std::vector<double> in = data;
  std::vector<double> out(data.size());

  for (int i = 0; i < 4; i++) {
    PetrovCountSort(in.data(), out.data(), len, 2 * i);
    PetrovCountSort(out.data(), in.data(), len, 2 * i + 1);
  }
  if (!PetrovCountSortSigns(in.data(), out.data(), len)) {
    in.swap(out);
  }
  return out;
}
std::vector<double> PetrovRadixSortDoubleSTL::PetrovBinaryMergeTree(std::vector<std::vector<double>>& sortedVectors) {
  while (sortedVectors.size() > 1) {
    std::vector<std::vector<double>> mergedVectors;
    std::vector<std::future<std::vector<double>>> futures;
    int vectorSize = static_cast<int>(sortedVectors.size());

    // Предварительное выделение памяти для futures
    futures.reserve(vectorSize / 2);

    // Создаем асинхронные задачи для слияния пар векторов
    for (int i = 0; i < vectorSize - 1; i += 2) {
      futures.push_back(std::async(std::launch::async, &PetrovRadixSortDoubleSTL::PetrovMerge,
                                   std::cref(sortedVectors[i]), std::cref(sortedVectors[i + 1])));
    }

    // Собираем результаты выполнения задач
    mergedVectors.reserve(futures.size());
    for (auto& future : futures) {
      mergedVectors.push_back(future.get());
    }

    // Если количество векторов нечетное, добавляем последний вектор в результат
    if (vectorSize % 2 != 0) {
      mergedVectors.push_back(std::move(sortedVectors.back()));
    }

    // Обновляем список отсортированных векторов для следующего уровня слияния
    sortedVectors.swap(mergedVectors);
  }

  return sortedVectors.front();
}

std::vector<double> PetrovRadixSortDoubleSTL::PetrovRadixSortStl(const std::vector<double>& data, int numParts) {
  std::vector<std::vector<double>> vectorsForParallel = PetrovSplitVector(data, numParts);
  std::vector<std::future<std::vector<double>>> futures;
  int vectorSize = static_cast<int>(vectorsForParallel.size());
  futures.reserve(vectorSize);

  for (int i = 0; i < vectorSize; ++i) {
    futures.push_back(
        std::async(std::launch::async, &PetrovRadixSortDoubleSTL::PetrovRadixSort, std::cref(vectorsForParallel[i])));
  }

  for (int i = 0; i < vectorSize; ++i) {
    vectorsForParallel[i] = futures[i].get();
  }

  std::vector<double> finalResult = PetrovBinaryMergeTree(vectorsForParallel);
  return finalResult;
}

bool PetrovRadixSortDoubleSTL::pre_processing() {
  internal_order_test();
  try {
    data_size = taskData->inputs_count[0];
    while (!sort.empty()) {
      sort.pop_back();
    }
    auto* inp = reinterpret_cast<double*>((taskData->inputs[0]));
    for (int i = 0; i < data_size; i++) {
      sort.push_back(inp[i]);
    }
  } catch (...) {
    std::cout << "\n";
    std::cout << "Double radix sort error";
    std::cout << "\n";
    return false;
  }
  return true;
}

bool PetrovRadixSortDoubleSTL::validation() {
  internal_order_test();
  // Check count elements of output
  return ((taskData->inputs_count[0] > 1) && (taskData->outputs_count[0] == taskData->inputs_count[0]));
}

bool PetrovRadixSortDoubleSTL::run() {
  internal_order_test();
  try {
    sort = (PetrovRadixSortStl(sort, 6));
  } catch (...) {
    std::cout << "\n";
    std::cout << "Double radix sort error";
    std::cout << "\n";
    return false;
  }
  return true;
}

bool PetrovRadixSortDoubleSTL::post_processing() {
  internal_order_test();
  try {
    auto* outputs = reinterpret_cast<double*>(taskData->outputs[0]);
    for (int i = 0; i < data_size; i++) {
      outputs[i] = sort[i];
    }
  } catch (...) {
    std::cout << "\n";
    std::cout << "Double radix sort error";
    std::cout << "\n";
    return false;
  }
  return true;
}
