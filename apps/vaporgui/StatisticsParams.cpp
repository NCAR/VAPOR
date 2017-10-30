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

const string StatisticsParams::_minTSTag = "MinTS";
const string StatisticsParams::_maxTSTag = "MaxTS";
const string StatisticsParams::_autoUpdateTag = "AutoUpdate";
const string StatisticsParams::_minEnabledTag = "MinEnabled";
const string StatisticsParams::_maxEnabledTag = "MaxEnabled";
const string StatisticsParams::_meanEnabledTag = "MeanEnabled";
const string StatisticsParams::_medianEnabledTag = "MedianEnabled";
const string StatisticsParams::_stdDevEnabledTag = "StdDevEnabled";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<StatisticsParams> registrar(StatisticsParams::GetClassType());

StatisticsParams::StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave)
    : RenderParams(dmgr, ssave, StatisticsParams::GetClassType()) {}

StatisticsParams::StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node)
    : RenderParams(dmgr, ssave, node) {
    // If node isn't tagged correctly we correct the tag and reinitialize from scratch;
    //
    if (node->GetTag() != StatisticsParams::GetClassType()) {
        node->SetTag(StatisticsParams::GetClassType());
    }
}

StatisticsParams::~StatisticsParams() {
    MyBase::SetDiagMsg("StatisticsParams::~StatisticsParams() this=%p", this);
}

bool StatisticsParams::GetAutoUpdate() {
    return (GetValueLong(_autoUpdateTag, (long)false));
}

void StatisticsParams::SetAutoUpdate(bool val) {
    SetValueLong(_autoUpdateTag, "if we want stats auto-update", (long)val);
}

int StatisticsParams::GetMinTS() {
    return (int)(GetValueDouble(_minTSTag, 0.0));
}

void StatisticsParams::SetMinTS(int ts) {
    SetValueDouble(_minTSTag, "Minimum selected timestep for statistics", (double)ts);
}

int StatisticsParams::GetMaxTS() {
    return (double)(GetValueDouble(_maxTSTag, 0.0));
}

void StatisticsParams::SetMaxTS(int ts) {
    SetValueDouble(_maxTSTag, "Maximum selected timestep for statistics", (double)ts);
}

bool StatisticsParams::GetMinEnabled() {
    return GetValueLong(_minEnabledTag, (long)true);
}

void StatisticsParams::SetMinEnabled(bool state) {
    SetValueLong(_minEnabledTag, "Minimum statistic calculation", (long)state);
}

bool StatisticsParams::GetMaxEnabled() {
    return GetValueLong(_maxEnabledTag, (long)true);
}

void StatisticsParams::SetMaxEnabled(bool state) {
    SetValueLong(_maxEnabledTag, "Maximum statistic calculation", (long)state);
}

bool StatisticsParams::GetMeanEnabled() {
    return GetValueLong(_meanEnabledTag, (long)true);
}

void StatisticsParams::SetMeanEnabled(bool state) {
    SetValueLong(_meanEnabledTag, "Mean statistic calculation", (long)state);
}

bool StatisticsParams::GetMedianEnabled() {
    return GetValueLong(_medianEnabledTag, (long)false);
}

void StatisticsParams::SetMedianEnabled(bool state) {
    SetValueLong(_medianEnabledTag, "Median statistic calculation", (long)state);
}

bool StatisticsParams::GetStdDevEnabled() {
    return GetValueLong(_stdDevEnabledTag, (long)false);
}

void StatisticsParams::SetStdDevEnabled(bool state) {
    SetValueLong(_stdDevEnabledTag, "Standard deviation statistic calculation", (long)state);
}
