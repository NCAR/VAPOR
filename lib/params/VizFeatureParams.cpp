//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		VizFeatureParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the VizFeatureParams class.
//		This class supports parameters associted with the
//		visualizer features in the vizfeatures panel
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning(disable : 4100)
#endif

#include <vector>
#include <string>
#include <iostream>

#include <vapor/VizFeatureParams.h>

using namespace VAPoR;

const string VizFeatureParams::_domainFrameTag = "DomainFrame";
const string VizFeatureParams::_domainColorTag = "DomainColor";
const string VizFeatureParams::_regionFrameTag = "RegionFrame";
const string VizFeatureParams::_regionColorTag = "RegionColor";
const string VizFeatureParams::_backgroundColorTag = "BackgroundColor";
const string VizFeatureParams::_axisAnnotationEnabledTag = "AxisAnnotation";
const string VizFeatureParams::_axisColorTag = "AxisColor";
const string VizFeatureParams::_axisDigitsTag = "AxisDigits";
const string VizFeatureParams::_axisTextHeightTag = "AxisTextHeight";
const string VizFeatureParams::_ticWidthTag = "TicWidths";
const string VizFeatureParams::_ticDirsTag = "TicDirections";
const string VizFeatureParams::_ticSizeTag = "TicSizes";
const string VizFeatureParams::_minTicsTag = "TicMinPositions";
const string VizFeatureParams::_maxTicsTag = "TicMaxPositions";
const string VizFeatureParams::_numTicsTag = "NumberTics";
const string VizFeatureParams::_latLonAxesTag = "LatLonAxes";
const string VizFeatureParams::_axisOriginTag = "NumberTics";
const string VizFeatureParams::_showAxisArrowsTag = "ShowAxisArrows";
const string VizFeatureParams::_axisArrowCoordsTag = "AxisArrowCoords";
vector<double> VizFeatureParams::_previousStretch;

//
// Register class with object factory!!!
//
static ParamsRegistrar<VizFeatureParams> registrar(VizFeatureParams::GetClassType());

VizFeatureParams::VizFeatureParams(
    ParamsBase::StateSave *ssave) : ParamsBase(ssave, VizFeatureParams::GetClassType()) {

    _init();
}

VizFeatureParams::VizFeatureParams(
    ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != VizFeatureParams::GetClassType()) {
        node->SetTag(VizFeatureParams::GetClassType());
        _init();
    }
}

//Reset vizfeature settings to initial state
void VizFeatureParams::_init() {

    vector<double> clr(3, 1.0);
    SetDomainColor(clr);
    SetAxisColor(clr);
    clr[1] = 0.;
    clr[2] = 0.;
    SetRegionColor(clr);
    clr[0] = 0.;
    SetBackgroundColor(clr);
    SetUseRegionFrame(false);
    SetUseDomainFrame(true);
    SetAxisAnnotation(false);
    vector<long> lvec(3, 0);
    vector<double> dvec(3, 0.0);
    SetMinTics(dvec);
    SetMaxTics(dvec);
    SetAxisArrowCoords(dvec);
    SetNumTics(lvec);
    SetTicSize(dvec);
    lvec[0] = 1;
    SetTicDirs(lvec);
    SetAxisTextHeight(10);
    SetAxisOrigin(dvec);
    SetLatLonAxes(false);
    SetTicWidth(1.);
    SetAxisDigits(4);
    SetShowAxisArrows(false);
}

void VizFeatureParams::m_getColor(double color[3], string tag) const {

    vector<double> defaultv(3, 1.0);
    vector<double> val = GetValueDoubleVec(tag, defaultv);
    for (int i = 0; i < val.size(); i++) {
        color[i] = val[i];
        if (color[i] < 0.0)
            color[i] = 0.0;
        if (color[i] > 1.0)
            color[i] = 1.0;
    }
}

void VizFeatureParams::m_setColor(vector<double> color, string tag, string msg) {

    assert(color.size() == 3);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0)
            color[i] = 0.0;
        if (color[i] > 1.0)
            color[i] = 1.0;
    }
    SetValueDoubleVec(tag, msg, color);
}

void VizFeatureParams::GetDomainColor(double color[3]) const {
    m_getColor(color, _domainColorTag);
}

void VizFeatureParams::SetDomainColor(vector<double> color) {
    m_setColor(color, _domainColorTag, "Set domain frame color");
}

void VizFeatureParams::GetRegionColor(double color[3]) const {
    m_getColor(color, _regionColorTag);
}

void VizFeatureParams::SetRegionColor(vector<double> color) {
    m_setColor(color, _regionColorTag, "Set region frame color");
}

void VizFeatureParams::GetBackgroundColor(double color[3]) const {
    m_getColor(color, _backgroundColorTag);
}

void VizFeatureParams::SetBackgroundColor(vector<double> color) {
    m_setColor(color, _backgroundColorTag, "Set background color");
}

void VizFeatureParams::GetAxisColor(double color[3]) const {
    m_getColor(color, _axisColorTag);
}

void VizFeatureParams::SetAxisColor(vector<double> color) {
    m_setColor(color, _axisColorTag, "Set axis color");
}

void VizFeatureParams::SetNumTics(vector<long> val) {
    assert(val.size() == 3);
    for (int i = 0; i < val.size(); i++) {
        if (val[i] < 0)
            val[i] = 0;
        if (val[i] > 100)
            val[i] = 100;
    }

    SetValueLongVec(_numTicsTag, "Set number of axis tics", val);
}

vector<long> VizFeatureParams::GetNumTics() const {
    vector<long> defaultv(3, 6);
    vector<long> val = GetValueLongVec(_numTicsTag, defaultv);

    for (int i = 0; i < val.size(); i++) {
        if (val[i] < 0)
            val[i] = 0;
        if (val[i] > 100)
            val[i] = 100;
    }
    return (val);
}

void VizFeatureParams::SetAxisOrigin(vector<double> val) {
    assert(val.size() == 3);
    SetValueDoubleVec(_axisOriginTag, "Set axis val", val);
}

vector<double> VizFeatureParams::GetAxisOrigin() const {
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_axisOriginTag, defaultv);
}

void VizFeatureParams::SetMinTics(vector<double> val) {
    assert(val.size() == 3);
    SetValueDoubleVec(_minTicsTag, "Set minimum tics", val);
}

vector<double> VizFeatureParams::GetMinTics() const {
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_minTicsTag, defaultv);
}

void VizFeatureParams::SetMaxTics(vector<double> val) {
    assert(val.size() == 3);
    SetValueDoubleVec(_maxTicsTag, "Set maximum tics", val);
}

vector<double> VizFeatureParams::GetMaxTics() const {
    vector<double> defaultv(3, 1.0);
    return GetValueDoubleVec(_maxTicsTag, defaultv);
}

void VizFeatureParams::SetTicSize(vector<double> val) {
    assert(val.size() == 3);
    SetValueDoubleVec(_ticSizeTag, "Set tic sizes", val);
}

vector<double> VizFeatureParams::GetTicSize() const {
    vector<double> defaultv(3, 0.05);
    return GetValueDoubleVec(_ticSizeTag, defaultv);
}

void VizFeatureParams::SetTicDirs(vector<long> val) {
    assert(val.size() == 3);
    SetValueLongVec(_ticDirsTag, "Set tic direction", val);
}

vector<long> VizFeatureParams::GetTicDirs() const {
    vector<long> defaultv(3, 0);
    defaultv[0] = 1;
    return GetValueLongVec(_ticDirsTag, defaultv);
}

void VizFeatureParams::SetAxisArrowCoords(vector<double> val) {
    assert(val.size() == 3);
    SetValueDoubleVec(_axisArrowCoordsTag, "Set axis arrow coords", val);
}

vector<double> VizFeatureParams::GetAxisArrowCoords() const {
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_axisArrowCoordsTag, defaultv);
}

#ifdef DEAD
void VizFeatureParams::changeStretch(vector<double> prevStretch, vector<double> newStretch) {

    _dataStatus->stretchExtents(newStretch);
    //Determine the relative scale:
    vector<double> scalefactor;
    for (int i = 0; i < 3; i++)
        scalefactor.push_back(newStretch[i] / prevStretch[i]);
    //Make all viewpoint params rescale
    ViewpointParams *p = (ViewpointParams *)_paramsMgr->GetDefaultParams(_viewpointParamsTag);
    p->rescale(scalefactor);
    vector<Params *> vpparams;
    _paramsMgr->GetAllParamsInstances(_viewpointParamsTag, vpparams);
    for (int j = 0; j < vpparams.size(); j++) {
        ((ViewpointParams *)vpparams[j])->rescale(scalefactor);
    }
}

#endif
