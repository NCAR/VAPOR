#ifndef GROWNGRID_H
#define GROWNGRID_H

/*
 * This class represents an essentially 2D grid, but grown to appear like 3D.
 * It does so by containing a 2D grid in the XY plain, and inserting a fixed value
 * in the Z dimension in a handful of "useful" functions.
 *
 * It also implements pure virtual functions of the Grid class in the simplest possible fashion,
 * but those functions do not perform any calculation, and should not be used.
 *
 * Finally, a DataMgr would not return a GrownGrid at any occasion; rather, this
 * class is supposed to be created and used locally by its user.
 */

#include "vapor/Grid.h"
#include "vapor/DataMgr.h"

namespace VAPoR {
class FLOW_API GrownGrid final : public Grid {
public:
    // Constructor: pass in the 2D grid to be managed, the data manager that will
    // later be used to destroy the grid, and the default Z value to be used.
    // Note: the passed in 2D grid should be queried with the "lock" option on,
    //       and GrownGrid will take ownership of the 2D grid afterwards.
    //       I.e., GrownGrid will be responsible deleting the underlying 2D grid.
    GrownGrid(const VAPoR::Grid *gp, VAPoR::DataMgr *mp, float z);
    virtual ~GrownGrid();

    //
    // Useful functions of GrownGrid.
    // Additional ones could be added when needed.
    //
    float       GetDefaultZ() const;
    std::string GetType() const override;
    void        GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const override;
    bool        InsideGrid(const DblArr3 &coords) const override;
    float       GetMissingValue() const override;
    float       GetValue(const DblArr3 &coords) const override;

private:
    //
    // Pure virtual functions from Grid class.
    // They do nothing and return meaningless values.
    // Do not use!
    //
    float                      GetValueNearestNeighbor(const DblArr3 &coords) const override;
    float                      GetValueLinear(const DblArr3 &coords) const override;
    std::vector<size_t>        GetCoordDimensions(size_t) const override;
    size_t                     GetGeometryDim() const override;
    const std::vector<size_t> &GetNodeDimensions() const override;
    const std::vector<size_t> &GetCellDimensions() const override;
    void                       GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const override {}
    bool                       GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const override { return (false); }
    virtual void               GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const override {}
    bool                       GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const override;
    bool                       GetCellNodes(const Size_tArr3 &cindices, std::vector<Size_tArr3> &nodes) const override;
    bool                       GetCellNeighbors(const Size_tArr3 &cindices, std::vector<Size_tArr3> &nodes) const override;
    bool                       GetNodeCells(const Size_tArr3 &cindices, std::vector<Size_tArr3> &nodes) const override;
    size_t                     GetMaxVertexPerFace() const override;
    size_t                     GetMaxVertexPerCell() const override;
    void                       ClampCoord(const double coords[3], double cCoords[3]) const override {}
    void                       ClampCoord(const DblArr3 &coords, DblArr3 &cCoords) const override { cCoords = coords; }
    ConstCoordItr              ConstCoordBegin() const override;
    ConstCoordItr              ConstCoordEnd() const override;

    // Private data member that holds this constant value.
    const VAPoR::Grid *const _grid2d;
    VAPoR::DataMgr *const    _dataMgr;    // The pointer itself cannot be changed
    const float              _defaultZ;

};    // end GrownGrid class
};    // namespace VAPoR
#endif
