#ifndef _RegularGrid_
#define _RegularGrid_

#include <ostream>
#include <vector>
#include <vapor/common.h>
#include <vapor/StructuredGrid.h>

namespace VAPoR {

//! \class RegularGrid
//! \brief This class implements a 2D or 3D regular grid.
//!
//! This class implements a 2D or 3D regular grid: a tessellation
//! of Euculidean space by rectangles (2D) or parallelpipeds (3D). Each
//! grid point can be addressed by an index(i,j,k), where \a i, \p a
//! and \a k range from 0 to \a dim - 1, where \a dim is the dimension of the
//! \a I, \a J, or \a K axis, respectively. Moreover, each grid point
//! has a coordinate in a user-defined coordinate system given by
//! (\a i * \a dx, \a j * \a dy, \a k * \a dz) for some real
//! numbers \a dx, \a dy, and \a dz representing the grid spacing.
//!
//

class VDF_API RegularGrid : public StructuredGrid {
public:
    //! \copydoc StructuredGrid::StructuredGrid(
    //!	const std::vector<size_t>&, const std::vector<bool>&,
    //!	const std::vector<float*>&
    //!	)
    //!
    //! Construct a regular grid sampling a 3D or 2D scalar function.
    //!
    //! Adds new parameters:
    //!
    //! \param[in] minu A vector provding the user coordinates of the first
    //! point in the grid.
    //! \param[in] maxu A vector provding the user coordinates of the last
    //! point in the grid. All elements of \p maxu must be greater than or
    //! equal to corresponding elements in \p minu.
    //!
    //
    RegularGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const std::vector<double> &minu, const std::vector<double> &maxu);

    RegularGrid() = default;
    virtual ~RegularGrid() = default;

    virtual size_t GetGeometryDim() const override { return (_geometryDim); }

    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const override;

    static std::string GetClassType() { return ("Regular"); }
    std::string        GetType() const override { return (GetClassType()); }

    //! \copydoc Grid::GetBoundingBox()
    //
    virtual void GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const override;

    //! \copydoc Grid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const DimsType &indices, CoordType &coords) const override;

    // For grandparent inheritance of
    // Grid::GetUserCoordinates(const size_t indices[], double coords[])
    //
    using Grid::GetUserCoordinates;

    //! \copydoc Grid::GetIndicesCell
    //!
    virtual bool GetIndicesCell(const CoordType &coords, DimsType &indices) const override;

    //! \copydoc Grid::InsideGrid()
    //
    virtual bool InsideGrid(const CoordType &coords) const override;

    class ConstCoordItrRG : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrRG(const RegularGrid *rg, bool begin);
        ConstCoordItrRG(const ConstCoordItrRG &rhs);

        ConstCoordItrRG();
        virtual ~ConstCoordItrRG() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstCoordType &deref() const { return (_coords); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCoordItrRG *itrptr = static_cast<const ConstCoordItrRG *>(rhs);

            return (_index == itrptr->_index);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(*this)); };

    private:
        std::vector<size_t> _index;
        std::vector<size_t> _dims;
        std::vector<double> _minu;
        std::vector<double> _delta;
        std::vector<double> _coords;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(this, false))); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const RegularGrid &rg);

protected:
    virtual float GetValueNearestNeighbor(const CoordType &coords) const override;

    virtual float GetValueLinear(const CoordType &coords) const override;

    //! \copydoc Grid::GetUserExtents()
    //
    virtual void GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const override;

private:
    void _SetExtents(const std::vector<double> &minu, const std::vector<double> &maxu);

    CoordType           _minu = {{0.0, 0.0, 0.0}};
    CoordType           _maxu = {{0.0, 0.0, 0.0}};
    size_t              _geometryDim;
    std::vector<double> _delta;    // increment between grid points in user coords
};
};    // namespace VAPoR
#endif
