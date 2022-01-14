#pragma once

#include <vapor/glutil.h>
#include <glm/glm.hpp>

// clang-format off

class RegularGrid;

struct planeDescription {
    size_t sideSize;
    std::vector<double> origin;
    std::vector<double> rotation;
    VAPoR::CoordType boxMin;
    VAPoR::CoordType boxMax;
    VAPoR::CoordType domainMin;
    VAPoR::CoordType domainMax;
};

//! Create a 2D RegularGrid that samples a given 3D Grid object along a plane, 
//! through the entire domain.  The "planeDescription" variable includes parameters 
//! that define the 2D plane's origin, rotation, and valid extents.  Points on the
//! 2D RegularGrid that are outside of the valid extents will be assigned as
//! missingValues.
//! 
//! Note: The boxMin and boxMax values within the planeDescription should come from
//! the current RenderParams' Box class.  The domainMin and domainMax values should 
//! come from the DataMgr::GetVariableExtents function.  Taking these values from
//! the given 3D grid can produce erroneous results when the 3D grid's region is reduced.
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
    std::unique_ptr<float>& data, 
    std::vector<double>& windingOrder, 
    std::vector<double>& rectangle3D
);

// clang-format on
