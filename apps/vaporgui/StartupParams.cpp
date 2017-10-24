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

#include <QDir>
#include <iostream>
#include <ostream>
#include <fstream>
#include <vector>
#include <vapor/GetAppPath.h>

#include "StartupParams.h"


using namespace VAPoR;
using namespace Wasp;

const string StartupParams::_classType = "StartupParams";

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

namespace {
	string StartupFile = ".vapor3_startup";
}


StartupParams::StartupParams(
    ParamsBase::StateSave *ssave
) : ParamsBase(ssave, _classType) {


	_startupPath = QDir::homePath().toStdString();
	_startupPath += QDir::separator().toAscii();
	_startupPath += StartupFile;

	// Try to get startup params from .startup file
	//
	bool ok = _loadFromStartupFile();
	if (ok) return;

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

		// Try to get startup params from .startup file
		//
		bool ok = _loadFromStartupFile();
		if (ok) return;

		_init();
	}
}

void StartupParams::Reinit() {
	_init();
}


StartupParams::~StartupParams(){
	
}

bool StartupParams::_loadFromStartupFile() {

    XmlNode *node = GetNode();
    assert(node != NULL);

    bool enabled = MyBase::GetEnableErrMsg();
    MyBase::EnableErrMsg(false);

    XmlParser xmlparser;
    int rc = xmlparser.LoadFromFile(node, _startupPath);
	bool status = rc >= 0;

    MyBase::EnableErrMsg(enabled);

	return(status);
}

int StartupParams::SaveStartup() const {

    ofstream fileout;
    string s;

    fileout.open(_startupPath.c_str());
    if (! fileout) {
        MyBase::SetErrMsg(
            "Unable to open output startup file \"_startupPath.c_str()\" : %M"
        );
        return(-1);
    }

    const XmlNode *node = GetNode();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        MyBase::SetErrMsg(
            "Unable to write output startup file \"_startupPath.c_str()\" : %M"
        );
		return(-1);
    }

    fileout.close();
	return(0);
}


//Reset startup settings to initial state
void StartupParams::_init() {


	SetAutoStretch(false);
	SetCacheMB(2000);
	SetTextureSize(0);
	SetTexSizeEnable(false);
	SetWinSizeLock(false);
	SetWinSize(1028, 1024);

	SetSessionDir(string("~"));
	SetMetadataDir(string("~"));
	SetImageDir(string("~"));
	SetFlowDir(string("~"));

	SetFidelityDefault3D(4.,4.);
	SetFidelityDefault2D(2.,2.);

	vector <string> ppaths = {"palettes"};
	string palettes = GetAppPath("VAPOR", "share", ppaths);
	SetTFDir(string(palettes));

	vector <string> ipaths = {"images"};
	string images = GetAppPath("VAPOR", "share", ipaths);
	SetImageDir(string(images));

	vector <string> pypaths = {"python"};
	string python = GetAppPath("VAPOR", "share", pypaths);
	SetPythonDir(string(python));
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

