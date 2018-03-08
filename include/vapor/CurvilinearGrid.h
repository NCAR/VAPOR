#ifndef _CurvilinearGrid_
#define _CurvilinearGrid_
#include <vapor/common.h>
#include <vapor/Grid.h>
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
 //! \param[in] kdtree A KDTreeRG instance that contains a KD tree
 //! that may be used to find the nearest grid vertex to a given point
 //! expressed in user coordintes. The offsets returned by \p kdtree will
 //! be used as indeces into \p xrg and \p yrg.
 //!
 //!
 //! \sa RegularGrid()
 //
 CurvilinearGrid(
	const std::vector <size_t> &dims,
	const std::vector <size_t> &bs,
	const std::vector <float *> &blks,
	const RegularGrid &xrg,
	const RegularGrid &yrg,
	const std::vector <double> &zcoords,
	const KDTreeRG *kdtree
 );

 CurvilinearGrid() = default;
 virtual ~CurvilinearGrid() = default;

 virtual size_t GetGeometryDim() const override;


 // \copydoc GetGrid::GetUserExtents()
 //
 virtual void GetUserExtents(
    std::vector <double> &minu, std::vector <double> &maxu
 ) const override {
	if (! _minu.size()) {
		_GetUserExtents(_minu, _maxu);
	}
	minu = _minu;
	maxu = _maxu;
 }


 // \copydoc GetGrid::GetBoundingBox()
 //
 virtual void GetBoundingBox(
	const std::vector <size_t> &min, const std::vector <size_t> &max,
	std::vector <double> &minu, std::vector <double> &maxu
 ) const override;

 // \copydoc GetGrid::GetEnclosingRegion()
 //
 virtual void    GetEnclosingRegion(
	const std::vector <double> &minu, const std::vector <double> &maxu,
	std::vector <size_t> &min, std::vector <size_t> &max
 ) const override;


 // \copydoc GetGrid::GetUserCoordinates()
 //
 virtual void GetUserCoordinates(
	const std::vector <size_t> &indices,
	std::vector <double> &coords
 ) const override;

 // \copydoc GetGrid::GetIndices()
 //
 virtual void GetIndices(
	const std::vector <double> &coords,
	std::vector <size_t> &indices
 ) const override;

 //! \copydoc Grid::GetIndicesCell
 //!
 virtual bool GetIndicesCell(
	const std::vector <double> &coords,
	std::vector <size_t> &indices
 ) const override;

 // \copydoc GetGrid::InsideGrid()
 //
 virtual bool InsideGrid(const std::vector <double> &coords) const override;





 //! Returns reference to RegularGrid instance containing X user coordinates
 //!
 //! Returns reference to RegularGrid instance passed to constructor 
 //! containing X user coordinates
 //!
 const RegularGrid &GetXRG() const { return(_xrg); };

 //! Returns reference to RegularGrid instance containing Y user coordinates
 //!
 //! Returns reference to RegularGrid instance passed to constructor 
 //! containing Y user coordinates
 //!
 const RegularGrid &GetYRG() const { return(_yrg); };

 //! Returns reference to vector containing Z user coordinates
 //!
 //! Returns reference to vector passed to constructor 
 //! containing Z user coordinates
 //!
 const std::vector <double> &GetZCoords() const { return(_zcoords); };

 class ConstCoordItrCG : public Grid::ConstCoordItrAbstract {
 public:
  ConstCoordItrCG(const CurvilinearGrid *cg, bool begin);
  ConstCoordItrCG(const ConstCoordItrCG &rhs);
  

  ConstCoordItrCG();
  virtual ~ConstCoordItrCG() {}

  virtual void next();
  virtual void next(const long &offset);
  virtual ConstCoordType &deref() const {
	return(_coords);
  }
  virtual const void *address() const {return this; };

  virtual bool equal(const void* rhs) const {
	const ConstCoordItrCG *itrptr = 
		static_cast<const ConstCoordItrCG *> (rhs);

	return(_index == itrptr->_index);
  }

  virtual std::unique_ptr<ConstCoordItrAbstract> clone() const {
	return std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrCG(*this));
  };

 private:
	const CurvilinearGrid *_cg;
	std::vector <size_t> _index;
	std::vector <double> _coords;
	ConstIterator _xCoordItr;
	ConstIterator _yCoordItr;
 };

 virtual ConstCoordItr ConstCoordBegin() const override {
	return ConstCoordItr(
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrCG(this, true))
	);
 }
 virtual ConstCoordItr ConstCoordEnd() const override {
	return ConstCoordItr(
		std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrCG(this, false))
	);
 }

protected:
 virtual float GetValueNearestNeighbor(
	const std::vector <double> &coords
 ) const override;

 virtual float GetValueLinear(
	const std::vector <double> &coords
 ) const override;


private:
 std::vector <double> _zcoords;
 mutable std::vector <double> _minu;
 mutable std::vector <double> _maxu;
 const KDTreeRG *_kdtree;
 RegularGrid _xrg;
 RegularGrid _yrg;

 void _curvilinearGrid(
	const RegularGrid &xrg,
	const RegularGrid &yrg,
	const std::vector <double> &zcoords,
	const KDTreeRG *kdtree
 );

 void _GetUserExtents(
	std::vector <double> &minu, std::vector <double> &maxu
 ) const ;

 int _binarySearchRange(
	const std::vector <double> &sorted, double x, size_t &i
 ) const;

 bool _insideGrid(
	double x, double y, double z,
	size_t &i, size_t &j, size_t &k,
	double lambda[4], double zwgt[2]
 ) const;

 virtual void _getMinCellExtents(std::vector <double> &minCellExtents) const; 

};
};
#endif
