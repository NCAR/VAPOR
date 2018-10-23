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

#include "../../apps/vaporgui/images/wheel.xpm"
#include "../../apps/vaporgui/images/cube.xpm"
#include "../../apps/vaporgui/images/sphere.xpm"
#ifdef VAPOR3_0_0_ALPHA
    #include "../../apps/vaporgui/images/twoDData.xpm"
    #include "../../apps/vaporgui/images/twoDImage.xpm"
    #include "../../apps/vaporgui/images/cube.xpm"
    #include "../../apps/vaporgui/images/arrowrake.xpm"
    #include "../../apps/vaporgui/images/isoline.xpm"
#endif
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

//! Static method called at startup to register all the built-in mouse modes
void MouseModeParams::_setUpDefault()
{
    _modes.push_back({GetNavigateModeName(), wheel});
    _modes.push_back({GetRegionModeName(), cube});

    // RegisterMouseMode("Barb rake", 1, arrowrake );
    // RegisterMouseMode("Contours", 3, isoline);
    // RegisterMouseMode(Params::GetClassType(),1, "Flow rake",rake );
    // RegisterMouseMode(Params::GetClassType(),3,"Probe", probe);
    // RegisterMouseMode("2D Data", 2, twoDData);
    // RegisterMouseMode("Image", 2, twoDImage);
}

void MouseModeParams::SetCurrentMouseMode(string t)
{
    // Make sure mode is registered. If not, use default
    //
    auto itr = _modes.cbegin();
    for (; itr != _modes.cend(); ++itr)
        if (itr->name == t) break;
    if (itr == _modes.cend()) t = GetNavigateModeName();

    SetValueString(_currentMouseModeTag, "Set mouse mode", t);
}
