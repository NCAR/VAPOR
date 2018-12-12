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
    Histo(int numberBins, float mnData, float mxData, string var, int ts);
    Histo(const Histo *histo);
#ifdef VAPOR3_0_0_ALPHA
    // Special constructor for unsigned char data:
    //
    Histo(const VAPoR::StructuredGrid *rg, const double exts[6], const float range[2]);
#endif
    ~Histo();
    void  reset(int newNumBins = -1);
    void  reset(int newNumBins, float mnData, float mxData);
    void  addToBin(float val);
    int   getMaxBinSize();
    int   getBinSize(int posn) { return _binArray[posn]; }
    float getMinData() { return _minData; }
    float getMaxData() { return _maxData; }

    int    getTimestepOfUpdate() { return _timestepOfUpdate; }
    string getVarnameOfUpdate() { return _varnameOfUpdate; }

private:
    Histo() {}
    long *_binArray;
    long  _numBelow, _numAbove;
    int   _numBins;
    float _minData, _maxData, _range;
    long  _maxBinSize;

    int    _timestepOfUpdate;
    string _varnameOfUpdate;
};

#endif    // HISTO_H
