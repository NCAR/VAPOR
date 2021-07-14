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
    // It also requires specification of the dimentionality.
    ConstantGrid(float v, size_t d);

    //
    // Useful functions of ConstantGrid.
    // Additional ones could be added when needed.
    //
    // The following four GetValue methods all return the constant value of this grid.
    float GetConstantValue() const;
    float GetValue(const CoordType &coords) const override;
    float GetValueNearestNeighbor(const CoordType &coords) const override;
    float GetValueLinear(const CoordType &coords) const override;

    // This version of ConstantGrid is considered to have infinity extents,
    // so the following method will return numerical mins and maxes.
    // Note: other flavors of ConstantGrids may have specific user extents.
    virtual void GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const override;
    // Similarly, this will always return true.
    virtual bool InsideGrid(const CoordType &coords) const override;

    std::string GetType() const override;

    // Overwrites the same function from Grid, which will always give you a zero.
    virtual size_t GetTopologyDim() const override;

private:
    //
    // Pure virtual functions from Grid class.
    // They do nothing and return meaningless values.
    // Do not use!
    //
    std::vector<size_t>        GetCoordDimensions(size_t) const override;
    size_t                     GetGeometryDim() const override;
    const DimsType &           GetNodeDimensions() const override;
    const size_t               GetNumNodeDimensions() const override;
    const DimsType &           GetCellDimensions() const override;
    const size_t               GetNumCellDimensions() const override;
    void                       GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const override {}
    bool                       GetEnclosingRegion(const CoordType &minu, const CoordType &maxu, DimsType &min, DimsType &max) const override { return (false); }
    virtual void               GetUserCoordinates(const DimsType &, CoordType &) const override {}
    bool                       GetIndicesCell(const CoordType &coords, DimsType &indices) const override;
    bool                       GetCellNodes(const DimsType &, std::vector<DimsType> &) const override;
    bool                       GetCellNeighbors(const DimsType &, std::vector<DimsType> &) const override;
    bool                       GetNodeCells(const DimsType &, std::vector<DimsType> &) const override;
    size_t                     GetMaxVertexPerFace() const override;
    size_t                     GetMaxVertexPerCell() const override;
    void                       ClampCoord(const CoordType &coords, CoordType &cCoords) const override { cCoords = coords; }
    ConstCoordItr              ConstCoordBegin() const override;
    ConstCoordItr              ConstCoordEnd() const override;

    // Private data member that holds this constant value.
    const float  _value;
    const size_t _topologyDim;    // Not to be confused with _topologyDimension in
                                  // the base Grid class, which is private to Grid.

    mutable VAPoR::DimsType _duplicate;

};    // end ConstantGrid class
};    // namespace VAPoR
#endif
