#ifndef _Grid_
#define _Grid_

#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>
#include <array>
#include <string>
#include <limits>
#include "vapor/VAssert.h"
#include <memory>
#include <vapor/common.h>

#ifdef WIN32
    #pragma warning(disable : 4661 4251)    // needed for template class
#endif

namespace VAPoR {

using DblArr3 = std::array<double, 3>;
using Size_tArr3 = std::array<size_t, 3>;

//! \class Grid
//! \brief Abstract base class for a 2D or 3D structured or unstructured
//!  grid.
//! \author John Clyne
//!
//! This abstract base class defines a 2D or 3D structured or unstructured
//! grid.
//!
//! The grid samples a scalar function at each grid point.  Each
//! grid point can be addressed by a multi-dimensional
//! vector <size_t> indices. The fastest varying dimension is
//! given by indices[0], etc.
//!
//! Because grid samples are repesented internally as arrays, when accessing
//! multiple grid points better performance is achieved by using
//! unit stride.
//!
//! \param indices A vector of integer indices i in the range 0..max,
//! where max is one minus the value of the corresponding element
//! returned by GetDimensions().
//!
//! \param coords A vector of floating point values with size given by
//! GetGeometryDim() containig the coordinates of a point in user-defined
//! coordinates.
//!
//
class VDF_API Grid {
public:
    //!
    //! Construct a structured or unstructured grid sampling a 3D or
    //! 2D scalar function
    //!
    //! \param[in] dims Dimensions of arrays containing grid data.
    //!
    //! \param[in] bs A vector with size matching \p dims, specifying the
    //! dimensions of
    //! each block storing the sampled scalar function.
    //!
    //! \param[in] blks An array of blocks containing the sampled function.
    //! The dimensions of each block
    //! is given by \p bs. The number of blocks is given by the product
    //! of the terms:
    //!
    //! \code (((dims[i]-1) / bs[i]) + 1) \endcode
    //!
    //! over i = 0..dims.size()-1
    //!
    //! A shallow copy of the blocks is made by the constructor. Memory
    //! referenced by the elements of \p blks should remain valid
    //! until the class instance is destroyed.
    //!
    //! \param[in] topology_dimension Topological dimension of
    //! grid: 2 or 3, for 2D or 3D, respectively. Grids with 2D
    //! topology are described by 2D spatial coordiantes, while
    //! grids with 2D topology are described by 3D spatial coordinates.
    //!
    Grid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, size_t topology_dimension);

    Grid() = default;
    virtual ~Grid() = default;

    //! Return the dimensions of grid connectivity array
    //!
    //! \param[out] dims The value of \p dims parameter provided to
    //! the constructor. If the parameter has less than 3 values, then
    //! number 1 will be filled.
    //!
    std::array<size_t, 3> GetDimensions() const
    {
        auto tmp = std::array<size_t, 3>{1, 1, 1};
        std::copy(_dims.begin(), _dims.end(), tmp.begin());
        return tmp;
    }

    //! Return the useful number of dimensions of grid connectivity array
    //!
    //! \param[out] dims The number of values of \p dims parameter provided to
    //! the constructor.
    //!
    size_t GetNumDimensions() const { return _dims.size(); }

    //! Return the dimensions of the specified coordinate variable
    //!
    //! \param[in] dim An integer between 0 and the return value
    //! of GetGeometryDim() - 1, indicating which
    //! coordinate dimensions are to be returned.
    //!
    //! if \p dim is greater than or equal to GetGeometryDim() - 1
    //! a vector of length one with its single component equal to
    //! one is returned.
    //!
    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const = 0;

    virtual std::string GetType() const = 0;

    //! Get geometric dimension of cells
    //!
    //! Returns the geometric dimension of the cells in the mesh. I.e.
    //! the number of spatial coordinates for each grid point.
    //! Valid values are 0..3. The geometric dimension must be equal to
    //! or greater than the topology dimension.
    //!
    //! \sa GetTopologyDim()
    //
    virtual size_t GetGeometryDim() const = 0;

    virtual const std::vector<size_t> &GetNodeDimensions() const = 0;
    virtual const std::vector<size_t> &GetCellDimensions() const = 0;

    //! Return the ijk dimensions of grid in blocks
    //!
    //! Returns the number of blocks defined along each axis of the grid
    //!
    //! \param[out] dims A two or three element array containing the grid
    //! dimension in blocks
    //!
    //! \sa GetBlockSize();
    //
    const std::vector<size_t> GetDimensionInBlks() const { return (_bdimsDeprecated); }

    //! Return the internal blocking factor
    //!
    //! This method returns the internal blocking factor passed
    //! to the constructor.
    //
    const std::vector<size_t> &GetBlockSize() const { return (_bsDeprecated); }

    //! Return the internal data structure containing a copy of the blocks
    //! passed in by the constructor
    //!
    const std::vector<float *> &GetBlks() const { return (_blks); };

    //! Get the data value at the indicated grid point
    //!
    //! This method provides read access to the scalar data value
    //! defined at the grid point indicated by \p indices. The range
    //! of valid indices is between zero and \a dim - 1, where \a dim
    //! is the dimesion of the grid returned by GetDimensions()
    //!
    //! If any of the \p indecies are outside of the
    //! valid range the results are undefined
    //!
    //! \param[in] indices of grid point along fastest varying dimension. The
    //! size of \p indices must be equal to that of the \p dims vector
    //! returned by GetDimensions()
    //!
    virtual float GetValueAtIndex(const Size_tArr3 &indices) const;

    //! \deprecated
    //
    virtual float GetValueAtIndex(const std::vector<size_t> &indices) const
    {
        Size_tArr3 a = {0, 0, 0};
        CopyToArr3(indices, a);
        return (GetValueAtIndex(a));
    }

    //! Set the data value at the indicated grid point
    //!
    //! This method sets the data value of the grid point indexed by
    //! \p indices to \p value.
    //!
    virtual void SetValue(const Size_tArr3 &indices, float value);

    //! \deprecated
    //
    virtual void SetValue(const size_t indices[3], float value)
    {
        Size_tArr3 i3 = {0, 0, 0};
        CopyToArr3(indices, GetNodeDimensions().size(), i3);
        SetValue(i3, value);
    }

    //! This method provides an alternate interface to Grid::GetValueAtIndex()
    //! If the dimensionality of the grid as determined by GetDimensions() is
    //! less than three subsequent parameters are ignored. Parameters
    //! that are outside of range are clamped to boundaries.
    //!
    //! \param[in] i Index into first fastest varying dimension
    //! \param[in] j Index into second fastest varying dimension
    //! \param[in] k Index into third fastest varying dimension
    //
    virtual float AccessIJK(size_t i, size_t j = 0, size_t k = 0) const;

    void SetValueIJK(size_t i, size_t j, size_t k, float v);
    void SetValueIJK(size_t i, size_t j, float v) { SetValueIJK(i, j, 0, v); }
    void SetValueIJK(size_t i, float v) { SetValueIJK(i, 0, 0, v); }

    //! Get the reconstructed value of the sampled scalar function
    //!
    //! This method reconstructs the scalar field at an arbitrary point
    //! in space. If the point's coordinates are outside of the grid's
    //! coordinate extents as returned by GetUserExtents(), and the grid
    //! is not periodic along the out-of-bounds axis, the value
    //! returned will be the \a missing_value.
    //!
    //! If the value of any of the grid point samples used in the reconstruction
    //! is the \a missing_value then the result returned is the \a missing_value.
    //!
    //! The reconstruction method used is determined by interpolation
    //! order returned by GetInterpolationOrder()
    //!
    //! \param[in] coords A vector of size matching the topology dimension
    //! of the mesh whose contents specify the coordinates of a point in space.
    //!
    //! \sa GetInterpolationOrder(), HasPeriodic(), GetMissingValue()
    //! \sa GetUserExtents()
    //!
    virtual float GetValue(const DblArr3 &coords) const;

    //! \deprecated
    //
    virtual float GetValue(const std::vector<double> &coords) const
    {
        DblArr3 c3 = {0.0, 0.0, 0.0};
        CopyToArr3(coords, c3);
        return (GetValue(c3));
    };

    //! \deprecated
    //
    virtual float GetValue(const double coords[]) const
    {
        DblArr3 c3 = {0.0, 0.0, 0.0};
        CopyToArr3(coords, GetGeometryDim(), c3);
        return (GetValue(c3));
    }

    virtual float GetValue(double x, double y) const
    {
        double coords[] = {x, y, 0.0};
        return (GetValue(coords));
    }
    virtual float GetValue(double x, double y, double z) const
    {
        double coords[] = {x, y, z};
        return (GetValue(coords));
    }

    //! Return the extents of the user coordinate system
    //!
    //! This pure virtual method returns min and max extents of
    //! the user coordinate
    //! system defined on the grid. The extents of the returned box
    //! are guaranteed to contain all points in the grid.
    //!
    //! \param[out] minu A two or three element array containing the minimum
    //! user coordinates.
    //! \param[out] maxu A two or three element array containing the maximum
    //! user coordinates.
    //!
    //! \sa GetDimensions(), Grid()
    //!
    virtual void GetUserExtents(DblArr3 &minu, DblArr3 &maxu) const;

    //! \deprecated
    //
    virtual void GetUserExtents(double minu[3], double maxu[3]) const
    {
        DblArr3 minu3 = {0.0, 0.0, 0.0};
        DblArr3 maxu3 = {0.0, 0.0, 0.0};
        GetUserExtents(minu3, maxu3);
        CopyFromArr3(minu3, minu);
        CopyFromArr3(maxu3, maxu);
    }

    //! \deprecated
    //
    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const
    {
        DblArr3 minu3 = {0.0, 0.0, 0.0};
        DblArr3 maxu3 = {0.0, 0.0, 0.0};
        GetUserExtents(minu3, maxu3);
        CopyFromArr3(minu3, minu);
        CopyFromArr3(maxu3, maxu);

        // Much of the use of this method assumes that the size of the minu,maxu
        // vectors can be used to determine the number of coordinates. So we
        // maintain this property for now.
        //
        minu.resize(GetGeometryDim());
        maxu.resize(GetGeometryDim());
    }

    //! Return the extents of the axis-aligned bounding box enclosign a region
    //!
    //! This pure virtual method returns min and max extents, in user coordinates,
    //! of the smallest axis-aligned box enclosing the region defined
    //! by the grid indices, \p min and \p max. Every grid point in
    //! the range \p min to \p max will be contained in, or reside on, the
    //! box (rectangle) whose extents are given by \p minu and \p maxu.
    //!
    //! \note The results are undefined if any index of \p min is
    //! greater than the coresponding coordinate of \p max, or if \p max
    //! is outside of the valid dimension range (See GetDimension()).
    //!
    //! The size of \p min and \p max must match the grid's dimension
    //! as returned by GetDimension()
    //!
    //! \param[in] min An array containing the minimum
    //! grid indices (offsets).
    //! \param[in] max An array containing the maximum
    //! grid indices (offsets).
    //! \param[out] minu A two-to-three element array containing the minimum
    //! user coordinates.
    //! \param[out] maxu A two-to-three element array containing the maximum
    //! user coordinates.
    //!
    //! \sa GetDimensions(), Grid()
    //!
    virtual void GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const = 0;

    virtual void GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const
    {
        VAssert(min.size() == max.size());
        Size_tArr3 min3 = {0, 0, 0};
        Size_tArr3 max3 = {0, 0, 0};
        DblArr3    minu3 = {0.0, 0.0, 0.0};
        DblArr3    maxu3 = {0.0, 0.0, 0.0};

        CopyToArr3(min, min3);
        CopyToArr3(max, max3);
        GetBoundingBox(min3, max3, minu3, maxu3);
        CopyFromArr3(minu3, minu);
        CopyFromArr3(maxu3, maxu);
    }

    //!
    //! Get bounding indices of grid containing a region
    //!
    //! Calculates the starting and ending grid indices of the
    //! smallest grid
    //! completely containing the rectangular region defined by the user
    //! coordinates \p minu and \p maxu
    //! If rectangluar region defined by \p minu and \p maxu can
    //! not be contained the
    //! minimum and maximum indices are returned in
    //! \p min and \p max, respectively
    //!
    //! \param[in] minu User coordinates of minimum coorner
    //! \param[in] maxu User coordinates of maximum coorner
    //! \param[out] min Integer coordinates of minimum coorner
    //! \param[out] max Integer coordinates of maximum coorner
    //!
    virtual bool GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const = 0;

    //! Return the value of the \a missing_value parameter
    //!
    //! The missing value is a special value intended to indicate that the
    //! value of the sampled or reconstructed function is unknown at a
    //! particular point.
    //!
    virtual float GetMissingValue() const;

    //! Set the missing value indicator
    //!
    //! This method sets the value of the missing value indicator. The
    //! method does not alter the value of any grid point locations, nor
    //! does it alter the missing data flag set with the constructor.
    //!
    //! \sa HasMissingData(), GetMissingValue()
    //!
    void SetMissingValue(float missing_value) { _missingValue = missing_value; };

    //! Set missing data flag
    //!
    //! Set this flag if missing values may exist in the data. This missing
    //! values are denoted by the value GetMissingValue(). Subsequent processing
    //! of grid data will be compared against the value of GetMissingValue()
    //! if this flag is set.
    //
    void SetHasMissingValues(bool flag) { _hasMissing = flag; }

    //! Return missing data flag
    //!
    //! This method returns true if the missing data flag is set.
    //! This does not
    //! imply that grid points exist with missing data, only that the
    //! the grid points should be compared against the value of
    //! GetMissingValue() whenever operations are performed on them.
    //
    bool HasMissingData() const { return (_hasMissing); };

    //! Return true if mesh primitives have counter clockwise winding
    //! order.
    //
    virtual bool HasInvertedCoordinateSystemHandiness() const { return (true); }

    //! Return the interpolation order to be used during function reconstruction
    //!
    //! This method returns the order of the interpolation method that will
    //! be used when reconstructing the sampled scalar function
    //!
    //! \sa SetInterpolationOrder()
    //!
    virtual int GetInterpolationOrder() const { return _interpolationOrder; };

    //! Set the interpolation order to be used during function reconstruction
    //!
    //! This method sets the order of the interpolation method that will
    //! be used when reconstructing the sampled scalar function. Valid values
    //! of \p order are 0 and 1, corresponding to nearest-neighbor and linear
    //! interpolation, respectively. If \p order is invalid it will be silently
    //! set to 1. The default interpolation order is 1
    //!
    //! \param[in] order interpolation order
    //! \sa GetInterpolationOrder()
    //!
    virtual void SetInterpolationOrder(int order);

    //! Return the user coordinates of a grid point
    //!
    //! This method returns the user coordinates of the grid point
    //! specified by \p indices
    //!
    //! Results are undefined if \p indices is out of range.
    //!
    //! \param[in] index Logical index into coordinate array. The dimensionality
    //! and range of component values are given by GetNodeDimensions(). The
    //! starting value for each component of \p index is zero. \p index must contain
    //! GetNodeDimensions().size() elements.
    //!
    //! \param[out] coord User coordinates of grid point with indices
    //! given by \p indices. \p coord must have space for the number of elements
    //! returned by GetGeometryDim().
    //!
    virtual void GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const = 0;

    virtual void GetUserCoordinates(const size_t indices[], double coords[]) const
    {
        Size_tArr3 indices3 = {0, 0, 0};
        DblArr3    coords3 = {0.0, 0.0, 0.0};
        CopyToArr3(indices, GetNodeDimensions().size(), indices3);
        GetUserCoordinates(indices3, coords3);
        CopyFromArr3(coords3, coords);
    }

    virtual void GetUserCoordinates(const std::vector<size_t> &indices, std::vector<double> &coords) const
    {
        Size_tArr3 indices3 = {0, 0, 0};
        DblArr3    coords3 = {0.0, 0.0, 0.0};
        CopyToArr3(indices, indices3);
        GetUserCoordinates(indices3, coords3);
        CopyFromArr3(coords3, coords);
    }

    virtual void GetUserCoordinates(size_t i, double &x, double &y, double &z) const;
    virtual void GetUserCoordinates(size_t i, size_t j, double &x, double &y, double &z) const;
    virtual void GetUserCoordinates(size_t i, size_t j, size_t k, double &x, double &y, double &z) const;

    //! Return the indices of the cell containing the
    //! specified user coordinates
    //!
    //! This method returns the cell ID (index) of the cell containing
    //! the specified user coordinates. If any
    //! of the input coordinates correspond to periodic dimensions the
    //! the coordinate(s) are first re-mapped to lie inside the grid
    //! extents as returned by GetUserExtents()
    //!
    //! If the specified coordinates lie outside of the grid (are not
    //! contained by any cell) the method returns false, otherwise true is
    //! returned.
    //!
    //! \retval status true on success, false if the point is not contained
    //! by any cell.
    //!
    virtual bool GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const = 0;

    //! \deprecated
    //
    virtual bool GetIndicesCell(const double coords[3], size_t indices[3]) const
    {
        DblArr3    c3 = {0.0, 0.0, 0.0};
        Size_tArr3 i3 = {0, 0, 0};
        CopyToArr3(coords, GetGeometryDim(), c3);
        bool status = GetIndicesCell(c3, i3);
        CopyFromArr3(i3, indices);
        return (status);
    }

    //! \deprecated
    //
    virtual bool GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const
    {
        DblArr3    c3 = {0.0, 0.0, 0.0};
        Size_tArr3 i3 = {0, 0, 0};
        CopyToArr3(coords, c3);
        bool status = GetIndicesCell(c3, i3);
        CopyFromArr3(i3, indices);
        return (status);
    }

    //! Return the min and max data value
    //!
    //! This method returns the values of grid points with min and max values,
    //! respectively.
    //!
    //! For dataless grids the missing value is returned.
    //!
    //! \param[out] range[2] A two-element array containing the mininum and
    //! maximum values, in that order
    //!
    virtual void GetRange(float range[2]) const;

    virtual void GetRange(const Size_tArr3 &min, const Size_tArr3 &max, float range[2]) const;

    //! \deprecated
    //
    virtual void GetRange(std::vector<size_t> min, std::vector<size_t> max, float range[2]) const
    {
        Size_tArr3 min3 = {0, 0, 0};
        Size_tArr3 max3 = {0, 0, 0};
        CopyToArr3(min, min3);
        CopyToArr3(max, max3);
        GetRange(min3, max3, range);
    }

    //! Return true if the specified point lies inside the grid
    //!
    //! This method can be used to determine if a point expressed in
    //! user coordinates reside inside or outside the grid
    //!
    //! \param[in] coords User coordinates of point in space
    //!
    //! \retval bool True if point is inside the grid
    //!
    virtual bool InsideGrid(const DblArr3 &coords) const = 0;

    //! \deprecated
    //
    virtual bool InsideGrid(const double coords[3]) const
    {
        DblArr3 c3 = {0.0, 0.0, 0.0};
        CopyToArr3(coords, GetGeometryDim(), c3);
        return (InsideGrid(c3));
    }

    //! \deprecated
    //
    virtual bool InsideGrid(const std::vector<double> &coords) const
    {
        DblArr3 c3 = {0.0, 0.0, 0.0};
        CopyToArr3(coords, c3);
        return (InsideGrid(c3));
    }

    //! Get the indices of the nodes that define a cell
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices for a node that defines the given cell ID
    //! \p indices.
    //!
    //! For 2D cells the node IDs are returned in counter-clockwise order.
    //! For 3D cells the ordering is dependent on the shape of the node. TBD.
    //!
    //! \param[in] cindices An array of size Grid::GetDimensions.size() specifying
    //! the index of the cell to query.
    //! \param[out] nodes An array of size
    //! Grid::GetMaxVertexPerCell() x Grid::GetDimensions.size() that will contain
    //! the concatenated list of node indices on return.
    //! \param[out] n The number of node indices returned in \p nodes, an integer
    //! in the range (0..Grid::GetMaxVertexPerCell()).
    //!
    //!
    virtual bool GetCellNodes(const Size_tArr3 &cindices, std::vector<Size_tArr3> &nodes) const = 0;

    //! \deprecated
    //
    virtual bool GetCellNodes(const size_t cindices[], std::vector<Size_tArr3> &nodes) const
    {
        Size_tArr3 i3 = {0, 0, 0};
        CopyToArr3(cindices, GetCellDimensions().size(), i3);
        return (GetCellNodes(i3, nodes));
    }

    //! Get the IDs (indices) of all of the cells that border a cell
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices of a cell that borders the cell given by
    //! \p indices. If a
    //! cell edge is a boundary edge, having no neighbors, the associated
    //! index vector for that border will be empty.
    //! The cell IDs are returned in counter-clockwise order
    //!
    //! \param[in] cindices An ordered vector of multi-dimensional cell
    //! indices.
    //! \param[out] cells A vector of index vectors. Each index vector
    //! has size given by GetDimensions.size()
    //!
    virtual bool GetCellNeighbors(const Size_tArr3 &cindices, std::vector<Size_tArr3> &cells) const = 0;

    //! Get the IDs (indices) of all of the cells that share a node
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices of a cell that share the node given by
    //! \p indices.
    //! The cell IDs are returned in counter-clockwise order
    //!
    //! \param[out] nodes A vector of index vectors . Each index vector
    //! has size given by GetDimensions.size()
    //!
    virtual bool GetNodeCells(const Size_tArr3 &indices, std::vector<Size_tArr3> &cells) const = 0;

    //! Return the maximum number of vertices per cell face
    //!
    virtual size_t GetMaxVertexPerFace() const = 0;

    //! Return the maximum number of vertices per cell
    //!
    virtual size_t GetMaxVertexPerCell() const = 0;

    //! Clamp periodic coordinates and ensure valid coordinate vector dimension
    //!
    //! This method ensures that periodic coordinates are within the bounding
    //! box of the grid and that the coordinate vector dimension does not
    //! exceed the number of allowable coordinates as returned by
    //! GetGeometryDim().
    //!
    //! \param[in] coords A coordinate vector
    //! \param[out] cCoords The clamped coordintes \p coords
    //! \sa GetGeometryDim()
    //
    virtual void ClampCoord(const DblArr3 &coords, DblArr3 &cCoords) const = 0;

    //! \deprecated
    //
    virtual void ClampCoord(const double coords[3], double cCoords[3]) const
    {
        DblArr3 c3 = {coords[0], coords[1], coords[2]};
        DblArr3 cC3;
        ClampCoord(c3, cC3);
        cCoords[0] = cC3[0];
        cCoords[1] = cC3[1];
        cCoords[2] = cC3[2];
    }

    //! Clamp grid array indices
    //!
    //! This method ensures that grid indices are not out of bounds, clamping
    //! any elements of \p indices that exceeds or equals the corresponding element of
    //! GetNodeDimensions() to that value minus 1.
    //!
    //! \param[in] indices An array index vector
    //! \param[out] cindices An array index vector, clamped as described above
    //!
    //! \sa GetNodeDimensions()
    //
    virtual void ClampIndex(const Size_tArr3 &indices, Size_tArr3 &cIndices) const { ClampIndex(GetNodeDimensions(), indices, cIndices); }

    //! Clamp grid cell indices
    //!
    //! Same as ClampIndex() accept that indices are clamped to GetCellDimensions()
    //!
    //! \sa ClampIndex()
    //
    virtual void ClampCellIndex(const Size_tArr3 &indices, Size_tArr3 &cIndices) const { ClampIndex(GetCellDimensions(), indices, cIndices); }

    //! Set periodic boundaries
    //!
    //! This method changes the periodicity of boundaries set
    //! by the class constructor
    //!
    //! \param[in] periodic A boolean vector of size given by
    //! GetGeometryDim() indicating
    //! which coordinate axes are periodic.
    //
    virtual void SetPeriodic(const std::vector<bool> &periodic)
    {
        _periodic.clear();
        int i = 0;
        for (; i < periodic.size() && i < GetGeometryDim(); i++) { _periodic.push_back(periodic[i]); }
        for (; i < GetGeometryDim(); i++) { _periodic.push_back(false); }
    }

    //! Check for periodic boundaries
    //!
    //
    virtual const std::vector<bool> &GetPeriodic() const { return (_periodic); }

    //! Get topological dimension of the mesh
    //!
    //! Return the number of topological dimensions for the mesh cells. Valid
    //! values are in the range 0..3, with, for example, 0 corresponding
    //! to points; 1 to lines; 2 to triangles, quadrilaterals, etc.; and
    //! 3 to hexahedron, tetrahedron, etc.
    //!
    //! \sa GetGeometryDim()
    //
    virtual size_t GetTopologyDim() const { return (_topologyDimension); }

    //! Get the linear offset to the node IDs
    //!
    //! Return the smallest node ID. The default is zero
    //
    virtual long GetNodeOffset() const { return (_nodeIDOffset); }
    virtual void SetNodeOffset(long offset) { _nodeIDOffset = offset; }

    //! Get the linear offset to the cell IDs
    //!
    //! Return the smallest Cell ID. The default is zero
    //
    virtual long GetCellOffset() const { return (_cellIDOffset); }
    virtual void SetCellOffset(long offset) { _cellIDOffset = offset; }

    //! Return the absolute minimum grid coordinate
    //!
    //! This method returns the absolute minimum grid coordinate. If
    //! the Grid contains a subregion extracted from a larger mesh this
    //! absolute minimum grid coordinate gives the offset of the first
    //! gridpoint in this grid relative to the larger mesh.
    //!
    //! \note The value of returned is not used within the Grid class
    //! and any value can be stored here using SetMinAbs().
    //!
    virtual std::vector<size_t> GetMinAbs() const { return (_minAbs); }

    //! Set the absolute minimum grid coordinate
    //!
    //! \param[in] minAbs Must have same dimensionality as constructors \p dims
    //! parameter. Otherwise may contain any value, but is intended to contain
    //! the offset to the first grid point in the mesh. The default is the
    //! zero vector
    //
    virtual void SetMinAbs(const std::vector<size_t> &minAbs)
    {
        VAssert(minAbs.size() == this->GetNumDimensions());
        _minAbs = minAbs;
    }

    //! Test whether a point is contained in a bounding rectangle
    //!
    //! This static method checks to see if a 2D point \p pt is contained
    //! in the smallest rectangle that bounds the list of 2D points (vertices)
    //! given by \p verts. If \p pt is inside or on the boundary of the bounding
    //! rectangle true is returned, otherwise false
    //!
    //! \param[in] pt A two-element array of point coordinates
    //! \param[in] verts An array of dimensions \p n * 2 containing
    //! a list of \p points.
    //! \param[in] n The number of 2D points in \p verts
    //!
    //! \retval status Returns true if \pt is inside or on the bounding rectangle
    //! of \p verts. False otherwise.
    //
    static bool PointInsideBoundingRectangle(const double pt[], const double verts[], int n)
    {
        VAssert(n > 2);

        double left = verts[0];
        double right = verts[0];
        double top = verts[1];
        double bottom = verts[1];

        for (int i = 1; i < n; i++) {
            if (verts[i * 2 + 0] < left) left = verts[i * 2 + 0];
            if (verts[i * 2 + 0] > right) right = verts[i * 2 + 0];
            if (verts[i * 2 + 1] < top) top = verts[i * 2 + 1];
            if (verts[i * 2 + 1] > bottom) bottom = verts[i * 2 + 1];
        }

        return ((left <= pt[0]) && (right >= pt[0]) && (top <= pt[1]) && (bottom >= pt[1]));
    }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const Grid &g);

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    //! Inside a box functor
    //!
    //! operator() returns true if point pt is on or inside the axis-aligned
    //! box defined by min and max
    //
    class InsideBox {
    public:
        InsideBox(const std::vector<double> &min, const std::vector<double> &max) : _min(min), _max(max) {}
        InsideBox() {}

        bool operator()(const std::vector<double> &pt) const
        {
            for (int i = 0; i < _min.size() && i < pt.size(); i++) {
                if (pt[i] < _min[i] || pt[i] > _max[i]) return (false);
            }
            return (true);
        }
        bool operator()(const double pt[]) const
        {
            for (int i = 0; i < _min.size(); i++) {
                if (pt[i] < _min[i] || pt[i] > _max[i]) return (false);
            }
            return (true);
        }

        size_t Size() const { return (_min.size()); }

    private:
        std::vector<double> _min;
        std::vector<double> _max;
    };

    //
    // Define polymorphic iterator that can be used with any
    // class derived from this class
    //
    //

    // Interface for iterator specializations
    //
    template<typename T> class AbstractIterator {
    public:
        virtual ~AbstractIterator() {}
        virtual void                              next() = 0;
        virtual void                              next(const long &offset) = 0;
        virtual T &                               deref() const = 0;
        virtual const void *                      address() const = 0;
        virtual bool                              equal(const void *other) const = 0;
        virtual std::unique_ptr<AbstractIterator> clone() const = 0;
    };

    // Overloaded operators that will act on spealizations of
    // AbstractIterator
    //
    template<typename T> class PolyIterator {
    public:
        PolyIterator(std::unique_ptr<AbstractIterator<T>> it) : _impl(std::move(it)) {}

        PolyIterator(PolyIterator const &rhs) : _impl(rhs._impl->clone()) {}

        PolyIterator &operator=(PolyIterator const &rhs)
        {
            _impl = rhs._impl->clone();
            return *this;
        }

        // PolyIterator has a unique_ptr member so we must provide
        // std::move constructors
        //
        PolyIterator(PolyIterator &&rhs) { _impl = std::move(rhs._impl); }

        PolyIterator &operator=(PolyIterator &&rhs)
        {
            if (this != &rhs) { _impl = std::move(rhs._impl); }
            return (*this);
        }

        PolyIterator() : _impl(nullptr) {}

        PolyIterator &operator++()
        {    // ++prefix
            _impl->next();
            return *this;
        };

        PolyIterator operator++(int)
        {    // postfix++
            PolyIterator temp(*this);
            ++(*this);
            return (temp);
        };

        PolyIterator &operator+=(const long &offset)
        {
            _impl->next(offset);
            return (*this);
        };

        PolyIterator operator+(const long &offset) const
        {
            PolyIterator temp(*this);
            temp += offset;
            return (temp);
        }

        const T &operator*() const { return _impl->deref(); }

        bool operator==(const PolyIterator &rhs) const { return (_impl->equal(rhs._impl->address())); }

        bool operator!=(const PolyIterator &rhs) const { return (!(*this == rhs)); }

    private:
        std::unique_ptr<AbstractIterator<T>> _impl;
    };

    //! Coordinate iterator. Iterates over grid node/cell coordinates
    //!
    //! The ConstCoordItr can be dererenced to return a grid node or
    //! cell's coordinates. The determination, node or cell, is determined
    //! by the location of the sampled data within the grid (node, face,
    //! cell, etc)
    //!
    //! N.B. Current only works with node coordinates
    //!
    //
    typedef const std::vector<double>              ConstCoordType;
    typedef Grid::PolyIterator<ConstCoordType>     ConstCoordItr;
    typedef Grid::AbstractIterator<ConstCoordType> ConstCoordItrAbstract;

    //! Return constant grid coordinate iterator
    //
    virtual ConstCoordItr ConstCoordBegin() const = 0;
    virtual ConstCoordItr ConstCoordEnd() const = 0;

    //! Node index iterator. Iterates over grid node indices
    //!
    //! The ConstNodeIterator is dereferenced to give the index of
    //! a node within the grid
    //!
    typedef const std::vector<size_t>              ConstIndexType;
    typedef Grid::PolyIterator<ConstIndexType>     ConstNodeIterator;
    typedef Grid::AbstractIterator<ConstIndexType> ConstNodeIteratorAbstract;

    //! Cell index iterator. Iterates over grid cell indices
    //
    typedef Grid::PolyIterator<ConstIndexType>     ConstCellIterator;
    typedef Grid::AbstractIterator<ConstIndexType> ConstCellIteratorAbstract;

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    //
    // Node index iterator. Iterates over node indices
    //
    class ConstNodeIteratorSG : public Grid::ConstNodeIteratorAbstract {
    public:
        ConstNodeIteratorSG(const Grid *g, bool begin);
        ConstNodeIteratorSG(const ConstNodeIteratorSG &rhs);
        ConstNodeIteratorSG();

        virtual ~ConstNodeIteratorSG() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstIndexType &deref() const { return (_index); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstNodeIteratorSG *itrptr = static_cast<const ConstNodeIteratorSG *>(rhs);
            return (_index == itrptr->_index);
        }

        virtual std::unique_ptr<ConstNodeIteratorAbstract> clone() const { return std::unique_ptr<ConstNodeIteratorAbstract>(new ConstNodeIteratorSG(*this)); };

    protected:
        std::vector<size_t> _dims;
        std::vector<size_t> _index;
        std::vector<size_t> _lastIndex;
    };

    class ConstNodeIteratorBoxSG : public ConstNodeIteratorSG {
    public:
        ConstNodeIteratorBoxSG(const Grid *g, const std::vector<double> &minu, const std::vector<double> &maxu);
        ConstNodeIteratorBoxSG(const ConstNodeIteratorBoxSG &rhs);
        ConstNodeIteratorBoxSG();

        virtual ~ConstNodeIteratorBoxSG() {}

        virtual void next();
        virtual void next(const long &offset);

        virtual std::unique_ptr<ConstNodeIteratorAbstract> clone() const { return std::unique_ptr<ConstNodeIteratorAbstract>(new ConstNodeIteratorBoxSG(*this)); };

    private:
        InsideBox     _pred;
        ConstCoordItr _coordItr;
    };

    //! Return constant grid node coordinate iterator
    //!
    //! If \p minu and \p maxu are specified the iterator is constrained to
    //! operation within the axis-aligned box defined by \p minu and \p maxu.
    //!
    //! \param[in] minu Minimum box coordinate.
    //! \param[in] maxu Maximum box coordinate.
    //!
    virtual ConstNodeIterator ConstNodeBegin() const { return ConstNodeIterator(std::unique_ptr<ConstNodeIteratorAbstract>(new ConstNodeIteratorSG(this, true))); }

    virtual ConstNodeIterator ConstNodeBegin(const std::vector<double> &minu, const std::vector<double> &maxu) const
    {
        return ConstNodeIterator(std::unique_ptr<ConstNodeIteratorAbstract>(new ConstNodeIteratorBoxSG(this, minu, maxu)));
    }

    virtual ConstNodeIterator ConstNodeEnd() const { return ConstNodeIterator(std::unique_ptr<ConstNodeIteratorAbstract>(new ConstNodeIteratorSG(this, false))); }

    //
    // Cell index iterator. Iterates over cell indices
    //
    class ConstCellIteratorSG : public Grid::ConstCellIteratorAbstract {
    public:
        ConstCellIteratorSG(const Grid *g, bool begin);
        ConstCellIteratorSG(const ConstCellIteratorSG &rhs);
        ConstCellIteratorSG();

        virtual ~ConstCellIteratorSG() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstIndexType &deref() const { return (_index); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCellIteratorSG *itrptr = static_cast<const ConstCellIteratorSG *>(rhs);

            return (_index == itrptr->_index);
        }

        virtual std::unique_ptr<ConstCellIteratorAbstract> clone() const { return std::unique_ptr<ConstCellIteratorAbstract>(new ConstCellIteratorSG(*this)); };

    protected:
        std::vector<size_t> _dims;
        std::vector<size_t> _index;
        std::vector<size_t> _lastIndex;
    };

    class ConstCellIteratorBoxSG : public ConstCellIteratorSG {
    public:
        ConstCellIteratorBoxSG(const Grid *g, const std::vector<double> &minu, const std::vector<double> &maxu);
        ConstCellIteratorBoxSG(const ConstCellIteratorBoxSG &rhs);
        ConstCellIteratorBoxSG();

        virtual ~ConstCellIteratorBoxSG() {}

        virtual void next();
        virtual void next(const long &offset);

        virtual std::unique_ptr<ConstCellIteratorAbstract> clone() const { return std::unique_ptr<ConstCellIteratorAbstract>(new ConstCellIteratorBoxSG(*this)); };

    private:
        InsideBox _pred;
#ifdef VAPOR3_0_0
        ConstCoordItr _coordItr;
#else
        const Grid *_g;
        bool        _cellInsideBox(const size_t cindices[]) const;
#endif
    };

    //! Return constant grid cell coordinate iterator
    //!
    //! If \p minu and \p maxu are specified the iterator is constrained to
    //! operation within the axis-aligned box defined by \p minu and \p maxu.
    //!
    //! \param[in] minu Minimum box coordinate.
    //! \param[in] maxu Maximum box coordinate.
    //!
    virtual ConstCellIterator ConstCellBegin() const { return ConstCellIterator(std::unique_ptr<ConstCellIteratorAbstract>(new ConstCellIteratorSG(this, true))); }

    virtual ConstCellIterator ConstCellBegin(const std::vector<double> &minu, const std::vector<double> &maxu) const
    {
        return ConstCellIterator(std::unique_ptr<ConstCellIteratorAbstract>(new ConstCellIteratorBoxSG(this, minu, maxu)));
    }

    virtual ConstCellIterator ConstCellEnd() const { return ConstCellIterator(std::unique_ptr<ConstCellIteratorAbstract>(new ConstCellIteratorSG(this, false))); }

    //! A forward iterator for accessing the data elements of the
    //! structured grid.
    //!
    //! This class provides a C++ STL style Forward Iterator for
    //! accessing grid elements passed to the constructor. All Forward
    //! Iterator expressions are supported. In addition, the following
    //! Random Access Iterator expressions are supported:
    //!
    //! \li \c a + n
    //! \li \c n + a
    //!
    //! where \i a are objects of type ForwardIterator, and \i n is an int.
    //!
    //! N.B. this class does NOT need to be a template. It's a historical
    //! implementation.
    //
    template<class T> class VDF_API ForwardIterator {
    public:
        ForwardIterator(T *rg, bool begin = true, const std::vector<double> &minu = {}, const std::vector<double> &maxu = {});
        ForwardIterator();
        ForwardIterator(const ForwardIterator<T> &) = default;
        ForwardIterator(ForwardIterator<T> &&rhs);
        ~ForwardIterator() {}

        float &operator*() { return (*_itr); }

        ForwardIterator<T> &operator++();    // ++prefix

        ForwardIterator<T> operator++(int);    // postfix++

        ForwardIterator<T> &operator+=(const long int &offset);
        ForwardIterator<T>  operator+(const long int &offset) const;

        ForwardIterator<T> &operator=(ForwardIterator<T> rhs);
        ForwardIterator<T> &operator=(ForwardIterator<T> &rhs) = delete;

        bool operator==(const ForwardIterator<T> &rhs) const { return (_indexL == rhs._indexL); }
        bool operator!=(const ForwardIterator<T> &rhs) { return (_indexL != rhs._indexL); }

        const ConstCoordItr &GetCoordItr() { return (_coordItr); }

        friend void swap(Grid::ForwardIterator<T> &a, Grid::ForwardIterator<T> &b)
        {
            std::swap(a._ndims, b._ndims);
            std::swap(a._blks, b._blks);
            std::swap(a._dims3d, b._dims3d);
            std::swap(a._bdims3d, b._bdims3d);
            std::swap(a._bs3d, b._bs3d);
            std::swap(a._blocksize, b._blocksize);
            std::swap(a._coordItr, b._coordItr);
            std::swap(a._index, b._index);
            std::swap(a._indexL, b._indexL);
            std::swap(a._end_indexL, b._end_indexL);
            std::swap(a._xb, b._xb);
            std::swap(a._itr, b._itr);
            std::swap(a._pred, b._pred);
        }

    private:
        size_t               _ndims;
        std::vector<float *> _blks;
        std::vector<size_t>  _dims3d;
        std::vector<size_t>  _bdims3d;
        std::vector<size_t>  _bs3d;
        size_t               _blocksize;
        ConstCoordItr        _coordItr;
        std::vector<size_t>  _index;         // current index into grid
        size_t               _indexL;        // current index into grid
        size_t               _end_indexL;    // Last valid index
        size_t               _xb;            // x index within a block
        float *              _itr;
        InsideBox            _pred;
    };

    typedef Grid::ForwardIterator<Grid>       Iterator;
    typedef Grid::ForwardIterator<Grid const> ConstIterator;

    //! Construct a begin iterator that will iterate through elements
    //! inside or on the box defined by \p minu and \p maxu
    //
    Iterator begin(const std::vector<double> &minu, const std::vector<double> &maxu) { return (Iterator(this, true, minu, maxu)); }
    Iterator begin() { return (Iterator(this, true)); }

    Iterator end() { return (Iterator(this, false)); }

    ConstIterator cbegin(const std::vector<double> &minu, const std::vector<double> &maxu) const { return (ConstIterator(this, true, minu, maxu)); }
    ConstIterator cbegin() const { return (ConstIterator(this, true)); }

    ConstIterator cend() const { return (ConstIterator(this, false)); }

    template<typename T> static void CopyToArr3(const std::vector<T> &src, std::array<T, 3> &dst)
    {
        for (int i = 0; i < src.size() && i < dst.size(); i++) { dst[i] = src[i]; }
    }
    template<typename T> static void CopyToArr3(const T *src, size_t n, std::array<T, 3> &dst)
    {
        for (int i = 0; i < n && i < dst.size(); i++) { dst[i] = src[i]; }
    }
    template<typename T> static void CopyFromArr3(const std::array<T, 3> &src, std::vector<T> &dst)
    {
        dst.resize(src.size());
        for (int i = 0; i < src.size() && i < dst.size(); i++) { dst[i] = src[i]; }
    }
    template<typename T> static void CopyFromArr3(const std::array<T, 3> &src, T *dst)
    {
        for (int i = 0; i < src.size(); i++) { dst[i] = src[i]; }
    }

protected:
    virtual float GetValueNearestNeighbor(const DblArr3 &coords) const = 0;

    virtual float GetValueLinear(const DblArr3 &coords) const = 0;

    virtual void GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const = 0;

    virtual float *GetValuePtrAtIndex(const std::vector<float *> &blks, const Size_tArr3 &indices) const;

    virtual void ClampIndex(const std::vector<size_t> &dims, const Size_tArr3 indices, Size_tArr3 &cIndices) const
    {
        cIndices = {0, 0, 0};

        for (int i = 0; i < dims.size(); i++) {
            cIndices[i] = indices[i];
            if (cIndices[i] >= dims[i]) { cIndices[i] = dims[i] - 1; }
        }
    }

private:
    std::vector<size_t>  _dims;                   // dimensions of grid arrays
    Size_tArr3           _bs = {{1, 1, 1}};       // dimensions of each block
    Size_tArr3           _bdims = {{1, 1, 1}};    // dimensions (specified in blocks) of ROI
    std::vector<size_t>  _bsDeprecated;           // legacy API
    std::vector<size_t>  _bdimsDeprecated;        // legacy API
    std::vector<float *> _blks;
    std::vector<bool>    _periodic;    // periodicity of boundaries
    std::vector<size_t>  _minAbs;      // Offset to start of grid
    size_t               _topologyDimension = 0;
    float                _missingValue = std::numeric_limits<float>::infinity();
    bool                 _hasMissing = false;
    int                  _interpolationOrder = 0;    // Order of interpolation
    long                 _nodeIDOffset = 0;
    long                 _cellIDOffset = 0;
    mutable DblArr3      _minuCache = {{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()}};
    mutable DblArr3      _maxuCache = {{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()}};

    virtual void _getUserCoordinatesHelper(const std::vector<double> &coords, double &x, double &y, double &z) const;
};
};    // namespace VAPoR
#endif
