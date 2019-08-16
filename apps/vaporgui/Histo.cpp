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

Histo::Histo(int numberBins, float mnData, float mxData, string var, int ts)
{
    setProperties(mnData, mxData, var, ts);
    reset(numberBins);
}

Histo::Histo(int numberBins)
{
    autoSetProperties = true;
    reset(numberBins);
}

Histo::Histo() : Histo(0) {}

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
	_maxBinSize = -1;
    _populated = false;
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
	
long Histo::getMaxBinSize()
{
    if (_maxBinSize == -1) // For legacy purposes. Can remove with new TF Editor
        calculateMaxBinSize();
    return _maxBinSize;
}

int Histo::getNumBins() const
{
    return _numBins;
}

float Histo::getBinSizeNormalized(int bin) const
{
    return getBinSize(bin) / (float)_maxBinSize;
}

int Histo::Populate(VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    size_t ts    = rp->GetCurrentTimestep();
    int refLevel = rp->GetRefinementLevel();
    int lod      = rp->GetCompressionLevel();
    vector<double> minExts, maxExts;
    rp->GetBox()->GetExtents(minExts, maxExts);
    
    if (autoSetProperties) {
        std::string varname = rp->GetVariableName();
        MapperFunction *mf = rp->GetMapperFunc(varname);
        setProperties(mf->getMinMapValue(), mf->getMaxMapValue(), varname, ts);
        _minExts = minExts;
        _maxExts = maxExts;
        _lod = lod;
        _refLevel = refLevel;
    }
    
    Grid *grid;
    int rc = DataMgrUtils::GetGrids(dm, ts, rp->GetVariableName(), minExts, maxExts, true, &refLevel, &lod, &grid);
    
    if (rc < 0)
        return -1;
    
    grid->SetInterpolationOrder(1);
    
    if (shouldUseSampling(dm, rp))
        populateSamplingHistogram(grid, minExts, maxExts);
    else
        populateIteratingHistogram(grid, calculateStride(dm, rp));
    
    calculateMaxBinSize();
    _populated = true;
    
    delete grid;
    return 0;
}

bool Histo::NeedsUpdate(VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    if (!dm || !rp) return false;
    if (!_populated) return true;
    
    std::string varname = rp->GetVariableName();
    MapperFunction *mf = rp->GetMapperFunc(varname);
    vector<double> minExts, maxExts;
    rp->GetBox()->GetExtents(minExts, maxExts);
    
    if (_minData != mf->getMinMapValue()) return true;
    if (_maxData != mf->getMaxMapValue()) return true;
    if (_refLevel != rp->GetRefinementLevel()) return true;
    if (_lod != rp->GetCompressionLevel()) return true;
    if (_varnameOfUpdate != varname) return true;
    if (_timestepOfUpdate != rp->GetCurrentTimestep()) return true;
    if (_minExts != minExts) return true;
    if (_maxExts != maxExts) return true;
    
    return false;
}

int Histo::PopulateIfNeeded(VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    if (!NeedsUpdate(dm, rp))
        return 0;
    
    return Populate(dm, rp);
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

void Histo::setProperties(float mnData, float mxData, string var, int ts)
{
    _minData = mnData;
    _maxData = mxData;
    if( _maxData < _minData )
        _maxData = _minData;
    _range = _maxData - _minData;
    
    _varnameOfUpdate = var;
    _timestepOfUpdate = ts;
}

void Histo::calculateMaxBinSize()
{
    int maxBinSize = 0;
    for( int i = 0; i < _numBins; i++ )
    {
        maxBinSize = maxBinSize > _binArray[i] ? maxBinSize : _binArray[i] ;
    }
    
    _maxBinSize = maxBinSize;
}
