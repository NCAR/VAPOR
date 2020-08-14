#ifndef _UnstructuredGridLayered_
#define _UnstructuredGridLayered_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid2D.h>
#include <vapor/QuadTreeRectangle.hpp>


#ifdef WIN32
#  pragma warning (disable: 4661 4251) // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGridLayered
//! \brief class for a Layered unstructured grid.
//! \author John Clyne
//!
//
class VDF_API UnstructuredGridLayered : public UnstructuredGrid {
public:


 //! Construct a unstructured grid sampling Layered scalar function
 //!
 //
 UnstructuredGridLayered(
	const std::vector <size_t> &vertexDims,
	const std::vector <size_t> &faceDims,
	const std::vector <size_t> &edgeDims,
	const std::vector <size_t> &bs,
	const std::vector <float *> &blks,
	const int *vertexOnFace,
	const int *faceOnVertex,
	const int *faceOnFace,
	Location location,	// node,face, edge
	size_t maxVertexPerFace,
	size_t maxFacePerVertex,
	long nodeOffset,
	long cellOffset,
	const UnstructuredGridCoordless &xug,
	const UnstructuredGridCoordless &yug,
	const UnstructuredGridCoordless &zug,
	std::shared_ptr<const QuadTreeRectangle<float, size_t> >qtr
 );

 UnstructuredGridLayered() = default;
 virtual ~UnstructuredGridLayered() = default;

 std::shared_ptr<const QuadTreeRectangle<float, size_t> >GetQuadTreeRectangle() const {
	return(_ug2d.GetQuadTreeRectangle());
 }

 virtual std::vector <size_t> GetCoordDimensions(size_t dim) const override; 

 virtual size_t GetGeometryDim() const override;

 static std::string GetClassType() {
	return("UnstructuredLayered");
 }  
 std::string GetType() const override {return (GetClassType()); }


 virtual void GetBoundingBox(
	const Size_tArr3 &min, const Size_tArr3 &max,
	DblArr3 &minu, DblArr3 &maxu
 ) const override;

 bool GetEnclosingRegion(
	const DblArr3 &minu, const DblArr3 &maxu,
	Size_tArr3 &min, Size_tArr3 &max
 ) const override;

 virtual void GetUserCoordinates(
	const Size_tArr3 &indices,
	DblArr3 &coords
 ) const override;
  
 bool GetIndicesCell(
	const DblArr3 &coords,
	Size_tArr3 &indices
 ) const override;

 bool InsideGrid(const DblArr3 &coords) const override;

 float GetValueNearestNeighbor (
	const DblArr3 &coords
 ) const override;

 float GetValueLinear (
	const DblArr3 &coords
 ) const override;


 /////////////////////////////////////////////////////////////////////////////
 //
 // Iterators
 //
 /////////////////////////////////////////////////////////////////////////////

 class ConstCoordItrULayered : public Grid::ConstCoordItrAbstract {
 public:
  ConstCoordItrULayered(const UnstructuredGridLayered *ug, bool begin);
  ConstCoordItrULayered(const ConstCoordItrULayered &rhs);
  

  ConstCoordItrULayered();
  virtual ~ConstCoordItrULayered() {}

  virtual void next();
  virtual void next(const long &offset);
  virtual ConstCoordType &deref() const {
	return(_coords);
  }
  virtual const void *address() const {return this; };

  virtual bool equal(const void* rhs) const {
	const ConstCoordItrULayered *itrptr = 
		static_cast<const ConstCoordItrULayered *> (rhs);

	return(
		_itr2D == itrptr->_itr2D && 
		_zCoordItr == itrptr->_zCoordItr
	);
  }

  virtual std::unique_ptr<ConstCoordItrAbstract> clone() const {
	return std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrULayered(*this));
  };

 private:
	const UnstructuredGridLayered *_ug;
	UnstructuredGrid2D::ConstCoordItr _itr2D;
	ConstIterator _zCoordItr;
	std::vector <double> _coords;
	size_t _nElements2D;
	size_t _index2D;
 };

 virtual ConstCoordItr ConstCoordBegin() const override {
	return ConstCoordItr (
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrULayered(this, true))
	);
 }
 virtual ConstCoordItr ConstCoordEnd() const override {
	return ConstCoordItr (
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrULayered(this, false))
	);
 }


 
 VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGridLayered &sg);

protected:

 virtual void GetUserExtentsHelper(
	DblArr3 &minu, DblArr3 &maxu
 ) const override;

private:
 UnstructuredGrid2D _ug2d;
 UnstructuredGridCoordless _zug;

 bool _insideGrid(
	const DblArr3 &coords,
	Size_tArr3 &cindices,
	std::vector <size_t> &nodes2D,
	std::vector <double> &lambda,
	float zwgt[2]
 ) const;

};
};

#endif
