//************************************************************************
//									*
//			 Copyright (C)  2014				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		StatisticsParams.cpp
//
//	Author:		Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2017
//
//	Description:	Implements the StatisticsParams class.
//
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <string>
#include <cassert>
#include <StatisticsParams.h>

using namespace VAPoR;

// const string StatisticsParams::_varsTag = "Vars";
// const string StatisticsParams::_vars3dTag = "Vars3d";
const string StatisticsParams::_statisticsTag = "Statistics";
const string StatisticsParams::_dataSourceTag = "DataSource";
// const string StatisticsParams::_refinementTag = "Refinement";
// const string StatisticsParams::_cRatioTag = "Lod";
const string StatisticsParams::_minTSTag = "MinTS";
const string StatisticsParams::_maxTSTag = "MaxTS";
const string StatisticsParams::_autoUpdateTag = "AutoUpdate";
// const string StatisticsParams::_minExtentsTag = "MinExtents";
// const string StatisticsParams::_maxExtentsTag = "MaxExtents";
// const string StatisticsParams::_regionSelectTag = "RegionSelect";
const string StatisticsParams::_minStatTag = "MinStat";
const string StatisticsParams::_maxStatTag = "MaxStat";
const string StatisticsParams::_meanStatTag = "MeanStat";
const string StatisticsParams::_medianStatTag = "MedianStat";
const string StatisticsParams::_stdDevStatTag = "StdDevStat";

//
// Register class with object factory!!!
//
static ParamsRegistrar<StatisticsParams> registrar(StatisticsParams::GetClassType());

StatisticsParams::StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave) : RenderParams(dmgr, ssave, StatisticsParams::GetClassType()) {}

StatisticsParams::StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dmgr, ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != StatisticsParams::GetClassType()) { node->SetTag(StatisticsParams::GetClassType()); }
}

StatisticsParams::~StatisticsParams() { MyBase::SetDiagMsg("StatisticsParams::~StatisticsParams() this=%p", this); }

bool StatisticsParams::GetAutoUpdate()
{
    string state = GetValueString(_autoUpdateTag, "false");
    if (state == "true") {
        return true;
    } else {
        return false;
    }
}

void StatisticsParams::SetAutoUpdate(bool state)
{
    string sState = "false";
    if (state == true) { sState = "true"; }
    SetValueString(_autoUpdateTag, "State of statistics auto-update", sState);
}
/*
int StatisticsParams::GetRegionSelection() {
    double state = GetValueDouble(_regionSelectTag, 0.f);
    return (int)state;
}

void StatisticsParams::SetRegionSelection(int state) {
    SetValueDouble(_regionSelectTag,
        "State of statistics region selector", (double)state);
}
*/
int StatisticsParams::GetMinTS()
{
    double minTS = GetValueDouble(_minTSTag, 0.f);
    return (int)minTS;
}

void StatisticsParams::SetMinTS(int ts) { SetValueDouble(_minTSTag, "Minimum selected timestep for statistics", (double)ts); }

int StatisticsParams::GetMaxTS()
{
    double maxTS = GetValueDouble(_maxTSTag, 0.f);
    return (int)maxTS;
}

void StatisticsParams::SetMaxTS(int ts) { SetValueDouble(_maxTSTag, "Maximum selected timestep for statistics", (double)ts); }

/*
vector<double> StatisticsParams::GetMinExtents() {
    vector<double> extents = GetValueDoubleVec(_minExtentsTag);
    return extents;
}

void StatisticsParams::SetMinExtents(vector<double> minExts) {
    SetValueDoubleVec(_minExtentsTag, "Minimum extents for statistics",
        minExts);
}

vector<double> StatisticsParams::GetMaxExtents() {
    vector<double> extents = GetValueDoubleVec(_maxExtentsTag);
    return extents;
}

void StatisticsParams::SetMaxExtents(vector<double> maxExts) {
    SetValueDoubleVec(_maxExtentsTag, "Maximum extents for statistics",
        maxExts);
}

int StatisticsParams::GetCRatio() {
    int cRatio = (int)GetValueDouble(_cRatioTag, 0);
    return cRatio;
}

void StatisticsParams::SetCRatio(int cRatio) {
    SetValueDouble(_cRatioTag, "Compression ratio for statistics",
        cRatio);
}

int StatisticsParams::GetRefinement() {
    int refinement = (int)GetValueDouble(_refinementTag, 0);
    return refinement;
}

void StatisticsParams::SetRefinement(int ref) {
    SetValueDouble(_refinementTag, "Refinement level for statistics",
        ref);
}
*/

bool StatisticsParams::GetMinStat()
{
    if (GetValueString(_minStatTag, "true") == "false") {
        return false;
    } else {
        return true;
    }
}

void StatisticsParams::SetMinStat(bool state)
{
    string s = state ? "true" : "false";
    SetValueString(_minStatTag, "Minimum statistic calculation", s);
}

bool StatisticsParams::GetMaxStat()
{
    if (GetValueString(_maxStatTag, "true") == "false") {
        return false;
    } else {
        return true;
    }
}

void StatisticsParams::SetMaxStat(bool state)
{
    string s = state ? "true" : "false";
    SetValueString(_maxStatTag, "Maximum statistic calculation", s);
}

bool StatisticsParams::GetMeanStat()
{
    if (GetValueString(_meanStatTag, "true") == "false") {
        return false;
    } else {
        return true;
    }
}

void StatisticsParams::SetMeanStat(bool state)
{
    string s = state ? "true" : "false";
    SetValueString(_meanStatTag, "Mean statistic calculation", s);
}

bool StatisticsParams::GetMedianStat()
{
    if (GetValueString(_medianStatTag, "false") == "false") {
        return false;
    } else {
        return true;
    }
}

void StatisticsParams::SetMedianStat(bool state)
{
    string s = state ? "true" : "false";
    SetValueString(_medianStatTag, "Median statistic calculation", s);
}

bool StatisticsParams::GetStdDevStat()
{
    if (GetValueString(_stdDevStatTag, "false") == "false") {
        return false;
    } else {
        return true;
    }
}

void StatisticsParams::SetStdDevStat(bool state)
{
    string s = state ? "true" : "false";
    SetValueString(_stdDevStatTag, "Standard deviation statistic calculation", s);
}

/*
vector<string> StatisticsParams::GetVarNames() {
    vector<string> varNames = GetValueStringVec(_varsTag);
    return varNames;
}

void StatisticsParams::SetVarNames(vector<string> varNames) {
    SetValueStringVec(_varsTag, "Variable names selected for statistics",
        varNames);
}
*/
