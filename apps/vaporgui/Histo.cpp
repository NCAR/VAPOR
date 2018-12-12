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
#include "Histo.h"
using namespace VAPoR;
using namespace Wasp;

Histo::Histo(int numberBins, float mnData, float mxData, string var, int ts)
{
    _numBins = numberBins;
    _minData = mnData;
    _maxData = mxData;
    if (_maxData < _minData) _maxData = _minData;
    _range = _maxData - _minData;
    _binArray = new long[_numBins];
    reset(numberBins);

    _varnameOfUpdate = var;
    _timestepOfUpdate = ts;
}

Histo::Histo(const Histo *histo)
{
    _numBins = histo->_numBins;

    _minData = histo->_minData;
    _maxData = histo->_maxData;
    if (_maxData < _minData) _maxData = _minData;
    _range = _maxData - _minData;

    _binArray = new long[_numBins];
    for (int i = 0; i < _numBins; i++) _binArray[i] = histo->_binArray[i];

    _numBelow = histo->_numBelow;
    _numAbove = histo->_numAbove;

    _maxBinSize = histo->_maxBinSize;

    _varnameOfUpdate = histo->_varnameOfUpdate;
    _timestepOfUpdate = histo->_timestepOfUpdate;
}

#ifdef VAPOR3_0_0_ALPHA
Histo::Histo(const StructuredGrid *rg, const double exts[6], const float range[2])
{
    _binArray = new int[256];
    _minData = range[0];
    _maxData = range[1];
    _numBins = 256;
    reset();

    unsigned int                  qv;    // quantized value
    float                         v;
    vector<double>                point;
    StructuredGrid::ConstIterator itr;
    StructuredGrid::ConstIterator enditr = rg->end();
    for (itr = rg->begin(); itr != enditr; ++itr) {
        v = *itr;
        if (v == rg->GetMissingValue()) continue;
        itr.GetUserCoordinates(point);
        bool isIn = true;
        for (int j = 0; j < point.size(); j++) {
            if (point[j] > exts[j + 3] || point[j] < exts[j]) isIn = false;
        }
        if (!isIn) continue;
        if (v < range[0])
            qv = 0;
        else if (v > range[1])
            qv = 255;
        else
            qv = (unsigned int)rint((v - range[0]) / (range[1] - range[0]) * 255);

        _binArray[qv]++;
        if (qv > 0 && qv < 255 && _binArray[qv] > _maxBinSize) {
            _maxBinSize = _binArray[qv];
            _largestBin = qv;
        }
    }
}
#endif

Histo::~Histo()
{
    if (_binArray) delete[] _binArray;
}
void Histo::reset(int newNumBins)
{
    if (newNumBins != _numBins && newNumBins != -1) {
        _numBins = newNumBins;
        if (_binArray) delete[] _binArray;
        _binArray = new long[_numBins];
    }
    for (int i = 0; i < _numBins; i++) _binArray[i] = 0;
    _numBelow = 0;
    _numAbove = 0;
    _maxBinSize = 0;
}

void Histo::reset(int newNumBins, float mnData, float mxData)
{
    reset(newNumBins);
    _minData = mnData;
    _maxData = mxData;
    if (_maxData < _minData) _maxData = _minData;
    _range = _maxData - _minData;
}

void Histo::addToBin(float val)
{
    if (val < _minData)
        _numBelow++;
    else if (val > _maxData)
        _numAbove++;
    else {
        int intVal = 0;
        if (_range == 0.f)
            intVal = 0;
        else
            intVal = (int)((val - _minData) / _range * (float)_numBins);
        intVal = intVal < _numBins ? intVal : _numBins - 1;
        _binArray[intVal]++;
    }
}

int Histo::getMaxBinSize()
{
    int maxBinSize = 0;
    for (int i = 0; i < _numBins; i++) { maxBinSize = maxBinSize > _binArray[i] ? maxBinSize : _binArray[i]; }

    return maxBinSize;
}
