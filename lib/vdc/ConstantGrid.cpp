#include <limits>

#include "vapor/ConstantGrid.h"

using VAPoR::ConstantGrid;

ConstantGrid::ConstantGrid(float v, size_t d) : _value(v), _topologyDim(d) {}

float ConstantGrid::GetConstantValue() const { return _value; }

float ConstantGrid::GetValue(const std::vector<double> &coords) const { return _value; }

float ConstantGrid::GetValueNearestNeighbor(const std::vector<double> &coords) const { return _value; }

float ConstantGrid::GetValueLinear(const std::vector<double> &coords) const { return _value; }

std::string ConstantGrid::GetType() const
{
    std::string type("ConstantGrid");
    return type;
}

size_t ConstantGrid::GetTopologyDim() const { return _topologyDim; }

void ConstantGrid::GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const
{
    minu.resize(_topologyDim, std::numeric_limits<double>::min());
    maxu.resize(_topologyDim, std::numeric_limits<double>::max());
}

bool ConstantGrid::InsideGrid(const std::vector<double> &coords) const { return true; }

std::vector<size_t> ConstantGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}

size_t ConstantGrid::GetGeometryDim() const { return 0; }

const std::vector<size_t> &ConstantGrid::GetNodeDimensions() const { return (GetDimensions()); }

const std::vector<size_t> &ConstantGrid::GetCellDimensions() const { return (GetDimensions()); }

bool ConstantGrid::GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const { return false; }

bool ConstantGrid::GetCellNodes(const size_t cindices[], size_t nodes[], int &n) const { return false; }

bool ConstantGrid::GetCellNeighbors(const std::vector<size_t> &cindices, std::vector<std::vector<size_t>> &cells) const { return false; }

bool ConstantGrid::GetNodeCells(const std::vector<size_t> &indices, std::vector<std::vector<size_t>> &cells) const { return false; }

size_t ConstantGrid::GetMaxVertexPerFace() const { return 0; }

size_t ConstantGrid::GetMaxVertexPerCell() const { return 0; }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordBegin() const { return VAPoR::Grid::ConstCoordItr(); }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordEnd() const { return VAPoR::Grid::ConstCoordItr(); }
