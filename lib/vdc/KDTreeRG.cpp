#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#include <vapor/utils.h>
#include <vapor/KDTreeRG.h>
#include "kdtree.h"

using namespace std;
using namespace VAPoR;

KDTreeRG::KDTreeRG()
{
    _kdtree = NULL;
    _offsets = NULL;
    _dims.clear();
}

KDTreeRG::KDTreeRG(const RegularGrid &xrg, const RegularGrid &yrg)
{
    assert(xrg.GetDimensions() == yrg.GetDimensions());
    assert(xrg.GetDimensions().size() == 2);

    _kdtree = NULL;
    _offsets = NULL;
    _dims.clear();

    _dims = xrg.GetDimensions();

    // number of elements
    //
    size_t nelem = 1;
    for (int i = 0; i < _dims.size(); i++) nelem *= _dims[i];

    // Need to create serialized array of offsets for each grid point.
    // The offsets are what gets stored in the k-d tree
    //
    _offsets = new size_t[nelem];
    for (size_t i = 0; i < nelem; i++) { _offsets[i] = i; }

    _kdtree = kd_create(2);

    // Store the point coordinates and associated offsets in the k-d tree
    //
    RegularGrid::ConstIterator xitr = xrg.cbegin();
    RegularGrid::ConstIterator yitr = yrg.cbegin();

    float posXY[2];
    for (size_t i = 0; i < nelem; ++i, ++xitr, ++yitr) {
        posXY[0] = *xitr;
        posXY[1] = *yitr;

        kd_insertf(_kdtree, posXY, &_offsets[i]);
    }
}

KDTreeRG::KDTreeRG(const RegularGrid &xrg, const RegularGrid &yrg, const RegularGrid &zrg)
{
    // Not implemented yet!
    assert(0);
}

KDTreeRG::~KDTreeRG()
{
    if (_offsets) delete[] _offsets;
    if (_kdtree) kd_free(_kdtree);
}

void KDTreeRG::Nearest(const vector<float> &coordu, vector<size_t> &coord) const
{
    assert(coordu.size() == _dims.size());
    coord.clear();

    float posXY[] = {coordu[0], coordu[1]};

    // Lookup offset of nearest point
    //
    struct kdres *result = kd_nearestf(_kdtree, posXY);
    size_t *      offptr = (size_t *)kd_res_item(result, NULL);

    // De-serialize the linear offset and put it back in vector form
    //
    coord = Wasp::VectorizeCoords(*offptr, _dims);
}

KDTreeRGSubset::KDTreeRGSubset()
{
    _kdtree = NULL;
    _min.clear();
    _max.clear();
}

KDTreeRGSubset::KDTreeRGSubset(const KDTreeRG *kdtreerg, const vector<size_t> &min, const vector<size_t> &max)
{
    assert(min.size() == max.size());

    vector<size_t> dims = kdtreerg->GetDimensions();
    assert(min.size() == dims.size());

    for (int i = 0; i < dims.size(); i++) {
        assert(min[i] <= max[i]);
        assert(max[i] < dims[i]);
    }

    _kdtree = kdtreerg;    // shallow copy
    _min = min;
    _max = max;
}

// Returned coordinates are relative to min and max used in constructor
// The origin is given by min
//
void KDTreeRGSubset::Nearest(const vector<float> &coordu, vector<size_t> &coord) const
{
    assert(coordu.size() == _min.size());

    coord.clear();

    vector<size_t> global_coords;

    // Find voxel coorindates of nearest point in global mesh
    //
    _kdtree->Nearest(coordu, global_coords);

    // Clamp global coordinates to region defined by _min, and _max
    //
    for (int i = 0; i < global_coords.size(); i++) {
        if (global_coords[i] < _min[i]) global_coords[i] = _min[i];
        if (global_coords[i] > _max[i]) global_coords[i] = _max[i];
    }

    // Translate global coordinates to ROI coordinates
    //
    for (int i = 0; i < global_coords.size(); i++) { coord.push_back(global_coords[i] - _min[i]); }
}
