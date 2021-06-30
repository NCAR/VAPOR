#include <limits>

#include "vapor/ConstantGrid.h"

using VAPoR::ConstantGrid;

ConstantGrid::ConstantGrid(float v, size_t d) : _value(v), _topologyDim(d) {}

float ConstantGrid::GetConstantValue() const { return _value; }

float ConstantGrid::GetValue(const VAPoR::DblArr3 &coords) const { return _value; }
float ConstantGrid::GetValueNearestNeighbor(const VAPoR::DblArr3 &coords) const { return _value; }

float ConstantGrid::GetValueLinear(const VAPoR::DblArr3 &coords) const { return _value; }

std::string ConstantGrid::GetType() const
{
    std::string type("ConstantGrid");
    return type;
}

size_t ConstantGrid::GetTopologyDim() const { return _topologyDim; }

void ConstantGrid::GetUserExtentsHelper(VAPoR::DblArr3 &minu, VAPoR::DblArr3 &maxu) const
{
    for (int i = 0; i < minu.size(); i++) {
        minu[i] = std::numeric_limits<double>::lowest();
        maxu[i] = std::numeric_limits<double>::max();
    }
}

bool ConstantGrid::InsideGrid(const VAPoR::DblArr3 &coords) const { return true; }

std::vector<size_t> ConstantGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}

size_t ConstantGrid::GetGeometryDim() const { return 3; }

const std::vector<size_t> &ConstantGrid::GetNodeDimensions() const
{
    auto tmp = GetDimensions();
    _duplicate = {tmp[0], tmp[1], tmp[2]};
    _duplicate.resize(GetNumDimensions());
    return _duplicate;
}

const std::vector<size_t> &ConstantGrid::GetCellDimensions() const
{
    auto tmp = GetDimensions();
    _duplicate = {tmp[0], tmp[1], tmp[2]};
    _duplicate.resize(GetNumDimensions());
    return _duplicate;
}

bool ConstantGrid::GetIndicesCell(const VAPoR::DblArr3 &coords, VAPoR::Size_tArr3 &indices) const { return false; }

bool ConstantGrid::GetCellNodes(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &nodes) const { return false; }

bool ConstantGrid::GetCellNeighbors(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &cells) const { return false; }

bool ConstantGrid::GetNodeCells(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &cells) const { return false; }

size_t ConstantGrid::GetMaxVertexPerFace() const { return 0; }

size_t ConstantGrid::GetMaxVertexPerCell() const { return 0; }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordBegin() const { return VAPoR::Grid::ConstCoordItr(); }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordEnd() const { return VAPoR::Grid::ConstCoordItr(); }
