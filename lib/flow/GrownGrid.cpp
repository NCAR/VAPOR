#include "GrownGrid.h"

using VAPoR::GrownGrid;

// Constructor
GrownGrid::GrownGrid(const VAPoR::Grid *gp, VAPoR::DataMgr *mp, float z) : _grid2d(gp), _dataMgr(mp), _defaultZ(z) {}

// Destructor
GrownGrid::~GrownGrid()
{
    if (_grid2d && _dataMgr) {
        _dataMgr->UnlockGrid(_grid2d);
        delete _grid2d;
    }
}

float GrownGrid::GetDefaultZ() const { return _defaultZ; }

std::string GrownGrid::GetType() const
{
    std::string type("GrownGrid");
    return type;
}

float GrownGrid::GetMissingValue() const { return _grid2d->GetMissingValue(); }

void GrownGrid::GetUserExtentsHelper(VAPoR::DblArr3 &minu, VAPoR::DblArr3 &maxu) const
{
    _grid2d->GetUserExtents(minu, maxu);

    if (_grid2d->GetGeometryDim() < 3) {
        minu[2] = _defaultZ;
        maxu[2] = _defaultZ;
    }
}

bool GrownGrid::InsideGrid(const VAPoR::DblArr3 &coords) const
{
    // Note that we don't use defaultZ to decide if a position is inside of
    // a grid or not.
    return (_grid2d->InsideGrid(coords));
}

float GrownGrid::GetValue(const VAPoR::DblArr3 &coords) const { return _grid2d->GetValue(coords); }

float GrownGrid::GetValueNearestNeighbor(const VAPoR::DblArr3 &coords) const { return _grid2d->GetValue(coords); }

float GrownGrid::GetValueLinear(const VAPoR::DblArr3 &coords) const { return _grid2d->GetValue(coords); }

std::vector<size_t> GrownGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}

//
// Start meaningless functions!
//
size_t GrownGrid::GetGeometryDim() const { return 3; }

const std::vector<size_t> &GrownGrid::GetNodeDimensions() const { return (GetDimensions()); }

const std::vector<size_t> &GrownGrid::GetCellDimensions() const { return (GetDimensions()); }

bool GrownGrid::GetIndicesCell(const VAPoR::DblArr3 &coords, VAPoR::Size_tArr3 &indices) const { return false; }

bool GrownGrid::GetCellNodes(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &nodes) const { return false; }

bool GrownGrid::GetCellNeighbors(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &nodes) const { return false; }

bool GrownGrid::GetNodeCells(const VAPoR::Size_tArr3 &cindices, std::vector<VAPoR::Size_tArr3> &nodes) const { return false; }

size_t GrownGrid::GetMaxVertexPerFace() const { return 0; }

size_t GrownGrid::GetMaxVertexPerCell() const { return 0; }

VAPoR::Grid::ConstCoordItr GrownGrid::ConstCoordBegin() const { return VAPoR::Grid::ConstCoordItr(); }

VAPoR::Grid::ConstCoordItr GrownGrid::ConstCoordEnd() const { return VAPoR::Grid::ConstCoordItr(); }
