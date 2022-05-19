#include <iostream>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <random>
#include <array>

#include "vapor/RegularGrid.h"
#include "vapor/OpenMPSupport.h"

// Allocate a bunch of raw pointers.
// The caller will need to delete[] them.
//
auto AllocateBlocks(std::array<size_t, 3> bs, std::array<size_t, 3> dims) -> std::vector<float*>
{
    size_t block_size = 1;
    size_t nblocks = 1;

    for (size_t i = 0; i < bs.size(); i++) {
        block_size *= bs[i];
        nblocks *= ((dims[i] - 1) / bs[i]) + 1;
    }

    auto blks = std::vector<float*>(nblocks, nullptr);
    for (size_t i = 0; i < nblocks; i++)
      blks[i] = new float[block_size];
    
    return (blks);
}

// Copy of Grid::GetRange() function from VAPOR release 3.6
//
void GetRange_36(VAPoR::Grid* g, float range[2])
{
    float missingValue = g->GetMissingValue();
    auto itr = g->cbegin();
    auto enditr = g->cend();

    // Edge case: all values are missing values.
    //
    range[0] = range[1] = missingValue;
    while (*itr == missingValue && itr != enditr) { ++itr; }
    if (itr == enditr) return;

    range[0] = *itr;
    range[1] = range[0];
    while (itr != enditr) {
        if (*itr < range[0] && *itr != missingValue)
            range[0] = *itr;
        else if (*itr > range[1] && *itr != missingValue)
            range[1] = *itr;
        ++itr;
    }
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cout << "Help:  This program measures the serial and parallel (OpenMP) execution time\n"
                 "       given a regular grid of size (Dim x Dim x Dim).\n"
                 "Note:  the environment variable OMP_NUM_THREADS controls the number of threads.\n"
                 "Usage: ./GetRange Dim\n";
    return 1;
  }
  const size_t dim = std::stol(argv[1]);
  const auto dims = std::array<size_t, 3>{dim, dim, dim};
  size_t num_threads = 1;

#pragma omp parallel
  {
    if (omp_get_thread_num() == 0)
      num_threads = omp_get_num_threads();
  }
  std::printf("Testing a grid of size (%ld, %ld, %ld), using %ld threads...\n",
              dim, dim, dim, num_threads);

  // Create a grid to test
  const auto blk_size = std::array<size_t, 3>{128, 128, 128};
  auto blks = AllocateBlocks(blk_size, dims);
  auto* grid = new VAPoR::RegularGrid(dims, blk_size, blks, {0.0, 0.0, 0.0}, {100.0, 100.0, 100.0}); 

  // Fill in random values
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(-1.0, 10.0);
  const auto stride_size = dim * dim * dim / num_threads;

#pragma omp parallel for
  for (size_t i = 0; i < num_threads; i++) {
    auto beg = grid->begin() + i * stride_size;
    auto end = beg;
    if (i < num_threads - 1)
      end += stride_size;
    else
      end = grid->end();
    for (auto itr = beg; itr != end; ++itr)
      *itr = dist(gen);
  }

  // Time a serial run
  float range_36[2] = {0.0, 1.1};
  const auto serial_start = std::chrono::steady_clock::now();
  GetRange_36(grid, range_36);
  const auto serial_end = std::chrono::steady_clock::now();
  const auto serial_time = std::chrono::duration_cast<std::chrono::milliseconds>(serial_end - serial_start).count();
  std::cout << "GetRange() in serial time (milliseconds): " << serial_time << std::endl;

  // Time a parallel run
  float range_omp[2] = {2.2, 3.3};
  const auto omp_start = std::chrono::steady_clock::now();
  grid->GetRange(range_omp);
  const auto omp_end = std::chrono::steady_clock::now();
  const auto omp_time = std::chrono::duration_cast<std::chrono::milliseconds>(omp_end - omp_start).count();
  std::cout << "GetRange() in OpenMP time (milliseconds): " << omp_time << std::endl;

  // Clean up
  delete grid;
  grid = nullptr;
  for (size_t i = 0; i < blks.size(); i++) {
    delete[](blks[i]);
    blks[i] = nullptr;
  }

  return 0;
}
