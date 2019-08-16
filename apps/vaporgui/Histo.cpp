//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Histo.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:  Implementation of Histo class 
//
#include <vapor/MyBase.h>
#include <vapor/DataMgrUtils.h>
#include "Histo.h"
using namespace VAPoR;
using namespace Wasp;

#define REQUIRED_SAMPLE_SIZE 1000000
#define SAMPLE_RATE 30

Histo::Histo(
	int numberBins, 
	float mnData, 
	float mxData, 
	string var,
	int ts
){
	_numBins = numberBins;
	_minData = mnData;
	_maxData = mxData;
    if( _maxData < _minData )
        _maxData = _minData;
    _range = _maxData - _minData;
	_binArray = new long[_numBins];
	reset(numberBins);

	_varnameOfUpdate = var;
	_timestepOfUpdate = ts;
}

Histo::Histo(
    const Histo* histo
){
    _numBins = histo->_numBins;

    _minData = histo->_minData;
    _maxData = histo->_maxData;
    if( _maxData < _minData )
        _maxData = _minData;
    _range = _maxData - _minData;

    _binArray = new long[_numBins];
    for (int i=0; i<_numBins; i++)
        _binArray[i] = histo->_binArray[i];
    
    _numBelow = histo->_numBelow;
    _numAbove = histo->_numAbove;

    _maxBinSize = histo->_maxBinSize;

    _varnameOfUpdate = histo->_varnameOfUpdate;
    _timestepOfUpdate = histo->_timestepOfUpdate;
}

#ifdef	VAPOR3_0_0_ALPHA
Histo::Histo(const StructuredGrid *rg, const double exts[6], const float range[2]) {
	_binArray = new int[256];
	_minData = range[0];
	_maxData = range[1];
	_numBins = 256;
	reset();

	unsigned int qv;	// quantized value
	float v;
	vector <double> point;
	StructuredGrid::ConstIterator itr;
	StructuredGrid::ConstIterator enditr = rg->end();
	for (itr = rg->begin(); itr!=enditr; ++itr) {
		v = *itr;
		if (v == rg->GetMissingValue()) continue;
		itr.GetUserCoordinates(point);
		bool isIn = true;
		for (int j = 0; j<point.size(); j++){
			if (point[j]>exts[j+3] || point[j] < exts[j]) isIn = false;
		}
		if (!isIn) continue;
		if (v<range[0]) qv=0;
		else if (v>range[1]) qv=255;
		else qv = (unsigned int) rint((v-range[0])/(range[1]-range[0]) * 255);

		_binArray[qv]++;
		if (qv > 0 && qv < 255 && _binArray[qv] > _maxBinSize) {
			_maxBinSize = _binArray[qv];
			_largestBin = qv;
		}
	}
	
}
#endif
	
Histo::~Histo(){
	if (_binArray) delete [] _binArray;
}
void Histo::reset(int newNumBins) {
	if ( newNumBins != _numBins && newNumBins != -1 ) {
		_numBins = newNumBins;
		if (_binArray) delete [] _binArray;
		_binArray = new long[_numBins];
	}
	for (int i = 0; i< _numBins; i++) _binArray[i] = 0;
	_numBelow = 0;
	_numAbove = 0;
	_maxBinSize = 0;
}

void Histo::reset(int newNumBins, float mnData, float mxData)
{
    reset(newNumBins); 
    _minData = mnData; 
    _maxData = mxData;
    if( _maxData < _minData )
        _maxData = _minData;
    _range = _maxData - _minData;
}

void Histo::addToBin(float val) {
	if (val < _minData) 
        _numBelow++; 
	else if (val > _maxData) 
        _numAbove++;
	else 
    {
        int intVal = 0;
		if (_range == 0.f ) 
            intVal = 0;
		else 
            intVal = (int)((val - _minData) / _range * (float)_numBins );
        intVal = intVal < _numBins ? intVal : _numBins - 1;
		_binArray[intVal]++;
	}
}
	
int Histo::getMaxBinSize()
{
    int maxBinSize = 0;
    for( int i = 0; i < _numBins; i++ )
    {
        maxBinSize = maxBinSize > _binArray[i] ? maxBinSize : _binArray[i] ; 
    }
    
    return maxBinSize;
}

int Histo::Populate(VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp)
{
    size_t ts    = rp->GetCurrentTimestep();
    int refLevel = rp->GetRefinementLevel();
    int lod      = rp->GetCompressionLevel();
    vector<double> minExts, maxExts;
    rp->GetBox()->GetExtents(minExts, maxExts);
    
    Grid *grid;
    DataMgrUtils::GetGrids(dm, ts, rp->GetVariableName(), minExts, maxExts, true, &refLevel, &lod, &grid);
    
    if (grid == nullptr)
        return -1;
    
    grid->SetInterpolationOrder(1);
    
    if (shouldUseSampling(dm, rp))
        populateSamplingHistogram(grid, minExts, maxExts);
    else
        populateIteratingHistogram(grid, calculateStride(dm, rp));
    
    delete grid;
    return 0;
}

void Histo::populateIteratingHistogram(const Grid *grid, const int stride)
{
    VAssert(grid);
    
    float missingValue = grid->GetMissingValue();
    Grid::ConstIterator enditr = grid->cend();
    
    for (auto itr = grid->cbegin(); itr != enditr; itr += stride) {
        float v = *itr;
        if (v != missingValue)
            addToBin(v);
    }
}

#define X 0
#define Y 1
#define Z 2

void Histo::populateSamplingHistogram(const Grid *grid, const vector<double> &minExts, const vector<double> &maxExts)
{
    VAssert(grid);
    
    double dx = (maxExts[X]-minExts[X]) / SAMPLE_RATE;
    double dy = (maxExts[Y]-minExts[Y]) / SAMPLE_RATE;
    double dz = (maxExts[Z]-minExts[Z]) / SAMPLE_RATE;
    std::vector<double> deltas = {dx, dy, dz};
    
    float varValue, missingValue;
    std::vector<double> coords(3, 0.0);
    
    double xStartPoint = minExts[X] + deltas[X]/2.f;
    double yStartPoint = minExts[Y] + deltas[Y]/2.f;
    double zStartPoint = minExts[Z] + deltas[Z]/2.f;
    
    coords[X] = xStartPoint;
    coords[Y] = yStartPoint;
    coords[Z] = zStartPoint;
    
    int iSamples = SAMPLE_RATE;
    int jSamples = SAMPLE_RATE;
    int kSamples = SAMPLE_RATE;
    
    if (deltas[X] == 0)
        iSamples = 1;
    if (deltas[Y] == 0)
        jSamples = 1;
    if (deltas[Z] == 0)
        kSamples = 1;
    
    for (int k=0; k<kSamples; k++) {
        coords[Y] = yStartPoint;
        
        for (int j=0; j<jSamples; j++) {
            coords[X] = xStartPoint;
            
            for (int i=0; i<iSamples; i++) {
                varValue = grid->GetValue(coords);
                missingValue = grid->GetMissingValue();
                if (varValue != missingValue)
                    addToBin(varValue);
                coords[X] += deltas[X];
            }
            coords[Y] += deltas[Y];
        }
        coords[Z] += deltas[Z];
    }
}

#undef X
#undef Y
#undef Z

int Histo::calculateStride(VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const
{
    std::string varname = rp->GetVariableName();
    std::vector<size_t> dimsAtLevel;
    int ref = rp->GetRefinementLevel();
    int rc = dm->GetDimLensAtLevel(varname, ref, dimsAtLevel);
    VAssert(rc>=0);
    
    long size = 1;
    for (int i=0; i<dimsAtLevel.size(); i++)
        size *= dimsAtLevel[i];
    
    int stride = 1;
    if (size > REQUIRED_SAMPLE_SIZE)
        stride = 1 + size / REQUIRED_SAMPLE_SIZE;
    
    return stride;
}

bool Histo::shouldUseSampling(VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const
{
    // TODO does DC::GetNumDimensions include the time dimension?
    int nDims = dm->GetNumDimensions(rp->GetVariableName());
    printf("nDims = %i\n", nDims);
    if (nDims == 3)
        return true;
    return false;
}
