#pragma once

#include <vapor/Grid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/StretchedGrid.h>
#include <vapor/UnstructuredGrid2D.h>

void DeleteHeap();

std::vector<float *> AllocateBlocks(const std::vector<size_t> &bs, const std::vector<size_t> &dims);

void MakeTriangle(VAPoR::Grid *grid, float minVal, float maxVal, bool addRandomMissingValues=true);

void MakeConstantField(VAPoR::Grid *grid, float value, bool addRandomMissingValues=true);

void MakeRamp(VAPoR::Grid *grid, float minVal, float maxVal, bool addRandomMissingValues=true);

void MakeRampOnAxis(VAPoR::Grid *grid, float minVal, float maxVal, size_t axis, bool addRandomMissingValues=true);

bool CompareIndexToCoords(VAPoR::Grid *grid,
                          double &     rms,                 // Root Mean Square error
                          size_t &     numMissingValues,    // Counter for receiving MissingValue upon query
                          size_t &     disagreements        // Counter for when AccessIJK() and GetValue() disagree
);

// Returns the expected node count for Grid::Iterator
bool TestIterator(VAPoR::Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time);

// Returns the expected node count for Grid::ConstCoordIterator
bool TestConstCoordItr(const VAPoR::Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time);

// Returns the expected node count for Grid::ConstNodeIterator
bool TestConstNodeIterator(const VAPoR::Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time, bool withCoordBounds);

void PrintStats(double rms, size_t numMissingValues, size_t disagreements, double time, bool silenceTime);

bool RunTests(VAPoR::Grid *grid, const std::vector<std::string> &tests, float minVal, float maxVal, bool silenceTime);

bool RunTest(VAPoR::Grid *grid, bool silenceTime);

void PrintGridIteratorResults(std::string &gridType, std::string itrType, size_t count, size_t expectedCount, size_t disagreements, double time, bool silenceTime);

VAPoR::CurvilinearGrid *MakeCurvilinearTerrainGrid(const std::vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu, const std::vector<size_t> &dims);

VAPoR::LayeredGrid *MakeLayeredGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu);

VAPoR::StretchedGrid *MakeStretchedGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu);

VAPoR::UnstructuredGrid2D *MakeUnstructuredGrid2D(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu);

