#ifndef _CurvilinearGrid_
#define _CurvilinearGrid_
#include <vapor/common.h>
#include <vapor/StructuredGrid.h>
#include <vapor/RegularGrid.h>
#include <vapor/KDTreeRG.h>

namespace VAPoR {
//! \class CurvilinearGrid
//!
//! \brief This class implements a 2D or 3D curvilinear grid.
//!
//! This class implements a 2D or 3D curvilinear grid: a
//! specialization of StructuredGrid class where cells are
//! quadrilaterals (2D), or cuboids (3D). Hence, curvlinear grids are
//! topologically, but the location of each grid point is expressed
//! by functions:
//!
//! \code
//! x = X(i,j,k)
//! y = Y(i,j,k)
//! z = Z(i,j,k)
//! \endcode
//!
//!
//
class VDF_API CurvilinearGrid : public StructuredGrid {
  public:
    //! \copydoc StructuredGrid::StructuredGrid()
    //!
    //! Construct a regular grid sampling a 3D or 2D scalar function.
    //!
    //! This constructor instantiates a curvilinear grid  where the x,y,z
    //! user coordinates are expressed as follows:
    //!
    //! \code
    //! x = X(i,j)
    //! y = Y(i,j)
    //! z = Z(k)
    //! \endcode
    //!
    //! The X and Y user coordinates are specified with \p xrg and \p yrg,
    //! respectively, and the Z coordinates (if 3D) are specified by the
    //! vector \p zcoords.
    //!
    //! Adds new parameters:
    //!
    //! \param[in] xrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the X user coordinates.
    //! \param[in] yrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the Y user coordinates.
    //! \param[in] zcoords  A 1D vector whose size matches that of the K
    //! dimension of this class, and whose values specify the Z user coordinates.
    //! \param[in] kdtree A KDTreeRGSubset instance that contains a KD tree
    //! that may be used to find the nearest grid vertex to a given point
    //! expressed in user coordintes. The offsets returned by \p kdtree will
    //! be used as indeces into \p xrg and \p yrg.
    //!
    //!
    //! \sa RegularGrid()
    //
    CurvilinearGrid(
        const std::vector<size_t> &dims,
        const std::vector<size_t> &bs,
        const std::vector<float *> &blks,
        const RegularGrid &xrg,
        const RegularGrid &yrg,
        const std::vector<double> &zcoords,
        const KDTreeRGSubset &kdtree);

    virtual ~CurvilinearGrid();

    // \copydoc GetGrid::GetUserExtents()
    //
    virtual void GetUserExtents(
        std::vector<double> &minu, std::vector<double> &maxu) const {
        minu = _minu;
        maxu = _maxu;
    }

    // \copydoc GetGrid::GetBoundingBox()
    //
    virtual void GetBoundingBox(
        const std::vector<size_t> &min, const std::vector<size_t> &max,
        std::vector<double> &minu, std::vector<double> &maxu) const;

    // \copydoc GetGrid::GetEnclosingRegion()
    //
    virtual void GetEnclosingRegion(
        const std::vector<double> &minu, const std::vector<double> &maxu,
        std::vector<size_t> &min, std::vector<size_t> &max) const;

    // \copydoc GetGrid::GetUserCoordinates()
    //
    virtual int GetUserCoordinates(
        const std::vector<size_t> &indices,
        std::vector<double> &coords) const;

    // \copydoc GetGrid::GetIndices()
    //
    virtual void GetIndices(
        const std::vector<double> &coords,
        std::vector<size_t> &indices) const;

    // \copydoc GetGrid::InsideGrid()
    //
    virtual bool InsideGrid(const std::vector<double> &coords) const;

    //! Returns reference to RegularGrid instance containing X user coordinates
    //!
    //! Returns reference to RegularGrid instance passed to constructor
    //! containing X user coordinates
    //!
    const RegularGrid *GetXRG() const { return (_xrg); };

    //! Returns reference to RegularGrid instance containing Y user coordinates
    //!
    //! Returns reference to RegularGrid instance passed to constructor
    //! containing Y user coordinates
    //!
    const RegularGrid *GetYRG() const { return (_yrg); };

    //! Returns reference to vector containing Z user coordinates
    //!
    //! Returns reference to vector passed to constructor
    //! containing Z user coordinates
    //!
    const std::vector<double> &GetZCoords() const { return (_zcoords); };

  protected:
    virtual void _ClampCoord(const std::vector<double> &coords) const;
    virtual float _GetValueNearestNeighbor(const std::vector<double> &coords) const;
    virtual float _GetValueLinear(const std::vector<double> &coords) const;

  private:
    std::vector<double> _zcoords;
    std::vector<double> _minu;
    std::vector<double> _maxu;
    KDTreeRGSubset _kdtree;
    RegularGrid *_xrg;
    RegularGrid *_yrg;

    void _curvilinearGrid(
        const RegularGrid &xrg,
        const RegularGrid &yrg,
        const std::vector<double> &zcoords,
        const KDTreeRGSubset &kdtree);

    void _GetUserExtents(
        std::vector<double> &minu, std::vector<double> &maxu) const;

    bool _binarySearchRange(
        const std::vector<double> &sorted, double x, int &i) const;

    bool _insideGrid(
        double x, double y, double z,
        int &i, int &j, int &k,
        double lambda[4], double zwgt[2]) const;

    virtual void _getMinCellExtents(std::vector<double> &minCellExtents) const;
};
}; // namespace VAPoR
#endif
