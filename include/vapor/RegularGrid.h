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

    RegularGrid();

    virtual ~RegularGrid();

    //! \copydoc Grid::GetUserExtents()
    //
    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const override;

    //! \copydoc Grid::GetBoundingBox()
    //
    virtual void GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const override;

    //! \copydoc Grid::GetEnclosingRegion()
    //
    virtual void GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const override;

    //! \copydoc Grid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const std::vector<size_t> &indices, std::vector<double> &coords) const override;

    //! \copydoc Grid::GetIndices()
    //
    virtual void GetIndices(const std::vector<double> &coords, std::vector<size_t> &indices) const override;

    //! \copydoc Grid::GetIndicesCell
    //!
    virtual bool GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const override;

    //! \copydoc Grid::InsideGrid()
    //
    virtual bool InsideGrid(const std::vector<double> &coords) const override;

    class ConstCoordItrRG : public StructuredGrid::ConstCoordItrAbstract {
    public:
        ConstCoordItrRG(const RegularGrid *rg, bool begin);
        ConstCoordItrRG(const ConstCoordItrRG &rhs);

        ConstCoordItrRG();
        virtual ~ConstCoordItrRG() {}

        virtual void                       next();
        virtual const std::vector<double> &deref() const { return (_coords); }
        virtual const void *               address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCoordItrRG *itrptr = static_cast<const ConstCoordItrRG *>(rhs);

            return (_x == itrptr->_x && _y == itrptr->_y && _z == itrptr->_z);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(*this)); };

    private:
        size_t              _x, _y, _z;
        std::vector<size_t> _dims;
        std::vector<double> _minu;
        std::vector<double> _delta;
        std::vector<double> _coords;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrRG(this, false))); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const RegularGrid &rg);

protected:
    virtual float _GetValueNearestNeighbor(const std::vector<double> &coords) const override;

    virtual float _GetValueLinear(const std::vector<double> &coords) const override;

private:
    void _SetExtents(const std::vector<double> &minu, const std::vector<double> &maxu);

    std::vector<double> _minu;
    std::vector<double> _maxu;     // User coords of first and last voxel
    std::vector<double> _delta;    // increment between grid points in user coords
};
};    // namespace VAPoR
#endif
