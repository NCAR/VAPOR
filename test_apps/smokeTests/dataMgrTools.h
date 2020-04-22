#pragma once

#include <iostream>

#include <vapor/DataMgr.h>

void PrintDimensions(const VAPoR::DataMgr &dataMgr);

void PrintMeshes(const VAPoR::DataMgr &dataMgr, bool verbose = false);

void PrintCoordVariables(const VAPoR::DataMgr &dataMgr);

void PrintTimeCoordinates(const VAPoR::DataMgr &dataMgr);

void PrintVariables(const VAPoR::DataMgr &dataMgr, bool verbose = false, bool testVars = false);

void PrintCompressionInfo(const VAPoR::DataMgr &dataMgr, const std::string &varname);

void TestVariables(VAPoR::DataMgr &dataMgr);

int TestDataMgr(const std::string &fileType, size_t memsize, size_t nthreads, const std::vector<std::string> &files, const std::vector<std::string> &options);
