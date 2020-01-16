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
    float       GetValue(const std::vector<double> &coords) const override;
    void        GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const override;
    bool        InsideGrid(const std::vector<double> &coords) const override;
    float       GetMissingValue() const override;

private:
    //
    // Pure virtual functions from Grid class.
    // They do nothing and return meaningless values.
    // Do not use!
    //
    float                      GetValueNearestNeighbor(const std::vector<double> &coords) const override;
    float                      GetValueLinear(const std::vector<double> &coords) const override;
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
    const VAPoR::Grid *const _grid2d;
    VAPoR::DataMgr *const    _dataMgr;    // The pointer itself cannot be changed
    const float              _defaultZ;

};    // end GrownGrid class
};    // namespace VAPoR
#endif
