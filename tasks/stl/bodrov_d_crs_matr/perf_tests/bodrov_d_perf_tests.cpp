// Copyright 2024 Bodrov Daniil
#include <gtest/gtest.h>

#include <chrono>
#include <complex>
#include <random>
#include <vector>

#include "core/perf/include/perf.hpp"
#include "stl/bodrov_d_crs_matr/include/bodrov_d_crs_matr_stl.hpp"

using namespace bodrov_stl;

SparseMatrixBodrovOMP generate_random_matrix_bodrov_stl(int n, int m, double proba, int seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> random(-2.0, 2.0);
  std::bernoulli_distribution bernoulli(proba);

  SparseMatrixBodrovOMP result;
  result.Rows = n;
  result.Columns = m;
  result.DataPointer.assign(result.Rows + 1, 0);

  std::vector<int> ColumnsIndexes;
  std::vector<std::complex<double>> Values;

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      if (bernoulli(gen)) {
        ColumnsIndexes.push_back(j);
        Values.emplace_back(random(gen), random(gen));
      }
    }
    result.DataPointer[i + 1] = ColumnsIndexes.size();
  }

  result.ColumnsIndexes = std::move(ColumnsIndexes);
  result.Values = std::move(Values);

  return result;
}

TEST(bodrov_d_crs_matr_stl, test_pipeline_run) {
  SparseMatrixBodrovOMP A = generate_random_matrix_bodrov_stl(100, 100, 0.6, 4113);
  SparseMatrixBodrovOMP B = generate_random_matrix_bodrov_stl(100, 100, 0.6, 2134);
  SparseMatrixBodrovOMP Result;

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
  taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t *>(&A));
  taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t *>(&B));
  taskDataSeq->outputs.emplace_back(reinterpret_cast<uint8_t *>(&Result));

  // Create Task
  auto taskTBB = std::make_shared<SparseMatrixSolverBodrovOMPParallel>(taskDataSeq);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(taskTBB);
  perfAnalyzer->pipeline_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);
}

TEST(bodrov_d_crs_matr_stl, test_task_run) {
  SparseMatrixBodrovOMP A = generate_random_matrix_bodrov_stl(100, 100, 0.6, 4113);
  SparseMatrixBodrovOMP B = generate_random_matrix_bodrov_stl(100, 100, 0.6, 2134);
  SparseMatrixBodrovOMP Result;

  // Create TaskData
  std::shared_ptr<ppc::core::TaskData> taskDataSeq = std::make_shared<ppc::core::TaskData>();
  taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t *>(&A));
  taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t *>(&B));
  taskDataSeq->outputs.emplace_back(reinterpret_cast<uint8_t *>(&Result));

  // Create Task
  auto taskTBB = std::make_shared<SparseMatrixSolverBodrovOMPParallel>(taskDataSeq);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(taskTBB);
  perfAnalyzer->task_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);
}
