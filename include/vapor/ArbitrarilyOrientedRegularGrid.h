#pragma once

#include <ostream>
#include <vector>
#include <vapor/common.h>
#include <vapor/RegularGrid.h>
//#include <vapor/SliceGridAlongPlane.h>
#include <vapor/planeDescription.h>
#include <glm/glm.hpp>

/*struct planeDescription {
    size_t sideSize;
    std::vector<double> origin;
    std::vector<double> rotation;
    VAPoR::CoordType boxMin;
    VAPoR::CoordType boxMax;
    VAPoR::CoordType domainMin;
    VAPoR::CoordType domainMax;
    VAPoR::CoordType axis1;
    VAPoR::CoordType axis2;
};*/

namespace VAPoR {

class VDF_API ArbitrarilyOrientedRegularGrid : public RegularGrid {
public:
    // clang-format off
    ArbitrarilyOrientedRegularGrid(
        const VAPoR::Grid* grid3d,
        planeDescription& pd,
        const DimsType& dims,
        std::shared_ptr<float>& blks,
        //const std::vector<double> &extents
        //std::vector<glm::tvec2<double, glm::highp>> extents
        std::vector<double>& windingOrder,
        std::vector<double>& rectangle3D
    );
    // clang-format on

    ArbitrarilyOrientedRegularGrid() = default;
    virtual ~ArbitrarilyOrientedRegularGrid() = default;

    static std::string GetClassType() { return ("ArbitrarilyOrientedRegular"); }
    std::string        GetType() const override { return (GetClassType()); }

    //! \copydoc Grid::GetBoundingBox()
    //
    //virtual void GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const override;

    //! \copydoc Grid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const DimsType &indices, CoordType &coords) const override;
    //void GetUserCoordinates(size_t i, size_t j, size_t k=0) const;

    // For grandparent inheritance of
    // Grid::GetUserCoordinates(const size_t indices[], double coords[])
    //
    //using Grid::GetUserCoordinates;

    //! \copydoc Grid::GetIndicesCell
    //!
    //virtual bool GetIndicesCell(const CoordType &coords, DimsType &indices) const override;

    //! \copydoc Grid::InsideGrid()
    //
    //virtual bool InsideGrid(const CoordType &coords) const;

private:
    std::vector<glm::tvec2<double, glm::highp>> _rectangle2D;
    std::vector<double> _rectangle3D;
    size_t _sideSize;
    glm::tvec3<double, glm::highp> _normal, _origin, _axis1, _axis2, _rotation;

    void _getWindingOrder(
        std::vector<double>& windingOrder,
        const std::vector<glm::tvec3<double, glm::highp>> tmpRectangle3D
    );

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
    
    void _rotate();
};
};    // namespace VAPoR
