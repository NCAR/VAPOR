#pragma once

#include <vapor/glutil.h>
#include <glm/glm.hpp>

// clang-format off

class RegularGrid;

struct planeDescription {
    std::vector<double> origin;
    std::vector<double> rotation;
    VAPoR::CoordType boxMin;
    VAPoR::CoordType boxMax;
};

//! Create a 2D grid that is sampled along the orientation of a 3D grid.
//! The "planeDescription" variable includes parameters that define the
//! 2D grid's origin, rotation, and extents.  The sampling rate is defined
//! by the "sideSize" parameter, which defines how many samples along the X
//! and Y axes are taken along the plane inside of the 3D grid.
//!
//! \param[in] grid3d A variable's 3D grid to be sampled along a plane, to generate a 2D grid object
//! \param[in] description A set of std::vector<dobule> values that define the origin, rotation, and extents of the returned 2D grid
//! \param[in] sideSize The sampling rate that the 2D grid will be defined upon, along its X and Y axes
//! \param[out] data An array of sideSize*sideSize values that contain floating point values of the sampled data.
//! \param[out] windingOrder A vector of double values that contain the spatial coordinates of the vertices that correspond to the two triangles that define the texture being drawn by the "data" array.
//! \param[out] rectangle3D A set of four vertices that define the rectangle containing the returned 2D grid's texture.  Useful for debugging.
//!

VDF_API VAPoR::RegularGrid* SliceGridAlongPlane(
    const VAPoR::Grid *grid3d, 
    planeDescription description, 
    size_t sideSize, 
    //std::shared_ptr<float> data, 
    std::unique_ptr<float>& data, 
    std::vector<double>& windingOrder, 
    std::vector<double>& rectangle3D
);

// clang-format on
