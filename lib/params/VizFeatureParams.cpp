//************************************************************************
//									*
//			 Copyright (C)  2015				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
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
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <vector>
#include <string>
#include <iostream>

#include <vapor/VizFeatureParams.h>

using namespace VAPoR;

const string   VizFeatureParams::_domainFrameTag = "DomainFrame";
const string   VizFeatureParams::_domainColorTag = "DomainColor";
const string   VizFeatureParams::_regionFrameTag = "RegionFrame";
const string   VizFeatureParams::_regionColorTag = "RegionColor";
const string   VizFeatureParams::_backgroundColorTag = "BackgroundColor";
const string   VizFeatureParams::_axisAnnotationEnabledTag = "AxisAnnotationiEnabled";
const string   VizFeatureParams::_axisColorTag = "AxisColor";
const string   VizFeatureParams::_axisDigitsTag = "AxisDigits";
const string   VizFeatureParams::_axisTextHeightTag = "AxisTextHeight";
const string   VizFeatureParams::_axisFontSizeTag = "AxisFontSize";
const string   VizFeatureParams::_ticWidthTag = "TicWidths";
const string   VizFeatureParams::_ticDirsTag = "TicDirections";
const string   VizFeatureParams::_ticSizeTag = "TicSizes";
const string   VizFeatureParams::_minTicsTag = "TicMinPositions";
const string   VizFeatureParams::_maxTicsTag = "TicMaxPositions";
const string   VizFeatureParams::_numTicsTag = "NumberTics";
const string   VizFeatureParams::_currentAxisDataMgrTag = "AxisDataMgr";
const string   VizFeatureParams::_latLonAxesTag = "LatLonAxes";
const string   VizFeatureParams::_axisOriginTag = "AxisOrigin";
const string   VizFeatureParams::_showAxisArrowsTag = "ShowAxisArrows";
const string   VizFeatureParams::_axisArrowCoordsTag = "AxisArrowCoords";
const string   VizFeatureParams::_axisAnnotationsTag = "AxisAnnotations";
vector<double> VizFeatureParams::_previousStretch;

//
// Register class with object factory!!!
//
static ParamsRegistrar<VizFeatureParams> registrar(VizFeatureParams::GetClassType());

VizFeatureParams::VizFeatureParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, VizFeatureParams::GetClassType())
{
    _init();

    _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
    _axisAnnotations->SetParent(this);
}

VizFeatureParams::VizFeatureParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != VizFeatureParams::GetClassType()) {
        node->SetTag(VizFeatureParams::GetClassType());
        _init();
    }

    if (node->HasChild(_axisAnnotationsTag)) {
        _axisAnnotations = new ParamsContainer(ssave, node->GetChild(_axisAnnotationsTag));
    } else {
        _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
        _axisAnnotations->SetParent(this);
    }
}

VizFeatureParams::VizFeatureParams(const VizFeatureParams &rhs) : ParamsBase(rhs) { _axisAnnotations = new ParamsContainer(*(rhs._axisAnnotations)); }

// Reset vizfeature settings to initial state
void VizFeatureParams::_init()
{
    vector<double> clr(3, 1.0);
    SetDomainColor(clr);
    // SetAxisColor(clr);
    clr[1] = 0.;
    clr[2] = 0.;
    SetRegionColor(clr);
    clr[0] = 0.;
    SetBackgroundColor(clr);
    SetUseRegionFrame(false);
    SetUseDomainFrame(true);
    vector<double> dvec(3, 0.0);
    SetAxisArrowCoords(dvec);
    SetShowAxisArrows(false);
}

void VizFeatureParams::m_getColor(double color[3], string tag) const
{
    vector<double> defaultv(3, 1.0);
    vector<double> val = GetValueDoubleVec(tag, defaultv);
    for (int i = 0; i < val.size(); i++) {
        color[i] = val[i];
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
}

void VizFeatureParams::_getColor(vector<double> &color, string tag) const
{
    color.clear();

    vector<double> defaultv(3, 1.0);
    color = GetValueDoubleVec(tag, defaultv);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
}

void VizFeatureParams::m_setColor(vector<double> color, string tag, string msg)
{
    assert(color.size() == 3);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
    SetValueDoubleVec(tag, msg, color);
}

void VizFeatureParams::GetDomainColor(double color[3]) const { m_getColor(color, _domainColorTag); }

void VizFeatureParams::SetDomainColor(vector<double> color) { m_setColor(color, _domainColorTag, "Set domain frame color"); }

void VizFeatureParams::GetRegionColor(double color[3]) const { m_getColor(color, _regionColorTag); }

void VizFeatureParams::SetRegionColor(vector<double> color) { m_setColor(color, _regionColorTag, "Set region frame color"); }

void VizFeatureParams::GetBackgroundColor(double color[3]) const { m_getColor(color, _backgroundColorTag); }

void VizFeatureParams::SetBackgroundColor(vector<double> color) { m_setColor(color, _backgroundColorTag, "Set background color"); }

/*
void VizFeatureParams::SetAxisAnnotationEnabled(bool val){
    _currentAxisAnnotation->SetAxisAnnotationEnabled(val);
}
bool VizFeatureParams::GetAxisAnnotationEnabled() const{
    return _currentAxisAnnotation->GetAxisAnnotationEnabled();
}

std::vector<double> VizFeatureParams::GetAxisColor() const {
    return _currentAxisAnnotation->GetAxisColor();
}
void VizFeatureParams::SetAxisColor(vector<double> color) {
    _currentAxisAnnotation->SetAxisColor(color);
}

std::vector<double> VizFeatureParams::GetAxisBackgroundColor() const {
    return _currentAxisAnnotation->GetAxisBackgroundColor();
}
void VizFeatureParams::SetAxisBackgroundColor(vector<double> color) {
    _currentAxisAnnotation->SetAxisBackgroundColor(color);
}

void VizFeatureParams::SetNumTics(std::vector<double> numTics) {
    _currentAxisAnnotation->SetNumTics(numTics);
}
std::vector<double> VizFeatureParams::GetNumTics() const {
    return _currentAxisAnnotation->GetNumTics();
}


void VizFeatureParams::SetAxisOrigin(std::vector<double> orig) {
    _currentAxisAnnotation->SetAxisOrigin(orig);
}
std::vector<double> VizFeatureParams::GetAxisOrigin() const {
    return _currentAxisAnnotation->GetAxisOrigin();
}


void VizFeatureParams::SetMinTics(std::vector<double> minTics) {
    _currentAxisAnnotation->SetMinTics(minTics);
}
vector<double> VizFeatureParams::GetMinTics() const {
    return _currentAxisAnnotation->GetMinTics();
}


void VizFeatureParams::SetMaxTics(vector<double> maxTics) {
    _currentAxisAnnotation->SetMaxTics(maxTics);
}
vector<double> VizFeatureParams::GetMaxTics() const {
    return _currentAxisAnnotation->GetMaxTics();
}


void VizFeatureParams::SetTicSize(vector<double> ticSize) {
    _currentAxisAnnotation->SetTicSize(ticSize);
}
vector<double> VizFeatureParams::GetTicSize() const {
    return _currentAxisAnnotation->GetTicSize();
}


void VizFeatureParams::SetTicDirs(vector<double> ticDirs) {
    _currentAxisAnnotation->SetTicDirs(ticDirs);
}
vector<double> VizFeatureParams::GetTicDirs() const {
    return _currentAxisAnnotation->GetTicDirs();
}


double VizFeatureParams::GetTicWidth() const{
    return _currentAxisAnnotation->GetTicWidth();
}
void VizFeatureParams::SetTicWidth(double val){
    _currentAxisAnnotation->SetTicWidth(val);
}


long VizFeatureParams::GetAxisTextHeight() const{
    return _currentAxisAnnotation->GetAxisTextHeight();
}
void VizFeatureParams::SetAxisTextHeight(long val){
    _currentAxisAnnotation->SetAxisTextHeight(val);
}


long VizFeatureParams::GetAxisDigits() const{
    return _currentAxisAnnotation->GetAxisDigits();
}
void VizFeatureParams::SetAxisDigits(long val){
    _currentAxisAnnotation->SetAxisDigits(val);
}


void VizFeatureParams::SetLatLonAxesEnabled(bool val){
    _currentAxisAnnotation->SetLatLonAxesEnabled(val);
}
bool VizFeatureParams::GetLatLonAxesEnabled() const{
    return _currentAxisAnnotation->GetLatLonAxesEnabled();
}


void VizFeatureParams::SetAxisFontSize(int size) {
    SetValueDouble(_axisFontSizeTag, "Axis annotation font size", size);
}
int VizFeatureParams::GetAxisFontSize() {
    return (int)GetValueDouble(_axisFontSizeTag, 24);
}
*/

string VizFeatureParams::GetCurrentAxisDataMgrName() const { return GetValueString(_currentAxisDataMgrTag, ""); }
void   VizFeatureParams::SetCurrentAxisDataMgrName(string dmName)
{
    string msg = "Setting current DataMgr w.r.t. axis annotations";
    SetValueString(_currentAxisDataMgrTag, msg, dmName);
}

AxisAnnotation *VizFeatureParams::GetAxisAnnotation(string dataMgr)
{
    if (dataMgr == "") dataMgr = GetCurrentAxisDataMgrName();
    // assert(dataMgr!="");

    if (_axisAnnotations->GetParams(dataMgr) == NULL) {
        AxisAnnotation newAnnotation(_ssave);
        _axisAnnotations->Insert(&newAnnotation, dataMgr);
    }
    return (AxisAnnotation *)_axisAnnotations->GetParams(dataMgr);
}

void VizFeatureParams::SetShowAxisArrows(bool val) { SetValueLong(_showAxisArrowsTag, "Toggle Axis Arrows", val); }

bool VizFeatureParams::GetShowAxisArrows() const { return (0 != GetValueLong(_showAxisArrowsTag, (long)false)); }

void VizFeatureParams::SetAxisArrowCoords(vector<double> val)
{
    assert(val.size() == 3);
    SetValueDoubleVec(_axisArrowCoordsTag, "Set axis arrow coords", val);
}

vector<double> VizFeatureParams::GetAxisArrowCoords() const
{
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_axisArrowCoordsTag, defaultv);
}

#ifdef DEAD
void VizFeatureParams::changeStretch(vector<double> prevStretch, vector<double> newStretch)
{
    _dataStatus->stretchExtents(newStretch);
    // Determine the relative scale:
    vector<double> scalefactor;
    for (int i = 0; i < 3; i++) scalefactor.push_back(newStretch[i] / prevStretch[i]);
    // Make all viewpoint params rescale
    ViewpointParams *p = (ViewpointParams *)_paramsMgr->GetDefaultParams(_viewpointParamsTag);
    p->rescale(scalefactor);
    vector<Params *> vpparams;
    _paramsMgr->GetAllParamsInstances(_viewpointParamsTag, vpparams);
    for (int j = 0; j < vpparams.size(); j++) { ((ViewpointParams *)vpparams[j])->rescale(scalefactor); }
}

#endif
