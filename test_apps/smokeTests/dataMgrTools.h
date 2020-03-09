#pragma once

#include <iostream>

#include <vapor/DataMgr.h>

void PrintDimensions( VAPoR::DataMgr &dataMgr );

void PrintMeshes( VAPoR::DataMgr &dataMgr, bool verbose=false );

void PrintCoordVariables( VAPoR::DataMgr &dataMgr );

void PrintTimeCoordinates( VAPoR::DataMgr &dataMgr );

void PrintVariables(
    VAPoR::DataMgr &dataMgr,
    bool verbose  = false,
    bool testVars = false
);

int TestWRF(
    std::string& fileType,
    size_t memsize,
    size_t nthreads,
    std::vector< std::string > &files,
    std::vector< std::string > &options
);
