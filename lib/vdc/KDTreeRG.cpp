#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#include <vapor/utils.h>
#include <vapor/KDTreeRG.h>
#include "kdtree.h"


using namespace std;
using namespace VAPoR;

KDTreeRG::KDTreeRG( const Grid &xg, 
                    const Grid &yg ) 
                :   _points( xg, yg ), 
                    _kdtree(2 /* dimension */, _points, nanoflann::KDTreeSingleIndexAdaptorParams(20 /* max leaf num */))
{
    _dims = xg.GetDimensions();
    _kdtree.buildIndex();
}

KDTreeRG::~KDTreeRG() { }

void KDTreeRG::Nearest( const vector <float> &coordu, vector <size_t> &coord) const 
{
    assert( coordu.size() == 2 );   // 3D case isn't supported yet

    size_t  ret_index;
    float   dist_sqr;
    nanoflann::KNNResultSet<float, size_t> resultSet(1);
    resultSet.init( &ret_index, &dist_sqr );
    bool rt = _kdtree.findNeighbors( resultSet, coordu.data(), nanoflann::SearchParams() );
    assert( rt );

    // De-serialize the linear offset and put it back in vector form
    coord.clear();
    coord = Wasp::VectorizeCoords( ret_index, _dims );
}


KDTreeRGSubset::KDTreeRGSubset() {
    _kdtree = NULL;
    _min.clear();
    _max.clear();
}

KDTreeRGSubset::KDTreeRGSubset(
    const KDTreeRG *kdtreerg,
    const vector <size_t> &min,
    const vector <size_t> &max
) {
    assert(min.size() == max.size());


    vector <size_t> dims = kdtreerg->GetDimensions();
    assert(min.size() == dims.size());

    for (int i=0; i<dims.size(); i++) {
        assert(min[i] <= max[i]);
        assert(max[i] < dims[i]);
    }

    _kdtree = kdtreerg; // shallow copy
    _min = min;
    _max = max;
}

// Returned coordinates are relative to min and max used in constructor
// The origin is given by min
//
void KDTreeRGSubset::Nearest(
    const vector <float> &coordu,
    vector <size_t> &coord
) const {
    assert(coordu.size() == _min.size());

    coord.clear();

    vector <size_t> global_coords;

    // Find voxel coorindates of nearest point in global mesh
    //
    _kdtree->Nearest(coordu, global_coords);

    // Clamp global coordinates to region defined by _min, and _max
    //
    for (int i=0; i<global_coords.size(); i++) {
        if (global_coords[i] < _min[i]) global_coords[i] = _min[i];
        if (global_coords[i] > _max[i]) global_coords[i] = _max[i];
    }

    // Translate global coordinates to ROI coordinates
    //
    for (int i=0; i<global_coords.size(); i++) {
        coord.push_back(global_coords[i] - _min[i]);
    }
}
