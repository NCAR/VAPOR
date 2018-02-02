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

const string PlotParams::_minTSTag = "MinTS";
const string PlotParams::_maxTSTag = "MaxTS";
const string PlotParams::_spaceTimeTag = "SpaceTime";

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

int PlotParams::GetMinTS() const
{
    assert(!this->GetSpaceTimeMode());    // make sure we're at "time" mode
    return (int)(GetValueDouble(_minTSTag, 0.0));
}

void PlotParams::SetMinTS(int ts)
{
    assert(!this->GetSpaceTimeMode());    // make sure we're at "time" mode
    SetValueDouble(_minTSTag, "Minimum timestep set", (double)ts);
}

int PlotParams::GetMaxTS() const
{
    assert(!this->GetSpaceTimeMode());    // make sure we're at "time" mode
    return (int)(GetValueDouble(_maxTSTag, 0.0));
}

void PlotParams::SetMaxTS(int ts)
{
    assert(!this->GetSpaceTimeMode());    // make sure we're at "time" mode
    SetValueDouble(_maxTSTag, "Maximum timestep set", (double)ts);
}

bool PlotParams::GetSpaceTimeMode() const { return GetValueLong(_spaceTimeTag, (long)true); }

void PlotParams::SetSpaceTimeMode(bool val) { SetValueLong(_spaceTimeTag, "Set Space or Time mode", (long)val); }
