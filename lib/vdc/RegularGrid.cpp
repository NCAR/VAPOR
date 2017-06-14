#include <iostream>
#include <vector>
#include <cassert>
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

#include "vapor/RegularGrid.h"

using namespace std;
using namespace VAPoR;

void RegularGrid::_SetExtents(const vector<double> &minu, const vector<double> &maxu)
{
    assert(minu.size() == maxu.size());
    assert(minu.size() == _ndim);

    _minu.clear();
    _maxu.clear();
    _delta.clear();

    _minu = minu;
    _maxu = maxu;

    for (int i = 0; i < _minu.size(); i++) { _delta.push_back((_maxu[i] - _minu[i]) / (double)(_max[i] - _min[i])); }
}

RegularGrid::RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], const vector<float *> &blks)
: StructuredGrid(bs, min, max, periodic, blks)
{
    vector<double> minu;
    vector<double> maxu;

    for (int i = 0; i < _ndim; i++) {
        minu.push_back(extents[i]);
        maxu.push_back(extents[i + 3]);
    }

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid(const vector<size_t> &bs, const vector<size_t> &min, const vector<size_t> &max, const vector<double> &minu, const vector<double> &maxu, const vector<bool> &periodic,
                         const vector<float *> &blks)
: StructuredGrid(bs, min, max, periodic, blks)
{
    assert(bs.size() == _ndim);
    assert(bs.size() == min.size());
    assert(bs.size() == max.size());
    assert(bs.size() == minu.size());
    assert(bs.size() == maxu.size());
    assert(bs.size() == periodic.size());

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid(const vector<size_t> &bs, const vector<size_t> &min, const vector<size_t> &max, const vector<bool> &periodic, const vector<float *> &blks)
: StructuredGrid(bs, min, max, periodic, blks)
{
    assert(bs.size() == _ndim);
    assert(bs.size() == min.size());
    assert(bs.size() == max.size());
    assert(bs.size() == periodic.size());

    vector<double> minu;
    vector<double> maxu;
    for (int i = 0; i < _ndim; i++) {
        minu.push_back(0.0);
        maxu.push_back(1.0);
    }

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], const vector<float *> &blks, float missing_value)
: StructuredGrid(bs, min, max, periodic, blks, missing_value)
{
    vector<double> minu;
    vector<double> maxu;

    for (int i = 0; i < _ndim; i++) {
        minu.push_back(extents[i]);
        maxu.push_back(extents[i + 3]);
    }

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid(const vector<size_t> &bs, const vector<size_t> &min, const vector<size_t> &max, const vector<double> &minu, const vector<double> &maxu, const vector<bool> &periodic,
                         const vector<float *> &blks, float missing_value)
: StructuredGrid(bs, min, max, periodic, blks, missing_value)
{
    assert(bs.size() == _ndim);
    assert(bs.size() == min.size());
    assert(bs.size() == max.size());
    assert(bs.size() == minu.size());
    assert(bs.size() == maxu.size());
    assert(bs.size() == periodic.size());

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid(const vector<size_t> &bs, const vector<size_t> &min, const vector<size_t> &max, const vector<bool> &periodic, const vector<float *> &blks, float missing_value)
: StructuredGrid(bs, min, max, periodic, blks, missing_value)
{
    assert(bs.size() == _ndim);
    assert(bs.size() == min.size());
    assert(bs.size() == max.size());
    assert(bs.size() == periodic.size());

    vector<double> minu;
    vector<double> maxu;

    for (int i = 0; i < _ndim; i++) {
        minu.push_back(0.0);
        maxu.push_back(1.0);
    }

    _SetExtents(minu, maxu);
}

RegularGrid::RegularGrid() : StructuredGrid()
{
    vector<double> minu;
    vector<double> maxu;

    for (int i = 0; i < _ndim; i++) {
        minu.push_back(0.0);
        maxu.push_back(1.0);
    }

    _SetExtents(minu, maxu);
}

RegularGrid::~RegularGrid() {}

float RegularGrid::_GetValueNearestNeighbor(double x, double y, double z) const
{
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) i = (size_t)floor((x - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) j = (size_t)floor((y - _minu[1]) / _delta[1]);

    if (_ndim == 3)
        if (_delta[2] != 0.0) k = (size_t)floor((z - _minu[2]) / _delta[2]);

    assert(i <= (_max[0] - _min[0]));
    assert(j <= (_max[1] - _min[1]));

    if (_ndim == 3) assert(k <= (_max[2] - _min[2]));

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) iwgt = ((x - _minu[0]) - (i * _delta[0])) / _delta[0];
    if (_delta[1] != 0.0) jwgt = ((y - _minu[1]) - (j * _delta[1])) / _delta[1];
    if (_ndim == 3)
        if (_delta[2] != 0.0) kwgt = ((z - _minu[2]) - (k * _delta[2])) / _delta[2];

    if (iwgt > 0.5) i++;
    if (jwgt > 0.5) j++;

    if (_ndim == 3)
        if (kwgt > 0.5) k++;

    return (AccessIJK(i, j, k));
}

float RegularGrid::_GetValueLinear(double x, double y, double z) const
{
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    if (_delta[0] != 0.0) i = (size_t)floor((x - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) j = (size_t)floor((y - _minu[1]) / _delta[1]);

    if (_ndim == 3)
        if (_delta[2] != 0.0) k = (size_t)floor((z - _minu[2]) / _delta[2]);

    assert(i <= (_max[0] - _min[0]));
    assert(j <= (_max[1] - _min[1]));

    if (_ndim == 3) assert(k <= (_max[2] - _min[2]));

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) iwgt = ((x - _minu[0]) - (i * _delta[0])) / _delta[0];
    if (_delta[1] != 0.0) jwgt = ((y - _minu[1]) - (j * _delta[1])) / _delta[1];

    if (_ndim == 3)
        if (_delta[2] != 0.0) kwgt = ((z - _minu[2]) - (k * _delta[2])) / _delta[2];

    double p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = AccessIJK(i, j, k);
    if (p0 == _missingValue) return (_missingValue);

    if (iwgt != 0.0) {
        p1 = AccessIJK(i + 1, j, k);
        if (p1 == _missingValue) return (_missingValue);
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i, j + 1, k);
        if (p2 == _missingValue) return (_missingValue);
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i + 1, j + 1, k);
        if (p3 == _missingValue) return (_missingValue);
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i, j, k + 1);
        if (p4 == _missingValue) return (_missingValue);
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i + 1, j, k + 1);
        if (p5 == _missingValue) return (_missingValue);
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i, j + 1, k + 1);
        if (p6 == _missingValue) return (_missingValue);
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i + 1, j + 1, k + 1);
        if (p7 == _missingValue) return (_missingValue);
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}

void RegularGrid::_ClampCoord(double &x, double &y, double &z) const
{
    if (x < _minu[0] && _periodic[0]) {
        while (x < _minu[0]) x += _maxu[0] - _minu[0];
    }
    if (x > _maxu[0] && _periodic[0]) {
        while (x > _maxu[0]) x -= _maxu[0] - _minu[0];
    }

    if (y < _minu[1] && _periodic[1]) {
        while (y < _minu[1]) y += _maxu[1] - _minu[1];
    }
    if (y > _maxu[1] && _periodic[1]) {
        while (y > _maxu[1]) y -= _maxu[1] - _minu[1];
    }

    if (_ndim == 3) {
        if (z < _minu[2] && _periodic[2]) {
            while (z < _minu[2]) z += _maxu[2] - _minu[2];
        }
        if (z > _maxu[2] && _periodic[2]) {
            while (z > _maxu[2]) z -= _maxu[2] - _minu[2];
        }
    }

    //
    // Handle coordinates for dimensions of length 1
    //
    if (_min[0] == _max[0]) x = _minu[0];
    if (_min[1] == _max[1]) y = _minu[1];

    if (_ndim == 3)
        if (_min[2] == _max[2]) z = _minu[2];
}

void RegularGrid::GetUserExtents(vector<double> &minu, vector<double> &maxu) const
{
    minu = _minu;
    maxu = _maxu;
}

void RegularGrid::GetUserExtents(double extents[6]) const
{
    for (int i = 0; i < 6; i++) extents[i] = 0.0;

    vector<double> minu;
    vector<double> maxu;
    GetUserExtents(minu, maxu);

    assert(minu.size() == maxu.size());
    for (int i = 0; i < minu.size(); i++) {
        extents[i] = minu[i];
        extents[i + 3] = maxu[i];
    }
}

void RegularGrid::GetBoundingBox(const vector<size_t> &min, const vector<size_t> &max, vector<double> &minu, vector<double> &maxu) const
{
    assert(min.size() == max.size());
    assert(min.size() <= _ndim);

    size_t a_min[] = {0, 0, 0};
    size_t a_max[] = {0, 0, 0};
    double a_minu[] = {0, 0, 0};
    double a_maxu[] = {0, 0, 0};

    for (int i = 0; i < _ndim; i++) {
        a_min[i] = min[i];
        a_max[i] = max[i];
    }

    RegularGrid::GetUserCoordinates(a_min[0], a_min[1], a_min[2], &(a_minu[0]), &(a_minu[1]), &(a_minu[2]));
    RegularGrid::GetUserCoordinates(a_max[0], a_max[1], a_max[2], &(a_maxu[0]), &(a_maxu[1]), &(a_maxu[2]));

    for (int i = 0; i < _ndim; i++) {
        minu.push_back(a_minu[i]);
        maxu.push_back(a_maxu[i]);
    }
}

void RegularGrid::GetBoundingBox(const size_t min[3], const size_t max[3], double extents[6]) const
{
    for (int i = 0; i < 6; i++) extents[i] = 0.0;

    vector<size_t> v_min;
    vector<size_t> v_max;
    for (int i = 0; i < _ndim; i++) {
        v_min.push_back(min[i]);
        v_max.push_back(max[i]);
    }

    vector<double> v_minu;
    vector<double> v_maxu;
    GetBoundingBox(v_min, v_max, v_minu, v_maxu);

    assert(v_minu.size() == v_maxu.size());
    for (int i = 0; i < v_minu.size(); i++) {
        extents[i] = v_minu[i];
        extents[i + 3] = v_maxu[i];
    }
}

void RegularGrid::GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const
{
    assert(minu.size() == maxu.size());
    assert(minu.size() <= _ndim);

    min.clear();
    max.clear();

    double a_minu[] = {0.0, 0.0, 0.0};
    double a_maxu[] = {0.0, 0.0, 0.0};
    for (int i = 0; i < minu.size(); i++) {
        a_minu[i] = minu[i];
        a_maxu[i] = maxu[i];
    }

    // Find the closest voxels to the desired user coordinates
    //
    size_t temp_min[3], temp_max[3];
    RegularGrid::GetIJKIndex(a_minu[0], a_minu[1], a_minu[2], &temp_min[0], &temp_min[1], &temp_min[2]);
    RegularGrid::GetIJKIndex(a_maxu[0], a_maxu[1], a_maxu[2], &temp_max[0], &temp_max[1], &temp_max[2]);

    // Map the the closest voxels back to their actual user coordinates
    //
    double temp_minu[3], temp_maxu[3];
    RegularGrid::GetUserCoordinates(temp_min[0], temp_min[1], temp_min[2], &temp_minu[0], &temp_minu[1], &temp_minu[2]);
    RegularGrid::GetUserCoordinates(temp_max[0], temp_max[1], temp_max[2], &temp_maxu[0], &temp_maxu[1], &temp_maxu[2]);

    // The actual user coordinates may be inside the desired region. If so
    // we need to expand the selected region
    //
    vector<size_t> dims;
    GetDimensions(dims);
    for (int i = 0; i < dims.size(); i++) {
        if (temp_minu[i] > minu[i] && (temp_min[i] > 0)) { temp_min[i]--; }
        if (temp_maxu[i] < maxu[i] && (temp_max[i] < (dims[i] - 1))) { temp_max[i]++; }
        min.push_back(temp_min[i]);
        max.push_back(temp_max[i]);
    }
}

int RegularGrid::GetUserCoordinates(size_t i, size_t j, size_t k, double *x, double *y, double *z) const
{
    if (i > _max[0] - _min[0]) return (-1);
    if (j > _max[1] - _min[1]) return (-1);

    if (_ndim == 3)
        if (k > _max[2] - _min[2]) return (-1);

    *x = i * _delta[0] + _minu[0];
    *y = j * _delta[1] + _minu[1];

    if (_ndim == 3) *z = k * _delta[2] + _minu[2];

    return (0);
}

void RegularGrid::GetIJKIndex(double x, double y, double z, size_t *i, size_t *j, size_t *k) const
{
    _ClampCoord(x, y, z);

    // Make sure the point xyz is within the bounds _minu, _maxu.
    //
    if (x < _minu[0]) x = _minu[0];
    if (x > _maxu[0]) x = _maxu[0];

    if (y < _minu[1]) y = _minu[1];
    if (y > _maxu[1]) y = _maxu[1];

    if (_ndim == 3) {
        if (z < _minu[2]) z = _minu[2];
        if (z > _maxu[2]) z = _maxu[2];
    }

    *i = *j = *k = 0;

    if (_delta[0] != 0.0) *i = (size_t)floor((x - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) *j = (size_t)floor((y - _minu[1]) / _delta[1]);

    if (_ndim == 3)
        if (_delta[2] != 0.0) *k = (size_t)floor((z - _minu[2]) / _delta[2]);

    assert(*i <= (_max[0] - _min[0]));
    assert(*j <= (_max[1] - _min[1]));

    if (_ndim == 3) assert(*k <= (_max[2] - _min[2]));

    double iwgt = 0.0;
    double jwgt = 0.0;
    double kwgt = 0.0;

    if (_delta[0] != 0.0) iwgt = ((x - _minu[0]) - (*i * _delta[0])) / _delta[0];
    if (_delta[1] != 0.0) jwgt = ((y - _minu[1]) - (*j * _delta[1])) / _delta[1];

    if (_ndim == 3)
        if (_delta[2] != 0.0) kwgt = ((z - _minu[2]) - (*k * _delta[2])) / _delta[2];

    if (iwgt > 0.5) *i += 1;
    if (jwgt > 0.5) *j += 1;

    if (_ndim == 3)
        if (kwgt > 0.5) *k += 1;
}

void RegularGrid::GetIJKIndexFloor(double x, double y, double z, size_t *i, size_t *j, size_t *k) const
{
    _ClampCoord(x, y, z);

    // Make sure the point xyz is within the bounds _minu, _maxu.
    //
    if (x < _minu[0]) x = _minu[0];
    if (x > _maxu[0]) x = _maxu[0];

    if (y < _minu[1]) y = _minu[1];
    if (y > _maxu[1]) y = _maxu[1];

    if (_ndim == 3) {
        if (z < _minu[2]) z = _minu[2];
        if (z > _maxu[2]) z = _maxu[2];
    }

    *i = *j = *k = 0;

    if (_delta[0] != 0.0) *i = (size_t)floor((x - _minu[0]) / _delta[0]);
    if (_delta[1] != 0.0) *j = (size_t)floor((y - _minu[1]) / _delta[1]);

    if (_ndim == 3)
        if (_delta[2] != 0.0) *k = (size_t)floor((z - _minu[2]) / _delta[2]);

    assert(*i <= (_max[0] - _min[0]));
    assert(*j <= (_max[1] - _min[1]));

    if (_ndim == 3) assert(*k <= (_max[2] - _min[2]));
}

bool RegularGrid::InsideGrid(double x, double y, double z) const
{
    if (_minu[0] < _maxu[0]) {
        if (x < _minu[0] || x > _maxu[0]) return (false);
    } else {
        if (x > _minu[0] || x < _maxu[0]) return (false);
    }
    if (_minu[1] < _maxu[1]) {
        if (y < _minu[1] || y > _maxu[1]) return (false);
    } else {
        if (y > _minu[1] || y < _maxu[1]) return (false);
    }

    if (_ndim == 3) {
        if (_minu[2] < _maxu[2]) {
            if (z < _minu[2] || z > _maxu[2]) return (false);
        } else {
            if (z > _minu[2] || z < _maxu[2]) return (false);
        }
    }

    return (true);
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const RegularGrid &rg)
{
    o << "RegularGrid " << endl << " Min coord " << rg._minu[0] << " " << rg._minu[1] << " ";
    if (rg._ndim == 3) { o << rg._minu[2]; }
    o << endl;

    o << " Max coord " << rg._maxu[0] << " " << rg._maxu[1] << " ";
    if (rg._ndim == 3) { o << rg._maxu[2] << endl; }
    o << endl;

    o << (const StructuredGrid &)rg;

    return o;
}
};    // namespace VAPoR
