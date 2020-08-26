#ifndef _StretchedGrid_
#define _StretchedGrid_
#include <vapor/common.h>
#include <vapor/Grid.h>
#include <vapor/StructuredGrid.h>


namespace VAPoR {
//! \class StretchedGrid
//!
//! \brief This class implements a 2D or 3D stretched grid.
//!
//! This class implements a 2D or 3D stretched grid: a 
//! specialization of StructuredGrid class where cells are 
//! quadrilaterals (2D), or cuboids (3D). Hence, stretched grids are 
//! topologically, but the location of each grid point is expressed
//! by functions:
//!
//! \code
//! x = X(i)
//! y = Y(j)
//! z = Z(k)
//! \endcode
//!
//!
//
class VDF_API StretchedGrid : public StructuredGrid {
public:

 //! \copydoc StructuredGrid::StructuredGrid()
 //!
 //! Construct a regular grid sampling a 3D or 2D scalar function.
 //!
 //! This constructor instantiates a stretched grid  where the x,y,z
 //! user coordinates are expressed as follows:
 //!
 //! \code
 //! x = X(i)
 //! y = Y(j)
 //! z = Z(k)
 //! \endcode
 //!
 //! The X, Y, and Z user coordinates are specified with \p xcoords, \p xcoords,
 //! and \p zcoords (if 3D), respectively.
 //!
 //! Adds new parameters:
 //!
 //! \param[in] xcoords  A 1D vector whose size matches that of the I
 //! dimension of this class, and whose values specify the X user coordinates.
 //! \param[in] ycoords  A 1D vector whose size matches that of the J
 //! dimension of this class, and whose values specify the Y user coordinates.
 //! \param[in] zcoords  A 1D vector whose size matches that of the K
 //! dimension of this class, and whose values specify the Z user coordinates.
 //!
 //! \sa RegularGrid()
 //
 StretchedGrid(
	const std::vector <size_t> &dims,
	const std::vector <size_t> &bs,
	const std::vector <float *> &blks,
	const std::vector <double> &xcoords,
	const std::vector <double> &ycoords,
	const std::vector <double> &zcoords
 );

 StretchedGrid() = default;
 virtual ~StretchedGrid() = default;

 virtual size_t GetGeometryDim() const override;

 virtual std::vector <size_t> GetCoordDimensions(size_t dim) const override;

 static std::string GetClassType() {
	return("Stretched");
 }  
 std::string GetType() const override {return (GetClassType()); }


 // \copydoc GetGrid::GetBoundingBox()
 //
 virtual void GetBoundingBox(
	const Size_tArr3 &min, const Size_tArr3 &max,
	DblArr3 &minu, DblArr3 &maxu
 ) const override;

 // \copydoc GetGrid::GetUserCoordinates()
 //
 virtual void GetUserCoordinates(
	const Size_tArr3 &indices,
	DblArr3 &coords
 ) const override;

 //! \copydoc Grid::GetIndicesCell
 //!
 virtual bool GetIndicesCell(
	const DblArr3 &coords,
	Size_tArr3 &indices
 ) const override;

 // \copydoc GetGrid::InsideGrid()
 //
 virtual bool InsideGrid(const DblArr3 &coords) const override;

 //! Returns reference to vector containing X user coordinates
 //!
 //! Returns reference to vector passed to constructor 
 //! containing X user coordinates
 //!
 const std::vector <double> &GetXCoords() const { return(_xcoords); };

 //! Returns reference to vector containing Y user coordinates
 //!
 //! Returns reference to vector passed to constructor 
 //! containing Y user coordinates
 //!
 const std::vector <double> &GetYCoords() const { return(_ycoords); };

 //! Returns reference to vector containing Z user coordinates
 //!
 //! Returns reference to vector passed to constructor 
 //! containing Z user coordinates
 //!
 const std::vector <double> &GetZCoords() const { return(_zcoords); };

 class ConstCoordItrSG : public Grid::ConstCoordItrAbstract {
 public:
  ConstCoordItrSG(const StretchedGrid *cg, bool begin);
  ConstCoordItrSG(const ConstCoordItrSG &rhs);
  

  ConstCoordItrSG();
  virtual ~ConstCoordItrSG() {}

  virtual void next();
  virtual void next(const long &offset);
  virtual ConstCoordType &deref() const {
	return(_coords);
  }
  virtual const void *address() const {return this; };

  virtual bool equal(const void* rhs) const {
	const ConstCoordItrSG *itrptr = 
		static_cast<const ConstCoordItrSG *> (rhs);

	return(_index == itrptr->_index);
  }

  virtual std::unique_ptr<ConstCoordItrAbstract> clone() const {
	return std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrSG(*this));
  };

 private:
	const StretchedGrid *_sg;
	std::vector <size_t> _index;
	std::vector <double> _coords;
 };

 virtual ConstCoordItr ConstCoordBegin() const override {
	return ConstCoordItr(
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrSG(this, true))
	);
 }
 virtual ConstCoordItr ConstCoordEnd() const override {
	return ConstCoordItr(
		std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrSG(this, false))
	);
 }

protected:
 virtual float GetValueNearestNeighbor(
	const DblArr3 &coords
 ) const override;

 virtual float GetValueLinear(
	const DblArr3 &coords
 ) const override;

 void GetUserExtentsHelper(
	DblArr3 &minu, DblArr3 &maxu
 ) const override;


private:
 std::vector <double> _xcoords;
 std::vector <double> _ycoords;
 std::vector <double> _zcoords;
 DblArr3 _minu = {{0.0, 0.0, 0.0}};
 DblArr3 _maxu = {{0.0, 0.0, 0.0}};

 void _stretchedGrid(
	const std::vector <double> &xcoords,
	const std::vector <double> &ycoords,
	const std::vector <double> &zcoords
 );


 bool _insideGrid(
	double x, double y, double z,
	size_t &i, size_t &j, size_t &k,
	double xwgt[2], double ywgt[2], double zwgt[2]
 ) const;


};
};
#endif
