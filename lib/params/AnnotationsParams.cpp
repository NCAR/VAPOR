//************************************************************************
//									*
//			 Copyright (C)  2015				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnnotationsParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the AnnotationsParams class.
//		This class supports parameters associted with the
//		visualizer features in the vizfeatures panel
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <vector>
#include <string>
#include <iostream>

#include <vapor/AnnotationsParams.h>


using namespace VAPoR;

const string AnnotationsParams::_domainFrameTag = "DomainFrame";
const string AnnotationsParams::_domainColorTag = "DomainColor";
const string AnnotationsParams::_regionFrameTag = "RegionFrame";
const string AnnotationsParams::_regionColorTag = "RegionColor";
const string AnnotationsParams::_backgroundColorTag = "BackgroundColor";
const string AnnotationsParams::_axisAnnotationEnabledTag = "AxisAnnotationiEnabled";
const string AnnotationsParams::_axisColorTag = "AxisColor";
const string AnnotationsParams::_axisDigitsTag = "AxisDigits";
const string AnnotationsParams::_axisTextHeightTag = "AxisTextHeight";
const string AnnotationsParams::_axisFontSizeTag = "AxisFontSize";
const string AnnotationsParams::_ticWidthTag = "TicWidths";
const string AnnotationsParams::_ticDirsTag = "TicDirections";
const string AnnotationsParams::_ticSizeTag = "TicSizes";
const string AnnotationsParams::_minTicsTag = "TicMinPositions";
const string AnnotationsParams::_maxTicsTag = "TicMaxPositions";
const string AnnotationsParams::_numTicsTag = "NumberTics";
const string AnnotationsParams::_currentAxisDataMgrTag = "AxisDataMgr";
const string AnnotationsParams::_latLonAxesTag = "LatLonAxes";
const string AnnotationsParams::_axisOriginTag = "AxisOrigin";
const string AnnotationsParams::_showAxisArrowsTag = "ShowAxisArrows";
const string AnnotationsParams::_axisArrowCoordsTag = "AxisArrowCoords";
const string AnnotationsParams::_axisAnnotationsTag = "AxisAnnotations";
vector<double> AnnotationsParams::_previousStretch;

//
// Register class with object factory!!!
//
static ParamsRegistrar<AnnotationsParams> registrar(AnnotationsParams::GetClassType());


AnnotationsParams::AnnotationsParams(
	ParamsBase::StateSave *ssave
) : ParamsBase(ssave, AnnotationsParams::GetClassType()) {

	_init();

	_axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
	_axisAnnotations->SetParent(this);
}


AnnotationsParams::AnnotationsParams(
	ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node) 
{
	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != AnnotationsParams::GetClassType()) {
		node->SetTag(AnnotationsParams::GetClassType());
		_init();
	}

	if (node->HasChild(_axisAnnotationsTag)) {
		_axisAnnotations = new ParamsContainer(
			ssave, 
			node->GetChild(_axisAnnotationsTag)
		);
	}
	else {
		_axisAnnotations = new ParamsContainer(
			ssave, _axisAnnotationsTag);
		_axisAnnotations->SetParent(this);
	}
}

AnnotationsParams::AnnotationsParams(const AnnotationsParams &rhs
) : ParamsBase(rhs) {
	_axisAnnotations = new ParamsContainer(*(rhs._axisAnnotations));
}


//Reset vizfeature settings to initial state
void AnnotationsParams::_init() {

	vector<double> clr (3,1.0);
	SetDomainColor(clr);
	//SetAxisColor(clr);
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


void AnnotationsParams::m_getColor(double color[3], string tag) const {

	vector <double> defaultv(3,1.0);
	vector <double> val =  GetValueDoubleVec(tag, defaultv);
	for (int i=0; i<val.size(); i++) {
		color[i] = val[i];
		if (color[i] < 0.0) color[i] = 0.0;
		if (color[i] > 1.0) color[i] = 1.0;
	}
}

void AnnotationsParams::_getColor(vector <double> &color, string tag) const {
	color.clear();

	vector <double> defaultv(3,1.0);
	color =  GetValueDoubleVec(tag, defaultv);
	for (int i=0; i<color.size(); i++) {
		if (color[i] < 0.0) color[i] = 0.0;
		if (color[i] > 1.0) color[i] = 1.0;
	}
}

void AnnotationsParams::m_setColor(vector<double> color, string tag, string msg){

	assert(color.size() == 3);
	for (int i=0; i<color.size(); i++) {
		if (color[i] < 0.0) color[i] = 0.0;
		if (color[i] > 1.0) color[i] = 1.0;
	}
	SetValueDoubleVec(tag, msg, color);
}

void AnnotationsParams::GetDomainColor(double color[3]) const {
	m_getColor(color, _domainColorTag);
}

void AnnotationsParams::SetDomainColor(vector<double> color) {
	m_setColor(color, _domainColorTag, "Set domain frame color");
}


void AnnotationsParams::GetRegionColor(double color[3]) const {
	m_getColor(color, _regionColorTag);
}

void AnnotationsParams::SetRegionColor(vector<double> color) {
	m_setColor(color, _regionColorTag, "Set region frame color");
}

void AnnotationsParams::GetBackgroundColor(double color[3]) const {
	m_getColor(color, _backgroundColorTag);
}

void AnnotationsParams::SetBackgroundColor(vector<double> color) {
	m_setColor(color, _backgroundColorTag, "Set background color");
}




/*
void AnnotationsParams::SetAxisAnnotationEnabled(bool val){
	_currentAxisAnnotation->SetAxisAnnotationEnabled(val);
}   
bool AnnotationsParams::GetAxisAnnotationEnabled() const{
	return _currentAxisAnnotation->GetAxisAnnotationEnabled();
}   

std::vector<double> AnnotationsParams::GetAxisColor() const {
	return _currentAxisAnnotation->GetAxisColor();
}   
void AnnotationsParams::SetAxisColor(vector<double> color) {
	_currentAxisAnnotation->SetAxisColor(color);
}

std::vector<double> AnnotationsParams::GetAxisBackgroundColor() const {
	return _currentAxisAnnotation->GetAxisBackgroundColor();
}   
void AnnotationsParams::SetAxisBackgroundColor(vector<double> color) {
	_currentAxisAnnotation->SetAxisBackgroundColor(color);
}

void AnnotationsParams::SetNumTics(std::vector<double> numTics) {
	_currentAxisAnnotation->SetNumTics(numTics);
}
std::vector<double> AnnotationsParams::GetNumTics() const {
	return _currentAxisAnnotation->GetNumTics();
}


void AnnotationsParams::SetAxisOrigin(std::vector<double> orig) {
	_currentAxisAnnotation->SetAxisOrigin(orig);
}
std::vector<double> AnnotationsParams::GetAxisOrigin() const {
	return _currentAxisAnnotation->GetAxisOrigin();
}


void AnnotationsParams::SetMinTics(std::vector<double> minTics) {
	_currentAxisAnnotation->SetMinTics(minTics);
}
vector<double> AnnotationsParams::GetMinTics() const {
	return _currentAxisAnnotation->GetMinTics();
}


void AnnotationsParams::SetMaxTics(vector<double> maxTics) {
	_currentAxisAnnotation->SetMaxTics(maxTics);
}
vector<double> AnnotationsParams::GetMaxTics() const {
	return _currentAxisAnnotation->GetMaxTics();
}


void AnnotationsParams::SetTicSize(vector<double> ticSize) {
	_currentAxisAnnotation->SetTicSize(ticSize);
}
vector<double> AnnotationsParams::GetTicSize() const {
	return _currentAxisAnnotation->GetTicSize();
}


void AnnotationsParams::SetTicDirs(vector<double> ticDirs) {
	_currentAxisAnnotation->SetTicDirs(ticDirs);
}
vector<double> AnnotationsParams::GetTicDirs() const {
	return _currentAxisAnnotation->GetTicDirs();
}


double AnnotationsParams::GetTicWidth() const{
	return _currentAxisAnnotation->GetTicWidth();
}   
void AnnotationsParams::SetTicWidth(double val){
	_currentAxisAnnotation->SetTicWidth(val);
}   


long AnnotationsParams::GetAxisTextHeight() const{
	return _currentAxisAnnotation->GetAxisTextHeight();
}   
void AnnotationsParams::SetAxisTextHeight(long val){
	_currentAxisAnnotation->SetAxisTextHeight(val);
}   


long AnnotationsParams::GetAxisDigits() const{
	return _currentAxisAnnotation->GetAxisDigits();
}
void AnnotationsParams::SetAxisDigits(long val){
	_currentAxisAnnotation->SetAxisDigits(val);
}


void AnnotationsParams::SetLatLonAxesEnabled(bool val){
	_currentAxisAnnotation->SetLatLonAxesEnabled(val);
}
bool AnnotationsParams::GetLatLonAxesEnabled() const{
	return _currentAxisAnnotation->GetLatLonAxesEnabled();
}


void AnnotationsParams::SetAxisFontSize(int size) {
	SetValueDouble(_axisFontSizeTag, "Axis annotation font size", size);
}
int AnnotationsParams::GetAxisFontSize() {
	return (int)GetValueDouble(_axisFontSizeTag, 24);
}
*/

string AnnotationsParams::GetCurrentAxisDataMgrName() const {
	return GetValueString(_currentAxisDataMgrTag, "");
}

void AnnotationsParams::SetCurrentAxisDataMgrName(string dmName) {
	string msg = "Setting current DataMgr w.r.t. axis annotations";
	SetValueString(_currentAxisDataMgrTag, msg, dmName);
}

AxisAnnotation* AnnotationsParams::GetAxisAnnotation(string dataMgr) {
	if (dataMgr == "")
		dataMgr = GetCurrentAxisDataMgrName();

	vector<string> names = _axisAnnotations->GetNames();

	if (_axisAnnotations->GetParams(dataMgr) == NULL) {
		AxisAnnotation newAnnotation(_ssave);
		_axisAnnotations->Insert(&newAnnotation, dataMgr);
	}
	AxisAnnotation* aa = (AxisAnnotation*)_axisAnnotations->GetParams(dataMgr);
	return aa;
}

void AnnotationsParams::SetShowAxisArrows(bool val) {
	SetValueLong(_showAxisArrowsTag, "Toggle Axis Arrows", val);
}

bool AnnotationsParams::GetShowAxisArrows() const {
	return (0 != GetValueLong(_showAxisArrowsTag, (long)false));
}

void AnnotationsParams::SetAxisArrowCoords(vector<double> val){
	assert(val.size() == 3);
	SetValueDoubleVec(_axisArrowCoordsTag, "Set axis arrow coords", val);
}

vector<double> AnnotationsParams::GetAxisArrowCoords() const{
	vector <double> defaultv(3,0.0);
	return GetValueDoubleVec(_axisArrowCoordsTag, defaultv);
}

#ifdef	DEAD
void AnnotationsParams::changeStretch(vector<double> prevStretch, vector<double> newStretch){

	_dataStatus->stretchExtents(newStretch);
	//Determine the relative scale:
	vector<double>scalefactor;
	for (int i = 0; i<3; i++) scalefactor.push_back(newStretch[i]/prevStretch[i]);
	//Make all viewpoint params rescale
	ViewpointParams* p = (ViewpointParams*)_paramsMgr->GetDefaultParams(_viewpointParamsTag);
	p->rescale(scalefactor);
	vector<Params*> vpparams;
	_paramsMgr->GetAllParamsInstances(_viewpointParamsTag,vpparams);
	for (int j = 0; j<vpparams.size(); j++){
		((ViewpointParams*)vpparams[j])->rescale(scalefactor);
	}
}

#endif
