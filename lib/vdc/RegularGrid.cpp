#include <iostream>
#include <vector>
#include "vapor/VAssert.h"
#include <cmath>
#include <time.h>
#ifdef Darwin
    #include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
    #include "windows.h"
    #include "Winbase.h"
    #include <limits>
#endif

#include <vapor/utils.h>
#include "vapor/RegularGrid.h"

using namespace std;
using namespace VAPoR;

void RegularGrid::_regularGrid(const CoordType &minu, const CoordType &maxu)
{
    VAssert(minu.size() == maxu.size());

    _delta = {0.0, 0.0, 0.0};

    _geometryDim = 0;
    for (int i = 0; i < minu.size(); i++) {
        if (minu[i] != maxu[i])
            _geometryDim++;
        else
            break;
    }
    VAssert(_geometryDim >= GetNumDimensions());

    _minu = minu;
    _maxu = maxu;

    DimsType dims = GetDimensions();
    for (int i = 0; i < minu.size(); i++) {
        if (dims[i] > 1) {
            _delta[i] = (_maxu[i] - _minu[i]) / (double)(dims[i] - 1);
        } else {
            _delta[i] = 0.0;
        }
    }
}

RegularGrid::RegularGrid(const DimsType &dims, const DimsType &bs, const vector<float *> &blks, const CoordType &minu, const CoordType &maxu) : StructuredGrid(dims, bs, blks)
{
    _regularGrid(minu, maxu);
}

RegularGrid::RegularGrid(const vector<size_t> &dimsv, const vector<size_t> &bsv, const vector<float *> &blks, const vector<double> &minuv, const vector<double> &maxuv)
: StructuredGrid(dimsv, bsv, blks)
{
    VAssert(minuv.size() == maxuv.size());
    VAssert(minuv.size() >= GetNumDimensions());

    CoordType minu = {0.0, 0.0, 0.0};
    CoordType maxu = {0.0, 0.0, 0.0};
    CopyToArr3(minuv, minu);
    CopyToArr3(maxuv, maxu);

    _regularGrid(minu, maxu);
}

DimsType RegularGrid::GetCoordDimensions(size_t dim) const
{
    DimsType dims = {1, 1, 1};

    if (dim == 0) {
        dims[0] = GetDimensions()[0];
    } else if (dim == 1) {
        dims[0] = GetDimensions()[1];
    } else if (dim == 2) {
        dims[0] = GetDimensions()[2];
    }

    return (dims);
}

float RegularGrid::GetValueNearestNeighbor(const CoordType &coords) const
{
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    if (!InsideGrid(cCoords)) return (GetMissingValue());

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) i = (size_t)floor((cCoords[0] - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) j = (size_t)floor((cCoords[1] - _minu[1]) / _delta[1]);

    auto dims = GetDimensions();

    if (GetGeometryDim() == 3)
        if (_delta[2] != 0.0) k = (size_t)floor((cCoords[2] - _minu[2]) / _delta[2]);

    VAssert(i < dims[0]);
    VAssert(j < dims[1]);

    if (GetNumDimensions() == 3) VAssert(k < dims[2]);

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) { iwgt = ((cCoords[0] - _minu[0]) - (i * _delta[0])) / _delta[0]; }

    if (_delta[1] != 0.0) { jwgt = ((cCoords[1] - _minu[1]) - (j * _delta[1])) / _delta[1]; }

    if (GetGeometryDim() == 3) {
        if (_delta[2] != 0.0) { kwgt = ((cCoords[2] - _minu[2]) - (k * _delta[2])) / _delta[2]; }
    }

    if (iwgt > 0.5) i++;
    if (jwgt > 0.5) j++;

    if (GetNumDimensions() == 3 && kwgt > 0.5) k++;


    return (AccessIJK(i, j, k));
}

float RegularGrid::GetValueLinear(const CoordType &coords) const
{
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    float mv = GetMissingValue();
    if (!InsideGrid(cCoords)) return (mv);

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) { i = (size_t)floor((cCoords[0] - _minu[0]) / _delta[0]); }
    if (_delta[1] != 0.0) { j = (size_t)floor((cCoords[1] - _minu[1]) / _delta[1]); }
    if (_delta[2] != 0.0) { k = (size_t)floor((cCoords[2] - _minu[2]) / _delta[2]); }

    auto dims = GetDimensions();
    VAssert(i < dims[0]);
    VAssert(j < dims[1]);
    VAssert(k < dims[2]);

    double xwgt = 0.0;
    double ywgt = 0.0;
    double zwgt = 0.0;

    if (_delta[0] != 0.0) { xwgt = 1.0 - (((cCoords[0] - _minu[0]) - (i * _delta[0])) / _delta[0]); }
    if (_delta[1] != 0.0) { ywgt = 1.0 - (((cCoords[1] - _minu[1]) - (j * _delta[1])) / _delta[1]); }
    if (_delta[2] != 0.0) { zwgt = 1.0 - (((cCoords[2] - _minu[2]) - (k * _delta[2])) / _delta[2]); }

    return (TrilinearInterpolate(i, j, k, xwgt, ywgt, zwgt));
}

void RegularGrid::GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const
{
    minu = _minu;
    maxu = _maxu;
}

void RegularGrid::GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const
{
    DimsType cMin;
    ClampIndex(min, cMin);

    DimsType cMax;
    ClampIndex(max, cMax);

    GetUserCoordinates(cMin, minu);
    GetUserCoordinates(cMax, maxu);
}

void RegularGrid::GetUserCoordinates(const DimsType &indices, CoordType &coords) const
{
    coords = {0.0, 0.0, 0.0};

    DimsType cIndices;
    ClampIndex(indices, cIndices);

    auto dims = GetDimensions();

    for (int i = 0; i < dims.size(); i++) {
        size_t index = cIndices[i];
        VAssert(dims[i] > 0);

        if (index >= dims[i]) { index = dims[i] - 1; }

        coords[i] = cIndices[i] * _delta[i] + _minu[i];
    }
}

bool RegularGrid::GetIndicesCell(const CoordType &coords, DimsType &indices) const
{
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    auto dims = GetDimensions();

    vector<double> wgts;

    VAssert(GetGeometryDim() <= 3);
    for (int i = 0; i < GetGeometryDim(); i++) {
        VAssert(dims[i] > 0);
        if (cCoords[i] < _minu[i] || cCoords[i] > _maxu[i]) { return (false); }

        if (_delta[i] != 0.0) {
            indices[i] = (size_t)floor((cCoords[i] - _minu[i]) / _delta[i]);

            // Edge case
            //
            if (indices[i] == dims[i] - 1) indices[i]--;
        }

        VAssert(indices[i] < dims[i] - 1);
    }

    return (true);
}

bool RegularGrid::InsideGrid(const CoordType &coords) const
{
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    VAssert(GetGeometryDim() <= 3);
    for (int i = 0; i < GetGeometryDim(); i++) {
        if (cCoords[i] < _minu[i]) return (false);

        if (cCoords[i] > _maxu[i]) return (false);
    }

    return (true);
}

RegularGrid::ConstCoordItrRG::ConstCoordItrRG(const RegularGrid *rg, bool begin) : ConstCoordItrAbstract()
{
    _dims = rg->GetDimensions();
    _delta = rg->_delta;
    _minu = rg->_minu;
    _coords = rg->_minu;
    _index = {0, 0, 0};


    if (!begin) { _index = {0, 0, _dims[_dims.size() - 1]}; }
}

RegularGrid::ConstCoordItrRG::ConstCoordItrRG(const ConstCoordItrRG &rhs) : ConstCoordItrAbstract()
{
    _index = rhs._index;
    _dims = rhs._dims;
    _minu = rhs._minu;
    _delta = rhs._delta;
    _coords = rhs._coords;
}

RegularGrid::ConstCoordItrRG::ConstCoordItrRG() : ConstCoordItrAbstract()
{
    _index = {0, 0, 0};
    _dims = {1, 1, 1};
    _minu = {0.0, 0.0, 0.0};
    _delta = {0.0, 0.0, 0.0};
    _coords = {0.0, 0.0, 0.0};
}

void RegularGrid::ConstCoordItrRG::next()
{
    _index[0]++;
    _coords[0] += _delta[0];
    if (_index[0] < _dims[0]) { return; }

    _index[0] = 0;
    _coords[0] = _minu[0];
    _index[1]++;
    _coords[1] += _delta[1];

    if (_index[1] < _dims[1]) { return; }

    _index[1] = 0;
    _coords[1] = _minu[1];
    _index[2]++;
    _coords[2] += _delta[2];

    if (_index[2] < _dims[2]) { return; }

    _index[2] = _dims[2];    // last index;
}

void RegularGrid::ConstCoordItrRG::next(const long &offset)
{
    long maxIndexL = Wasp::VProduct(_dims.data(), _dims.size()) - 1;
    long newIndexL = Wasp::LinearizeCoords(_index.data(), _dims.data(), _dims.size()) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = {0, 0, _dims[_dims.size() - 1]};
        return;
    }

    Wasp::VectorizeCoords(newIndexL, _dims.data(), _index.data(), _dims.size());

    for (int i = 0; i < _dims.size(); i++) { _coords[i] = _index[i] * _delta[i] + _minu[i]; }
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const RegularGrid &rg)
{
    o << "RegularGrid " << endl;
    o << " Min coord ";
    for (int i = 0; i < rg._minu.size(); i++) { o << rg._minu[i] << " "; }
    o << endl;

    o << " Max coord ";
    for (int i = 0; i < rg._maxu.size(); i++) { o << rg._maxu[i] << " "; }
    o << endl;

    o << (const StructuredGrid &)rg;

    return o;
}
};    // namespace VAPoR
