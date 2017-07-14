//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		StartupParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the StartupParams class.
//		This class supports parameters associted with the
//		initial VAPOR startup settings
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <iostream>
#include <vector>

#include "StartupParams.h"


using namespace VAPoR;

const string StartupParams::m_classType = "StartupParams";

const string StartupParams::_shortName = "Startup";
const string StartupParams::_cacheMBTag = "CacheMBs";
const string StartupParams::_texSizeTag = "TexSize";
const string StartupParams::_texSizeEnableTag = "TexSizeEnabled";
const string StartupParams::_winSizeTag = "WinSize";
const string StartupParams::_winSizeLockTag = "WinSizeLocked";
const string StartupParams::_sessionDirTag = "SessionDir";
const string StartupParams::_metadataDirTag = "MetadataDir";
const string StartupParams::_imageDirTag = "ImageDir";
const string StartupParams::_tfDirTag = "TFDir";
const string StartupParams::_flowDirTag = "FlowDir";
const string StartupParams::_pythonDirTag = "PythonDir";
const string StartupParams::_currentPrefsPathTag = "CurrentPrefsPath";
const string StartupParams::_fidelityDefault2DTag = "Fidelity2DDefault";
const string StartupParams::_fidelityDefault3DTag = "Fidelity3DDefault";
const string StartupParams::_autoStretchTag = "AutoStretch";
const string StartupParams::_numExecutionThreads = "NumExecutionThreads";

//
// Register class with object factory!!!
//
static ParamsRegistrar<StartupParams> registrar(StartupParams::GetClassType());


StartupParams::StartupParams(
    ParamsBase::StateSave *ssave
) : ParamsBase(ssave, m_classType) {

    _init();
}


StartupParams::StartupParams(
    ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node)
{
	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != StartupParams::GetClassType()) {
		node->SetTag(StartupParams::GetClassType());
		_init();
	}
}


StartupParams::~StartupParams(){
	
}


//Reset startup settings to initial state
void StartupParams::_init(){
	SetAutoStretch(false);
	SetCacheMB(2000);
	SetTextureSize(0);
	SetTexSizeEnable(false);
	SetWinSizeLock(false);
	SetWinSize(1028, 1024);
	SetSessionDir(string("."));
	SetMetadataDir(string("."));
	SetImageDir(string("."));
	SetFlowDir(string("."));
	SetTFDir(string("."));
	SetPythonDir(string("."));
	SetFidelityDefault3D(4.,4.);
	SetFidelityDefault2D(2.,2.);
}

void StartupParams::SetWinSize(size_t width, size_t height) {
	if (width < 400) width = 400;
	if (height < 400) height = 400;

	vector <long> val;
	val.push_back(width);
	val.push_back(height);
	SetValueLongVec(_winSizeTag, "Set window size", val);
}

void StartupParams::GetWinSize(size_t &width, size_t &height) const {
	vector <long> defaultv;
	defaultv.push_back(1280);
	defaultv.push_back(1024);
	vector <long> val = GetValueLongVec(_winSizeTag,defaultv);
	if (val[0] < 400) val[0] = defaultv[0];
	if (val[1] < 400) val[1] = defaultv[1];
	width = val[0];
	height = val[1];
}

#ifdef	DEAD
void StartupParams::CopyPathsToSession(){
	PathParams::SetCurrentDataPath(GetMetadataDir());
	PathParams::SetCurrentImagePath(GetImageDir());
	PathParams::SetCurrentSessionPath(GetSessionDir());
	PathParams::SetCurrentFlowPath(GetFlowDir());
	PathParams::SetCurrentPythonPath(GetPythonDir());
	PathParams::SetCurrentTFPath(GetTFDir());
}
#endif
