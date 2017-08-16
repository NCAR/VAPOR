#include <iostream>
#include <vector>
#include <cassert>
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

#include "vapor/RegularGrid.h"

using namespace std;
using namespace VAPoR;


void RegularGrid::_SetExtents(
	const vector <double> &minu,
	const vector <double> &maxu
) {
	assert(minu.size() == maxu.size());
	assert(minu.size() == GetTopologyDim());

	_minu.clear();
	_maxu.clear();
	_delta.clear();

	_minu = minu;
	_maxu = maxu;

	vector <size_t> dims = GetDimensions();
	for (int i=0; i<_minu.size(); i++) {
		_delta.push_back((_maxu[i] - _minu[i])/(double) (dims[i] - 1));
	}
}


RegularGrid::RegularGrid(
	const vector <size_t> &dims,
	const vector <size_t> &bs,
	const vector <float *> &blks,
	const vector <double> &minu,
	const vector <double> &maxu
) : StructuredGrid(dims, bs, blks) {

	assert(GetTopologyDim() == minu.size());
	assert(GetTopologyDim() == maxu.size());

	_SetExtents(minu, maxu);
}

RegularGrid::RegularGrid() {
	_minu.clear();
	_maxu.clear();
	_delta.clear() ;
}

RegularGrid::~RegularGrid() {
}


float RegularGrid::_GetValueNearestNeighbor(
	const std::vector <double> &coords
) const {

	int ndim = GetTopologyDim();
	assert(coords.size() == ndim);

	size_t i = 0;
	size_t j = 0;
	size_t k = 0;

	if (_delta[0] != 0.0) i = (size_t) floor ((coords[0]-_minu[0]) / _delta[0]);
	if (_delta[1] != 0.0) j = (size_t) floor ((coords[1]-_minu[1]) / _delta[1]);

	if (ndim == 3) 
		if (_delta[2] != 0.0) k = (size_t) floor ((coords[2]-_minu[2]) / _delta[2]);

	vector <size_t> dims = GetDimensions();
	assert(i<dims[0]);
	assert(j<dims[1]);

	if (ndim == 3) 
		assert(k<dims[2]);

	double iwgt = 0.0;
	double jwgt = 0.0;
	double kwgt = 0.0;

	if (_delta[0] != 0.0) {
		iwgt = ((coords[0] - _minu[0]) - (i * _delta[0])) / _delta[0];
	}

	if (_delta[1] != 0.0) {
		jwgt = ((coords[1] - _minu[1]) - (j * _delta[1])) / _delta[1];
	}

	if (ndim == 3)  {
		if (_delta[2] != 0.0) {
			kwgt = ((coords[2] - _minu[2]) - (k * _delta[2])) / _delta[2];
		}
	}


	if (iwgt>0.5) i++;
	if (jwgt>0.5) j++;

	if (ndim == 3) {
		if (kwgt>0.5) k++;
	}

	return(AccessIJK(i,j,k));

}

float RegularGrid::_GetValueLinear(const std::vector <double> &coords) const {

	int ndim = GetTopologyDim();
	assert(coords.size() == ndim);

	size_t i = 0;
	size_t j = 0;
	size_t k = 0;

	if (_delta[0] != 0.0) {
		i = (size_t) floor ((coords[0]-_minu[0]) / _delta[0]);
	}
	if (_delta[1] != 0.0) {
		j = (size_t) floor ((coords[1]-_minu[1]) / _delta[1]);
	}

	if (ndim == 3 && _delta[2] != 0.0) {
		k = (size_t) floor ((coords[2]-_minu[2]) / _delta[2]);
	}

	vector <size_t> dims = GetDimensions();
	assert(i<dims[0]);
	assert(j<dims[1]);

	if (ndim == 3) {
		assert(k<dims[2]);
	}

	double iwgt = 0.0;
	double jwgt = 0.0;
	double kwgt = 0.0;

	if (_delta[0] != 0.0) {
		iwgt = ((coords[0] - _minu[0]) - (i * _delta[0])) / _delta[0];
	}
	if (_delta[1] != 0.0) {
		jwgt = ((coords[1] - _minu[1]) - (j * _delta[1])) / _delta[1];
	}

	if (ndim == 3 && _delta[2] != 0.0) {
		kwgt = ((coords[2] - _minu[2]) - (k * _delta[2])) / _delta[2];
	}

	float missingValue = GetMissingValue();
	double p0,p1,p2,p3,p4,p5,p6,p7;

	p0 = AccessIJK(i,j,k); 
	if (p0 == missingValue) return (missingValue);

	if (iwgt!=0.0) {
		p1 = AccessIJK(i+1,j,k);
		if (p1 == missingValue) return (missingValue);
	}
	else p1 = 0.0;

	if (jwgt!=0.0) {
		p2 = AccessIJK(i,j+1,k);
		if (p2 == missingValue) return (missingValue);
	}
	else p2 = 0.0;

	if (iwgt!=0.0 && jwgt!=0.0) {
		p3 = AccessIJK(i+1,j+1,k);
		if (p3 == missingValue) return (missingValue);
	}
	else p3 = 0.0;

	if (kwgt!=0.0) {
		p4 = AccessIJK(i,j,k+1); 
		if (p4 == missingValue) return (missingValue);
	}
	else p4 = 0.0;

	if (kwgt!=0.0 && iwgt!=0.0) {
		p5 = AccessIJK(i+1,j,k+1);
		if (p5 == missingValue) return (missingValue);
	}
	else p5 = 0.0;

	if (kwgt!=0.0 && jwgt!=0.0) {
		p6 = AccessIJK(i,j+1,k+1);
		if (p6 == missingValue) return (missingValue);
	}
	else p6 = 0.0;

	if (kwgt!=0.0 && iwgt!=0.0 && jwgt!=0.0) {
		p7 = AccessIJK(i+1,j+1,k+1);
		if (p7 == missingValue) return (missingValue);
	}
	else p7 = 0.0;


	double c0 = p0+iwgt*(p1-p0) + jwgt*((p2+iwgt*(p3-p2))-(p0+iwgt*(p1-p0)));
	double c1 = p4+iwgt*(p5-p4) + jwgt*((p6+iwgt*(p7-p6))-(p4+iwgt*(p5-p4)));

	return(c0+kwgt*(c1-c0));

}

void RegularGrid::GetUserExtents(
	vector <double> &minu, vector <double> &maxu
) const {
	minu = _minu;
	maxu = _maxu;
}

void RegularGrid::GetBoundingBox(
    const vector <size_t> &min, const vector <size_t> &max,
    vector <double> &minu, vector <double> &maxu
) const {
	assert(min.size() == max.size());
	assert(min.size() <= GetTopologyDim());

	RegularGrid::GetUserCoordinates(min, minu);
	RegularGrid::GetUserCoordinates(max, maxu);
}

void    RegularGrid::GetEnclosingRegion(
	const std::vector <double> &minu, const std::vector <double> &maxu,
	std::vector <size_t> &min, std::vector <size_t> &max
) const {
	assert(minu.size() == maxu.size());
	assert(minu.size() == GetTopologyDim());

	min.clear();
	max.clear();

	for (int i=0; i<minu.size(); i++) {
		assert(minu[i] <= maxu[i]);
		double u = minu[i]; 
		if (u < minu[i]) {
			u = minu[i];
		}
		size_t index = (u - _minu[i]) / _delta[i];
		min.push_back(index);

		u = maxu[i]; 
		if (u > maxu[i]) {
			u = maxu[i];
		}
		index = (u - _maxu[i]) / _delta[i];
		max.push_back(index);
	}

}

void RegularGrid::GetUserCoordinates(
	const std::vector <size_t> &indices,
	std::vector <double> &coords
) const {
	coords.clear();

	vector <size_t> dims = GetDimensions();
	assert(indices.size() == GetTopologyDim());

	for (int i=0; i<indices.size(); i++) {
		size_t index = indices[i];
		
		if (index >= dims[i]) {
			index = dims[i] - 1;
		}

		coords.push_back(indices[i] * _delta[i] + _minu[i]);
	}
}

void RegularGrid::GetIndices(
    const std::vector <double> &coords,
    std::vector <size_t> &indices
) const {
	assert(coords.size() >= GetTopologyDim());
	indices.clear();

	std::vector <double> clampedCoords = coords;
	ClampCoord(clampedCoords);

	vector <size_t> dims = GetDimensions();

	vector <double> wgts;
	
	for (int i=0; i<clampedCoords.size(); i++) {
		indices.push_back(0);
	
		if (clampedCoords[i] < _minu[i]) {
			indices[i] = 0;
			continue;
		}
		if (clampedCoords[i] > _maxu[i]) {
			indices[i] = dims[i]-1;
			continue;
		}

		if (_delta[i] != 0.0) {
			indices[i] = (size_t) floor (
				(clampedCoords[i]-_minu[i]) / _delta[i]
			);
		}

		assert(indices[i]<dims[i]);

		double wgt = 0.0;

		if (_delta[0] != 0.0) {
			wgt = ((clampedCoords[i]-_minu[i]) - (indices[i]*_delta[i])) /
				_delta[i];
		}
		if (wgt > 0.5) indices[i] += 1;
	}
}

bool RegularGrid::GetIndicesCell(
    const std::vector <double> &coords,
    std::vector <size_t> &indices
) const {
	assert(coords.size() >= GetTopologyDim());
	indices.clear();

	std::vector <double> clampedCoords = coords;
	ClampCoord(clampedCoords);

	vector <size_t> dims = GetDimensions();

	vector <double> wgts;
	
	for (int i=0; i<clampedCoords.size(); i++) {
		indices.push_back(0);

		if (clampedCoords[i] < _minu[i] || clampedCoords[i] > _maxu[i]) {
			return(false);
		}

		if (_delta[i] != 0.0) {
			indices[i] = (size_t) floor (
				(clampedCoords[i]-_minu[i]) / _delta[i]
			);
		}

		assert(indices[i]<dims[i]);

	}

	return(true);

}

bool RegularGrid::InsideGrid(const std::vector <double> &coords) const
{
	assert(coords.size() == GetTopologyDim());

	std::vector <double> clampedCoords = coords;
	ClampCoord(clampedCoords);

	for (int i=0; i<clampedCoords.size(); i++) {
		if (clampedCoords[i] < _minu[i]) return(false);

		if (clampedCoords[i] > _maxu[i]) return(false);
	}

	return(true);
		
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const RegularGrid &rg)
{
	o << "RegularGrid " << endl;
    o << " Min coord ";
    for (int i=0; i<rg._minu.size(); i++) {
        o << rg._minu[i] << " ";
    }
    o << endl;

    o << " Max coord ";
    for (int i=0; i<rg._maxu.size(); i++) {
        o << rg._maxu[i] << " ";
    }
    o << endl;

    o << (const StructuredGrid &) rg;

	return o;
}
};
