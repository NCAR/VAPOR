//************************************************************************
//                                    *
//             Copyright (C)  2004                *
//     University Corporation for Atmospheric Research            *
//             All Rights Reserved                *
//                                    *
//************************************************************************/
//
//    File:        Histo.cpp
//
//    Author:        Alan Norton
//            National Center for Atmospheric Research
//            PO 3000, Boulder, Colorado
//
//    Date:        November 2004
//
//    Description:  Implementation of Histo class
//
#include <vapor/MyBase.h>
#include <vapor/DataMgrUtils.h>
#include "Histo.h"
#include <cassert>
using namespace VAPoR;
using namespace Wasp;

#define SAMPLE_RATE 30

#ifndef __FLT_EPSILON__
    #define __FLT_EPSILON__ FLT_EPSILON
#endif

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

Histo::~Histo()
{
    if (_binArray) delete[] _binArray;
    if (_below) delete[] _below;
    if (_above) delete[] _above;
}
void Histo::reset(int newNumBins)
{
    if (newNumBins != _numBins && newNumBins != -1) {
        _numBins = newNumBins;
        if (_binArray) delete[] _binArray;
        _binArray = new unsigned int[_numBins];
        if (_below) delete[] _below;
        if (_above) delete[] _above;
        _below = nullptr;
        _above = nullptr;
    }
    for (int i = 0; i < _numBins; i++) _binArray[i] = 0;
    if (_below) memset(_below, 0, _nBinsBelow * sizeof(*_below));
    if (_above) memset(_above, 0, _nBinsAbove * sizeof(*_above));
    _numSamplesBelow = 0;
    _numSamplesAbove = 0;
    _maxBinSize = -1;
    _populated = false;
}

void Histo::reset(int newNumBins, float mnData, float mxData)
{
    reset(newNumBins);
    _minMapData = mnData;
    _maxMapData = mxData;
    if (_maxMapData < _minMapData) _maxMapData = _minMapData;
    _range = _maxMapData - _minMapData;
}

void Histo::addToBin(float val)
{
    // The additional checks below are because
    // 1. The data min/max are imperfect, e.g. calculated max is 1 but E value of 1.1
    // 2. Float precision errors, e.g.
    //    >  Range is 0 - 1, outer range is -1 - 2
    //    >  val = -0.00000000001 so it is below
    //    >  -0.00000000001 - -1 = -1
    //    >  0 - -1 = 1
    //    >  1 * array size = out of bounds

    if (val < _minMapData) {
        _numSamplesBelow++;
        if (_below) {
            assert(_minMapData - _minData > 0);
            int index = (val - _minData) / (_minMapData - _minData) * _nBinsBelow;

            if (index >= _nBinsBelow) index = _nBinsBelow - 1;
            if (index >= 0) _below[index]++;
        }
    } else if (val > _maxMapData) {
        _numSamplesAbove++;
        if (_above) {
            assert(_maxData - _maxMapData > 0);
            int index = (val - _maxMapData) / (_maxData - _maxMapData) * _nBinsAbove;

            if (index < 0) index = 0;
            if (index < _nBinsAbove) _above[index]++;
        }
    } else {
        int intVal = 0;
        if (_range == 0.f)
            intVal = 0;
        else
            intVal = (int)((val - _minMapData) / _range * (float)_numBins);

        if (intVal < 0) intVal = 0;
        if (intVal >= _numBins) intVal = _numBins - 1;
        _binArray[intVal]++;
    }
}

int Histo::getMaxBinSize()
{
    if (_maxBinSize == -1)    // For legacy purposes. Can remove with new TF Editor
        calculateMaxBinSize();
    if (_maxBinSize == 0)
        return 1;
    else
        return _maxBinSize;
}

int Histo::getMaxBinSizeBetweenIndices(const int start, const int end) const
{
    int maxBin = 0;

    if (start < 0)
        for (int i = max(0, start + _nBinsBelow); i < min(end + _nBinsBelow, _nBinsBelow); i++) maxBin = maxBin < _below[i] ? _below[i] : maxBin;

    for (int i = max(start, 0); i < min(end, _numBins); i++) maxBin = maxBin < _binArray[i] ? _binArray[i] : maxBin;

    if (end >= _numBins)
        for (int i = max(start - _numBins, 0); i < min(end - _numBins, _nBinsAbove); i++) maxBin = maxBin < _above[i] ? _above[i] : maxBin;

    if (maxBin == 0)
        return 1;
    else
        return maxBin;
}

int Histo::getNumBins() const { return _numBins; }

int Histo::getBinSize(int index) const
{
    if (index < 0) {
        index += _nBinsBelow;
        if (index < 0) return 0;
        return _below[index];
    } else if (index >= _numBins) {
        index -= _numBins;
        if (index >= _nBinsAbove) return 0;
        return _above[index];
    } else
        return _binArray[index];
}

int Histo::getBinSize(int index, int stride) const
{
    int max = 0;
    for (int i = index; i < index + stride; i++) max = std::max(max, getBinSize(i));
    return max;
}

float Histo::getNormalizedBinSize(int bin) const
{
    if (_maxBinSize == 0) return 0;
    return getBinSize(bin) / (float)_maxBinSize;
}

float Histo::getNormalizedBinSizeForValue(float v) const
{
    float nv = (v - _minMapData) / _range;
    return getNormalizedBinSizeForNormalizedValue(nv);
}

float Histo::getNormalizedBinSizeForNormalizedValue(float v) const
{
    if (v < 0) {
        float belowNormMin = 0 - (_minMapData - _minData) / _range;
        if (v < belowNormMin || !_below) return 0;
        int index = (v - belowNormMin) / (0 - belowNormMin) * _nBinsBelow;
        return std::min(1.0f, _below[index] / (float)_maxBinSize);
    } else if (v >= 1.0) {
        float aboveNormMax = 1 + (_maxData - _maxMapData) / _range;
        if (v >= aboveNormMax || !_above) return 0;
        int index = (v - 1) / (aboveNormMax - 1) * _nBinsAbove;
        return std::min(1.0f, _above[index] / (float)_maxBinSize);
    } else
        return getNormalizedBinSize(v * _numBins);
}

int Histo::getBinIndexForValue(float v) { return (v - _minMapData) / _range * _numBins; }

int Histo::Populate(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    size_t         ts = rp->GetCurrentTimestep();
    int            refLevel = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    vector<double> minExts, maxExts;
    rp->GetBox()->GetExtents(minExts, maxExts);

    if (autoSetProperties) {
        MapperFunction *mf = rp->GetMapperFunc(varName);
        setProperties(mf->getMinMapValue(), mf->getMaxMapValue(), varName, ts);
        _minExts = minExts;
        _maxExts = maxExts;
        _lod = lod;
        _refLevel = refLevel;
    }

    if (_below) {
        delete[] _below;
        _below = nullptr;
    }
    if (_above) {
        delete[] _above;
        _above = nullptr;
    }

    _getDataRange(varName, dm, rp, &_minData, &_maxData);

#ifdef WIN32
    if (_maxMapData - _minMapData > FLT_EPSILON) {
#else
    if (_maxMapData - _minMapData > __FLT_EPSILON__) {
#endif
        _nBinsBelow = std::min(10000.0f, _numBins / (_maxMapData - _minMapData) * (_minMapData - _minData));
        _nBinsAbove = std::min(10000.0f, _numBins / (_maxMapData - _minMapData) * (_maxData - _maxMapData));
    } else {
        _nBinsAbove = 0;
        _nBinsBelow = 0;
    }

    if (_nBinsBelow > 0) _below = new unsigned int[_nBinsBelow];
    if (_nBinsAbove > 0) _above = new unsigned int[_nBinsAbove];
    if (_below) memset(_below, 0, _nBinsBelow * sizeof(*_below));
    if (_above) memset(_above, 0, _nBinsAbove * sizeof(*_above));

    Grid *grid;
    int   rc = DataMgrUtils::GetGrids(dm, ts, varName, minExts, maxExts, true, &refLevel, &lod, &grid);

    if (rc < 0) return -1;

    grid->SetInterpolationOrder(1);

    if (shouldUseSampling(varName, dm, rp))
        populateSamplingHistogram(grid, minExts, maxExts);
    else
        populateIteratingHistogram(grid, calculateStride(varName, dm, rp));

    calculateMaxBinSize();
    _populated = true;

    dm->UnlockGrid(grid);
    delete grid;
    return 0;
}

bool Histo::NeedsUpdate(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    if (!dm || !rp) return false;
    if (!_populated) return true;

    MapperFunction *mf = rp->GetMapperFunc(varName);
    vector<double>  minExts, maxExts;
    rp->GetBox()->GetExtents(minExts, maxExts);

    if (_minMapData != mf->getMinMapValue()) return true;
    if (_maxMapData != mf->getMaxMapValue()) return true;
    if (_refLevel != rp->GetRefinementLevel()) return true;
    if (_lod != rp->GetCompressionLevel()) return true;
    if (_varnameOfUpdate != varName) return true;
    if (_timestepOfUpdate != rp->GetCurrentTimestep()) return true;
    if (_minExts != minExts) return true;
    if (_maxExts != maxExts) return true;

    return false;
}

int Histo::PopulateIfNeeded(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp)
{
    if (!NeedsUpdate(varName, dm, rp)) return 0;

    reset(_numBins);
    return Populate(varName, dm, rp);
}

void Histo::populateIteratingHistogram(const Grid *grid, const int stride)
{
    VAssert(grid);

    float               missingValue = grid->GetMissingValue();
    Grid::ConstIterator enditr = grid->cend();

    for (auto itr = grid->cbegin(); itr != enditr; itr += stride) {
        float v = *itr;
        if (v != missingValue) addToBin(v);
    }
}

#define X 0
#define Y 1
#define Z 2

void Histo::populateSamplingHistogram(const Grid *grid, const vector<double> &minExts, const vector<double> &maxExts)
{
    VAssert(grid);
    VAssert(minExts.size() == 3 && maxExts.size() == 3);

    double              dx = (maxExts[X] - minExts[X]) / SAMPLE_RATE;
    double              dy = (maxExts[Y] - minExts[Y]) / SAMPLE_RATE;
    double              dz = (maxExts[Z] - minExts[Z]) / SAMPLE_RATE;
    std::vector<double> deltas = {dx, dy, dz};

    float               varValue, missingValue;
    std::vector<double> coords(3, 0.0);

    double xStartPoint = minExts[X] + deltas[X] / 2.f;
    double yStartPoint = minExts[Y] + deltas[Y] / 2.f;
    double zStartPoint = minExts[Z] + deltas[Z] / 2.f;

    coords[X] = xStartPoint;
    coords[Y] = yStartPoint;
    coords[Z] = zStartPoint;

    int iSamples = SAMPLE_RATE;
    int jSamples = SAMPLE_RATE;
    int kSamples = SAMPLE_RATE;

    if (deltas[X] == 0) iSamples = 1;
    if (deltas[Y] == 0) jSamples = 1;
    if (deltas[Z] == 0) kSamples = 1;

    for (int k = 0; k < kSamples; k++) {
        coords[Y] = yStartPoint;

        for (int j = 0; j < jSamples; j++) {
            coords[X] = xStartPoint;

            for (int i = 0; i < iSamples; i++) {
                varValue = grid->GetValue(coords);
                missingValue = grid->GetMissingValue();
                if (varValue != missingValue) addToBin(varValue);
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

int Histo::calculateStride(const std::string &varName, VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const
{
    return DataMgrUtils::GetDefaultMetaInfoStride(dm, varName, rp->GetRefinementLevel());
}

bool Histo::shouldUseSampling(const std::string &varName, VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const
{
    // TODO how does this handle with unstructured grids?
    int nDims = dm->GetNumDimensions(varName);
    if (nDims == 3) return true;
    return false;
}

void Histo::setProperties(float mnData, float mxData, string var, int ts)
{
    _minMapData = mnData;
    _maxMapData = mxData;
    if (_maxMapData < _minMapData) _maxMapData = _minMapData;
    _range = _maxMapData - _minMapData;

    _varnameOfUpdate = var;
    _timestepOfUpdate = ts;
}

void Histo::calculateMaxBinSize()
{
    int maxBinSize = 0;
    for (int i = 0; i < _numBins; i++) { maxBinSize = maxBinSize > _binArray[i] ? maxBinSize : _binArray[i]; }

    _maxBinSize = maxBinSize;
}

void Histo::_getDataRange(const std::string &varName, VAPoR::DataMgr *d, VAPoR::RenderParams *r, float *min, float *max) const
{
    vector<double> minExt, maxExt;
    r->GetBox()->GetExtents(minExt, maxExt);

    std::vector<double> range;
    d->GetDataRange(r->GetCurrentTimestep(), varName, r->GetRefinementLevel(), r->GetCompressionLevel(), minExt, maxExt, range);
    *min = range[0];
    *max = range[1];
}
