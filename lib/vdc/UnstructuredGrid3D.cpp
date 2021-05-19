#include <iostream>
#include <vector>
#include "vapor/VAssert.h"
#include <cmath>
#include <time.h>
#ifdef  Darwin
#include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
#include "windows.h"
#include "Winbase.h"
#include <limits>
#endif

#include <vapor/utils.h>
#include <vapor/vizutil.h>
#include <vapor/UnstructuredGrid3D.h>

using namespace std;
using namespace VAPoR;

UnstructuredGrid3D::UnstructuredGrid3D(
	const std::vector <size_t> &vertexDims,
	const std::vector <size_t> &faceDims,
	const std::vector <size_t> &edgeDims,
	const std::vector <size_t> &bs,
	const std::vector <float *> &blks,
	const int *vertexOnFace,
	const int *faceOnVertex,
	const int *faceOnFace,
	Location location,  // node,face, edge
	size_t maxVertexPerFace,
	size_t maxFacePerVertex,
	long nodeOffset,
	long cellOffset,
    const UnstructuredGridCoordless &xug,
    const UnstructuredGridCoordless &yug,
    const UnstructuredGridCoordless &zug,
	std::shared_ptr <const QuadTreeRectangle<float, size_t> >qtr
) : UnstructuredGrid(
        vertexDims, faceDims, edgeDims, bs, blks, 3,
        vertexOnFace, faceOnVertex, faceOnFace, location,
        maxVertexPerFace, maxFacePerVertex,
		nodeOffset, cellOffset
    ), 
	_xug(xug),
    _yug(yug),
	_zug(zug) 
{

	VAssert(xug.GetDimensions().size() == 1);
	VAssert(yug.GetDimensions().size() == 1);
	VAssert(zug.GetDimensions().size() == 1);

	VAssert(location == NODE);
}


vector <size_t> UnstructuredGrid3D::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return _xug.GetDimensions();
    }
    else if (dim == 1) {
        return _yug.GetDimensions();
    }
    else if (dim == 2) {
		return _zug.GetDimensions();
    }
	else {
        return(vector <size_t> (1,1));
    }
}


size_t UnstructuredGrid3D::GetGeometryDim() const
{
	return(3);
}


void UnstructuredGrid3D::GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const
{
	float range[2];
    
    _xug.GetRange(range);
    minu[0] = range[0];
    maxu[0] = range[1];
    
    _yug.GetRange(range);
    minu[1] = range[0];
    maxu[1] = range[1];
    
	_zug.GetRange(range);
	minu[2] = range[0];
	maxu[2] = range[1];
}


void UnstructuredGrid3D::GetBoundingBox(
	const Size_tArr3 &min, const Size_tArr3 &max,
	DblArr3 &minu, DblArr3 &maxu) const
{
	Size_tArr3 cMin;
	ClampIndex(min, cMin);

	Size_tArr3 cMax;
	ClampIndex(max, cMax);

	float range[2];
    
    _xug.GetRange(min, max, range);
    minu[0] = range[0];
    maxu[0] = range[1];
    
    _yug.GetRange(min, max, range);
    minu[1] = range[0];
    maxu[1] = range[1];
    
	_zug.GetRange(min, max, range);
	minu[2] = range[0];
	maxu[2] = range[1];
    
//    printf("UnstructuredGrid3D::%s(-, -)\n", __func__);
//    printf("\t min = [%f, %f, %f]\n", minu[0], minu[1], minu[2]);
//    printf("\t max = [%f, %f, %f]\n", maxu[0], maxu[1], maxu[2]);
}


bool UnstructuredGrid3D::GetEnclosingRegion(
	const DblArr3 &minu, const DblArr3 &maxu,
	Size_tArr3 &min, Size_tArr3 &max) const
{
	VAssert(0 && "Not implemented");
	return(true);
}


void UnstructuredGrid3D::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xug.GetValueAtIndex(cIndices);
    coords[1] = _yug.GetValueAtIndex(cIndices);
	coords[2] = _zug.GetValueAtIndex(cIndices);
}


bool UnstructuredGrid3D::GetIndicesCell(
    const DblArr3 &coords,
    Size_tArr3 &indices) const
{
    vector <size_t> nodes2D;
    vector <double> lambda;
    float zwgt[2];

    return(_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}


bool UnstructuredGrid3D::InsideGrid(const DblArr3 &coords) const
{
    Size_tArr3 indices;
    std::vector <size_t> nodes2D;
    vector <double> lambda;
    float zwgt[2];

    return(_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}


bool UnstructuredGrid3D::_insideGrid(
	const DblArr3 &coords,
	Size_tArr3 &cindices,
    std::vector <size_t> &nodes2D,
    std::vector <double> &lambda,
	float zwgt[2]) const
{
	VAssert (_location == NODE);
    VAssert(!"Not implemented");
    return false;

	nodes2D.clear();
	lambda.clear();

	DblArr3 cCoords;
	ClampCoord(coords, cCoords);

	// Find the 2D horizontal cell containing the X,Y coordinates
	//
    std::vector <std::vector <size_t> > nodes;

    bool status = false;
//	status = _ug2d.GetIndicesCell(cCoords, cindices, nodes, lambda);
	if (! status) return (status);

	VAssert(lambda.size() == nodes.size());
	for (int i=0; i<nodes.size(); i++) {
		nodes2D.push_back(nodes[i][0]);
	}

	// Find k index of cell containing z
	//
	vector <double> zcoords;

	size_t nz = GetDimensions()[1];
	for (int kk=0; kk<nz; kk++) {
		float z = 0.0;
		for (int i=0; i<lambda.size(); i++) {
			z += _zug.AccessIJK(nodes2D[i],kk) * lambda[i];
		}
			
		zcoords.push_back(z);
	}

	size_t k;
	if (! Wasp::BinarySearchRange(zcoords, cCoords[2], k)) return(false);

	VAssert(k>=0 && k<nz);
	cindices[1] = k;

	float z = cCoords[2];
    zwgt[0] = 1.0 - (z - zcoords[k]) / (zcoords[k+1] - zcoords[k]);
    zwgt[1] = 1.0 - zwgt[0];

	return(true);
}


float UnstructuredGrid3D::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    VAssert(!"Not implemented");
    return NAN;
}


float UnstructuredGrid3D::GetValueLinear(const DblArr3 &coords) const
{

	VAssert(!"Not implemented");
    return NAN;
}



/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////



UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D(
	const UnstructuredGrid3D *ug, bool begin
) : ConstCoordItrAbstract()
{
	_ug = ug;
	_coords = vector <double> (3, 0.0);
	if (begin) {
        _xCoordItr = ug->_xug.cbegin();
        _yCoordItr = ug->_yug.cbegin();
		_zCoordItr = ug->_zug.cbegin();
		_coords[0] = *_xCoordItr;
		_coords[1] = *_yCoordItr;
		_coords[2] = *_zCoordItr;
	}
	else {
        _xCoordItr = ug->_xug.cend();
        _yCoordItr = ug->_yug.cend();
		_zCoordItr = ug->_zug.cend();
	}

}


UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D(const ConstCoordItrU3D &rhs)
: ConstCoordItrAbstract()
{
	_ug = rhs._ug;
    _xCoordItr = rhs._xCoordItr;
    _yCoordItr = rhs._yCoordItr;
	_zCoordItr = rhs._zCoordItr;
	_coords = rhs._coords;
}


UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D() :
ConstCoordItrAbstract() {}


void UnstructuredGrid3D::ConstCoordItrU3D::next()
{
    ++_xCoordItr;
    ++_yCoordItr;
    ++_zCoordItr;
    
	_coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;
    _coords[2] = *_zCoordItr;
}


void UnstructuredGrid3D::ConstCoordItrU3D::next(const long &offset)
{
	VAssert(offset >= 0);

    _xCoordItr += offset;
    _yCoordItr += offset;
	_zCoordItr += offset;
    
	_coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;
    _coords[2] = *_zCoordItr;
}


namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGrid3D &ug)
{
	o << "UnstructuredGrid3D " << endl;
	o << (const Grid &) ug;

	return o;
}
};
