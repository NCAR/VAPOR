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

const string AnnotationParams::_domainFrameTag = "DomainFrame";
const string AnnotationParams::_domainColorTag = "DomainColor";
const string AnnotationParams::_regionFrameTag = "RegionFrame";
const string AnnotationParams::_regionColorTag = "RegionColor";
const string AnnotationParams::_backgroundColorTag = "BackgroundColor";
const string AnnotationParams::_axisAnnotationEnabledTag = "AxisAnnotationiEnabled";
const string AnnotationParams::_axisColorTag = "AxisColor";
const string AnnotationParams::_axisDigitsTag = "AxisDigits";
const string AnnotationParams::_axisTextHeightTag = "AxisTextHeight";
const string AnnotationParams::_axisFontSizeTag = "AxisFontSize";
const string AnnotationParams::_ticWidthTag = "TicWidths";
const string AnnotationParams::_ticDirsTag = "TicDirections";
const string AnnotationParams::_ticSizeTag = "TicSizes";
const string AnnotationParams::_minTicsTag = "TicMinPositions";
const string AnnotationParams::_maxTicsTag = "TicMaxPositions";
const string AnnotationParams::_numTicsTag = "NumberTics";
const string AnnotationParams::_currentAxisDataMgrTag = "AxisDataMgr";
const string AnnotationParams::_latLonAxesTag = "LatLonAxes";
const string AnnotationParams::_axisOriginTag = "AxisOrigin";
const string AnnotationParams::AxisArrowEnabledTag = "ShowAxisArrows";
const string AnnotationParams::AxisArrowSizeTag = "AxisArrowSize";
const string AnnotationParams::AxisArrowXPosTag = "AxisArrowXPos";
const string AnnotationParams::AxisArrowYPosTag = "AxisArrowYPos";
const string AnnotationParams::_axisAnnotationsTag = "AxisAnnotations";
const string AnnotationParams::_timeLLXTag = "TimeLLX";
const string AnnotationParams::_timeLLYTag = "TimeLLY";
const string AnnotationParams::_timeColorTag = "TimeColor";
const string AnnotationParams::_timeTypeTag = "TimeType";
const string AnnotationParams::_timeSizeTag = "TimeSize";
const string AnnotationParams::_projStringTag = "ProjString";

vector<double> AnnotationParams::_previousStretch;

//
// Register class with object factory!!!
//
static ParamsRegistrar<AnnotationParams> registrar(AnnotationParams::GetClassType());

namespace {
string defaultAnnotation = "default";
}

AnnotationParams::AnnotationParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, AnnotationParams::GetClassType())
{
    _init();

    _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
    _axisAnnotations->SetParent(this);
}

AnnotationParams::AnnotationParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    if (node->HasChild(_axisAnnotationsTag)) {
        _axisAnnotations = new ParamsContainer(ssave, node->GetChild(_axisAnnotationsTag));
    } else {
        _axisAnnotations = new ParamsContainer(ssave, _axisAnnotationsTag);
        _axisAnnotations->SetParent(this);
    }
}

AnnotationParams::AnnotationParams(const AnnotationParams &rhs) : ParamsBase(rhs) { _axisAnnotations = new ParamsContainer(*(rhs._axisAnnotations)); }

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
    SetValueLong(AxisArrowEnabledTag, "Initializing AxisArrowEnabledTag", 0);
    SetValueDouble(AxisArrowSizeTag, "Initializing AxisArrowSizeTag", .2);
    SetValueDouble(AxisArrowXPosTag, "Initializing AxisArrowXPosTag", .05);
    SetValueDouble(AxisArrowYPosTag, "Initializing AxisArrowYPosTag", .05);
    SetValueDouble(_timeLLXTag, "", 0.01);
    SetValueDouble(_timeLLYTag, "", 0.01);
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
    VAssert(color.size() == 3);
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

string AnnotationParams::GetCurrentAxisDataMgrName() const { return GetValueString(_currentAxisDataMgrTag, defaultAnnotation); }

void AnnotationParams::SetCurrentAxisDataMgrName(string dmName)
{
    string msg = "Setting current DataMgr w.r.t. axis annotations";
    SetValueString(_currentAxisDataMgrTag, msg, dmName);
}

AxisAnnotation *AnnotationParams::GetAxisAnnotation()
{
    vector<string> names = _axisAnnotations->GetNames();
    if (_axisAnnotations->GetParams(defaultAnnotation) == NULL) {
        AxisAnnotation newAnnotation(_ssave);
        _axisAnnotations->Insert(&newAnnotation, defaultAnnotation);
    }
    AxisAnnotation *aa;
    aa = (AxisAnnotation *)_axisAnnotations->GetParams(defaultAnnotation);
    return aa;
}

void AnnotationParams::SetAxisArrowEnabled(bool enabled) { SetValueLong(AxisArrowEnabledTag, "Set axis arrow to enabled/disabled", (long)enabled); }

void AnnotationParams::SetAxisArrowSize(double val) { SetValueDouble(AxisArrowSizeTag, "Set axis arrow size", val); }

void AnnotationParams::SetAxisArrowXPos(double val) { SetValueDouble(AxisArrowXPosTag, "Set axis arrow X coordinate", val); }

void AnnotationParams::SetAxisArrowYPos(double val) { SetValueDouble(AxisArrowYPosTag, "Set axis arrow Y coordinate", val); }

bool AnnotationParams::GetAxisArrowEnabled() const { return (bool)GetValueLong(AxisArrowEnabledTag, false); }

double AnnotationParams::GetAxisArrowSize() const { return GetValueDouble(AxisArrowSizeTag, .2); }

double AnnotationParams::GetAxisArrowXPos() const { return GetValueDouble(AxisArrowXPosTag, .05); }

double AnnotationParams::GetAxisArrowYPos() const { return GetValueDouble(AxisArrowYPosTag, .05); }

double AnnotationParams::GetTimeLLX() const { return GetValueDouble(_timeLLXTag, 0.01); }

void AnnotationParams::SetTimeLLX(double llx) { SetValueDouble(_timeLLXTag, "Timestep llx coordinate", llx); }

double AnnotationParams::GetTimeLLY() const { return GetValueDouble(_timeLLYTag, 0.01); }

void AnnotationParams::SetTimeLLY(double lly) { SetValueDouble(_timeLLYTag, "Timestep lly coordinate", lly); }

std::vector<double> AnnotationParams::GetTimeColor() const
{
    std::vector<double> defaultv(3, 1.0);
    std::vector<double> val = GetValueDoubleVec(_timeColorTag, defaultv);
    return val;
}

void AnnotationParams::SetTimeColor(vector<double> color) { SetValueDoubleVec(_timeColorTag, "Timestep color", color); }

int AnnotationParams::GetTimeType() const { return (int)GetValueDouble(_timeTypeTag, 0); }

void AnnotationParams::SetTimeType(int type) { SetValueDouble(_timeTypeTag, "Timestep annotation type", type); }

int AnnotationParams::GetTimeSize() const { return (int)GetValueDouble(_timeSizeTag, 24); }

void AnnotationParams::SetTimeSize(int size) { SetValueDouble(_timeSizeTag, "Timestep font size", size); }
