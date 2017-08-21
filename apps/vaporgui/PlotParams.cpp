//************************************************************************
//									*
//			 Copyright (C)  2014				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		PlotParams.cpp
//
//	Author:		Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2017
//
//	Description:	Implements the PlotParams class.
//
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <string>
#include <cassert>
#include <PlotParams.h>

using namespace VAPoR;

const string PlotParams::_varsTag = "Vars";
const string PlotParams::_vars3dTag = "Vars3d";
const string PlotParams::_dataSourceTag = "DataSource";
const string PlotParams::_refinementTag = "Refinement";
const string PlotParams::_cRatioTag = "Lod";
const string PlotParams::_minTSTag = "MinTS";
const string PlotParams::_maxTSTag = "MaxTS";
const string PlotParams::_spaceOrTimeTag = "SpaceOrTime";
const string PlotParams::_minExtentsTag = "MinExtents";
const string PlotParams::_maxExtentsTag = "MaxExtents";

//
// Register class with object factory!!!
//
static ParamsRegistrar<PlotParams> registrar(PlotParams::GetClassType());

PlotParams::PlotParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, PlotParams::GetClassType()) {}

PlotParams::PlotParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != PlotParams::GetClassType()) { node->SetTag(PlotParams::GetClassType()); }
}

PlotParams::~PlotParams() { MyBase::SetDiagMsg("PlotParams::~PlotParams() this=%p", this); }

int PlotParams::GetMinTS()
{
    double minTS = GetValueDouble(_minTSTag, 0.f);
    return (int)minTS;
}

void PlotParams::SetMinTS(int ts) { SetValueDouble(_minTSTag, "Minimum selected timestep for plot", (double)ts); }

int PlotParams::GetMaxTS()
{
    double maxTS = GetValueDouble(_maxTSTag, 0.f);
    return (int)maxTS;
}

void PlotParams::SetMaxTS(int ts) { SetValueDouble(_maxTSTag, "Maximum selected timestep for plot", (double)ts); }

vector<double> PlotParams::GetMinExtents()
{
    vector<double> extents = GetValueDoubleVec(_minExtentsTag);
    return extents;
}

void PlotParams::SetMinExtents(vector<double> minExts) { SetValueDoubleVec(_minExtentsTag, "Minimum extents for plot", minExts); }

vector<double> PlotParams::GetMaxExtents()
{
    vector<double> extents = GetValueDoubleVec(_maxExtentsTag);
    return extents;
}

void PlotParams::SetMaxExtents(vector<double> maxExts) { SetValueDoubleVec(_maxExtentsTag, "Maximum extents for plot", maxExts); }

int PlotParams::GetCRatio()
{
    int cRatio = (int)GetValueDouble(_cRatioTag, -1);
    return cRatio;
}

void PlotParams::SetCRatio(int cRatio) { SetValueDouble(_cRatioTag, "Compression ratio for plot", cRatio); }

int PlotParams::GetRefinement()
{
    int refinement = (int)GetValueDouble(_refinementTag, -1);
    return refinement;
}

void PlotParams::SetRefinement(int ref) { SetValueDouble(_refinementTag, "Refinement level for plot", ref); }

vector<string> PlotParams::GetVarNames()
{
    vector<string> varNames = GetValueStringVec(_varsTag);
    return varNames;
}

void PlotParams::SetVarNames(vector<string> varNames) { SetValueStringVec(_varsTag, "Variable names selected for plot", varNames); }

void PlotParams::SetSpaceOrTime(string state)
{
    SetValueString(_spaceOrTimeTag,
                   "Configure plots to show trends in "
                   "space or time",
                   state);
}

string PlotParams::GetSpaceOrTime()
{
    string state = GetValueString(_spaceOrTimeTag, "space");
    return state;
}
