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

    void GetWindingOrder( std::vector<double>& windingOrder ) const;

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
