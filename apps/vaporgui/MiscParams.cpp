//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MiscParams.cpp
//
//	Author:		Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2017
//
//	Description:	Implements the MiscParams class
//		This is derived from the Params class
//		It contains all the parameters required for miscellaneous components

//

#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>

#include "MiscParams.h"

using namespace VAPoR;
const string MiscParams::_timeStepTag = "TimeStep";
const string MiscParams::_timeStampTag = "TimeStamp";
const string MiscParams::_timeAnnotLLXTag = "TimeAnnotLLX";
const string MiscParams::_timeAnnotLLYTag = "TimeAnnotLLY";
const string MiscParams::_timeAnnotSizeTag = "TimeAnnotSize";
const string MiscParams::_timeAnnotColorTag = "TimeAnnotColor";

//
// Register class with object factory!!!
//
static ParamsRegistrar<MiscParams> registrar(MiscParams::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
MiscParams::MiscParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, MiscParams::GetClassType()) { _init(); }

MiscParams::MiscParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != MiscParams::GetClassType()) {
        node->SetTag(MiscParams::GetClassType());
        _init();
    }
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
MiscParams::~MiscParams() { MyBase::SetDiagMsg("MiscParams::~MiscParams() this=%p", this); }

// Reset to initial state
//
void MiscParams::_init()
{
    SetTimeStamp(false);
    SetTimeStep(false);
}
