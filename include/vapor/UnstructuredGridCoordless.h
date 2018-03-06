#ifndef _UnstructuredGridCoordless_
#define _UnstructuredGridCoordless_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid.h>
#include <vapor/KDTreeRG.h>


#ifdef WIN32
#  pragma warning (disable: 4661 4251) // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGridCoordless
//! \brief class for a 2D unstructured grid.
//! \author John Clyne
//!
//
class VDF_API UnstructuredGridCoordless : public UnstructuredGrid {
public:


 //! Construct a unstructured grid sampling 2D scalar function
 //!
 //
 UnstructuredGridCoordless(
	const std::vector <size_t> &vertexDims,
	const std::vector <size_t> &faceDims,
	const std::vector <size_t> &edgeDims,
	const std::vector <size_t> &bs,
	const std::vector <float *> &blks,
	size_t topology_dimension,
	const int *vertexOnFace,
	const int *faceOnVertex,
	const int *faceOnFace,
	Location location,	// node,face, edge
	size_t maxVertexPerFace,
	size_t maxFacePerVertex
 ) : UnstructuredGrid(
		vertexDims, faceDims, edgeDims, bs, blks, topology_dimension,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	)
	{}

 UnstructuredGridCoordless() = default;
 virtual ~UnstructuredGridCoordless() = default;

 virtual void GetUserExtents(
	std::vector <double> &minu, std::vector <double> &maxu
 ) const override {
	minu.clear(); maxu.clear();
 }

 virtual void GetBoundingBox(
    const std::vector <size_t> &, const std::vector <size_t> &,
    std::vector <double> &minu, std::vector <double> &maxu
 ) const override {
	minu.clear(); maxu.clear();
 }

 void GetEnclosingRegion(
	const std::vector <double> &, const std::vector <double> &,
	std::vector <size_t> &min, std::vector <size_t> &max
 ) const override {
	min.clear(); max.clear();
 }

 void GetUserCoordinates(
	const std::vector <size_t> &,
	std::vector <double> &coords
 ) const override {
	coords.clear();
 }
 
  
void GetIndices(
	const std::vector <double> &,
	std::vector <size_t> &indices
 ) const override {
	for (int i=0; i<GetDimensions().size(); i++) {
		indices.push_back(0);
	}
 }

 bool GetIndicesCell(
	const std::vector <double> &,
	std::vector <size_t> &indices
 ) const override {
	return(false);
 }

 bool InsideGrid(const std::vector <double> &coords) const override {
	return(false);
 }

 float GetValueNearestNeighbor (
	const std::vector <double> &coords
 ) const override {
	return(0.0);
 }

 float GetValueLinear (
	const std::vector <double> &coords
 ) const override {
 	return(0.0);
 }

 virtual size_t GetGeometryDim() const override {
	return(0);
 }

 /////////////////////////////////////////////////////////////////////////////
 //
 // Iterators
 //
 /////////////////////////////////////////////////////////////////////////////

 class ConstCoordItrUCoordless : public Grid::ConstCoordItrAbstract {
 public:
  ConstCoordItrUCoordless(const UnstructuredGridCoordless *ug, bool begin) {}
  ConstCoordItrUCoordless(const ConstCoordItrUCoordless &rhs) {}
  

  ConstCoordItrUCoordless();
  virtual ~ConstCoordItrUCoordless() {}

  virtual void next() {}
  virtual void next(const long &offset) {}
  virtual ConstCoordType &deref() const {
	return(_coords);
  }
  virtual const void *address() const {return this; };

  virtual bool equal(const void* rhs) const {
	const ConstCoordItrUCoordless *itrptr = 
		static_cast<const ConstCoordItrUCoordless *> (rhs);

	return(true);
  }

  virtual std::unique_ptr<ConstCoordItrAbstract> clone() const {
	return std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrUCoordless(*this));
  };

 private:
	std::vector <double> _coords;
 };

 virtual ConstCoordItr ConstCoordBegin() const override {
	return ConstCoordItr (
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrUCoordless(this, true))
	);
 }
 virtual ConstCoordItr ConstCoordEnd() const override {
	return ConstCoordItr (
		std::unique_ptr<ConstCoordItrAbstract> (new ConstCoordItrUCoordless(this, false))
	);
 }


 
 VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGridCoordless &sg);

private:
};
};

#endif
