//************************************************************************
//                                                                       *
//           Copyright (C)  2014                                         *
//   University Corporation for Atmospheric Research                     *
//           All Rights Reserved                                         *
//                                                                       *
//************************************************************************/
//
//  File:       PlotParams.cpp
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//
//  Description:    Implements the PlotParams class.
//
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include "PlotParams.h"

using namespace VAPoR;

const string PlotParams::_minMaxTSTag = "MinMaxTS";
const string PlotParams::_p1Tag = "Point2";
const string PlotParams::_p2Tag = "Point1";
const string PlotParams::_numSamplesTag = "NumberOfSamplesTag";
const string PlotParams::_singlePtTag = "SinglePoint";
const string PlotParams::_lockAxisTag = "LockAxis";
const string PlotParams::_minExtentTag = "MinExtentTag";
const string PlotParams::_maxExtentTag = "MaxExtentTag";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<PlotParams> registrar(PlotParams::GetClassType());

PlotParams::PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave) : RenderParams(dmgr, ssave, PlotParams::GetClassType()) {}

PlotParams::PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dmgr, ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize from scratch;
    //
    if (node->GetTag() != PlotParams::GetClassType()) node->SetTag(PlotParams::GetClassType());
}

PlotParams::~PlotParams() { MyBase::SetDiagMsg("PlotParams::~PlotParams() this=%p", this); }

std::vector<long int> PlotParams::GetMinMaxTS() const { return GetValueLongVec(_minMaxTSTag); }

void PlotParams::SetMinMaxTS(const std::vector<long int> &minmax) { SetValueLongVec(_minMaxTSTag, "Time range in the Time mode", minmax); }

std::vector<double> PlotParams::GetSinglePoint() const { return GetValueDoubleVec(_singlePtTag); }

void PlotParams::SetSinglePoint(const std::vector<double> &point) { SetValueDoubleVec(_singlePtTag, "Single point in the time mode", point); }

std::vector<double> PlotParams::GetPoint1() const { return GetValueDoubleVec(_p1Tag); }

void PlotParams::SetPoint1(const std::vector<double> &point) { SetValueDoubleVec(_p1Tag, "Point 1 in the space mode", point); }

std::vector<double> PlotParams::GetPoint2() const { return GetValueDoubleVec(_p2Tag); }

void PlotParams::SetPoint2(const std::vector<double> &point) { SetValueDoubleVec(_p2Tag, "Point 2 in the space mode", point); }

void PlotParams::SetNumOfSamples(long val) { SetValueLong(_numSamplesTag, "Set number of samples", (long)val); }

long PlotParams::GetNumOfSamples() const { return GetValueLong(_numSamplesTag, 100); }

void PlotParams::SetAxisLocks(const std::vector<bool> &locks)
{
    std::vector<long> locksL(3, 0);
    for (int i = 0; i < 3; i++) locksL[i] = (long int)locks[i];
    SetValueLongVec(_lockAxisTag, "Lock values along x, y, or z axes", locksL);
}

std::vector<bool> PlotParams::GetAxisLocks()
{
    std::vector<long> defaultVal(3, 0);
    std::vector<long> locksL = GetValueLongVec(_lockAxisTag, defaultVal);
    std::vector<bool> locks(3, false);
    for (int i = 0; i < 3; i++) locks[i] = (bool)locksL[i];

    return locks;
}

std::vector<double> PlotParams::GetMinExtents() const { return GetValueDoubleVec(_minExtentTag); }

void PlotParams::SetMinExtents(const std::vector<double> &point) { SetValueDoubleVec(_minExtentTag, "Minimal extent", point); }

std::vector<double> PlotParams::GetMaxExtents() const { return GetValueDoubleVec(_maxExtentTag); }

void PlotParams::SetMaxExtents(const std::vector<double> &point) { SetValueDoubleVec(_maxExtentTag, "Maximal Extent", point); }
