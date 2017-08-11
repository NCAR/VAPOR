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

#include <vapor/utils.h>
#include <vapor/Grid.h>

using namespace std;
using namespace VAPoR;

Grid::Grid(const vector<size_t> &dims, size_t topology_dimension)
{
    for (int i = 0; i < dims.size(); i++) { assert(dims[i] > 0); }

    _dims = dims;
    _periodic = vector<bool>(topology_dimension, false);
    _topologyDimension = topology_dimension;
    _missingValue = INFINITY;
    _hasMissing = false;
    _interpolationOrder = 0;
}

Grid::Grid()
{
    _dims.clear();
    _periodic.clear();
    _topologyDimension = 0;
    _missingValue = INFINITY;
    _hasMissing = false;
    _interpolationOrder = 0;
}

Grid::~Grid() {}

float Grid::GetValue(const std::vector<double> &coords) const
{
    vector<double> clampedCoords = coords;

    // Clamp coordinates on periodic boundaries to grid extents
    //
    ClampCoord(clampedCoords);

    // At this point xyz should be within the grid bounds
    //
    if (!InsideGrid(clampedCoords)) return (_missingValue);

    if (_interpolationOrder == 0) {
        return (_GetValueNearestNeighbor(clampedCoords));
    } else {
        return (_GetValueLinear(clampedCoords));
    }
}

void Grid::_getUserCoordinatesHelper(const vector<double> &coords, double &x, double &y, double &z) const
{
    if (coords.size() >= 1) { x = coords[0]; }
    if (coords.size() >= 2) { y = coords[1]; }
    if (coords.size() >= 3) { z = coords[2]; }
}

void Grid::GetUserCoordinates(size_t i, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    _getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::GetUserCoordinates(size_t i, size_t j, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i, j};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    _getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::GetUserCoordinates(size_t i, size_t j, size_t k, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i, j, k};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    _getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::SetInterpolationOrder(int order)
{
    if (order < 0 || order > 2) order = 1;
    _interpolationOrder = order;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const Grid &g)
{
    o << "Grid" << endl;

    o << " Dimensions ";
    for (int i = 0; i < g._dims.size(); i++) { o << g._dims[i] << " "; }
    o << endl;

    o << " Topological dimension " << g._topologyDimension << endl;

    o << " Periodicity ";
    for (int i = 0; i < g._periodic.size(); i++) { o << g._periodic[i] << " "; }
    o << endl;

    o << " Missing value flag" << g._hasMissing << endl;
    o << " Missing value " << g._missingValue << endl;
    o << " Interpolation order " << g._interpolationOrder << endl;

    return (o);
}
};    // namespace VAPoR
