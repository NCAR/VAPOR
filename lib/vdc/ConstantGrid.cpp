#include <limits>

#include "vapor/ConstantGrid.h"

using VAPoR::ConstantGrid;

ConstantGrid::ConstantGrid(float v, size_t d) : _value(v), _topologyDim(d) {}

float ConstantGrid::GetConstantValue() const { return _value; }

float ConstantGrid::GetValue(const VAPoR::CoordType &coords) const { return _value; }
float ConstantGrid::GetValueNearestNeighbor(const VAPoR::CoordType &coords) const { return _value; }

float ConstantGrid::GetValueLinear(const VAPoR::CoordType &coords) const { return _value; }

std::string ConstantGrid::GetType() const
{
    std::string type("ConstantGrid");
    return type;
}

size_t ConstantGrid::GetTopologyDim() const { return _topologyDim; }

void ConstantGrid::GetUserExtentsHelper(VAPoR::CoordType &minu, VAPoR::CoordType &maxu) const
{
    for (int i = 0; i < minu.size(); i++) {
        minu[i] = std::numeric_limits<double>::lowest();
        maxu[i] = std::numeric_limits<double>::max();
    }
}

bool ConstantGrid::InsideGrid(const VAPoR::CoordType &coords) const { return true; }

std::vector<size_t> ConstantGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}

size_t ConstantGrid::GetGeometryDim() const { return 3; }

const VAPoR::DimsType &ConstantGrid::GetNodeDimensions() const { return (GetDimensions()); }

const size_t ConstantGrid::GetNumNodeDimensions() const { return (GetNumDimensions()); }

const VAPoR::DimsType &ConstantGrid::GetCellDimensions() const
{
    _duplicate = GetDimensions();
    return _duplicate;
}

const size_t ConstantGrid::GetNumCellDimensions() const { return (GetNumDimensions()); }

bool ConstantGrid::GetIndicesCell(const VAPoR::CoordType &coords, VAPoR::DimsType &indices) const { return false; }

bool ConstantGrid::GetCellNodes(const VAPoR::DimsType &cindices, std::vector<VAPoR::DimsType> &nodes) const { return false; }

bool ConstantGrid::GetCellNeighbors(const VAPoR::DimsType &cindices, std::vector<VAPoR::DimsType> &cells) const { return false; }

bool ConstantGrid::GetNodeCells(const VAPoR::DimsType &cindices, std::vector<VAPoR::DimsType> &cells) const { return false; }

size_t ConstantGrid::GetMaxVertexPerFace() const { return 0; }

size_t ConstantGrid::GetMaxVertexPerCell() const { return 0; }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordBegin() const { return VAPoR::Grid::ConstCoordItr(); }

VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordEnd() const { return VAPoR::Grid::ConstCoordItr(); }
