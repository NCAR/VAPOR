#include <iostream>
#include <cassert>
#include <cmath>
#include <vapor/StretchedGrid.h>
#ifdef _WINDOWS
    #pragma warning(disable : 4251 4100)
#endif

using namespace std;
using namespace VAPoR;

StretchedGrid::StretchedGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], float **blks, const vector<double> &xcoords,
                             const vector<double> &ycoords, const vector<double> &zcoords)
: RegularGrid(bs, min, max, extents, periodic, blks)
{
    for (int i = 0; i < 3; i++) {
        _min[i] = min[i];
        _max[i] = max[i];
        _delta[i] = 0.0;
        _extents[i] = extents[i];
        _extents[i + 3] = extents[i + 3];
    }

    _xcoords.clear();
    _ycoords.clear();
    _zcoords.clear();

    size_t xdim = max[0] - min[0] + 1;
    if (xcoords.size() >= xdim) {
        for (int i = 0; i < xdim; i++) _xcoords.push_back(xcoords[i]);
        _extents[0] = _xcoords[0];
        _extents[3] = _xcoords[xdim - 1];
    } else {
        _delta[0] = (extents[3] - extents[0]) / (double)(_max[0] - _min[0]);
    }

    size_t ydim = max[1] - min[1] + 1;
    if (ycoords.size() >= ydim) {
        for (int i = 0; i < ydim; i++) _ycoords.push_back(ycoords[i]);
        _extents[1] = _ycoords[0];
        _extents[4] = _ycoords[ydim - 1];
    } else {
        _delta[1] = (extents[4] - extents[1]) / (double)(_max[1] - _min[1]);
    }

    size_t zdim = max[2] - min[2] + 1;
    if (zcoords.size() >= zdim) {
        for (int i = 0; i < zdim; i++) _zcoords.push_back(zcoords[i]);
        _extents[2] = _zcoords[0];
        _extents[5] = _zcoords[zdim - 1];
    } else {
        _delta[2] = (extents[5] - extents[2]) / (double)(_max[2] - _min[2]);
    }
    RegularGrid::_SetExtents(_extents);
}

StretchedGrid::StretchedGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], float **blks, const vector<double> &xcoords,
                             const vector<double> &ycoords, const vector<double> &zcoords, float missing_value)
: RegularGrid(bs, min, max, extents, periodic, blks, missing_value)
{
    for (int i = 0; i < 3; i++) {
        _min[i] = min[i];
        _max[i] = max[i];
        _delta[i] = 0.0;
        _extents[i] = extents[i];
        _extents[i + 3] = extents[i + 3];
    }

    _xcoords.clear();
    _ycoords.clear();
    _zcoords.clear();

    size_t xdim = max[0] - min[0] + 1;
    if (xcoords.size() >= xdim) {
        for (int i = 0; i < xdim; i++) _xcoords.push_back(xcoords[i]);

        _extents[0] = _xcoords[0];
        _extents[3] = _xcoords[xdim - 1];
    } else {
        _delta[0] = (extents[3] - extents[0]) / (double)(_max[0] - _min[0]);
    }

    size_t ydim = max[1] - min[1] + 1;
    if (ycoords.size() >= ydim) {
        for (int i = 0; i < ydim; i++) _ycoords.push_back(ycoords[i]);
        _extents[1] = _ycoords[0];
        _extents[4] = _ycoords[ydim - 1];
    } else {
        _delta[1] = (extents[4] - extents[1]) / (double)(_max[1] - _min[1]);
    }

    size_t zdim = max[2] - min[2] + 1;
    if (zcoords.size() >= zdim) {
        for (int i = 0; i < zdim; i++) _zcoords.push_back(zcoords[i]);
        _extents[2] = _zcoords[0];
        _extents[5] = _zcoords[zdim - 1];
    } else {
        _delta[2] = (extents[5] - extents[2]) / (double)(_max[2] - _min[2]);
    }
    RegularGrid::_SetExtents(_extents);
}

float StretchedGrid::GetValue(double x, double y, double z) const
{
    RegularGrid::_ClampCoord(x, y, z);

    // At this point xyz should be within the bounds _minu, _maxu
    //
    if (!RegularGrid::InsideGrid(x, y, z)) return (GetMissingValue());

    int order = RegularGrid::GetInterpolationOrder();
    if (order == 0) {
        return (_GetValueNearestNeighbor(x, y, z));
    } else if (order == 1) {
        return (_GetValueLinear(x, y, z));
    } else {
        return (quadraticInterpolation(x, y, z));
    }
}

float StretchedGrid::_GetValueNearestNeighbor(double x, double y, double z) const
{
    // Get the indecies of the cell containing the point
    //
    size_t i, j, k;
    GetIJKIndexFloor(x, y, z, &i, &j, &k);

    double iwgt = 0.0;
    if (i < (_max[0] - _min[0])) {
        if (_xcoords.size()) {
            iwgt = (x - _xcoords[i]) / (_xcoords[i + 1] - _xcoords[i]);
        } else if (_delta[0] != 0.0) {
            iwgt = ((x - _extents[0]) - (i * _delta[0])) / _delta[0];
        }
    }
    double jwgt = 0.0;
    if (j < (_max[1] - _min[1])) {
        if (_ycoords.size()) {
            jwgt = (y - _ycoords[j]) / (_ycoords[j + 1] - _ycoords[j]);
        } else if (_delta[1] != 0.0) {
            jwgt = ((y - _extents[1]) - (j * _delta[1])) / _delta[1];
        }
    }
    double kwgt = 0.0;
    if (k < (_max[2] - _min[2])) {
        if (_zcoords.size()) {
            kwgt = (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
        } else if (_delta[2] != 0.0) {
            kwgt = ((z - _extents[2]) - (k * _delta[2])) / _delta[2];
        }
    }

    if (iwgt > 0.5) i++;
    if (jwgt > 0.5) j++;
    if (kwgt > 0.5) k++;

    return (AccessIJK(i, j, k));
}

float StretchedGrid::_GetValueLinear(double x, double y, double z) const
{
    // Get the indecies of the cell containing the point
    //
    size_t i, j, k;
    GetIJKIndexFloor(x, y, z, &i, &j, &k);

    double iwgt = 0.0;
    if (i < (_max[0] - _min[0])) {
        if (_xcoords.size()) {
            iwgt = (x - _xcoords[i]) / (_xcoords[i + 1] - _xcoords[i]);
        } else if (_delta[0] != 0.0) {
            iwgt = ((x - _extents[0]) - (i * _delta[0])) / _delta[0];
        }
    }
    double jwgt = 0.0;
    if (j < (_max[1] - _min[1])) {
        if (_ycoords.size()) {
            jwgt = (y - _ycoords[j]) / (_ycoords[j + 1] - _ycoords[j]);
        } else if (_delta[1] != 0.0) {
            jwgt = ((y - _extents[1]) - (j * _delta[1])) / _delta[1];
        }
    }
    double kwgt = 0.0;
    if (k < (_max[2] - _min[2])) {
        if (_zcoords.size()) {
            kwgt = (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
        } else if (_delta[2] != 0.0) {
            kwgt = ((z - _extents[2]) - (k * _delta[2])) / _delta[2];
        }
    }

    double p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = AccessIJK(i, j, k);
    if (p0 == GetMissingValue()) return (GetMissingValue());

    if (iwgt != 0.0) {
        p1 = AccessIJK(i + 1, j, k);
        if (p1 == GetMissingValue()) return (GetMissingValue());
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i, j + 1, k);
        if (p2 == GetMissingValue()) return (GetMissingValue());
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i + 1, j + 1, k);
        if (p3 == GetMissingValue()) return (GetMissingValue());
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i, j, k + 1);
        if (p4 == GetMissingValue()) return (GetMissingValue());
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i + 1, j, k + 1);
        if (p5 == GetMissingValue()) return (GetMissingValue());
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i, j + 1, k + 1);
        if (p6 == GetMissingValue()) return (GetMissingValue());
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i + 1, j + 1, k + 1);
        if (p7 == GetMissingValue()) return (GetMissingValue());
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}

float StretchedGrid::_GetValueQuadratic(double x, double y, double z) const
{
    // Get the indecies of the cell containing the point
    //
    size_t i, j, k;
    GetIJKIndexFloor(x, y, z, &i, &j, &k);

    double iwgt = 0.0;
    if (i < (_max[0] - _min[0])) {
        if (_xcoords.size()) {
            iwgt = (x - _xcoords[i]) / (_xcoords[i + 1] - _xcoords[i]);
        } else if (_delta[0] != 0.0) {
            iwgt = ((x - _extents[0]) - (i * _delta[0])) / _delta[0];
        }
    }
    double jwgt = 0.0;
    if (j < (_max[1] - _min[1])) {
        if (_ycoords.size()) {
            jwgt = (y - _ycoords[j]) / (_ycoords[j + 1] - _ycoords[j]);
        } else if (_delta[1] != 0.0) {
            jwgt = ((y - _extents[1]) - (j * _delta[1])) / _delta[1];
        }
    }
    double kwgt = 0.0;
    if (k < (_max[2] - _min[2])) {
        if (_zcoords.size()) {
            kwgt = (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
        } else if (_delta[2] != 0.0) {
            kwgt = ((z - _extents[2]) - (k * _delta[2])) / _delta[2];
        }
    }

    double p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = AccessIJK(i, j, k);
    if (p0 == GetMissingValue()) return (GetMissingValue());

    if (iwgt != 0.0) {
        p1 = AccessIJK(i + 1, j, k);
        if (p1 == GetMissingValue()) return (GetMissingValue());
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i, j + 1, k);
        if (p2 == GetMissingValue()) return (GetMissingValue());
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i + 1, j + 1, k);
        if (p3 == GetMissingValue()) return (GetMissingValue());
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i, j, k + 1);
        if (p4 == GetMissingValue()) return (GetMissingValue());
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i + 1, j, k + 1);
        if (p5 == GetMissingValue()) return (GetMissingValue());
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i, j + 1, k + 1);
        if (p6 == GetMissingValue()) return (GetMissingValue());
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i + 1, j + 1, k + 1);
        if (p7 == GetMissingValue()) return (GetMissingValue());
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}
int StretchedGrid::GetUserCoordinates(size_t i, size_t j, size_t k, double *x, double *y, double *z) const
{
    if (i > _max[0] - _min[0]) return (-1);
    if (j > _max[1] - _min[1]) return (-1);
    if (k > _max[2] - _min[2]) return (-1);

    RegularGrid::GetUserCoordinates(i, j, k, x, y, z);

    if (_xcoords.size()) *x = _xcoords[i];
    if (_ycoords.size()) *y = _ycoords[j];
    if (_zcoords.size()) *z = _zcoords[k];

    return (0);
}

void StretchedGrid::GetBoundingBox(const size_t min[3], const size_t max[3], double extents[6]) const
{
    StretchedGrid::GetUserCoordinates(min[0], min[1], min[2], &(extents[0]), &(extents[1]), &(extents[2]));
    StretchedGrid::GetUserCoordinates(max[0], max[1], max[2], &(extents[3]), &(extents[4]), &(extents[5]));
}

void StretchedGrid::GetEnclosingRegion(const double minu[3], const double maxu[3], size_t min[3], size_t max[3]) const
{
    size_t dims[3];
    StretchedGrid::GetDimensions(dims);
    for (int i = 0; i < 3; i++) {
        min[i] = 0;
        max[i] = dims[i] - 1;
    }

    size_t temp_min[3], temp_max[3];
    StretchedGrid::GetIJKIndex(minu[0], minu[1], minu[2], &temp_min[0], &temp_min[1], &temp_min[2]);
    StretchedGrid::GetIJKIndex(maxu[0], maxu[1], maxu[2], &temp_max[0], &temp_max[1], &temp_max[2]);

    double temp_minu[3], temp_maxu[3];

    StretchedGrid::GetUserCoordinates(temp_min[0], temp_min[1], temp_min[2], &temp_minu[0], &temp_minu[1], &temp_minu[2]);
    StretchedGrid::GetUserCoordinates(temp_max[0], temp_max[1], temp_max[2], &temp_maxu[0], &temp_maxu[1], &temp_maxu[2]);

    double extents[6];
    StretchedGrid::GetUserExtents(extents);

    for (int i = 0; i < 3; i++) {
        if (extents[i] < extents[i + 3]) {
            if (temp_minu[i] > minu[i] && (temp_min[i] > 0)) { temp_min[i]--; }
            if (temp_maxu[i] < maxu[i] && (temp_max[i] < (dims[i] - 1))) { temp_max[i]++; }
        } else {
            if (temp_minu[i] < minu[i] && (temp_min[i] > 0)) { temp_min[i]--; }
            if (temp_maxu[i] > maxu[i] && (temp_max[i] < (dims[i] - 1))) { temp_max[i]++; }
        }
        min[i] = temp_min[i];
        max[i] = temp_max[i];
    }
}
void StretchedGrid::GetIJKIndex(double x, double y, double z, size_t *i, size_t *j, size_t *k) const
{
    RegularGrid::_ClampCoord(x, y, z);

    size_t dims[3];
    RegularGrid::GetDimensions(dims);

    size_t i0, j0, k0;
    StretchedGrid::GetIJKIndexFloor(x, y, z, &i0, &j0, &k0);

    //
    // Point with coordinates x,y,z is in cell with index i0,j0,k0, but
    // may be closer to adjacent cell grid points.
    //

    if (i0 < dims[0] - 1) {
        if (_xcoords.size()) {
            if (((_xcoords[i0 + 1] - _xcoords[i0]) != 0.0) && fabs((x - _xcoords[i0]) / (_xcoords[i0 + 1] - _xcoords[i0])) > 0.5) { i0++; }
        } else if ((_delta[0] != 0.0) && (((x - _extents[0]) - (i0 * _delta[0])) / _delta[0]) > 0.5) {
            i0++;
        }
    }

    if (j0 < dims[1] - 1) {
        if (_ycoords.size()) {
            if (((_ycoords[j0 + 1] - _ycoords[j0]) != 0.0) && fabs((y - _ycoords[j0]) / (_ycoords[j0 + 1] - _ycoords[j0])) > 0.5) { j0++; }
        } else if ((_delta[1] != 0.0) && (((y - _extents[1]) - (j0 * _delta[1])) / _delta[1]) > 0.5) {
            j0++;
        }
    }

    if (k0 < dims[2] - 1) {
        if (_zcoords.size()) {
            if (((_zcoords[k0 + 1] - _zcoords[k0]) != 0.0) && fabs((z - _zcoords[k0]) / (_zcoords[k0 + 1] - _zcoords[k0])) > 0.5) { k0++; }
        } else if ((_delta[2] != 0.0) && (((z - _extents[0]) - (k0 * _delta[2])) / _delta[2]) > 0.5) {
            k0++;
        }
    }

    *i = i0;
    *j = j0;
    *k = k0;
}

void StretchedGrid::GetIJKIndexFloor(double x, double y, double z, size_t *i, size_t *j, size_t *k) const
{
    RegularGrid::_ClampCoord(x, y, z);

    size_t dims[3];
    RegularGrid::GetDimensions(dims);

    //
    // Get IJK indecies for any non-stretched coords, or a coordinate on
    // the boundary or outside the grid. The indecies returned
    // for any stretched coords are bogus.
    //
    RegularGrid::GetIJKIndexFloor(x, y, z, i, j, k);

    //
    // Now get indecies for stretched coords not on or outside boundary
    //
    if (_xcoords.size() != 0 && ((x - _extents[0]) * (x - _extents[3]) < 0)) {
        size_t i0 = 0;
        size_t i1 = dims[0] - 1;
        double x0 = _xcoords[i0];
        double x1 = _xcoords[i1];
        while (i1 - i0 > 1) {
            x1 = _xcoords[(i0 + i1) >> 1];
            if (x1 == x) {    // pathological case
                //*i = (i0+i1)>>1;
                i0 = (i0 + i1) >> 1;
                break;
            }

            // if the signs of differences change then the coordinate
            // is between x0 and x1
            //
            if ((x - x0) * (x - x1) <= 0.0) {
                i1 = (i0 + i1) >> 1;
            } else {
                i0 = (i0 + i1) >> 1;
                x0 = x1;
            }
        }
        *i = i0;
    }

    if (_ycoords.size() != 0 && ((y - _extents[1]) * (y - _extents[4]) < 0)) {
        size_t j0 = 0;
        size_t j1 = dims[1] - 1;
        double y0 = _ycoords[j0];
        double y1 = _ycoords[j1];
        while (j1 - j0 > 1) {
            y1 = _ycoords[(j0 + j1) >> 1];
            if (y1 == y) {    // pathological case
                //*j = (j0+j1)>>1;
                j0 = (j0 + j1) >> 1;
                break;
            }

            // if the signs of differences change then the coordinate
            // is between y0 and y1
            //
            if ((y - y0) * (y - y1) <= 0.0) {
                j1 = (j0 + j1) >> 1;
            } else {
                j0 = (j0 + j1) >> 1;
                y0 = y1;
            }
        }
        *j = j0;
    }

    if (_zcoords.size() != 0 && ((z - _extents[2]) * (z - _extents[5]) < 0)) {
        size_t k0 = 0;
        size_t k1 = dims[2] - 1;
        double z0 = _zcoords[k0];
        double z1 = _zcoords[k1];
        while (k1 - k0 > 1) {
            z1 = _zcoords[(k0 + k1) >> 1];
            if (z1 == z) {    // pathological case
                //*k = (k0+k1)>>1;
                k0 = (k0 + k1) >> 1;
                break;
            }

            // if the signs of differences change then the coordinate
            // is between z0 and z1
            //
            if ((z - z0) * (z - z1) <= 0.0) {
                k1 = (k0 + k1) >> 1;
            } else {
                k0 = (k0 + k1) >> 1;
                z0 = z1;
            }
        }
        *k = k0;
    }
}

void StretchedGrid::GetMinCellExtents(double *x, double *y, double *z) const
{
    *x = _delta[0];
    *y = _delta[1];
    *z = _delta[2];

    if (_xcoords.size()) {
        for (int i = 0; i < _xcoords.size() - 1; i++) {
            double tmp = fabs(_xcoords[i] - _xcoords[i + 1]);

            if (i == 0) *x = tmp;
            if (tmp < *x) *x = tmp;
        }
    }
    if (_ycoords.size()) {
        for (int i = 0; i < _ycoords.size() - 1; i++) {
            double tmp = fabs(_ycoords[i] - _ycoords[i + 1]);

            if (i == 0) *y = tmp;
            if (tmp < *y) *y = tmp;
        }
    }
    if (_zcoords.size()) {
        for (int i = 0; i < _zcoords.size() - 1; i++) {
            double tmp = fabs(_zcoords[i] - _zcoords[i + 1]);

            if (i == 0) *z = tmp;
            if (tmp < *z) *z = tmp;
        }
    }
}
