#include <vector>
#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>
#include <vapor/DC.h>
#include <vapor/MyBase.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/RegularGrid.h>
#include <vapor/StretchedGrid.h>
#include <vapor/UnstructuredGrid2D.h>
#include <vapor/UnstructuredGridLayered.h>
#include "vapor/unique_ptr_cache.hpp"

#ifndef	GRIDMGR_H
#define GRIDMGR_H

namespace VAPoR {

class VDF_API GridHelper : public Wasp::MyBase {

public:

 GridHelper(size_t max_size = 10) : _qtrCache(max_size) {}

 ~GridHelper();

 string GetGridType(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <vector <string>> &cdimnames
 ) const;

 bool IsUnstructured(std::string gridType) const;
 bool IsStructured(std::string gridType) const;

 //	var: variable info
 //  roi_dims: spatial dimensions of ROI
 //	dims: spatial dimensions of full variable domain in voxels
 //	blkvec: data blocks, and coordinate blocks
 //	bsvec: data block dimensions, and coordinate block dimensions
 //  bminvec: ROI offsets in blocks, full domain, data and coordinates
 //  bmaxvec: ROI offsets in blocks, full domain, data and coordinates
 //
 StructuredGrid *MakeGridStructured(
	string gridType,
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <size_t> &roi_dims, 
	const std::vector <size_t> &dims,
	const std::vector <float *> &blkvec,
	const std::vector < std::vector <size_t > > &bsvec,
	const std::vector < std::vector <size_t > > &bminvec,
	const std::vector < std::vector <size_t > > &bmaxvec
 ) ;

 UnstructuredGrid *MakeGridUnstructured(
	string gridType,
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <size_t> &roi_dims, 
	const std::vector <size_t> &dims,
	const std::vector <float *> &blkvec,
	const std::vector < std::vector <size_t > > &bsvec,
	const std::vector < std::vector <size_t > > &bminvec,
	const std::vector < std::vector <size_t > > &bmaxvec,
	const std::vector <int *> &conn_blkvec,
	const std::vector < std::vector <size_t > > &conn_bsvec,
	const std::vector < std::vector <size_t > > &conn_bminvec,
	const std::vector < std::vector <size_t > > &conn_bmaxvec,
	const std::vector <size_t> &vertexDims,
	const std::vector <size_t> &faceDims,
	const std::vector <size_t> &edgeDims,
    UnstructuredGrid::Location location,
    size_t maxVertexPerFace,
    size_t maxFacePerVertex,
	long vertexOffset, 
	long faceOffset
 ) ;

private:

    using cacheType = unique_ptr_cache< std::string, QuadTreeRectangle<float, size_t> >;
    cacheType   _qtrCache;


 RegularGrid *_make_grid_regular(
	const std::vector <size_t> &dims,
    const std::vector <float *> &blkvec,
	const std::vector <size_t> &bs,
	const std::vector <size_t> &bmin,
	const std::vector <size_t> &bmax

 ) const;

 StretchedGrid *_make_grid_stretched(
	const std::vector <size_t> &dims,
    const std::vector <float *> &blkvec,
	const std::vector <size_t> &bs,
	const std::vector <size_t> &bmin,
	const std::vector <size_t> &bmax
 ) const;

 LayeredGrid *_make_grid_layered(
	const std::vector <size_t> &dims,
    const std::vector <float *> &blkvec,
	const std::vector <size_t > &bs,
	const std::vector <size_t > &bmin,
	const std::vector <size_t > &bmax
 ) const;

 CurvilinearGrid *_make_grid_curvilinear(
	size_t ts,
	int level,
	int lod,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <size_t> &dims,
    const std::vector <float *> &blkvec,
	const std::vector <size_t> &bs,
	const std::vector <size_t> &bmin,
	const std::vector <size_t> &bmax
 ) ;



 UnstructuredGrid2D *_make_grid_unstructured2d(
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <size_t> &dims,
    const std::vector <float *> &blkvec,
	const std::vector <size_t> &bs,
	const std::vector <size_t> &bmin,
	const std::vector <size_t> &bmax,
    const std::vector <int *> &conn_blkvec,
	const std::vector <size_t> &conn_bs,
	const std::vector <size_t> &conn_bmin,
	const std::vector <size_t> &conn_bmax,
	const std::vector <size_t> &vertexDims,
	const std::vector <size_t> &faceDims,
	const std::vector <size_t> &edgeDims,
    UnstructuredGrid::Location location,
    size_t maxVertexPerFace,
    size_t maxFacePerVertex,
	long vertexOffset, 
	long faceOffset
 ) ;

UnstructuredGridLayered *_make_grid_unstructured_layered(
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &dims,
	const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax,
	const vector <int *> &conn_blkvec,
	const vector <size_t> &conn_bs,
	const vector <size_t> &conn_bmin,
	const vector <size_t> &conn_bmax,
	const vector <size_t> &vertexDims,
	const vector <size_t> &faceDims,
	const vector <size_t> &edgeDims,
	UnstructuredGrid::Location location,
	size_t maxVertexPerFace,
	size_t maxFacePerVertex,
	long vertexOffset,
	long faceOffset
);


 void _makeGridHelper(
    const DC::DataVar &var,
    const std::vector <size_t> &roi_dims,
    const std::vector <size_t> &dims,
	Grid *g
 ) const;



 bool _isUnstructured2D(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;

 bool _isUnstructuredLayered(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;

 bool _isRegular(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;


 bool _isStretched(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;

 bool _isLayered(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;

 bool _isCurvilinear(
	const DC::Mesh &m,
	const std::vector <DC::CoordVar> &cvarsinfo,
	const std::vector <std::vector <string>> &cdimnames
 ) const;

 string _getQuadTreeRectangleKey(
	size_t ts,
	int level,
	int lod,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax
 ) const;


};

};
#endif
