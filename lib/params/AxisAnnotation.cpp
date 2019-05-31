//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AxisAnnotation.cpp
//
//	Author:		Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2017
//
//	Description:	Implements the AxisAnnotation class
//		Implements axis annotation ParamsContainer, which stores parameters
//		for Renderers or DataMgrs to draw axes.
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>
#include <vapor/AxisAnnotation.h>

using namespace VAPoR;
using namespace Wasp;

const string AxisAnnotation::_backgroundColorTag = "BackgroundColor";
const string AxisAnnotation::_annotationEnabledTag = "AxisAnnotation";
const string AxisAnnotation::_colorTag = "AxisColor";
const string AxisAnnotation::_digitsTag = "AxisDigits";
const string AxisAnnotation::_textHeightTag = "AxisTextHeight";
const string AxisAnnotation::_fontSizeTag = "AxisFontSize";
const string AxisAnnotation::_ticWidthTag = "TicWidths";
const string AxisAnnotation::_ticDirsTag = "TicDirections";
const string AxisAnnotation::_ticSizeTag = "TicSizes";
const string AxisAnnotation::_minTicsTag = "TicMinPositions";
const string AxisAnnotation::_maxTicsTag = "TicMaxPositions";
const string AxisAnnotation::_numTicsTag = "NumberTics";
const string AxisAnnotation::_dataMgrTag = "AxisDataMgr";
const string AxisAnnotation::_latLonAxesTag = "LatLonAxes";
const string AxisAnnotation::_originTag = "AxisOrigin";
const string AxisAnnotation::_initializedTag = "AxisAnnotaitonInitialized";

//
// Register class with object factory!!!
//
static ParamsRegistrar<AxisAnnotation> registrar(AxisAnnotation::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
AxisAnnotation::AxisAnnotation(ParamsBase::StateSave *ssave) : ParamsBase(ssave, AxisAnnotation::GetClassType()) {}

AxisAnnotation::AxisAnnotation(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
AxisAnnotation::~AxisAnnotation() { MyBase::SetDiagMsg("AxisAnnotation::~AxisAnnotation() this=%p", this); }

void AxisAnnotation::SetAxisAnnotationEnabled(bool val)
{
    string msg = "Toggle axis annotation on/off";
    SetValueLong(_annotationEnabledTag, msg, (long)val);
}

bool AxisAnnotation::GetAxisAnnotationEnabled() const { return (0 != GetValueLong(_annotationEnabledTag, (long)false)); }

std::vector<double> AxisAnnotation::GetAxisBackgroundColor() const
{
    vector<double> defaultv(4, 0.0);
    vector<double> val = GetValueDoubleVec(_backgroundColorTag, defaultv);
    if (val == defaultv) val[3] = 1.f;
    return val;
}

void AxisAnnotation::SetAxisBackgroundColor(std::vector<double> color)
{
    string msg = "Axis annotation background color";
    if (color.size() == 3) color.push_back(1.f);
    SetValueDoubleVec(_backgroundColorTag, msg, color);
}

std::vector<double> AxisAnnotation::GetAxisColor() const
{
    vector<double> defaultv(4, 1.0);
    vector<double> val = GetValueDoubleVec(_colorTag, defaultv);
    return val;
}

void AxisAnnotation::SetAxisColor(std::vector<double> color)
{
    string msg = "Axis annotation text color";
    SetValueDoubleVec(_colorTag, msg, color);
}

void AxisAnnotation::SetNumTics(std::vector<double> num)
{
    VAssert(num.size() == 3);
    for (int i = 0; i < num.size(); i++) {
        if (num[i] < 0) num[i] = 0;
        if (num[i] > 100) num[i] = 100;
    }

    SetValueDoubleVec(_numTicsTag, "Set number of axis tics", num);
}

std::vector<double> AxisAnnotation::GetNumTics() const
{
    vector<double> defaultv(3, 6.0);
    vector<double> val = GetValueDoubleVec(_numTicsTag, defaultv);

    for (int i = 0; i < val.size(); i++) {
        if (val[i] < 0) val[i] = 0;
        if (val[i] > 100) val[i] = 100;
    }
    return (val);
}

void AxisAnnotation::SetAxisOrigin(vector<double> orig)
{
    VAssert(orig.size() == 3);
    SetValueDoubleVec(_originTag, "Set axis val", orig);
}

vector<double> AxisAnnotation::GetAxisOrigin() const
{
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_originTag, defaultv);
}

void AxisAnnotation::SetMinTics(vector<double> ticMin)
{
    VAssert(ticMin.size() == 3);
    SetValueDoubleVec(_minTicsTag, "Set minimum tics", ticMin);
}

vector<double> AxisAnnotation::GetMinTics() const
{
    vector<double> defaultv(3, 0.0);
    return GetValueDoubleVec(_minTicsTag, defaultv);
}

void AxisAnnotation::SetMaxTics(vector<double> ticMax)
{
    VAssert(ticMax.size() == 3);
    SetValueDoubleVec(_maxTicsTag, "Set maximum tics", ticMax);
}

vector<double> AxisAnnotation::GetMaxTics() const
{
    vector<double> defaultv(3, 1.0);
    vector<double> myVec = GetValueDoubleVec(_maxTicsTag, defaultv);
    return myVec;
}

void AxisAnnotation::SetTicSize(vector<double> ticSizes)
{
    VAssert(ticSizes.size() == 3);
    SetValueDoubleVec(_ticSizeTag, "Set tic sizes", ticSizes);
}

vector<double> AxisAnnotation::GetTicSize() const
{
    vector<double> defaultv(3, 0.05);
    return GetValueDoubleVec(_ticSizeTag, defaultv);
}

void AxisAnnotation::SetTicDirs(vector<double> ticDirs)
{
    VAssert(ticDirs.size() == 3);
    SetValueDoubleVec(_ticDirsTag, "Set tic direction", ticDirs);
}

vector<double> AxisAnnotation::GetTicDirs() const
{
    vector<double> defaultv(3, 0);
    defaultv[0] = 1;
    return GetValueDoubleVec(_ticDirsTag, defaultv);
}

double AxisAnnotation::GetTicWidth() const { return GetValueDouble(_ticWidthTag, 1.0); }

void AxisAnnotation::SetTicWidth(double width)
{
    if (width < 1) width = 1;
    SetValueDouble(_ticWidthTag, "Set tic width", width);
}

long AxisAnnotation::GetAxisTextHeight() const { return GetValueLong(_textHeightTag, 10); }

void AxisAnnotation::SetAxisTextHeight(long height)
{
    if (height < 1) height = 1;
    SetValueLong(_textHeightTag, "Set axis text height", height);
}

long AxisAnnotation::GetAxisDigits() const { return GetValueLong(_digitsTag, 2); }

void AxisAnnotation::SetAxisDigits(long numDigits)
{
    if (numDigits < 0) numDigits = 4;
    SetValueLong(_digitsTag, "Set axis num digits", numDigits);
}

void AxisAnnotation::SetLatLonAxesEnabled(bool val) { SetValueLong(_latLonAxesTag, "toggle axes lat/lon", (long)val); }

bool AxisAnnotation::GetLatLonAxesEnabled() const { return (0 != GetValueLong(_latLonAxesTag, (long)false)); }

string AxisAnnotation::GetDataMgrName() const { return GetValueString(_dataMgrTag, ""); }

void AxisAnnotation::SetDataMgrName(string dataMgr)
{
    string msg = "Set DataManager currently associated "
                 "with the axis annotations";
    SetValueString(_dataMgrTag, msg, dataMgr);
}

void AxisAnnotation::SetAxisFontSize(int size) { SetValueDouble(_fontSizeTag, "Axis annotation font size", size); }

int AxisAnnotation::GetAxisFontSize() const { return (int)GetValueDouble(_fontSizeTag, 16); }

void AxisAnnotation::SetAxisAnnotationInitialized(bool val)
{
    string msg = "Axis annotation object initialized";
    SetValueDouble(_initializedTag, msg, val);
}

bool AxisAnnotation::GetAxisAnnotationInitialized() const { return (bool)GetValueDouble(_initializedTag, false); }
