//************************************************************************
//									*
//			 Copyright (C)  2015				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationParams.cpp
//
//	Author:	Scott Pearse
//			Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the AnnotationParams class.
//		This class supports parameters associted with the
//		visualizer features in the annotation panel
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <vector>
#include <string>
#include <iostream>

#include <vapor/AnnotationParams.h>

using namespace VAPoR;

const string   AnnotationParams::_domainFrameTag = "DomainFrame";
const string   AnnotationParams::_domainColorTag = "DomainColor";
const string   AnnotationParams::_regionFrameTag = "RegionFrame";
const string   AnnotationParams::_regionColorTag = "RegionColor";
const string   AnnotationParams::_backgroundColorTag = "BackgroundColor";
const string   AnnotationParams::_axisAnnotationEnabledTag = "AxisAnnotationiEnabled";
const string   AnnotationParams::_axisColorTag = "AxisColor";
const string   AnnotationParams::_axisDigitsTag = "AxisDigits";
const string   AnnotationParams::_axisTextHeightTag = "AxisTextHeight";
const string   AnnotationParams::_axisFontSizeTag = "AxisFontSize";
const string   AnnotationParams::_ticWidthTag = "TicWidths";
const string   AnnotationParams::_ticDirsTag = "TicDirections";
const string   AnnotationParams::_ticSizeTag = "TicSizes";
const string   AnnotationParams::_minTicsTag = "TicMinPositions";
const string   AnnotationParams::_maxTicsTag = "TicMaxPositions";
const string   AnnotationParams::_numTicsTag = "NumberTics";
const string   AnnotationParams::_currentAxisDataMgrTag = "AxisDataMgr";
const string   AnnotationParams::_latLonAxesTag = "LatLonAxes";
const string   AnnotationParams::_axisOriginTag = "AxisOrigin";
const string   AnnotationParams::_showAxisArrowsTag = "ShowAxisArrows";
const string   AnnotationParams::_axisArrowCoordsTag = "AxisArrowCoords";
const string   AnnotationParams::_axisAnnotationsTag = "AxisAnnotations";
vector<double> AnnotationParams::_previousStretch;

//
// Register class with object factory!!!
//
static ParamsRegistrar<AnnotationParams> registrar(AnnotationParams::GetClassType());

AnnotationParams::AnnotationParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, AnnotationParams::GetClassType())
{
    _init();

    _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
    _axisAnnotations->SetParent(this);
}

AnnotationParams::AnnotationParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != AnnotationParams::GetClassType()) {
        node->SetTag(AnnotationParams::GetClassType());
        _init();
    }

    if (node->HasChild(_axisAnnotationsTag)) {
        _axisAnnotations = new ParamsContainer(ssave, node->GetChild(_axisAnnotationsTag));
    } else {
        _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
        _axisAnnotations->SetParent(this);
    }
}

AnnotationParams::AnnotationParams(const AnnotationParams &rhs) : ParamsBase(rhs) { _axisAnnotations = new ParamsContainer(*(rhs._axisAnnotations)); }

// Reset vizfeature settings to initial state
void AnnotationParams::_init()
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

void AnnotationParams::m_getColor(double color[3], string tag) const
{
    vector<double> defaultv(3, 1.0);
    vector<double> val = GetValueDoubleVec(tag, defaultv);
    for (int i = 0; i < val.size(); i++) {
        color[i] = val[i];
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
}

void AnnotationParams::_getColor(vector<double> &color, string tag) const
{
    color.clear();

    vector<double> defaultv(3, 1.0);
    color = GetValueDoubleVec(tag, defaultv);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
}

void AnnotationParams::m_setColor(vector<double> color, string tag, string msg)
{
    assert(color.size() == 3);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
    SetValueDoubleVec(tag, msg, color);
}

void AnnotationParams::GetDomainColor(double color[3]) const { m_getColor(color, _domainColorTag); }

void AnnotationParams::SetDomainColor(vector<double> color) { m_setColor(color, _domainColorTag, "Set domain frame color"); }

void AnnotationParams::GetRegionColor(double color[3]) const { m_getColor(color, _regionColorTag); }

void AnnotationParams::SetRegionColor(vector<double> color) { m_setColor(color, _regionColorTag, "Set region frame color"); }

void AnnotationParams::GetBackgroundColor(double color[3]) const { m_getColor(color, _backgroundColorTag); }

void AnnotationParams::SetBackgroundColor(vector<double> color) { m_setColor(color, _backgroundColorTag, "Set background color"); }

/*
void AnnotationParams::SetAxisAnnotationEnabled(bool val){
    _currentAxisAnnotation->SetAxisAnnotationEnabled(val);
}
bool AnnotationParams::GetAxisAnnotationEnabled() const{
    return _currentAxisAnnotation->GetAxisAnnotationEnabled();
}

std::vector<double> AnnotationParams::GetAxisColor() const {
    return _currentAxisAnnotation->GetAxisColor();
}
void AnnotationParams::SetAxisColor(vector<double> color) {
    _currentAxisAnnotation->SetAxisColor(color);
}

std::vector<double> AnnotationParams::GetAxisBackgroundColor() const {
    return _currentAxisAnnotation->GetAxisBackgroundColor();
}
void AnnotationParams::SetAxisBackgroundColor(vector<double> color) {
    _currentAxisAnnotation->SetAxisBackgroundColor(color);
}

void AnnotationParams::SetNumTics(std::vector<double> numTics) {
    _currentAxisAnnotation->SetNumTics(numTics);
}
std::vector<double> AnnotationParams::GetNumTics() const {
    return _currentAxisAnnotation->GetNumTics();
}


void AnnotationParams::SetAxisOrigin(std::vector<double> orig) {
    _currentAxisAnnotation->SetAxisOrigin(orig);
}
std::vector<double> AnnotationParams::GetAxisOrigin() const {
    return _currentAxisAnnotation->GetAxisOrigin();
}


void AnnotationParams::SetMinTics(std::vector<double> minTics) {
    _currentAxisAnnotation->SetMinTics(minTics);
}
vector<double> AnnotationParams::GetMinTics() const {
    return _currentAxisAnnotation->GetMinTics();
}


void AnnotationParams::SetMaxTics(vector<double> maxTics) {
    _currentAxisAnnotation->SetMaxTics(maxTics);
}
vector<double> AnnotationParams::GetMaxTics() const {
    return _currentAxisAnnotation->GetMaxTics();
}


void AnnotationParams::SetTicSize(vector<double> ticSize) {
    _currentAxisAnnotation->SetTicSize(ticSize);
}
vector<double> AnnotationParams::GetTicSize() const {
    return _currentAxisAnnotation->GetTicSize();
}


void AnnotationParams::SetTicDirs(vector<double> ticDirs) {
    _currentAxisAnnotation->SetTicDirs(ticDirs);
}
vector<double> AnnotationParams::GetTicDirs() const {
    return _currentAxisAnnotation->GetTicDirs();
}


double AnnotationParams::GetTicWidth() const{
    return _currentAxisAnnotation->GetTicWidth();
}
void AnnotationParams::SetTicWidth(double val){
    _currentAxisAnnotation->SetTicWidth(val);
}


long AnnotationParams::GetAxisTextHeight() const{
    return _currentAxisAnnotation->GetAxisTextHeight();
}
void AnnotationParams::SetAxisTextHeight(long val){
    _currentAxisAnnotation->SetAxisTextHeight(val);
}


long AnnotationParams::GetAxisDigits() const{
    return _currentAxisAnnotation->GetAxisDigits();
}
void AnnotationParams::SetAxisDigits(long val){
    _currentAxisAnnotation->SetAxisDigits(val);
}


void AnnotationParams::SetLatLonAxesEnabled(bool val){
    _currentAxisAnnotation->SetLatLonAxesEnabled(val);
}
bool AnnotationParams::GetLatLonAxesEnabled() const{
    return _currentAxisAnnotation->GetLatLonAxesEnabled();
}


void AnnotationParams::SetAxisFontSize(int size) {
    SetValueDouble(_axisFontSizeTag, "Axis annotation font size", size);
}
int AnnotationParams::GetAxisFontSize() {
    return (int)GetValueDouble(_axisFontSizeTag, 24);
}
*/

string AnnotationParams::GetCurrentAxisDataMgrName() const { return GetValueString(_currentAxisDataMgrTag, ""); }

void AnnotationParams::SetCurrentAxisDataMgrName(string dmName)
{
    string msg = "Setting current DataMgr w.r.t. axis annotations";
    SetValueString(_currentAxisDataMgrTag, msg, dmName);
}

AxisAnnotation *AnnotationParams::GetAxisAnnotation(string dataMgr)
{
    if (dataMgr == "") dataMgr = GetCurrentAxisDataMgrName();

    vector<string> names = _axisAnnotations->GetNames();

    if (_axisAnnotations->GetParams(dataMgr) == NULL) {
        AxisAnnotation newAnnotation(_ssave);
        _axisAnnotations->Insert(&newAnnotation, dataMgr);
    }
    AxisAnnotation *aa = (AxisAnnotation *)_axisAnnotations->GetParams(dataMgr);
    return aa;
}

void AnnotationParams::SetShowAxisArrows(bool val) { SetValueLong(_showAxisArrowsTag, "Toggle Axis Arrows", val); }

bool AnnotationParams::GetShowAxisArrows() const { return (0 != GetValueLong(_showAxisArrowsTag, (long)false)); }

void AnnotationParams::SetAxisArrowCoords(vector<double> val)
{
    assert(val.size() == 3);
    SetValueDoubleVec(_axisArrowCoordsTag, "Set axis arrow coords", val);
}

void AnnotationParams::SetXAxisArrowPosition(float val)
{
    std::vector<double> pos = GetAxisArrowCoords();
    pos[0] = val;
    SetAxisArrowCoords(pos);
}

void AnnotationParams::SetYAxisArrowPosition(float val)
{
    std::vector<double> pos = GetAxisArrowCoords();
    pos[1] = val;
    SetAxisArrowCoords(pos);
}

void AnnotationParams::SetZAxisArrowPosition(float val)
{
    std::vector<double> pos = GetAxisArrowCoords();
    pos[2] = val;
    SetAxisArrowCoords(pos);
}

vector<double> AnnotationParams::GetAxisArrowCoords() const
{
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_axisArrowCoordsTag, defaultv);
}

#ifdef DEAD
void AnnotationParams::changeStretch(vector<double> prevStretch, vector<double> newStretch)
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
