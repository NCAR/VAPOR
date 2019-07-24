//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		OSPRayParams.cpp
//
//	Authors:
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the OSPRayParams class.
//

#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include <vapor/ResourcePath.h>

#include <vapor/OSPRayParams.h>


using namespace VAPoR;
using namespace Wasp;

const string OSPRayParams::_classType = "OSPRayParams";
const string OSPRayParams::_shortName = "OSPRay";
const string OSPRayParams::_numThreadsTag = "NumThreads";
const string OSPRayParams::_aoSamplesTag = "aoSamples";
const string OSPRayParams::_samplesPerPixelTag = "SamplesPerPixel";
const string OSPRayParams::_ambientIntensity = "AmbientIntensity";
const string OSPRayParams::_spotlightIntensity = "SpotlightIntensity";

//
// Register class with object factory!!!
//
static ParamsRegistrar<OSPRayParams> registrar(OSPRayParams::GetClassType());

namespace {
	string SettingsFile = ".vapor3_settings";
}

OSPRayParams::OSPRayParams(ParamsBase::StateSave *ssave)
: ParamsBase(ssave, _classType) {

    _init();
}

OSPRayParams::OSPRayParams(
    ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node)
{
	if (node->GetTag() != OSPRayParams::GetClassType()) {
		node->SetTag(OSPRayParams::GetClassType());
	
        _init();
	}
}

OSPRayParams::OSPRayParams(
	const OSPRayParams &rhs
) : ParamsBase(new ParamsBase::StateSave, _classType) {
	_init();
}

OSPRayParams &OSPRayParams::operator=( const OSPRayParams& rhs ) {
	ParamsBase::operator=(rhs);
	return (*this);
}

OSPRayParams::~OSPRayParams() {
	
}






int OSPRayParams::GetNumThreads() const {
    long val = GetValueLong(_numThreadsTag, 0);
    val = val >= 0 ? val : 0; 
    return((int) val);
}

void OSPRayParams::SetNumThreads(int val) {
	SetValueLong(_numThreadsTag, "Number of execution threads", val);
}

int OSPRayParams::GetAOSamples() const { return GetValueLong(_aoSamplesTag, 0); }
void OSPRayParams::SetAOSamples(int n) { SetValueLong(_aoSamplesTag, "Number of ambient occlusion samples", n); }

//Reset settings settings to initial state
void OSPRayParams::_init() {
    SetValueLong(_samplesPerPixelTag, "Samples Per Pixel", 1);
    SetValueDouble(_ambientIntensity, "Ambient Intensity", 0.6);
    SetValueDouble(_spotlightIntensity, "Spotlight Intensity", 1);
}
