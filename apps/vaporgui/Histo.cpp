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

Histo::Histo(int numberBins, float mnData, float mxData) {
    _numBins = numberBins;
    _minData = mnData;
    _maxData = mxData;
    _binArray = new int[_numBins];
    reset(numberBins);
}

#ifdef DEAD
Histo::Histo(const StructuredGrid *rg, const double exts[6], const float range[2]) {
    _binArray = new int[256];
    _minData = range[0];
    _maxData = range[1];
    _numBins = 256;
    reset();

    unsigned int qv; // quantized value
    float v;
    vector<double> point;
    StructuredGrid::ConstIterator itr;
    StructuredGrid::ConstIterator enditr = rg->end();
    //	for (itr = rg->begin(); itr!=rg->end(); ++itr) {
    for (itr = rg->begin(); itr != enditr; ++itr) {
        v = *itr;
        if (v == rg->GetMissingValue())
            continue;
        itr.GetUserCoordinates(point);
        bool isIn = true;
        for (int j = 0; j < point.size(); j++) {
            if (point[j] > exts[j + 3] || point[j] < exts[j])
                isIn = false;
        }
        if (!isIn)
            continue;
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

Histo::~Histo() {
    if (_binArray)
        delete[] _binArray;
}
void Histo::reset(int newNumBins) {
    if (newNumBins != -1) {
        _numBins = newNumBins;
        if (_binArray)
            delete[] _binArray;
        _binArray = new int[_numBins];
    }
    for (int i = 0; i < _numBins; i++)
        _binArray[i] = 0;
    _numBelow = 0;
    _numAbove = 0;
    _maxBinSize = 0;
    _largestBin = -1;
}
void Histo::addToBin(float val) {
    int intVal;
    if (val < _minData)
        _numBelow++;
    else if (val > _maxData)
        _numAbove++;
    else {
        if (_maxData == _minData)
            intVal = 0;
        else
            intVal = (int)(((double)val - (double)_minData) * (double)_numBins / ((double)_maxData - (double)_minData));
        if (intVal >= _numBins)
            intVal = _numBins - 1;
        if (intVal <= 0)
            intVal = 0;
        _binArray[intVal]++;
        if (_binArray[intVal] > _maxBinSize) {
            _maxBinSize = _binArray[intVal];
            _largestBin = intVal;
        }
    }
}
