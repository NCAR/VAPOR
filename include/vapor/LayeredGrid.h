#ifndef _LayeredGrid_
#define _LayeredGrid_
#include <vapor/common.h>
#include "RegularGrid.h"
#include "StretchedGrid.h"

namespace VAPoR {

//! \class LayeredGrid
//!
//! \brief This class implements a 2D or 3D layered grid.
//!
//! This class implements a 3D layered grid: a generalization
//! of a stretched grid where the spacing of grid points along the K dimension
//! varies at each grid point. The spacing along the remaining I and J
//! dimensions is invariant between grid points. I.e.
//! z coordinate is given by some
//! function f(i,j,k):
//!
//! z = f(i,j,k)
//!
//! where f() is monotonically increasing (or decreasing) with k.
//! The remaining x and y coordinates are givey by (i*dx, j*dy)
//! for some real dx and dy .
//!
//
class VDF_API LayeredGrid : public StructuredGrid {
public:
    //!
    //! Construct a layered grid sampling a 3D or 2D scalar function
    //!
    //! \copydoc StructuredGrid::StructuredGrid()
    //!
    //!
    //! Adds or changes parameters:
    //!
    //! \param[in] xcoords  A 1D vector whose size matches that of the I
    //! dimension of this class, and whose values specify the X user coordinates.
    //! \param[in] ycoords  A 1D vector whose size matches that of the J
    //! of the last grid point.
    //!
    //! \param[in] rg A RegularGrid instance with the same dimensionality and
    //! min/max offsets as specified by \p bs, \p min, and \p max. The
    //! data values of \p rg provide the user coordinates for the Z dinmension.
    //!
    LayeredGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const std::vector<double> &xcoords, const std::vector<double> &ycoords,
                const RegularGrid &rg);

    LayeredGrid() = default;
    virtual ~LayeredGrid() = default;

    virtual size_t GetGeometryDim() const override { return (3); }

    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const override;

    static std::string GetClassType() { return ("Layered"); }
    std::string        GetType() const override { return (GetClassType()); }

    //! \copydoc RegularGrid::GetValue()
    //!
    float GetValue(const DblArr3 &coords) const override;

    //! \copydoc Grid::GetInterpolationOrder()
    //
    virtual int GetInterpolationOrder() const override { return _interpolationOrder; };

    //! Set the interpolation order to be used during function reconstruction
    //!
    //! This method sets the order of the interpolation method that will
    //! be used when reconstructing the sampled scalar function. Valid values
    //! of \p order are 0,  1, and 2, corresponding to nearest-neighbor,linear,
    //! and quadratic
    //! interpolation, respectively. If \p order is invalid it will be silently
    //! set to 2. The default interpolation order is 1
    //!
    //! \param[in] order interpolation order
    //! \sa GetInterpolationOrder()
    //!
    virtual void SetInterpolationOrder(int order) override;

    //! \copydoc Grid::GetBoundingBox()
    //!
    virtual void GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const override;

    //! \copydoc Grid::GetUserCoordinates()
    //!
    virtual void GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const override;

    // For grandparent inheritance of
    // Grid::GetUserCoordinates(const size_t indices[], double coords[])
    //
    using Grid::GetUserCoordinates;

    //! \copydoc Grid::GetIndicesCell
    //!
    virtual bool GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const override;

    // For grandparent inheritance of
    // Grid::GetIndicesCell(const double coords[3], size_t indices[3])
    //
    using Grid::GetIndicesCell;

    //! \copydoc Grid::InsideGrid()
    //!
    bool InsideGrid(const DblArr3 &coords) const override;

    //! \copydoc Grid::GetPeriodic()
    //!
    //! Only horizonal dimensions can be periodic. Layered (third) dimension
    //! is ignored if set to periodic
    //
    virtual void SetPeriodic(const std::vector<bool> &periodic) override
    {
        VAssert(periodic.size() == 3);
        std::vector<bool> myPeriodic = periodic;
        myPeriodic[2] = false;
        Grid::SetPeriodic(myPeriodic);
    }

    //! Return the internal data structure containing a copy of the coordinate
    //! blocks passed in by the constructor
    //!
    const RegularGrid &GetZRG() const { return (_zrg); };

    class ConstCoordItrLayered : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrLayered(const LayeredGrid *rg, bool begin);
        ConstCoordItrLayered(const ConstCoordItrLayered &rhs);

        ConstCoordItrLayered();
        virtual ~ConstCoordItrLayered() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstCoordType &deref() const { return (_coords); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCoordItrLayered *itrptr = static_cast<const ConstCoordItrLayered *>(rhs);

            return (_zCoordItr == itrptr->_zCoordItr);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrLayered(*this)); };

    private:
        const LayeredGrid *          _lg;
        size_t                       _nElements2D;
        std::vector<double>          _coords;
        size_t                       _index2D;
        ConstIterator                _zCoordItr;
        StretchedGrid::ConstCoordItr _itr2D;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrLayered(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrLayered(this, false))); }

protected:
    //! \copydoc Grid::GetUserExtents()
    //!
    virtual void GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const override;

private:
    StretchedGrid       _sg2d;    // horizontal coordinates maintained in stretched grid
    RegularGrid         _zrg;     // vertical coords are the values of a regular grid
    std::vector<double> _xcoords;
    std::vector<double> _ycoords;
    DblArr3             _minu = {{0.0, 0.0, 0.0}};
    DblArr3             _maxu = {{0.0, 0.0, 0.0}};
    int                 _interpolationOrder;

    virtual float GetValueNearestNeighbor(const DblArr3 &coords) const override;

    virtual float GetValueLinear(const DblArr3 &coords) const override;

    //!
    //! Return the bilinear interpolation weights of a point given in user
    //! coordinates.  These weights apply to the x (iwgt) and y (jwgt) axes.
    //!
    //! This function applies the bilinear interpolation method to derive
    //! a the x and y axis weights of a point in user coordinates.
    //!
    //! \param[in] x coordinate of grid point along fastest varying dimension
    //! \param[in] y coordinate of grid point along second fastest
    //! varying dimension
    //! \param[out] a bilinearly calculated weight for the x axis
    //! \param[out] a bilinearly calculated weight for the y axis
    //
    void _getBilinearWeights(const double coords[3], double &iwgt, double &jwgt) const;

    //! This function applies the bilinear interpolation method to derive
    //! a the elevation from x and y axis weights of a point in user coordinates.
    //!
    //! \param[in] i index of bottom left cell corner
    //! \param[in] i index of top right cell corner
    //! \param[in] j index of bottom left cell corner
    //! \param[in] j index of top right cell corner
    //! \param[in] k index of the level to interpolate upon
    //! \param[in] the i-axis weight for bilinear interpolation
    //! \param[in] the j-axis weight for bilinear interpolation
    //! \param[out] a bilinearly calculated elevation value

    double _bilinearElevation(size_t i0, size_t i1, size_t j0, size_t j1, size_t k0, double iwgt, double jwgt) const;

    //! double _bilinearInterpolation(double x, double y, size_t k,
    //!						double *iwgt, double *jwgt) const;
    //!
    //! Return the bilinearly interpolated value of the currently opened variable
    //! of a point given in user coordinates.
    //!
    //! This function applies the bilinear interpolation method to derive
    //! a variable value from x and y axis weights of a point in user coordinates.
    //!
    //! \param[in] i index of bottom left cell corner
    //! \param[in] i index of top right cell corner
    //! \param[in] j index of bottom left cell corner
    //! \param[in] j index of top right cell corner
    //! \param[in] k index of the level to interpolate upon
    //! \param[in] the i-axis weight for bilinear interpolation
    //! \param[in] the j-axis weight for bilinear interpolation
    //! \param[out] a bilinearly calculated value of the currently open variable
    double _bilinearInterpolation(size_t i0, size_t i1, size_t j0, size_t j1, size_t k0, double iwgt, double jwgt) const;

    //! Return the interpolated value of a point in user
    //! coordinates.  This only interpolates in the vertical (z) direction.
    //!
    //! Return the quadratically interpolated value of a point in user
    //! coordinates.
    //!
    //! This function applies the quadratic interpolation method to derive
    //! a the value of a variable in user coordinates from its neighboring
    //! points in ijk space.  Linear interpolation is applied at the boundaries
    //! of the domain.
    //!
    //! \param[in] x coordinate of grid point along fastest varying dimension
    //! \param[in] y coordinate of grid point along second fastest
    //! varying dimension
    //! \param[in] z coordinate of grid point along third fastest
    //! varying dimension
    //! \param[out] a quadratically interpolated value of a point in user
    //! coordinates
    //!
    float _getValueQuadratic(const double coords[3]) const;

    //! Return the linearly interpolated value of a point in user
    //! coordinates.  This only interpolates in the vertical (z) direction.
    //!
    //! \param[in] x coordinate of grid point along fastest varying dimension
    //! \param[in] y coordinate of grid point along second fastest
    //! varying dimension
    //! \param[in] z coordinate of grid point along third fastest
    //! varying dimension
    //! \param[out] a linearly interpolated value of a point in user
    //! coordinates.
    //!
    double _verticalLinearInterpolation(double x, double y, double z) const;

    double _interpolateVaryingCoord(size_t i0, size_t j0, size_t k0, double x, double y) const;

    bool _insideGrid(const DblArr3 &coords, Size_tArr3 &indices, double wgts[3]) const;
};
};    // namespace VAPoR
#endif
