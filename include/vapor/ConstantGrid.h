#ifndef CONSTANTGRID_H
#define CONSTANTGRID_H

/*
 * This class represents a constant field. That means,
 * querying values from any location of this field will return that constant value.
 *
 * It also implements pure virtual functions of the Grid class in the simplest possible fashion,
 * but those functions do not perform any calculation, and should not be used.
 *
 * Finally, a DataMgr would not return a ConstantGrid at any occasion; rather, this
 * class is supposed to be created and used locally by its user.
 */

#include "vapor/Grid.h"

namespace VAPoR {

class VDF_API ConstantGrid : public Grid {
public:
    // The constant value is specified via the constructor
    ConstantGrid(float v);

    //
    // Useful functions of ConstantGrid.
    // Additional ones could be added when needed.
    //
    // The following three GetValue methods all return the constant value of this grid.
    float GetValue(const std::vector<double> &coords) const override;
    float GetValueNearestNeighbor(const std::vector<double> &coords) const override;
    float GetValueLinear(const std::vector<double> &coords) const override;

    // This version of ConstantGrid is considered to have infinity extents,
    // so the following method will return numerical mins and maxes.
    // Note: other flavors of ConstantGrids may have specific user extents.
    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const override;
    // Similarly, this will always return true.
    virtual bool InsideGrid(const std::vector<double> &coords) const override;

    std::string GetType() const override;

private:
    //
    // Pure virtual functions from Grid class.
    // They do nothing and return meaningless values.
    // Do not use!
    //
    std::vector<size_t>        GetCoordDimensions(size_t) const override;
    size_t                     GetGeometryDim() const override;
    const std::vector<size_t> &GetNodeDimensions() const override;
    const std::vector<size_t> &GetCellDimensions() const override;
    void                       GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const override {}
    bool          GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const override { return (false); }
    virtual void  GetUserCoordinates(const size_t indices[], double coords[]) const override {}
    bool          GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const override;
    bool          GetCellNodes(const size_t cindices[], size_t nodes[], int &n) const override;
    bool          GetCellNeighbors(const std::vector<size_t> &cindices, std::vector<std::vector<size_t>> &cells) const override;
    bool          GetNodeCells(const std::vector<size_t> &indices, std::vector<std::vector<size_t>> &cells) const override;
    size_t        GetMaxVertexPerFace() const override;
    size_t        GetMaxVertexPerCell() const override;
    void          ClampCoord(std::vector<double> &coords) const override {}
    ConstCoordItr ConstCoordBegin() const override;
    ConstCoordItr ConstCoordEnd() const override;

    // Private data member that holds this constant value.
    const float _value;

};    // end ConstantGrid class
};    // namespace VAPoR
#endif
