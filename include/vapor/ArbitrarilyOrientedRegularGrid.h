#pragma once

#include <ostream>
#include <vector>
#include <vapor/common.h>
#include <vapor/RegularGrid.h>
#include <glm/glm.hpp>

// clang-format off

struct planeDescription {
    size_t sideSize;
    std::vector<double> origin;
    std::vector<double> rotation;
    VAPoR::CoordType boxMin;
    VAPoR::CoordType boxMax;
    VAPoR::CoordType domainMin;
    VAPoR::CoordType domainMax;
};

namespace VAPoR {

//! Create a 2D RegularGrid that is rotated along an arbitrary orientation.  This new grid
//! samples a given 3D Grid object along a plane, through the entire domain.  The "planeDescription" 
//! variable includes parameters that define the 2D plane's origin, rotation, and valid extents.  Points on the
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
//! \param[in] dims The sampling rate that the new grid will be defined upon, along its X and Y axes
//! \param[out] data An array of sideSize*sideSize values that contain floating point values of the sampled data.
//!

class VDF_API ArbitrarilyOrientedRegularGrid : public RegularGrid {
public:
    ArbitrarilyOrientedRegularGrid(
        const VAPoR::Grid* grid3d,
        planeDescription& pd,
        const DimsType& dims,
        std::shared_ptr<float>& blks
    );

    ArbitrarilyOrientedRegularGrid() = default;
    virtual ~ArbitrarilyOrientedRegularGrid() = default;

    static std::string GetClassType() { return ("ArbitrarilyOrientedRegular"); }
    std::string        GetType() const override { return (GetClassType()); }

    //! \copydoc Grid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const DimsType &indices, CoordType &coords) const override;

    //! Retrieve the winding order that pertains to the generated array of data, if it is
    //! to be drawn as a texture as is done in the SliceRenderer's vertex buffer.  The winding order is a series
    //! of six 3D vertices that describe two triangles that define how the texture is drawn in space.
    void GetWindingOrder( std::vector<double>& windingOrder ) const;

    //! Retrieve the 3D coordinates of the corners of the sampled data.  This is a set of four 3D coordinates,
    //! used to configure the SliceRenderer's texture coordinate buffer.
    void Get3DRectangle( std::vector<double>& windingOrder ) const;

private:
    std::vector<glm::tvec2<double, glm::highp>> _rectangle2D;
    std::vector<double> _rectangle3D, _windingOrder;
    size_t _sideSize;
    glm::tvec3<double, glm::highp> _normal, _origin, _axis1, _axis2, _rotation;

    void _populateData(
        const VAPoR::Grid *grid,
        const planeDescription& description,
        std::shared_ptr<float>& dataValues
    );

    void _getMinimumAreaRectangle(
        const std::vector<glm::tvec3<double, glm::highp>>& vertices,
              std::vector<glm::tvec3<double, glm::highp>>& tmpRectangle3D
    );

    void _findIntercepts(
        const VAPoR::CoordType& boxMin,
        const VAPoR::CoordType& boxMax,
        std::vector<glm::tvec3<double, glm::highp>> &vertices
    );

    glm::tvec3<double, glm::highp> _getOrthogonal(
        const glm::tvec3<double, glm::highp>& u
    );
    
    void _generateWindingOrder(
        const std::vector<glm::tvec3<double, glm::highp>> tmpRectangle3D
    );

    void _rotate();
};
};    // namespace VAPoR

// clang-format on
