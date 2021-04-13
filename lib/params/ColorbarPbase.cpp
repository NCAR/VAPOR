//************************************************************************
//																	*
//		     Copyright (C)  2016									*
//     University Corporation for Atmospheric Research				*
//		     All Rights Reserved									*
//																	*
//************************************************************************/
//
//	File:		ColorbarPbase.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Implements the ColorbarPbase class.
//		Used to control parameters of a color bar.
//

#include <vector>
#include "vapor/VAssert.h"
#include <vapor/ColorbarPbase.h>

using namespace VAPoR;
const string ColorbarPbase::_colorbarBackColorTag = "ColorbarBackgroundColor";
const string ColorbarPbase::_colorbarSizeXTag = "ColorbarSize_X";
const string ColorbarPbase::_colorbarSizeYTag = "ColorbarSize_Y";
const string ColorbarPbase::_colorbarPositionXTag = "ColorbarPosition_X";
const string ColorbarPbase::_colorbarPositionYTag = "ColorbarPosition_Y";
const string ColorbarPbase::_colorbarFontSizeTag = "ColorbarFontsize";
const string ColorbarPbase::_colorbarNumDigitsTag = "ColorbarNumDigits";
const string ColorbarPbase::_colorbarTitleTag = "ColorbarTitle";
const string ColorbarPbase::_colorbarNumTicksTag = "ColorbarNumTicks";
const string ColorbarPbase::_colorbarEnabledTag = "ColorbarEnabled";

const string ColorbarPbase::UseScientificNotationTag = "UseScientificNotationTag";

//
// Register class with object factory!!!
//
static ParamsRegistrar<ColorbarPbase> registrar(ColorbarPbase::GetClassType());

ColorbarPbase::ColorbarPbase(ParamsBase::StateSave *ssave) : ParamsBase(ssave, ColorbarPbase::GetClassType())
{
    MyBase::SetDiagMsg("ColorbarPbase::ColorbarPbase() this=%p", this);

    // Initialize with default values
    SetCornerPosition(vector<double>(2, 0.03));
    SetSize(vector<double>(2, 0.1));
    SetTitle("");
    SetFontSize(10);
    SetNumDigits(4);
    SetNumTicks(6);
    SetBackgroundColor(vector<double>(3, 1.));
    SetEnabled(false);
    SetValueLong(UseScientificNotationTag, "", false);
}

ColorbarPbase::ColorbarPbase(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {}

ColorbarPbase::~ColorbarPbase() { MyBase::SetDiagMsg("ColorbarPbase::~ColorbarPbase() this=%p", this); }

vector<double> ColorbarPbase::GetCornerPosition() const
{
    vector<double> val(2, 0.0);
    val[0] = GetValueDouble(_colorbarPositionXTag, 0);
    val[1] = GetValueDouble(_colorbarPositionYTag, 0);
    for (int i = 0; i < 2; i++) {
        if (val[i] < 0.0) val[i] = 0.0;
        if (val[i] > 1.0) val[i] = 1.0;
    }
    return (val);
}

//! Set the X,Y corner (upper left) coordinates
//! Relative to [0,1]
//! \param[in] posn = x,y coordinates
void ColorbarPbase::SetCornerPosition(vector<double> posn)
{
    vector<double> defaultv(2, 0.0);
    if (posn.size() != 2) posn = defaultv;
    for (int i = 0; i < 2; i++) {
        if (posn[i] < 0.0) posn[i] = 0.0;
        if (posn[i] > 1.0) posn[i] = 1.0;
    }

    SetValueDouble(_colorbarPositionXTag, "", posn[0]);
    SetValueDouble(_colorbarPositionYTag, "", posn[1]);
}

//! Get the X,Y size
//! Relative to [0,1]
//! \retval pair of x,y sizes
vector<double> ColorbarPbase::GetSize() const
{
    vector<double> val(2, 0.0);
    val[0] = GetValueDouble(_colorbarSizeXTag, 0);
    val[1] = GetValueDouble(_colorbarSizeYTag, 0);
    for (int i = 0; i < 2; i++) {
        if (val[i] < 0.0) val[i] = 0.0;
        if (val[i] > 1.0) val[i] = 1.0;
    }
    return (val);
}

//! Set the X,Y sizes
//! Relative to [0,1]
//! \param[in] posn = x,y sizes
void ColorbarPbase::SetSize(vector<double> sz)
{
    vector<double> defaultv(2, 0.0);
    if (sz.size() != 2) sz = defaultv;
    for (int i = 0; i < 2; i++) {
        if (sz[i] < 0.0) sz[i] = 0.0;
        if (sz[i] > 1.0) sz[i] = 1.0;
    }
    SetValueDouble(_colorbarSizeXTag, "", sz[0]);
    SetValueDouble(_colorbarSizeYTag, "", sz[1]);
}

//! Determine colorbar text size
//! \return pointsize
long ColorbarPbase::GetFontSize() const
{
    float val = (float)GetValueLong(_colorbarFontSizeTag, 10);
    if (val < 2) val = 2;
    if (val > 96) val = 96;
    return (val);
}

//! Set colorbar text size
//! \param[in] val text point size
void ColorbarPbase::SetFontSize(long val)
{
    if (val < 2) val = 2;
    if (val > 96) val = 96;
    SetValueLong(_colorbarFontSizeTag, "Set colorbar fontsize", val);
}

//! Determine colorbar num tics
//! \return number of tics
long ColorbarPbase::GetNumTicks() const
{
    long val = GetValueLong(_colorbarNumTicksTag, 8);
    if (val < 0) val = 0;
    if (val > 20) val = 20;
    return (val);
}

//! Set colorbar number of tic marks
//! \param[in] val number of tics
void ColorbarPbase::SetNumTicks(long val)
{
    if (val < 0) val = 0;
    if (val > 20) val = 20;
    SetValueLong(_colorbarNumTicksTag, "set num tics", val);
}

//! Determine colorbar num digits to display
//! \return number of digits
long ColorbarPbase::GetNumDigits() const
{
    long val = GetValueLong(_colorbarNumDigitsTag, 4);
    if (val < 0) val = 0;
    if (val > 8) val = 8;
    return (val);
}

//! Set colorbar number of digits
//! \param[in] val number of digits
void ColorbarPbase::SetNumDigits(long val)
{
    if (val < 0) val = 0;
    if (val > 8) val = 8;
    SetValueLong(_colorbarNumDigitsTag, "set num digits", val);
}

//! Get the background color
//! as an rgb triple
//! \retval rgb color
vector<double> ColorbarPbase::GetBackgroundColor() const
{
    vector<double> defaultv(3, 1.0);
    vector<double> color = GetValueDoubleVec(_colorbarBackColorTag, defaultv);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
    return (color);
}

//! Set the background color as an rgb triple
//! \param[in] color = (r,g,b)
void ColorbarPbase::SetBackgroundColor(vector<double> color)
{
    VAssert(color.size() == 3);
    for (int i = 0; i < color.size(); i++) {
        if (color[i] < 0.0) color[i] = 0.0;
        if (color[i] > 1.0) color[i] = 1.0;
    }
    SetValueDoubleVec(_colorbarBackColorTag, "set colorbar background color", color);
}

void ColorbarPbase::copyTo(ColorbarPbase *target)
{
    target->SetBackgroundColor(GetBackgroundColor());
    target->SetSize(GetSize());
    target->SetFontSize(GetFontSize());
    target->SetNumDigits(GetNumDigits());
    target->SetNumTicks(GetNumTicks());
}
