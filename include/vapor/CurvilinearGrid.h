#ifndef _CurvilinearGrid_
#define _CurvilinearGrid_
#include <vapor/common.h>
#include <vapor/Grid.h>
#include <vapor/RegularGrid.h>
#include <vapor/QuadTreeRectangle.hpp>

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
//! x = X(i,j)
//! y = Y(i,j)
//! z = Z(i,j,k)
//! \endcode
//!
//! or
//!
//! \code
//! x = X(i,j)
//! y = Y(i,j)
//! z = Z(k)
//! \endcode
//!
//
class VDF_API CurvilinearGrid : public StructuredGrid {
public:
    //! \copydoc StructuredGrid::StructuredGrid()
    //!
    //! Construct a layered, horizontally curvlinear 3D grid
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
    //! respectively, and the Z coordinates  are specified by the
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
    //! \param[in] qtr A QuadTreeRectangle instance that contains a quad tree
    //! that may be used to find the cell(s) containing a given point
    //! expressed in user coordintes. if \p qtr is NULL the class will
    //! generate its own QuadTreeRectangle instance.
    //!
    //!
    //! \sa RegularGrid()
    //
    CurvilinearGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg,
                    const std::vector<double> &zcoords, std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr);

    //! \copydoc StructuredGrid::StructuredGrid()
    //!
    //! Construct a layered, horizontally curvlinear 3D grid
    //!
    //! This constructor instantiates a curvilinear grid  where the x,y,z
    //! user coordinates are expressed as follows:
    //!
    //! \code
    //! x = X(i,j)
    //! y = Y(i,j)
    //! z = Z(i,j,k)
    //! \endcode
    //!
    //! The X and Y user coordinates are specified with \p xrg and \p yrg,
    //! respectively, and the Z coordinates  are specified by the
    //! \p zrg
    //!
    //! Adds new parameters:
    //!
    //! \param[in] xrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the X user coordinates.
    //! \param[in] yrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the Y user coordinates.
    //! \param[in] zrg A 3D RegularGrid instance whose
    //! I, J, K dimensionality matches that of this class instance, and whose
    //! values specify the Z user coordinates.
    //! \param[in] qtr A QuadTreeRectangle instance that contains a quad tree
    //! that may be used to find the cell(s) containing a given point
    //! expressed in user coordintes. if \p qtr is NULL the class will
    //! generate its own QuadTreeRectangle instance.
    //!
    //!
    //! \sa RegularGrid()
    //
    CurvilinearGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg, const RegularGrid &zrg,
                    std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr);

    //! \copydoc StructuredGrid::StructuredGrid()
    //!
    //! Construct a curvlinear 2D grid
    //!
    //! This constructor instantiates a curvilinear grid  where the x,y
    //! user coordinates are expressed as follows:
    //!
    //! \code
    //! x = X(i,j)
    //! y = Y(i,j)
    //! \endcode
    //!
    //! The X and Y user coordinates are specified with \p xrg and \p yrg,
    //! respectively.
    //!
    //! Adds new parameters:
    //!
    //! \param[in] xrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the X user coordinates.
    //! \param[in] yrg A 2D RegularGrid instance whose
    //! I and J dimensionality matches that of this class instance, and whose
    //! values specify the Y user coordinates.
    //! \param[in] qtr A QuadTreeRectangle instance that contains a quad tree
    //! that may be used to find the cell(s) containing a given point
    //! expressed in user coordintes. if \p qtr is NULL the class will
    //! generate its own QuadTreeRectangle instance.
    //!
    //!
    //! \sa RegularGrid()
    //
    CurvilinearGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg,
                    std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr);

    CurvilinearGrid() = default;
    virtual ~CurvilinearGrid()
    {
        if (_qtr) {
            _qtr = nullptr;    // qtr is a C++ shared pointer
        }
    }

    std::shared_ptr<const QuadTreeRectangle<float, size_t>> GetQuadTreeRectangle() const { return (_qtr); }

    static std::string GetClassType() { return ("Curvilinear"); }
    std::string        GetType() const override { return (GetClassType()); }

    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const override;

    virtual size_t GetGeometryDim() const override { return (GetTopologyDim()); };

    // \copydoc GetGrid::GetBoundingBox()
    //
    virtual void GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const override;

    // \copydoc GetGrid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const override;

    //! \copydoc Grid::GetIndicesCell
    //!
    virtual bool GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const override;

    // \copydoc GetGrid::InsideGrid()
    //
    virtual bool InsideGrid(const DblArr3 &coords) const override;

    //! Returns reference to RegularGrid instance containing X user coordinates
    //!
    //! Returns reference to RegularGrid instance passed to constructor
    //! containing X user coordinates
    //!
    const RegularGrid &GetXRG() const { return (_xrg); };

    //! Returns reference to RegularGrid instance containing Y user coordinates
    //!
    //! Returns reference to RegularGrid instance passed to constructor
    //! containing Y user coordinates
    //!
    const RegularGrid &GetYRG() const { return (_yrg); };

    //! Returns reference to vector containing Z user coordinates
    //!
    //! Returns reference to vector passed to constructor
    //! containing Z user coordinates
    //!
    const std::vector<double> &GetZCoords() const { return (_zcoords); };

    class ConstCoordItrCG : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrCG(const CurvilinearGrid *cg, bool begin);
        ConstCoordItrCG(const ConstCoordItrCG &rhs);

        ConstCoordItrCG();
        virtual ~ConstCoordItrCG() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstCoordType &deref() const { return (_coords); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCoordItrCG *itrptr = static_cast<const ConstCoordItrCG *>(rhs);

            return (_index == itrptr->_index);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrCG(*this)); };

    private:
        const CurvilinearGrid *_cg;
        std::vector<size_t>    _index;
        std::vector<double>    _coords;
        ConstIterator          _xCoordItr;
        ConstIterator          _yCoordItr;
        ConstIterator          _zCoordItr;
        bool                   _terrainFollowing;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrCG(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrCG(this, false))); }

protected:
    virtual float GetValueNearestNeighbor(const DblArr3 &coords) const override;

    virtual float GetValueLinear(const DblArr3 &coords) const override;

    // \copydoc GetGrid::GetUserExtents()
    //
    virtual void GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const override;

private:
    std::vector<double>                                     _zcoords;
    DblArr3                                                 _minu = {{0.0, 0.0, 0.0}};
    DblArr3                                                 _maxu = {{0.0, 0.0, 0.0}};
    RegularGrid                                             _xrg;
    RegularGrid                                             _yrg;
    RegularGrid                                             _zrg;
    bool                                                    _terrainFollowing;
    std::shared_ptr<const QuadTreeRectangle<float, size_t>> _qtr;

    void _curvilinearGrid(const RegularGrid &xrg, const RegularGrid &yrg, const RegularGrid &zrg, const std::vector<double> &zcoords, std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr);

    bool _insideFace(const Size_tArr3 &face, double pt[2], double lambda[4], std::vector<Size_tArr3> &nodes) const;

    bool _insideGrid(double x, double y, double z, size_t &i, size_t &j, size_t &k, double lambda[4], double zwgt[2]) const;

    void _getIndicesHelper(const std::vector<double> &coords, std::vector<size_t> &indices) const;

    bool _insideGridHelperStretched(double z, size_t &k, double zwgt[2]) const;

    bool _insideGridHelperTerrain(double x, double y, double z, const size_t &i, const size_t &j, size_t &k, double zwgt[2]) const;

    std::shared_ptr<QuadTreeRectangle<float, size_t>> _makeQuadTreeRectangle() const;
};
};    // namespace VAPoR
#endif
