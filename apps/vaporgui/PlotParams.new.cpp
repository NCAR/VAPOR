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
#include <PlotParams.h>

using namespace VAPoR;

const string PlotParams::_minMaxTSTag = "MinMaxTS";
// const string PlotParams::_maxTSTag = "MaxTS";
const string PlotParams::_spaceTimeTag = "SpaceTime";

const string PlotParams::_p1Tag = "Point2";
const string PlotParams::_p2Tag = "Point1";
const string PlotParams::_singlePtTag = "SinglePoint";

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

std::vector<long int> PlotParams::GetMinMaxTS() const
{
    // assert( !this->GetSpaceTimeMode() );     // make sure we're at "time" mode

    std::vector<long int> vec(2, 0);
    return GetValueLongVec(_minMaxTSTag, vec);
}

void PlotParams::SetMinMaxTS(const std::vector<long int> &minmax)
{
    // assert( !this->GetSpaceTimeMode() );     // make sure we're at "time" mode
    SetValueLongVec(_minMaxTSTag, "Time range in the Time mode", minmax);
}

bool PlotParams::GetSpaceTimeMode() const { return GetValueLong(_spaceTimeTag, (long)true); }

void PlotParams::SetSpaceTimeMode(bool val) { SetValueLong(_spaceTimeTag, "Set Space or Time mode", (long)val); }

std::vector<double> PlotParams::GetSinglePoint() const
{
    // assert( !this->GetSpaceTimeMode() );     // make sure we're at "time" mode
    return GetValueDoubleVec(_singlePtTag);
}

void PlotParams::SetSinglePoint(const std::vector<double> &point)
{
    // assert( !this->GetSpaceTimeMode() );     // make sure we're at "time" mode
    SetValueDoubleVec(_singlePtTag, "Single point in the time mode", point);
}

std::vector<double> PlotParams::GetPoint1() const
{
    // assert( this->GetSpaceTimeMode() );     // make sure we're at "space" mode
    return GetValueDoubleVec(_p1Tag);
}

void PlotParams::SetPoint1(const std::vector<double> &point)
{
    // assert( this->GetSpaceTimeMode() );     // make sure we're at "space" mode
    SetValueDoubleVec(_p1Tag, "Point 1 in the space mode", point);
}

std::vector<double> PlotParams::GetPoint2() const
{
    // assert( this->GetSpaceTimeMode() );     // make sure we're at "space" mode
    return GetValueDoubleVec(_p2Tag);
}

void PlotParams::SetPoint2(const std::vector<double> &point)
{
    // assert( this->GetSpaceTimeMode() );     // make sure we're at "space" mode
    SetValueDoubleVec(_p2Tag, "Point 2 in the space mode", point);
}
