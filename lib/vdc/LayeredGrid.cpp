#include <stdio.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cfloat>
#include "vapor/LayeredGrid.h"

using namespace std;
using namespace VAPoR;

void LayeredGrid::_layeredGrid(
	const vector <bool> &periodic,
    const vector <double> &minu,
    const vector <double> &maxu,
	const RegularGrid &rg
) {
	assert(periodic.size() == _ndim);
	assert(minu.size() == maxu.size());
	assert(minu.size() == 2);

	_rg = rg;

	//
	// Periodic, varying dimensions are not supported
	//
	if (_ndim == 3 && periodic[2]) SetPeriodic(periodic);

	//
	// Set 2D horizontal extents in base class.
	//
	vector <double> my_minu = minu;
	my_minu.push_back(0.0);

	vector <double> my_maxu = maxu;
	my_maxu.push_back(1.0);
	RegularGrid::_SetExtents(my_minu, my_maxu);

	// Now get the extents of the varying dimension
	//
	_GetUserExtents(_minu, _maxu);

}

LayeredGrid::LayeredGrid(
	const vector <size_t> &bs,
	const vector <size_t> &min,
	const vector <size_t> &max,
	const vector <double> &minu,
	const vector <double> &maxu,
	const vector <bool> &periodic,
	const vector <float *> &blks,
	const RegularGrid &rg
 ) : RegularGrid(bs,min,max,periodic,blks) {

	_layeredGrid(periodic, minu, maxu, rg);
}

#ifdef	DEAD
LayeredGrid::LayeredGrid(
	const size_t bs[3],
	const size_t min[3],
	const size_t max[3],
	const double extents[6],
	const bool periodic[3],
	const vector <float *> &blks,
	const RegularGrid &rg
 ) : RegularGrid(bs,min,max,periodic,blks) {

	vector <bool> periodic_v;
	vector <double> minu;
	vector <double> maxu;

	// Extents contains horizontal 
	for (int i=0; i<2; i++) {
		minu.push_back(extents[i]);
		maxu.push_back(extents[i+3]);
	}

	for (int i=0; i<_ndim; i++) {
		periodic_v.push_back(periodic[i]);
	}

	_layeredGrid(periodic_v, minu, maxu, rg);
}
#endif

LayeredGrid::LayeredGrid(
	const vector <size_t> &bs,
	const vector <size_t> &min,
	const vector <size_t> &max,
	const vector <double> &minu,
	const vector <double> &maxu,
	const vector <bool> &periodic,
	const vector <float *> &blks,
	const RegularGrid &rg,
	float missing_value
 ) : RegularGrid(bs,min,max,periodic,blks, missing_value) {

	_layeredGrid(periodic, minu, maxu, rg);
}

#ifdef	DEAD
LayeredGrid::LayeredGrid(
	const size_t bs[3],
	const size_t min[3],
	const size_t max[3],
	const double extents[6],
	const bool periodic[3],
	const vector <float *> &blks,
	const RegularGrid &rg,
	float missing_value
 ) : RegularGrid(bs,min,max,periodic,blks, missing_value) {

	vector <bool> periodic_v;
	vector <double> minu;
	vector <double> maxu;

	// Extents contains horizontal 
	for (int i=0; i<2; i++) {
		minu.push_back(extents[i]);
		maxu.push_back(extents[i+3]);
	}

	for (int i=0; i<_ndim; i++) {
		periodic_v.push_back(periodic[i]);
	}

	_layeredGrid(periodic_v, minu, maxu, rg);
}

#endif

LayeredGrid::~LayeredGrid() {
}

void LayeredGrid::GetUserExtents(
    vector <double> &minu, vector <double> &maxu
) const {
	minu = _minu;
	maxu = _maxu;
}

void LayeredGrid::GetBoundingBox(
    const vector <size_t> &min,
    const vector <size_t> &max,
    vector <double> &minu,
    vector <double> &maxu
) const {
	assert(min.size() == max.size());
	assert(min.size() >= 2);

	minu.clear();
	maxu.clear();


	// Get extents of horizontal dimensions from base class. Also get vertical 
	// dimension, but it's bogus.
	//
    RegularGrid::GetBoundingBox(min, max, minu, maxu);

	if (_ndim == 2) return;
	
	// Initialize min and max coordinates of varying dimension with 
	// coordinates of "first" and "last" grid point. Coordinates of 
	// varying dimension are stored as values of a scalar function
	// sampling the coordinate space.
	//
	float mincoord = _rg.AccessIJK( min[0], min[1],min[2]);
	float maxcoord = _rg.AccessIJK( max[0], max[1],max[2]);

	// Now find the extreme values of the varying dimension's coordinates
	//
	for (int j = min[1]; j<=max[1]; j++) {
	for (int i = min[0]; i<=max[0]; i++) {
		float v = _rg.AccessIJK( i,j,min[2]);

		if (v<mincoord) mincoord = v;
	}
	}

	for (int j = min[1]; j<=max[1]; j++) {
	for (int i = min[0]; i<=max[0]; i++) {
		float v = _rg.AccessIJK( i,j,max[2]);

		if (v>maxcoord) maxcoord = v;
	}
	}

	minu[2] = mincoord;
	maxu[2] = maxcoord;
}


void    LayeredGrid::GetEnclosingRegion(
	const vector <double> &minu, const vector <double> &maxu,
	vector <size_t> &min, vector <size_t> &max
) const {
	assert(minu.size() == maxu.size());
	assert(minu.size() == _ndim);

	min.clear();
	max.clear();

	//
	// Get coords for non-varying dimension
	//
	RegularGrid::GetEnclosingRegion(minu, maxu, min, max);
	if (_ndim == 2) return;

	// we have the correct results
	// for X and Y dimensions, but the Z levels may not be completely
	// contained in the box. We need to verify and possibly expand
	// the min and max Z values.
	//

	size_t dims[3];
	GetDimensions(dims);

	bool done;
	double z;
	if (maxu[2] >= minu[2]) {
		//
		// first do max, then min
		//
		done = false;
		for (int k=0; k<dims[2] && ! done; k++) {
			done = true;
			max[2] = k;
			for (int j = min[1]; j<=max[1] && done; j++) {
			for (int i = min[0]; i<=max[0] && done; i++) {
				z = _rg.AccessIJK( i, j, k); // get Z coordinate
				if (z < maxu[2]) {
					done = false;
				}
			}
			}
		}
		done = false;
		for (int k = dims[2]-1; k>=0 && ! done; k--) {
			done = true;
			min[2] = k;
			for (int j = min[1]; j<=max[1] && done; j++) {
			for (int i = min[0]; i<=max[0] && done; i++) {
				z = _rg.AccessIJK( i, j, k); // get Z coordinate
				if (z > minu[2]) {
					done = false;
				}
			}
			}
		}
	}
	else {
		//
		// first do max, then min
		//
		done = false;
		for (int k=0; k<dims[2] && ! done; k++) {
			done = true;
			max[2] = k;
			for (int j = min[1]; j<=max[1] && done; j++) {
			for (int i = min[0]; i<=max[0] && done; i++) {
				z = _rg.AccessIJK( i, j, k); // get Z coordinate
				if (z > maxu[2]) {
					done = false;
				}
			}
			}
		}
		done = false;
		for (int k = dims[2]-1; k>=0 && ! done; k--) {
			done = true;
			min[2] = k;
			for (int j = min[1]; j<=max[1] && done; j++) {
			for (int i = min[0]; i<=max[0] && done; i++) {
				z = _rg.AccessIJK( i, j, k); // get Z coordinate
				if (z < maxu[2]) {
					done = false;
				}
			}
			}
		}
	}
}

float LayeredGrid::_GetValueNearestNeighbor (
	double x, double y, double z
) const {

	// Get the indecies of the cell containing the point
	//
	size_t i, j, k;
	GetIJKIndex(x,y,z, &i,&j,&k);

	return(AccessIJK(i,j,k));
}


float LayeredGrid::_GetValueLinear (double x, double y, double z) const {


	size_t dims[3];
	GetDimensions(dims);

	// Get the indecies of the cell containing the point
	//
	size_t i0, j0, k0;
	size_t i1, j1, k1;
	GetIJKIndexFloor(x,y,z, &i0,&j0,&k0);
	if (i0 == dims[0]-1) i1 = i0;
	else i1 = i0+1;
	if (j0 == dims[1]-1) j1 = j0;
	else j1 = j0+1;
	if (k0 == dims[2]-1) k1 = k0;
	else k1 = k0+1;


	// Get user coordinates of cell containing point
	//
	double x0, y0, z0, x1, y1, z1;
	RegularGrid::GetUserCoordinates(i0,j0,k0, &x0, &y0, &z0);
	RegularGrid::GetUserCoordinates(i1,j1,k1, &x1, &y1, &z1);

	//
	// Calculate interpolation weights. We always interpolate along
	// the varying dimension last (the kwgt)
	//
	double iwgt, jwgt, kwgt;
	z0 = _interpolateVaryingCoord(i0,j0,k0,x,y,z);
	z1 = _interpolateVaryingCoord(i0,j0,k1,x,y,z);

	if (x1!=x0) iwgt = fabs((x-x0) / (x1-x0));
	else iwgt = 0.0;
	if (y1!=y0) jwgt = fabs((y-y0) / (y1-y0));
	else jwgt = 0.0;
	if (z1!=z0) kwgt = fabs((z-z0) / (z1-z0));
	else kwgt = 0.0;

	if (GetInterpolationOrder() == 0) {
		if (iwgt>0.5) i0++;
		if (jwgt>0.5) j0++;
		if (kwgt>0.5) k0++;

		return(AccessIJK(i0,j0,k0));
	}

	//
	// perform tri-linear interpolation
	//
	double p0,p1,p2,p3,p4,p5,p6,p7;

	p0 = AccessIJK(i0,j0,k0); 
	if (p0 == GetMissingValue()) return (GetMissingValue());

	if (iwgt!=0.0) {
		p1 = AccessIJK(i1,j0,k0);
		if (p1 == GetMissingValue()) return (GetMissingValue());
	}
	else p1 = 0.0;

	if (jwgt!=0.0) {
		p2 = AccessIJK(i0,j1,k0);
		if (p2 == GetMissingValue()) return (GetMissingValue());
	}
	else p2 = 0.0;

	if (iwgt!=0.0 && jwgt!=0.0) {
		p3 = AccessIJK(i1,j1,k0);
		if (p3 == GetMissingValue()) return (GetMissingValue());
	}
	else p3 = 0.0;

	if (kwgt!=0.0) {
		p4 = AccessIJK(i0,j0,k1); 
		if (p4 == GetMissingValue()) return (GetMissingValue());
	}
	else p4 = 0.0;

	if (kwgt!=0.0 && iwgt!=0.0) {
		p5 = AccessIJK(i1,j0,k1);
		if (p5 == GetMissingValue()) return (GetMissingValue());
	}
	else p5 = 0.0;

	if (kwgt!=0.0 && jwgt!=0.0) {
		p6 = AccessIJK(i0,j1,k1);
		if (p6 == GetMissingValue()) return (GetMissingValue());
	}
	else p6 = 0.0;

	if (kwgt!=0.0 && iwgt!=0.0 && jwgt!=0.0) {
		p7 = AccessIJK(i1,j1,k1);
		if (p7 == GetMissingValue()) return (GetMissingValue());
	}
	else p7 = 0.0;


	double c0 = p0+iwgt*(p1-p0) + jwgt*((p2+iwgt*(p3-p2))-(p0+iwgt*(p1-p0)));
	double c1 = p4+iwgt*(p5-p4) + jwgt*((p6+iwgt*(p7-p6))-(p4+iwgt*(p5-p4)));

	return(c0+kwgt*(c1-c0));

}


float LayeredGrid::GetValue(double x, double y, double z) const {

	// Clamp coordinates on periodic boundaries to grid extents
	//
	_ClampCoord(x,y,z);

	if (! LayeredGrid::InsideGrid(x,y,z)) return(GetMissingValue());

	size_t dims[3];
	GetDimensions(dims);

	// Figure out interpolation order
	//
	int interp_order = _interpolationOrder;
	if (interp_order == 2 || _ndim != 3) {
		if (dims[2] < 3) interp_order = 1;
	}

    if (interp_order == 0) {
        return (_GetValueNearestNeighbor(x,y,z));
    }
    else if (interp_order == 1) {
        return (_GetValueLinear(x,y,z));
    }

	return _getValueQuadratic(x,y,z);

}


void LayeredGrid::_GetUserExtents(
	vector <double> &minu, vector <double> &maxu
) const {
	minu.clear(); 
	maxu.clear();

	vector <size_t> dims = GetDimensions();
	vector <size_t> min, max;

	for (int i=0; i<dims.size(); i++) {
		min.push_back(0);
		max.push_back(dims[i]-1);
	}

	LayeredGrid::GetBoundingBox(min, max, minu, maxu);
}


int LayeredGrid::GetUserCoordinates(
	size_t i, size_t j, size_t k, 
	double *x, double *y, double *z
) const {

	// First get coordinates of non-varying dimensions
	// The varying dimension coordinate returned is ignored
	//
	int rc = RegularGrid::GetUserCoordinates(i,j,k,x,y,z);
	if (rc<0) return(rc);

	// Now get coordinates of varying dimension
	//

	*z = _rg.AccessIJK( i,j,k);
	return(0);

}

void LayeredGrid::GetIJKIndex(
	double x, double y, double z,
	size_t *i, size_t *j, size_t *k
) const {

	// First get ijk index of non-varying dimensions
	// N.B. index returned for varying dimension is bogus
	//
	RegularGrid::GetIJKIndex(x,y,z, i,j,k);

	size_t dims[3];
	GetDimensions(dims);

	// At this point the ijk indecies are correct for the non-varying
	// dimensions. We only need to find the index for the varying dimension
	//
	size_t i0, j0, k0;
	LayeredGrid::GetIJKIndexFloor(x,y,z,&i0, &j0, &k0);
	if (k0 == dims[2]-1) {	// on boundary
		*k = k0;
		return;
	}

	double z0 = _interpolateVaryingCoord(i0,j0,k0,x,y,z);
	double z1 = _interpolateVaryingCoord(i0,j0,k0+1,x,y,z);
	if (fabs(z-z0) < fabs(z-z1)) {
		*k = k0;
	}
	else {
		*k = k0+1;
	}
}

void LayeredGrid::GetIJKIndexFloor(
	double x, double y, double z,
	size_t *i, size_t *j, size_t *k
) const {

	// First get ijk index of non-varying dimensions
	// N.B. index returned for varying dimension is bogus
	//
	size_t i0, j0, k0;
	RegularGrid::GetIJKIndexFloor(x,y,z, &i0,&j0,&k0);

	size_t dims[3];
	GetDimensions(dims);

	size_t i1, j1, k1;
	if (i0 == dims[0]-1) i1 = i0;
	else i1 = i0+1;
	if (j0 == dims[1]-1) j1 = j0;
	else j1 = j0+1;
	if (k0 == dims[2]-1) k1 = k0;
	else k1 = k0+1;


	// At this point the ijk indecies are correct for the non-varying
	// dimensions. We only need to find the index for the varying dimension
	//
	*i = i0; *j = j0;

	// Now search for the closest point
	//
	k0 = 0;
	k1 = dims[2]-1;
	double z0 = _interpolateVaryingCoord(i0,j0,k0,x,y,z);
	double z1 = _interpolateVaryingCoord(i0,j0,k1,x,y,z);

	// see if point is outside grid or on boundary
	//
	if ((z-z0) * (z-z1) >= 0.0) { 	
		if (z0<=z1) {
			if (z<=z0) *k = 0;
			else if (z>=z1) *k = dims[2]-1;
		}
		else {
			if (z>=z0) *k = 0;
			else if (z<=z1) *k = dims[2]-1;
		}
		return;
	}

	//
	// Z-coord of point must be between z0 and z1
	//
	while (k1-k0 > 1) {

		z1 = _interpolateVaryingCoord(i0,j0,(k0+k1)>>1,x,y,z);
		if (z1 == z) {	// pathological case
			//*k = (k0+k1)>>1;
			k0 = (k0+k1)>>1;
			break;
		}

		// if the signs of differences change then the coordinate 
		// is between z0 and z1
		//
		if ((z-z0) * (z-z1) <= 0.0) { 
			k1 = (k0+k1)>>1;
		}
		else {
			k0 = (k0+k1)>>1;
			z0 = z1;
		}
	}
	*k = k0;
}

void LayeredGrid::SetPeriodic(const std::vector <bool> &periodic) { 
	vector <bool> myperiodic = periodic;
	if (myperiodic.size() == 3) myperiodic[2] = false;

	RegularGrid::SetPeriodic(myperiodic);

}
void LayeredGrid::SetPeriodic(const bool periodic[3]) {

	vector <bool> pvec;
	for (int i=0; i<3; i++) pvec.push_back(periodic[i]);

	LayeredGrid::SetPeriodic(pvec);
}

bool LayeredGrid::InsideGrid(double x, double y, double z) const {

	// Clamp coordinates on periodic boundaries to reside within the 
	// grid extents (vary-dimensions can not have periodic boundaries)
	//
	_ClampCoord(x,y,z);

	// Do a quick check to see if the point is completely outside of 
	// the grid bounds.
	//
	if (x<_minu[0] || x>_maxu[0]) return (false);
	if (y<_minu[1] || y>_maxu[1]) return (false);
	if (_ndim == 2) return(true);

	if (z<_minu[2] || z>_maxu[2]) return (false);

	// If we get to here the point's non-varying dimension coordinates
	// must be inside the grid. It is only the varying dimension
	// coordinates that we need to check
	//

	// Get the indecies of the cell containing the point
	// Only the indecies for the non-varying dimensions are correctly
	// returned by GetIJKIndexFloor()
	//
	vector <size_t> dims = GetDimensions();
	assert(dims.size() == 3);

	size_t i0, j0, k0;
	size_t i1, j1, k1;
	RegularGrid::GetIJKIndexFloor(x,y,z, &i0,&j0,&k0);
	if (i0 == dims[0]-1) i1 = i0;
	else i1 = i0+1;

	if (j0 == dims[1]-1) j1 = j0;
	else j1 = j0+1;

	if (k0 == dims[2]-1) k1 = k0;
	else k1 = k0+1;

	//
	// See if the varying dimension coordinate of the point is 
	// completely above or below all of the corner points in the first (last)
	// cell in the column of cells containing the point
	//
	double t00, t01, t10, t11;	// varying dimension coord of "top" cell
	double b00, b01, b10, b11;	// varying dimension coord of "bottom" cell
	double vc; // varying coordinate value

	t00 = _rg.AccessIJK(  i0, j0, dims[2]-1);
	t01 = _rg.AccessIJK(  i1, j0, dims[2]-1);
	t10 = _rg.AccessIJK(  i0, j1, dims[2]-1);
	t11 = _rg.AccessIJK(  i1, j1, dims[2]-1);

	b00 = _rg.AccessIJK(  i0, j0, 0);
	b01 = _rg.AccessIJK(  i1, j0, 0);
	b10 = _rg.AccessIJK(  i0, j1, 0);
	b11 = _rg.AccessIJK(  i1, j1, 0);
	vc = z;

	if (b00<t00) {
		// If coordinate is between all of the cell's top and bottom
		// coordinates the point must be in the grid
		//
		if (b00<vc && b01<vc && b10<vc && b11<vc &&
			t00>vc && t01>vc && t10>vc && t11>vc) {

			return(true);
		}
		//
		// if coordinate is above or below all corner points 
		// the input point must be outside the grid
		//
		if (b00>vc && b01>vc && b10>vc && b11>vc) return(false);
		if (t00<vc && t01<vc && t10<vc && t11<vc) return(false);
	}
	else {
		if (b00>vc && b01>vc && b10>vc && b11>vc &&
			t00<vc && t01<vc && t10<vc && t11<vc) {

			return(true);
		}
		if (b00<vc && b01<vc && b10<vc && b11<vc) return(false);
		if (t00>vc && t01>vc && t10>vc && t11>vc) return(false);
	}

	// If we get this far the point is either inside or outside of a
	// boundary cell on the varying dimension. Need to interpolate
	// the varying coordinate of the cell

	// Varying dimesion coordinate value returned by GetUserCoordinates
	// is not valid
	//
	double x0, y0, z0, x1, y1, z1;
	RegularGrid::GetUserCoordinates(i0,j0,k0, &x0, &y0, &z0);
	RegularGrid::GetUserCoordinates(i1,j1,k1, &x1, &y1, &z1);

	double iwgt, jwgt;
	if (x1!=x0) iwgt = fabs((x-x0) / (x1-x0));
		else iwgt = 0.0;

	if (y1!=y0) jwgt = fabs((y-y0) / (y1-y0));
		else jwgt = 0.0;

	double t = t00+iwgt*(t01-t00) + jwgt*((t10+iwgt*(t11-t10))-(t00+iwgt*(t01-t00)));
	double b = b00+iwgt*(b01-b00) + jwgt*((b10+iwgt*(b11-b10))-(b00+iwgt*(b01-b00)));

	if (b<t) {
		if (vc<b || vc>t) return(false);
	} 
	else {
		if (vc>b || vc<t) return(false);
	}
	return(true);
}

void LayeredGrid::GetMinCellExtents(double *x, double *y, double *z) const {

	*x = *y = *z = 0;
	//
	// Get X and Y dimension minimums
	//
	RegularGrid::GetMinCellExtents(x,y,z);
	if (_ndim == 2) return;

	vector <size_t> dims = GetDimensions();

	if (dims[2] < 2) return;

	double tmp;
	*z = fabs(_maxu[2] - _minu[2]);

	for (int k=0; k<dims[2]-1; k++) {
	for (int j=0; j<dims[1]; j++) {
	for (int i=0; i<dims[0]; i++) {
		float z0 = _rg.AccessIJK( i,j,k);
		float z1 = _rg.AccessIJK( i,j,k+1);
		
		tmp = fabs(z1-z0);
		if (tmp<*z) *z = tmp;
	}
	}
	}
}

void LayeredGrid::_getBilinearWeights(double x, double y, double z,
									double &iwgt, double &jwgt) const {
	size_t dims[3];
	GetDimensions(dims);

	size_t i0, j0;
	size_t i1, j1;
	size_t k0;	

	GetIJKIndexFloor(x,y,z, &i0, &j0, &k0);
	if (i0 == dims[0]-1) i1=i0;
	else i1 = i0+1;
	if (j0 == dims[1]-1) j1=j0;
	else j1 = j0+1;

	double x0,y0,z0;
	double x1,y1,z1;
	GetUserCoordinates(i0,j0,k0,&x0,&y0,&z0);
	GetUserCoordinates(i1,j1,k0,&x1,&y1,&z1);
	if (x1!=x0) iwgt = fabs((x-x0) / (x1-x0));
	else iwgt = 0.0;
	if (y1!=y0) jwgt = fabs((y-y0) / (y1-y0));
	else jwgt = 0.0;
}

double LayeredGrid::_bilinearInterpolation(
	size_t i0, size_t i1, size_t j0, size_t j1,
	size_t k0,  double iwgt, double jwgt
) const {
	double val00, val01, val10, val11, xVal0, result;
	double xVal1 = 0.0;
	double mv = GetMissingValue();

	val00 = AccessIJK(i0,j0,k0);
	val10 = AccessIJK(i1,j0,k0);
	if ((val00==mv) || (val10==mv)) return mv;
	if (val00 == mv) xVal0 = val10;
	else if (val10 == mv) xVal0 = val00;
	else xVal0 = val00 * (1-iwgt) + val10 * iwgt;

	val01 = AccessIJK(i0,j1,k0);
	val11 = AccessIJK(i1,j1,k0);
	if ((val01==mv) || (val11==mv)) return mv;
	if (val01 == mv) xVal0 = val11;
	else if (val11 == mv) xVal0 = val01;
	else xVal1 = val01 * (1-iwgt) + val11 * iwgt;

	result = xVal0 * (1-jwgt) + xVal1 * jwgt;

	if ((val00 == mv) || (val01 == mv) || (val10 == mv) || (val11 == mv))
		return mv;
	else 
		return result;
}

double LayeredGrid::_bilinearElevation(
	size_t i0, size_t i1, size_t j0, size_t j1,
	size_t k0,  double iwgt, double jwgt
) const {
	double xVal0, result;
	double xVal1 = 0.0;
	double x, y, z00, z10, z01, z11;

	GetUserCoordinates(i0,j0,k0,&x,&y,&z00);
	GetUserCoordinates(i1,j0,k0,&x,&y,&z10);
	xVal0 = z00 * (1-iwgt) +  z10* iwgt;

	GetUserCoordinates(i0,j1,k0,&x,&y,&z01);
	GetUserCoordinates(i1,j1,k0,&x,&y,&z11);
	xVal1 = z01 * (1-iwgt) + z11 * iwgt;

	result = xVal0 * (1-jwgt) + xVal1 * jwgt;
	return result;
}

float LayeredGrid::_getValueQuadratic(double x, double y, double z) const {

	double mv = GetMissingValue();
    size_t dims[3];
    GetDimensions(dims);

    // Get the indecies of the hyperslab containing the point
    // k0 = level above the point
    // k1 = level below the point
    // k2 = two levels below the point
    size_t i0, i1, j0, j1; 
	size_t k0, k1, k2;
    GetIJKIndexFloor(x,y,z, &i0,&j0,&k1);
	if (i0 == dims[0]-1) i1=i0;
	else i1 = i0+1;
	if (j0 == dims[1]-1) j1=j0;
	else j1 = j0+1;
	if (k1 == 0) {
		k2 = 0;
		k1 = 1;
		k0 = 2;
	}
	else if (k1 == dims[2]-1) {
		k2 = dims[2]-3;
		k1 = dims[2]-2;	
		k0 = dims[2]-1;
	}
    else {
        k0 = k1+1;
        k2 = k1-1;
    }

	double iwgt, jwgt;
	_getBilinearWeights(x,y,z,iwgt,jwgt);

	// bilinearly interpolated values at each k0, k1 and k2
	double val0, val1, val2;
	val0 = _bilinearInterpolation(i0, i1, j0, j1, k0, iwgt, jwgt);
	val1 = _bilinearInterpolation(i0, i1, j0, j1, k1, iwgt, jwgt);
	val2 = _bilinearInterpolation(i0, i1, j0, j1, k2, iwgt, jwgt);
	if ((val0 == mv) || (val1 == mv) || (val2 == mv)) return mv;
	
	// bilinearly interpolated elevations at each k0, k1, and k2
	double z0, z1, z2;
	z0 = _bilinearElevation(i0, i1, j0, j1, k0, iwgt, jwgt);
	z1 = _bilinearElevation(i0, i1, j0, j1, k1, iwgt, jwgt);
	z2 = _bilinearElevation(i0, i1, j0, j1, k2, iwgt, jwgt);
	if ((z0 == mv) || (z1 == mv) || (z2 == mv)) return mv;

    // quadratic interpolation weights
    double w0, w1, w2;
    w0 = ((z-z1)*(z-z2)) / ((z0-z1)*(z0-z2));
    w1 = ((z-z0)*(z-z2)) / ((z1-z0)*(z1-z2));
    w2 = ((z-z0)*(z-z1)) / ((z2-z0)*(z2-z1));

    double val;
    val = val0*w0 + val1*w1 + val2*w2;

    return val;
}

double LayeredGrid::_interpolateVaryingCoord(
	size_t i0, size_t j0, size_t k0,
	double x, double y, double z) const {

	// varying dimension coord at corner grid points of cell face
	//
	double c00, c01, c10, c11;	

	size_t dims[3];
	GetDimensions(dims);

	size_t i1, j1, k1;
	if (i0 == dims[0]-1) i1 = i0;
	else i1 = i0+1;
	if (j0 == dims[1]-1) j1 = j0;
	else j1 = j0+1;
	if (k0 == dims[2]-1) k1 = k0;
	else k1 = k0+1;

	// Coordinates of grid points for non-varying dimensions 
	double x0, y0, z0, x1, y1, z1;	
	RegularGrid::GetUserCoordinates(i0,j0,k0, &x0, &y0, &z0);
	RegularGrid::GetUserCoordinates(i1,j1,k1, &x1, &y1, &z1);
	double iwgt, jwgt;

	c00 = _rg.AccessIJK( i0, j0, k0);
	c01 = _rg.AccessIJK( i1, j0, k0);
	c10 = _rg.AccessIJK( i0, j1, k0);
	c11 = _rg.AccessIJK( i1, j1, k0);

	if (x1!=x0) iwgt = fabs((x-x0) / (x1-x0));
	else iwgt = 0.0;
	if (y1!=y0) jwgt = fabs((y-y0) / (y1-y0));
	else jwgt = 0.0;

	double c = c00+iwgt*(c01-c00) + jwgt*((c10+iwgt*(c11-c10))-(c00+iwgt*(c01-c00)));
	return(c);
}
