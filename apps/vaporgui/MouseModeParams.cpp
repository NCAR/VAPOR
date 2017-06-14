//************************************************************************
//									*
//		     Copyright (C)  2014				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MouseModeParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		April 2014
//
//	Description:	Implements the MouseModeParams class.
//		This class supports parameters associted with the
//		mouse mode
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <vector>
#include <string>

#include "../../vaporgui/images/wheel.xpm"
#include "../../vaporgui/images/twoDData.xpm"
#include "../../vaporgui/images/twoDImage.xpm"
#include "../../vaporgui/images/cube.xpm"
#include "../../vaporgui/images/arrowrake.xpm"
#include "../../vaporgui/images/isoline.xpm"
#include "MouseModeParams.h"

const std::string MouseModeParams::_currentMouseModeTag = "CurrentMouseModeTag";

using namespace VAPoR;

MouseModeParams::MouseModeParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, MouseModeParams::GetClassType())
{
    _setUpDefault();
    _init();
}

MouseModeParams::MouseModeParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    _setUpDefault();

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != MouseModeParams::GetClassType()) {
        node->SetTag(MouseModeParams::GetClassType());
        _init();
    }
}

MouseModeParams::~MouseModeParams() {}

// Reset region settings to initial state
void MouseModeParams::_init()
{
    // Set to navigation mode
    SetCurrentMouseMode(GetNavigateModeName());
}

// Stash the name and icon where the gui can find them
void MouseModeParams::RegisterMouseMode(string name, int manipType, const char *const xpmIcon[])
{
    _typeMap[name] = manipType;
    _iconMap[name] = xpmIcon;
}

//! Static method called at startup to register all the built-in mouse modes,
//! by calling RegisterMouseMode() for each built-in mode.
//! Also calls InstallExtensionMouseModes() to register extension modes.
void MouseModeParams::_setUpDefault()
{
    RegisterMouseMode(GetNavigateModeName(), 0, wheel);
    RegisterMouseMode("Region", 1, cube);
    RegisterMouseMode("Barb rake", 1, arrowrake);
    RegisterMouseMode("Contours", 3, isoline);
    // RegisterMouseMode(Params::GetClassType(),1, "Flow rake",rake );
    // RegisterMouseMode(Params::GetClassType(),3,"Probe", probe);
    RegisterMouseMode("2D Data", 2, twoDData);
    RegisterMouseMode("Image", 2, twoDImage);
}

void MouseModeParams::SetCurrentMouseMode(string t)
{
    // Make sure mode is registered. If not, use default
    //
    if (_typeMap.find(t) == _typeMap.end()) t = GetNavigateModeName();

    SetValueString(_currentMouseModeTag, "Set mouse mode", t);
}
