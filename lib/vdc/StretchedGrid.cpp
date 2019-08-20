#include <stdio.h>
#include <iostream>
#include "vapor/VAssert.h"
#include <cmath>
#include <cfloat>
#include <vapor/utils.h>
#include <vapor/StretchedGrid.h>
#include <vapor/KDTreeRG.h>
#include <vapor/vizutil.h>

using namespace std;
using namespace VAPoR;

void StretchedGrid::_stretchedGrid(
	const vector <double> &xcoords,
	const vector <double> &ycoords,
	const vector <double> &zcoords
) {
	VAssert(xcoords.size() != 0);
	VAssert(ycoords.size() != 0);

	_xcoords.clear();
	_ycoords.clear();
	_zcoords.clear();
	_minu.clear();
	_maxu.clear();
	_xcoords = xcoords;
	_ycoords = ycoords;
	_zcoords = zcoords;

	// Get the user extents now. Do this only once.
	//
	_GetUserExtents(_minu, _maxu);

}

StretchedGrid::StretchedGrid(
	const vector <size_t> &dims,
	const vector <size_t> &bs,
	const vector <float *> &blks,
	const vector <double> &xcoords,
	const vector <double> &ycoords,
	const vector <double> &zcoords
 ) : StructuredGrid(dims, bs, blks) {

	VAssert(bs.size() == dims.size());
	VAssert(bs.size() >= 1 && bs.size() <= 3); 

	_stretchedGrid(xcoords, ycoords, zcoords);
}

size_t StretchedGrid::GetGeometryDim() const {
	return(_zcoords.size() == 0 ? 2 : 3);
}

vector <size_t> StretchedGrid::GetCoordDimensions(size_t dim) const {
	if (dim == 0) {
		return(vector <size_t> (1,GetDimensions()[0]));
	} 
	else if (dim == 1) {
		return(vector <size_t> (1,GetDimensions()[1]));
	}
	else if (dim == 2) {
		if (GetDimensions().size() == 3) {
			return(vector <size_t> (1,GetDimensions()[2]));
		}
		else {
			return(vector <size_t> (1,1));
		}
	}
	else {
		return(vector <size_t> (1,1));
	}
}


void StretchedGrid::GetBoundingBox(
	const std::vector <size_t> &min, const std::vector <size_t> &max,
	std::vector <double> &minu, std::vector <double> &maxu
) const {

	vector <size_t> cMin = min;
	ClampIndex(cMin);

	vector <size_t> cMax = max;
	ClampIndex(cMax); 

	VAssert(cMin.size() == cMax.size());

	for (int i=0; i<cMin.size(); i++) {
		VAssert(cMin[i] <= cMax[i]);
	}

	minu.clear();
	maxu.clear();
	
	for (int i=0; i<cMin.size(); i++) {
		minu.push_back(0.0);
		maxu.push_back(0.0);
	}

	minu[0] = _xcoords[cMin[0]];
	maxu[0] = _xcoords[cMax[0]];
	minu[1] = _ycoords[cMin[1]];
	maxu[1] = _ycoords[cMax[1]];

	// We're done if 2D grid
	//
	if (GetGeometryDim() == 2) return;

	minu[2] = _zcoords[cMin[2]];
	maxu[2] = _zcoords[cMax[2]];
}


void StretchedGrid::GetEnclosingRegion(
	const std::vector <double> &minu, const std::vector <double> &maxu,
	std::vector <size_t> &min, std::vector <size_t> &max
) const {

	vector <double> cMinu = minu;
	ClampCoord(cMinu);

	vector <double> cMaxu = maxu;
	ClampCoord(cMaxu);

	VAssert(cMinu.size() == cMaxu.size());

	// Initialize voxels coords to full grid
	//
	vector <size_t> dims = GetDimensions();
	for (int i=0; i<dims.size(); i++) {
		min[i] = 0;
		max[i] = dims[i] - 1;
	}

	float xmin = cMinu[0];
	int imin = min[0];
	bool outside = true;
	for (int i=0; i<_xcoords.size() && outside; i++) {
		if (_xcoords[i] < xmin) outside = false;
		if (outside) imin = i;
	}
	min[0] = imin;

	float xmax = cMaxu[0];
	int imax = max[0];
	outside = true;
	for (int i=_xcoords.size()-1; i>=min[0] && outside; i--) {
		if (_xcoords[i] > xmax) outside = false;
		if (outside) imax = i;
	}
	max[0] = imax;

	float ymin = cMinu[1];
	int jmin = min[1];
	outside = true;
	for (int j=0; j<_ycoords.size() && outside; j++) {
		if (_ycoords[j] < ymin) outside = false;
		if (outside) jmin = j;
	}
	min[1] = jmin;

	float ymax = cMaxu[1];
	int jmax = max[1];
	outside = true;
	for (int j=_ycoords.size()-1; j>=min[1] && outside; j--) {
		if (_ycoords[j] > ymax) outside = false;
		if (outside) jmax = j;
	}
	max[1] = jmax;


	if (dims.size() < 3) return;	// 2D => we're done.

	// Finally, get Z
	//
	float zmin = cMinu[2];
	int kmin = min[2];
	outside = true;
	for (int k=0; k<_zcoords.size() && outside; k++) {
		if (_zcoords[k] < zmin) outside = false;
		if (outside) kmin = k;
	}
	min[2] = kmin;

	float zmax = cMaxu[2];
	int kmax = max[2];
	outside = true;
	for (int k=_zcoords.size()-1; k>=min[2] && outside; k--) {
		if (_zcoords[k] > zmax) outside = false;
		if (outside) kmax = k;
	}
	max[2] = kmax;
}


void StretchedGrid::GetUserCoordinates(
	const size_t indices[],
	double coords[]
) const {

    size_t cIndices[3];
    ClampIndex(indices, cIndices);

	vector <size_t> dims = StructuredGrid::GetDimensions();

	coords[0] = _xcoords[cIndices[0]];
	coords[1] = _ycoords[cIndices[1]];

	if (GetGeometryDim() > 2) {
		coords[2] = _zcoords[cIndices[2]];
	}
}

void StretchedGrid::GetIndices(
	const std::vector <double> &coords,
	std::vector <size_t> &indices
) const {
	indices.clear();

	// Clamp coordinates on periodic boundaries to grid extents
	//
	vector <double> cCoords = coords;
	ClampCoord(cCoords);

	size_t i;
	int rc = Wasp::BinarySearchRange(_xcoords, cCoords[0], i);
	if (rc < 0) {
		indices.push_back(0);
	}
	else if (rc > 0) {
		indices.push_back(i);
	}
	else {
		indices.push_back(GetDimensions()[0] - 1);
	}

	size_t j;
	rc = Wasp::BinarySearchRange(_ycoords, cCoords[1], j);
	if (rc < 0) {
		indices.push_back(0);
	}
	else if (rc > 0) {
		indices.push_back(j);
	}
	else {
		indices.push_back(GetDimensions()[1] - 1);
	}

	if (cCoords.size() == 2) return;

	size_t k;
	rc = Wasp::BinarySearchRange(_zcoords, cCoords[2], k);
	if (rc < 0) {
		indices.push_back(0);
	}
	else if (rc > 0) {
		indices.push_back(k);
	}
	else {
		indices.push_back(GetDimensions()[2] - 1);
	}
}


bool StretchedGrid::GetIndicesCell(
	const std::vector <double> &coords,
	std::vector <size_t> &indices
) const {

	// Clamp coordinates on periodic boundaries to grid extents
	//
	vector <double> cCoords = coords;
	ClampCoord(cCoords);
	
	double x = cCoords[0];
	double y = cCoords[1];
	double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
	
	double xwgt[2], ywgt[2], zwgt[2];
	size_t i, j, k;
	bool inside = _insideGrid(x,y,z,i,j,k, xwgt, ywgt, zwgt);

	if (! inside) return (false);

	indices.push_back(i);
	indices.push_back(j);

	if (GetGeometryDim() == 2) return(true);

	indices.push_back(k);

	return(true);
}
	


bool StretchedGrid::InsideGrid(const std::vector <double> &coords) const {

	// Clamp coordinates on periodic boundaries to reside within the 
	// grid extents 
	//
	vector <double> cCoords = coords;
	ClampCoord(cCoords);

	// Do a quick check to see if the point is completely outside of 
	// the grid bounds.
	//
	for (int i=0; i<cCoords.size(); i++) {
		if (cCoords[i] < _minu[i] || cCoords[i] > _maxu[i]) return (false);
	}

	double xwgt[2], ywgt[2], zwgt[2];
	size_t i,j,k;	// not used
	double x = cCoords[0];
	double y = cCoords[1];
	double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
	
	bool inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

	return(inside);
}


StretchedGrid::ConstCoordItrSG::ConstCoordItrSG(
	const StretchedGrid *sg, bool begin
) : ConstCoordItrAbstract() {
	_sg = sg;
	vector <size_t> dims = _sg->GetDimensions();

	_index = vector<size_t> (dims.size(), 0);
	if (! begin) {
		_index[dims.size()-1] = dims[dims.size()-1];
	}

	_coords.push_back(_sg->_xcoords[0]);
	_coords.push_back(_sg->_ycoords[0]);
	if (dims.size() == 3) {
		_coords.push_back(_sg->_zcoords[0]);
	}
}


StretchedGrid::ConstCoordItrSG::ConstCoordItrSG(
	const ConstCoordItrSG &rhs
) : ConstCoordItrAbstract() {
	_sg = rhs._sg;
	_index = rhs._index;
	_coords = rhs._coords;
}

StretchedGrid::ConstCoordItrSG::ConstCoordItrSG() : ConstCoordItrAbstract() {
	_sg = NULL;
	_index.clear();
	_coords.clear();
}


void StretchedGrid::ConstCoordItrSG::next() {

	const vector <size_t> &dims = _sg->GetDimensions();

	_index[0]++;

	if (_index[0] < dims[0]) {
		_coords[0] = _sg->_xcoords[_index[0]];
		_coords[1] = _sg->_ycoords[_index[1]];
		return;
	}

	_index[0] = 0;
	_index[1]++;

	if (_index[1] < dims[1]) {
		_coords[0] = _sg->_xcoords[_index[0]];
		_coords[1] = _sg->_ycoords[_index[1]];
		return;
	}

	if (dims.size() == 2) return;


	_index[1] = 0;
	_index[2]++;
	if (_index[2] < dims[2]) {
		_coords[0] = _sg->_xcoords[_index[0]];
		_coords[1] = _sg->_ycoords[_index[1]];
		_coords[2] = _sg->_zcoords[_index[2]];
		return;
	}
}

void StretchedGrid::ConstCoordItrSG::next(const long &offset) {

	const vector <size_t> &dims = _sg->GetDimensions();

	if (! _index.size()) return;

	vector <size_t> maxIndex; ;
	for (int i=0; i<dims.size(); i++) maxIndex.push_back(dims[i]-1);

	long maxIndexL = Wasp::LinearizeCoords(maxIndex, dims);
	long newIndexL = Wasp::LinearizeCoords(_index, dims) + offset;
	if (newIndexL < 0) {
		newIndexL = 0;
	}
	if (newIndexL > maxIndexL) {
		_index = vector<size_t> (dims.size(), 0);
		_index[dims.size()-1] = dims[dims.size()-1];
		return;
	}

	_index = Wasp::VectorizeCoords(newIndexL, dims);

	_coords[0] = _sg->_xcoords[_index[0]];
	_coords[1] = _sg->_ycoords[_index[1]];

	if (dims.size() == 2) return;

	_coords[2] = _sg->_zcoords[_index[2]];
}



float StretchedGrid::GetValueNearestNeighbor(
	const std::vector <double> &coords
) const {

	// Clamp coordinates on periodic boundaries to grid extents
	//
	vector <double> cCoords = coords;
	ClampCoord(cCoords);

	double xwgt[2], ywgt[2], zwgt[2];
	size_t i,j,k;
	double x = cCoords[0];
	double y = cCoords[1];
	double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
	bool inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

	if (! inside) return(GetMissingValue());

	return(AccessIJK(i,j,k));
}

float StretchedGrid::GetValueLinear(
	const std::vector <double> &coords
) const {

	// Clamp coordinates on periodic boundaries to grid extents
	//
	vector <double> cCoords = coords;
	ClampCoord(cCoords);

	// handlese case where grid is 2D. I.e. if 2d then zwgt[0] == 1 && 
	// zwgt[1] = 0.0
	//
	double xwgt[2], ywgt[2], zwgt[2];
	size_t i,j,k;
	double x = cCoords[0];
	double y = cCoords[1];
	double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
	bool inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);


	if (! inside) return(GetMissingValue());

	vector <size_t> dims = GetDimensions();
	VAssert(i<dims[0]-1);
	VAssert(j<dims[1]-1);
	if (dims.size() > 2) VAssert(k<dims[2]-1);

	float v0 = 
		((AccessIJK(i,j,k)*xwgt[0] + AccessIJK(i+1,j,k)*xwgt[1]) * ywgt[0]) +
		((AccessIJK(i,j+1,k)*xwgt[0] + AccessIJK(i+1,j+1,k)*xwgt[1]) * ywgt[1]);


	if (GetGeometryDim() == 2) return(v0);

	k++;
	float v1 = 
		((AccessIJK(i,j,k)*xwgt[0] + AccessIJK(i+1,j,k)*xwgt[1]) * ywgt[0]) +
		((AccessIJK(i,j+1,k)*xwgt[0] + AccessIJK(i+1,j+1,k)*xwgt[1]) * ywgt[1]);

	// Linearly interpolate along Z axis
	//
	return(v0*zwgt[0] + v1*zwgt[1]);

}

void StretchedGrid::_GetUserExtents(
	vector <double> &minext, vector <double> &maxext
) const {

	vector <size_t> dims = StructuredGrid::GetDimensions();

	vector <size_t> min, max;
	for (int i=0; i<dims.size(); i++) {
		min.push_back(0);
		max.push_back(dims[i]-1);
	}

	StretchedGrid::GetBoundingBox(min, max, minext, maxext);
}


// Search for a point inside the grid. If the point is inside return true, 
// and provide the weights/coordinates for the point within 
// the XYZ cell containing the point 
// If the point is outside of the
// grid the values of 'xwgt', 'ywgt', and 'zwgt' are not defined
//
bool StretchedGrid::_insideGrid(
	double x, double y, double z,
	size_t &i, size_t &j, size_t &k,
	double xwgt[2], double ywgt[2], double zwgt[2]
) const {
	for (int l=0; l<2; l++) {
		xwgt[l] = 0.0;
		ywgt[l] = 0.0;
		zwgt[l] = 0.0;
	}
	i = j = k = 0;

	int rc  = Wasp::BinarySearchRange(_xcoords, x, i);

	if (rc != 0) return(false);

	xwgt[0] = 1.0 - (x - _xcoords[i]) / (_xcoords[i+1] - _xcoords[i]);
	xwgt[1] = 1.0 - xwgt[0];

	rc  = Wasp::BinarySearchRange(_ycoords, y, j);

	if (rc != 0) return(false);

	ywgt[0] = 1.0 - (y - _ycoords[j]) / (_ycoords[j+1] - _ycoords[j]);
	ywgt[1] = 1.0 - ywgt[0];

	if (GetGeometryDim() == 2) {
		zwgt[0] = 1.0;
		zwgt[1] = 0.0;
		return(true);
	}

	// Now verify that Z coordinate of point is in grid, and find
	// its interpolation weights if so.
	//
	rc  = Wasp::BinarySearchRange(_zcoords, z, k);

	if (rc != 0) return(false);

	zwgt[0] = 1.0 - (z - _zcoords[k]) / (_zcoords[k+1] - _zcoords[k]);
	zwgt[1] = 1.0 - zwgt[0];

	return(true);
}

void StretchedGrid::_getMinCellExtents(
	vector <double> &minCellExtents
) const {

	minCellExtents.clear();

	vector <size_t> dims = StructuredGrid::GetDimensions();

	// Find minimum cell extents along X
	//
	float minx = _xcoords[1] - _xcoords[0];
	float x0 = _xcoords[0];
	for (int i=1; i<dims[0]; i++)  {
		float x1 = _xcoords[i];

		if ((x1-x0) < minx) minx = x1-x0;

		x1 = x0;
	}
	minCellExtents.push_back(minx);

	// Find minimum cell extents along Y
	//
	float miny = _ycoords[1] - _ycoords[0];
	float y0 = _ycoords[0];
	for (int j=1; j<dims[1]; j++)  {
		float y1 = _ycoords[j];

		if ((y1-y0) < miny) miny = y1-y0;

		y1 = y0;
	}
	minCellExtents.push_back(miny);

	if (dims.size() < 3 || dims[2] < 2) return;

	// Find minimum cell extents along Z
	//
	float minz = _zcoords[1] - _zcoords[0];
	float z0 = _zcoords[0];
	for (int k=1; k<dims[2]; k++)  {
		float z1 = _zcoords[k];

		if ((z1-z0) < minz) minz = z1-z0;

		z1 = z0;
	}
	minCellExtents.push_back(minz);
}
