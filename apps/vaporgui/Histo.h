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
#include <vapor/MyBase.h>
#include <vapor/StructuredGrid.h>

class Histo {
public:
    Histo(int numberBins, float mnData, float mxData);

#ifdef DEAD
    // Special constructor for unsigned char data:
    //
    Histo(const VAPoR::StructuredGrid *rg, const double exts[6], const float range[2]);
#endif
    ~Histo();
    void reset(int newNumBins = -1);
    void reset(int newNumBins, float mnData, float mxData)
    {
        reset(newNumBins);
        _minData = mnData;
        _maxData = mxData;
    }
    void  addToBin(float val);
    int   getBinSize(int posn) { return _binArray[posn]; }
    int   getMaxBinSize() { return _maxBinSize; }
    float getMinData() { return _minData; }
    float getMaxData() { return _maxData; }

private:
    Histo() {}
    int * _binArray;
    int   _numBelow, _numAbove;
    int   _numBins;
    float _minData, _maxData;
    int   _maxBinSize;
    int   _largestBin;
};

#endif    // HISTO_H
