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

void RegularGrid::_SetExtents(const vector<double> &minu, const vector<double> &maxu)
{
    VAssert(minu.size() == maxu.size());

    _minu.clear();
    _maxu.clear();
    _delta.clear();

    _minu = minu;
    _maxu = maxu;

    vector<size_t> dims = GetDimensions();
    for (int i = 0; i < _minu.size(); i++) {
        if (dims[i] > 1) {
            _delta.push_back((_maxu[i] - _minu[i]) / (double)(dims[i] - 1));
        } else {
            _delta.push_back(0.0);
        }
    }
}

RegularGrid::RegularGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const vector<double> &minu, const vector<double> &maxu) : StructuredGrid(dims, bs, blks)
{
    VAssert(minu.size() == maxu.size());
    VAssert(minu.size() >= GetDimensions().size());

    _SetExtents(minu, maxu);
}

size_t RegularGrid::GetGeometryDim() const { return (_minu.size()); }

vector<size_t> RegularGrid::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return (vector<size_t>(1, GetDimensions()[0]));
    } else if (dim == 1) {
        return (vector<size_t>(1, GetDimensions()[1]));
    } else if (dim == 2) {
        if (GetDimensions().size() == 3) {
            return (vector<size_t>(1, GetDimensions()[2]));
        } else {
            return (vector<size_t>(1, 1));
        }
    } else {
        return (vector<size_t>(1, 1));
    }
}

float RegularGrid::GetValueNearestNeighbor(const std::vector<double> &coords) const
{
    std::vector<double> cCoords = coords;
    ClampCoord(cCoords);

    if (!InsideGrid(cCoords)) return (GetMissingValue());

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) i = (size_t)floor((cCoords[0] - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) j = (size_t)floor((cCoords[1] - _minu[1]) / _delta[1]);

    vector<size_t> dims = GetDimensions();

    if (dims.size() == 3)
        if (_delta[2] != 0.0) k = (size_t)floor((cCoords[2] - _minu[2]) / _delta[2]);

    VAssert(i < dims[0]);
    VAssert(j < dims[1]);

    if (dims.size() == 3) VAssert(k < dims[2]);

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) { iwgt = ((cCoords[0] - _minu[0]) - (i * _delta[0])) / _delta[0]; }

    if (_delta[1] != 0.0) { jwgt = ((cCoords[1] - _minu[1]) - (j * _delta[1])) / _delta[1]; }

    if (dims.size() == 3) {
        if (_delta[2] != 0.0) { kwgt = ((cCoords[2] - _minu[2]) - (k * _delta[2])) / _delta[2]; }
    }

    if (iwgt > 0.5) i++;
    if (jwgt > 0.5) j++;

    if (dims.size() == 3) {
        if (kwgt > 0.5) k++;
    }

    return (AccessIJK(i, j, k));
}

float RegularGrid::GetValueLinear(const std::vector<double> &coords) const
{
    std::vector<double> cCoords = coords;
    ClampCoord(cCoords);

    if (!InsideGrid(cCoords)) return (GetMissingValue());

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) { i = (size_t)floor((cCoords[0] - _minu[0]) / _delta[0]); }
    if (_delta[1] != 0.0) { j = (size_t)floor((cCoords[1] - _minu[1]) / _delta[1]); }

    vector<size_t> dims = GetDimensions();

    if (dims.size() == 3 && _delta[2] != 0.0) { k = (size_t)floor((cCoords[2] - _minu[2]) / _delta[2]); }

    VAssert(i < dims[0]);
    VAssert(j < dims[1]);

    if (dims.size() == 3) { VAssert(k < dims[2]); }

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) { iwgt = ((cCoords[0] - _minu[0]) - (i * _delta[0])) / _delta[0]; }
    if (_delta[1] != 0.0) { jwgt = ((cCoords[1] - _minu[1]) - (j * _delta[1])) / _delta[1]; }

    if (dims.size() == 3 && _delta[2] != 0.0) { kwgt = ((cCoords[2] - _minu[2]) - (k * _delta[2])) / _delta[2]; }

    float  missingValue = GetMissingValue();
    double p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = AccessIJK(i, j, k);
    if (p0 == missingValue) return (missingValue);

    if (iwgt != 0.0) {
        p1 = AccessIJK(i + 1, j, k);
        if (p1 == missingValue) return (missingValue);
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i, j + 1, k);
        if (p2 == missingValue) return (missingValue);
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i + 1, j + 1, k);
        if (p3 == missingValue) return (missingValue);
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i, j, k + 1);
        if (p4 == missingValue) return (missingValue);
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i + 1, j, k + 1);
        if (p5 == missingValue) return (missingValue);
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i, j + 1, k + 1);
        if (p6 == missingValue) return (missingValue);
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i + 1, j + 1, k + 1);
        if (p7 == missingValue) return (missingValue);
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}

void RegularGrid::GetUserExtents(vector<double> &minu, vector<double> &maxu) const
{
    minu = _minu;
    maxu = _maxu;
}

void RegularGrid::GetBoundingBox(const vector<size_t> &min, const vector<size_t> &max, vector<double> &minu, vector<double> &maxu) const
{
    vector<size_t> cMin = min;
    ClampIndex(cMin);

    vector<size_t> cMax = max;
    ClampIndex(cMax);

    VAssert(cMin.size() == cMax.size());

    Grid::GetUserCoordinates(cMin, minu);
    Grid::GetUserCoordinates(cMax, maxu);
}

void RegularGrid::GetUserCoordinates(const size_t indices[], double coords[]) const
{
    size_t cIndices[3];
    ClampIndex(indices, cIndices);

    const vector<size_t> &dims = GetDimensions();

    for (int i = 0; i < dims.size(); i++) {
        size_t index = cIndices[i];

        if (index >= dims[i]) { index = dims[i] - 1; }

        coords[i] = cIndices[i] * _delta[i] + _minu[i];
    }
}

bool RegularGrid::GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const
{
    indices.clear();

    std::vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> dims = GetDimensions();

    vector<double> wgts;

    for (int i = 0; i < clampedCoords.size(); i++) {
        indices.push_back(0);

        if (clampedCoords[i] < _minu[i] || clampedCoords[i] > _maxu[i]) { return (false); }

        if (_delta[i] != 0.0) { indices[i] = (size_t)floor((clampedCoords[i] - _minu[i]) / _delta[i]); }

        VAssert(indices[i] < dims[i]);
    }

    return (true);
}

bool RegularGrid::InsideGrid(const std::vector<double> &coords) const
{
    std::vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    for (int i = 0; i < clampedCoords.size(); i++) {
        if (clampedCoords[i] < _minu[i]) return (false);

        if (clampedCoords[i] > _maxu[i]) return (false);
    }

    return (true);
}

RegularGrid::ConstCoordItrRG::ConstCoordItrRG(const RegularGrid *rg, bool begin) : ConstCoordItrAbstract()
{
    _dims = rg->GetDimensions();
    _minu = rg->_minu;
    _delta = rg->_delta;
    _coords = rg->_minu;

    _index = vector<size_t>(_dims.size(), 0);
    if (!begin) { _index[_dims.size() - 1] = _dims[_dims.size() - 1]; }
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
    _index.clear();
    _dims.clear();
    _minu.clear();
    _delta.clear();
    _coords.clear();
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

    if (_dims.size() == 2) { return; }

    _index[1] = 0;
    _coords[1] = _minu[1];
    _index[2]++;
    _coords[2] += _delta[2];
}

void RegularGrid::ConstCoordItrRG::next(const long &offset)
{
    if (!_index.size()) return;

    static vector<size_t> maxIndex(_dims.size());
    ;
    for (int i = 0; i < _dims.size(); i++) maxIndex[i] = _dims[i] - 1;

    long maxIndexL = Wasp::LinearizeCoords(maxIndex, _dims);
    long newIndexL = Wasp::LinearizeCoords(_index, _dims) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = vector<size_t>(_dims.size(), 0);
        _index[_dims.size() - 1] = _dims[_dims.size() - 1];
        return;
    }

    _index = Wasp::VectorizeCoords(newIndexL, _dims);

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
