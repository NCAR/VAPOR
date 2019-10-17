//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Histo.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:  Definition of Histo class:
//		it contains a histogram derived from volume data.
//		Used by TFEditor to draw histogram behind transfer function opacity
//
#ifndef HISTO_H
#define HISTO_H
#include <string>
#include <vector>
#include <climits>
#include <vapor/MyBase.h>
#include <vapor/StructuredGrid.h>
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

class Histo {
public:
    Histo(int numberBins, float mnData, float mxData, string var, int ts);
    Histo(int numberBins);
    Histo();
    Histo(const Histo *histo);
    ~Histo();
    void  reset(int newNumBins = -1);
    void  reset(int newNumBins, float mnData, float mxData);
    void  addToBin(float val);
    int   getMaxBinSize();
    int   getMaxBinSizeBetweenIndices(const int start, const int end) const;
    int   getNumBins() const;
    int   getBinSize(int index) const;
    int   getBinSize(int index, int stride) const;
    float getNormalizedBinSize(int bin) const;
    float getNormalizedBinSizeForValue(float v) const;
    float getNormalizedBinSizeForNormalizedValue(float v) const;
    int   getBinIndexForValue(float v);
    float getMinMapData() { return _minMapData; }
    float getMaxMapData() { return _maxMapData; }

    int    getTimestepOfUpdate() { return _timestepOfUpdate; }
    string getVarnameOfUpdate() { return _varnameOfUpdate; }

    int  Populate(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp);
    bool NeedsUpdate(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp);
    int  PopulateIfNeeded(const std::string &varName, VAPoR::DataMgr *dm, VAPoR::RenderParams *rp);

private:
    unsigned int *_binArray = nullptr;
    unsigned int *_below = nullptr;
    unsigned int *_above = nullptr;
    int           _nBinsBelow = 0, _nBinsAbove = 0;
    long          _numSamplesBelow, _numSamplesAbove;
    int           _numBins = 0;
    float         _minMapData, _maxMapData, _range;
    float         _minData, _maxData;
    long          _maxBinSize = -1;

    int                 _refLevel = INT_MIN, _lod = INT_MIN;
    std::vector<double> _minExts, _maxExts;
    bool                _populated = false;

    int    _timestepOfUpdate;
    string _varnameOfUpdate;
    bool   autoSetProperties = false;

    void populateIteratingHistogram(const VAPoR::Grid *grid, const int stride);
    void populateSamplingHistogram(const VAPoR::Grid *grid, const vector<double> &minExts, const vector<double> &maxExts);
    int  calculateStride(const std::string &varName, VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const;
    bool shouldUseSampling(const std::string &varName, VAPoR::DataMgr *dm, const VAPoR::RenderParams *rp) const;
    void setProperties(float mnData, float mxData, string var, int ts);
    void calculateMaxBinSize();
    void _getDataRange(const std::string &varName, VAPoR::DataMgr *d, VAPoR::RenderParams *r, float *min, float *max) const;
};

#endif    // HISTO_H
