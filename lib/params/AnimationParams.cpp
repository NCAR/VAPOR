//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AnimationParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		January 2005
//
//	Description:	Implements the AnimationParams class
//		This is derived from the Params class
//		It contains all the parameters required for animation

//

#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>

#include <vapor/AnimationParams.h>

using namespace VAPoR;
const string AnimationParams::_maxRateTag = "MaxFrameRate";
const string AnimationParams::_startTimestepTag = "StartTimestep";
const string AnimationParams::_endTimestepTag = "EndTimestep";
const string AnimationParams::_currentTimestepTag = "CurrentTimestep";
const string AnimationParams::_playBackwardsTag = "PlayBackwards";
const string AnimationParams::CaptureStartTag = "CaptureStartTag";
const string AnimationParams::CaptureEndTag = "CaptureEndTag";
const string AnimationParams::CaptureModeTag = "CaptureModeTag";
const string AnimationParams::CaptureTypeTag = "CaptureTypeTag";
const string AnimationParams::CaptureFileNameTag = "CaptureFileNameTag";
const string AnimationParams::CaptureFileDirTag = "CaptureFileDirTag";
const string AnimationParams::CaptureFileTimeTag = "CaptureFileTimeTag";
const string AnimationParams::CaptureTimeSeriesFileNameTag = "CaptureTimeSeriesFileNameTag";
const string AnimationParams::CaptureTimeSeriesTimeTag = "CaptureTimeSeriesTimeTag";

//
// Register class with object factory!!!
//
static ParamsRegistrar<AnimationParams> registrar(AnimationParams::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
AnimationParams::AnimationParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, AnimationParams::GetClassType()) { _init(); }

AnimationParams::AnimationParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != AnimationParams::GetClassType()) {
        node->SetTag(AnimationParams::GetClassType());
        _init();
    }
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
AnimationParams::~AnimationParams() { MyBase::SetDiagMsg("AnimationParams::~AnimationParams() this=%p", this); }

// Reset to initial state
//
void AnimationParams::_init()
{
    // set everything to default state:
    SetPlayBackwards(false);
    SetStartTimestep(0);
    SetEndTimestep(10000000);
    SetCurrentTimestep(0);
    SetMaxFrameRate(10);
    SetValueLong(AnimationParams::CaptureModeTag, "Set default value for image capture mode", AnimationParams::SingleImage);
    SetValueLong(AnimationParams::CaptureTypeTag, "Set default value for capture mode image filetype", AnimationParams::TIFF);
    SetValueLong(AnimationParams::CaptureStartTag, "Set default value for capturing imagery start time", GetStartTimestep());
    SetValueLong(AnimationParams::CaptureEndTag, "Set default value for capturing imagery end time", GetEndTimestep());
}
