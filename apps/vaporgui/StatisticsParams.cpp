//************************************************************************
//                                  *
//           Copyright (C)  2014                *
//   University Corporation for Atmospheric Research            *
//           All Rights Reserved                *
//                                  *
//************************************************************************/
//
//  File:       StatisticsParams.cpp
//
//  Author:     Scott Pearse
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       August 2017
//
//  Description:    Implements the StatisticsParams class.
//
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <string>
#include <cassert>
#include <StatisticsParams.h>

using namespace VAPoR;

//const string StatisticsParams::_varsTag = "Vars";
//const string StatisticsParams::_vars3dTag = "Vars3d";
//const string StatisticsParams::_statisticsTag = "Statistics";
const string StatisticsParams::_dataSourceTag = "DataSource";
//const string StatisticsParams::_refinementTag = "Refinement";
//const string StatisticsParams::_cRatioTag = "Lod";
const string StatisticsParams::_minTSTag = "MinTS";
const string StatisticsParams::_maxTSTag = "MaxTS";
const string StatisticsParams::_autoUpdateTag = "AutoUpdate";
//const string StatisticsParams::_minExtentsTag = "MinExtents";
//const string StatisticsParams::_maxExtentsTag = "MaxExtents";
//const string StatisticsParams::_regionSelectTag = "RegionSelect";
const string StatisticsParams::_minEnabledTag = "MinEnabled";
const string StatisticsParams::_maxEnabledTag = "MaxEnabled";
const string StatisticsParams::_meanEnabledTag = "MeanEnabled";
const string StatisticsParams::_medianEnabledTag = "MedianEnabled";
const string StatisticsParams::_stdDevEnabledTag = "StdDevEnabled";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<StatisticsParams> registrar( StatisticsParams::GetClassType() );

StatisticsParams::StatisticsParams( DataMgr* dmgr, ParamsBase::StateSave *ssave) 
                : RenderParams( dmgr, ssave, StatisticsParams::GetClassType()) 
{ }

StatisticsParams::StatisticsParams( DataMgr* dmgr, ParamsBase::StateSave *ssave, XmlNode *node) 
                : RenderParams( dmgr, ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //  
    if (node->GetTag() != StatisticsParams::GetClassType()) {
        node->SetTag(StatisticsParams::GetClassType());
    }  
}

StatisticsParams::~StatisticsParams() 
{
    MyBase::SetDiagMsg("StatisticsParams::~StatisticsParams() this=%p",this);
}

bool StatisticsParams::GetAutoUpdate()
{
    return(GetValueLong(_autoUpdateTag, (long)true));
}

void StatisticsParams::SetAutoUpdate(bool val) 
{
    SetValueLong( _autoUpdateTag, "if we want stats auto-update", (long)val );
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
    return (int)(GetValueDouble(_minTSTag, 0.0));
}

void StatisticsParams::SetMinTS(int ts) 
{
    SetValueDouble(_minTSTag, "Minimum selected timestep for statistics", (double)ts);
}

int StatisticsParams::GetMaxTS() 
{
    return (double)(GetValueDouble(_maxTSTag, 0.0));
}

void StatisticsParams::SetMaxTS(int ts) 
{
    SetValueDouble(_maxTSTag, "Maximum selected timestep for statistics", (double)ts);
}

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

bool StatisticsParams::GetMinEnabled() 
{
	return GetValueLong(_minEnabledTag, (long)true);
}

void StatisticsParams::SetMinEnabled(bool state) 
{
    SetValueLong(_minEnabledTag, "Minimum statistic calculation", (long)state);
}

bool StatisticsParams::GetMaxEnabled() 
{
    return GetValueLong(_maxEnabledTag, (long)true);
}

void StatisticsParams::SetMaxEnabled(bool state) 
{
    SetValueLong(_maxEnabledTag, "Maximum statistic calculation", (long)state);
}

bool StatisticsParams::GetMeanEnabled() 
{
    return GetValueLong(_meanEnabledTag, (long)true); 
}

void StatisticsParams::SetMeanEnabled(bool state) 
{
    SetValueLong(_meanEnabledTag, "Mean statistic calculation", (long)state);
}

bool StatisticsParams::GetMedianEnabled() 
{
    return GetValueLong(_medianEnabledTag, (long)true);
}

void StatisticsParams::SetMedianEnabled(bool state) 
{
    SetValueLong(_medianEnabledTag, "Median statistic calculation", (long)state);
}

bool StatisticsParams::GetStdDevEnabled() 
{
    return GetValueLong(_stdDevEnabledTag, (long)true);
}

void StatisticsParams::SetStdDevEnabled(bool state) 
{
    SetValueLong(_stdDevEnabledTag, "Standard deviation statistic calculation", (long)state);
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
