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
const string PlotParams::_spaceMinTSTag = "SpaceSpaceMinTS";
const string PlotParams::_spaceMaxTSTag = "SpaceMaxTS";
const string PlotParams::_spaceMinExtentsTag = "SpaceMinExtents";
const string PlotParams::_spaceMaxExtentsTag = "SpaceMaxExtents";
const string PlotParams::_spaceOrTimeTag = "SpaceOrTime";
const string PlotParams::_timePointTag = "TimePoint";
const string PlotParams::_timeMinTSTag = "TimeMinTS";
const string PlotParams::_timeXTag = "TimeX";
const string PlotParams::_timeYTag = "TimeY";
const string PlotParams::_timeZTag = "TimeZ";
const string PlotParams::_timeMaxTSTag = "TimeMaxTS";
const string PlotParams::_xConstTag = "XConst";
const string PlotParams::_yConstTag = "YConst";
const string PlotParams::_zConstTag = "ZConst";
const string PlotParams::_timeConstTag = "TimeConst";

//
// Register class with object factory!!!
//
static ParamsRegistrar<PlotParams> registrar(
    PlotParams::GetClassType());

PlotParams::PlotParams(
    ParamsBase::StateSave *ssave) : ParamsBase(ssave, PlotParams::GetClassType()) {
}

PlotParams::PlotParams(
    ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != PlotParams::GetClassType()) {
        node->SetTag(PlotParams::GetClassType());
    }
}

PlotParams::~PlotParams() {
    MyBase::SetDiagMsg("PlotParams::~PlotParams() this=%p", this);
}

int PlotParams::GetSpaceTS() const {
    double maxTS = GetValueDouble(_spaceMaxTSTag, 0.f);
    return (int)maxTS;
}

void PlotParams::SetSpaceTS(int ts) {
    cout << "Plot set " << endl;
    SetValueDouble(_spaceMaxTSTag,
                   "Maximum selected spatial timestep for plot", (double)ts);
}

vector<double> PlotParams::GetSpaceMinExtents() const {
    vector<double> extents = GetValueDoubleVec(_spaceMinExtentsTag);
    if (extents.size() == 0) {
        extents.push_back(1.);
        extents.push_back(1.);
        extents.push_back(1.);
    }
    return extents;
}

void PlotParams::SetSpaceMinExtents(vector<double> minExts) {
    cout << "Plot set " << endl;
    SetValueDoubleVec(_spaceMinExtentsTag, "Minimum spatial extents for plot",
                      minExts);
}

vector<double> PlotParams::GetSpaceMaxExtents() const {
    vector<double> extents = GetValueDoubleVec(_spaceMaxExtentsTag);
    if (extents.size() == 0) {
        extents.push_back(1.);
        extents.push_back(1.);
        extents.push_back(1.);
    }
    return extents;
}

void PlotParams::SetSpaceMaxExtents(vector<double> maxExts) {
    cout << "Plot set " << endl;
    SetValueDoubleVec(_spaceMaxExtentsTag, "Maximum spatial extents for plot",
                      maxExts);
}

vector<double> PlotParams::GetTimePoint() const {
    vector<double> point = GetValueDoubleVec(_timePointTag);
    if (point.size() == 0) {
        point.push_back(0);
        point.push_back(0);
        point.push_back(0);
    }
    return point;
}

void PlotParams::SetTimePoint(vector<double> extents) {
    cout << "Plot set " << endl;
    SetValueDoubleVec(_timePointTag, "Spatial extents for time plotting",
                      extents);
}

int PlotParams::GetCRatio() const {
    int cRatio = (int)GetValueDouble(_cRatioTag, -1);
    return cRatio;
}

void PlotParams::SetCRatio(int cRatio) {
    cout << "Plot set " << endl;
    SetValueDouble(_cRatioTag, "Compression ratio for plot",
                   cRatio);
}

int PlotParams::GetRefinement() const {
    int refinement = (int)GetValueDouble(_refinementTag, -1);
    return refinement;
}

void PlotParams::SetRefinement(int ref) {
    cout << "Plot set " << endl;
    SetValueDouble(_refinementTag, "Refinement level for plot",
                   ref);
}

vector<string> PlotParams::GetVarNames() const {
    vector<string> varNames = GetValueStringVec(_varsTag);
    return varNames;
}

void PlotParams::SetVarNames(vector<string> varNames) {
    cout << "Plot set " << endl;
    SetValueStringVec(_varsTag, "Variable names selected for plot",
                      varNames);
}

void PlotParams::SetSpaceOrTime(string state) {
    cout << "Plot set " << endl;
    SetValueString(_spaceOrTimeTag, "Configure plots to show trends in "
                                    "space or time",
                   state);
}

string PlotParams::GetSpaceOrTime() const {
    string state = GetValueString(_spaceOrTimeTag, "space");
    return state;
}

int PlotParams::GetTimeMinTS() const {
    int ts = (int)GetValueDouble(_timeMinTSTag, 0);
    return ts;
}

void PlotParams::SetTimeMinTS(int time) {
    cout << "Plot set " << endl;
    SetValueDouble(_timeMinTSTag, "Minimum timestep for temporal plot",
                   time);
}
int PlotParams::GetTimeMaxTS() const {
    int ts = (int)GetValueDouble(_timeMaxTSTag, 0);
    return ts;
}

void PlotParams::SetTimeMaxTS(int time) {
    cout << "Plot set " << endl;
    SetValueDouble(_timeMaxTSTag, "Maximum timestep for temporal plot",
                   time);
}

double PlotParams::GetTimeXCoord() const {
    double coord = GetValueDouble(_timeXTag, 0.f);
    return coord;
}

void PlotParams::SetTimeXCoord(double coord) {
    cout << "Plot set " << endl;
    SetValueDouble(_timeXTag, "X coordinate for temporal plot", coord);
}

double PlotParams::GetTimeYCoord() const {
    double coord = GetValueDouble(_timeYTag, 0.f);
    return coord;
}

void PlotParams::SetTimeYCoord(double coord) {
    cout << "Plot set " << endl;
    SetValueDouble(_timeYTag, "Y coordinate for temporal plot", coord);
}

double PlotParams::GetTimeZCoord() const {
    double coord = GetValueDouble(_timeZTag, 0.f);
    return coord;
}

void PlotParams::SetTimeZCoord(double coord) {
    cout << "Plot set " << endl;
    SetValueDouble(_timeZTag, "Z coordinate for temporal plot", coord);
}

bool PlotParams::GetXConst() const {
    string sState = GetValueString(_xConstTag, "false");
    bool state = (sState == "true") ? true : false;
    return state;
}

void PlotParams::SetXConst(bool state) {
    cout << "Plot set " << endl;
    string sState = state ? "true" : "false";
    SetValueString(_xConstTag, "Variable for keeping the min/max X coordinate "
                               "constant",
                   sState);
}

bool PlotParams::GetYConst() const {
    string sState = GetValueString(_yConstTag, "false");
    bool state = (sState == "true") ? true : false;
    return state;
}

void PlotParams::SetYConst(bool state) {
    cout << "Plot set " << endl;
    string sState = state ? "true" : "false";
    SetValueString(_yConstTag, "Variable for keeping the min/max Y coordinate "
                               "constant",
                   sState);
}

bool PlotParams::GetZConst() const {
    string sState = GetValueString(_zConstTag, "false");
    bool state = (sState == "true") ? true : false;
    return state;
}

void PlotParams::SetZConst(bool state) {
    cout << "Plot set " << endl;
    string sState = state ? "true" : "false";
    SetValueString(_zConstTag, "Variable for keeping the min/max Z coordinate "
                               "constant",
                   sState);
}

bool PlotParams::GetTimeConst() const {
    string sState = GetValueString(_timeConstTag, "false");
    bool state = (sState == "true") ? true : false;
    return state;
}

void PlotParams::SetTimeConst(bool state) {
    cout << "Plot set " << endl;
    string sState = state ? "true" : "false";
    SetValueString(_timeConstTag, "Variable for keeping the min/max time "
                                  "coordinate constant",
                   sState);
}
