#pragma once

#include <vapor/Grid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/StretchedGrid.h>

std::vector <float*> AllocateBlocks(    
    const std::vector <size_t> &bs,
    const std::vector <size_t> &dims
);

void MakeTriangle( 
    VAPoR::Grid* grid, 
    float minVal, 
    float maxVal 
);

void MakeConstantField( 
    VAPoR::Grid* grid, 
    float value 
);

void MakeRamp( 
    VAPoR::Grid* grid, 
    float minVal, 
    float maxVal, 
    size_t axis 
);

void CompareIndexToCoords(
    VAPoR::Grid* grid,
    double &rms,                // Root Mean Square error
    size_t &numMissingValues,   // Counter for receiving MissingValue upon query
    size_t &disagreements,      // Counter for when AccessIJK() and GetValue() disagree
    bool nudge=false
);

void TestNodeIterator( 
    const VAPoR::Grid *g, 
    int& count, 
    double& time 
);

void PrintStats( 
    double rms, 
    size_t numMissingValues, 
    size_t disagreements 
);

void TestGrid(
    VAPoR::Grid* grid,
    const std::vector<std::string> &tests,
    float minVal,
    float maxVal
);

VAPoR::StretchedGrid* MakeStretchedGrid(
    const std::vector <size_t> &dims,
    const std::vector <size_t > &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu
);

VAPoR::CurvilinearGrid* MakeCurvilinearTerrainGrid(
    const std::vector<size_t> &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu,
    const std::vector<size_t> &dims
);

VAPoR::LayeredGrid* MakeLayeredGrid(
    const std::vector <size_t> &dims,
    const std::vector <size_t > &bs,
    const std::vector<double> &minu,
    const std::vector<double> &maxu
);

void DeleteHeap();
