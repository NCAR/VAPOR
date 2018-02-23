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
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
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
const string StartupParams::_jpegQualityTag = "JpegImageQuality";
const string StartupParams::_autoSaveSessionTag = "AutoSaveSession";
const string StartupParams::_changesPerAutoSaveTag = "ChangesPerAutoSave";
const string StartupParams::_autoSaveFileLocation = "AutoSaveFileLocation";

//
// Register class with object factory!!!
//
static ParamsRegistrar<StartupParams> registrar(StartupParams::GetClassType());

namespace {
string StartupFile = ".vapor3_startup";
}

StartupParams::StartupParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, _classType)
{
    _startupPath = QDir::homePath().toStdString();
    _startupPath += QDir::separator().toAscii();
    _startupPath += StartupFile;

    // Try to get startup params from .startup file
    //
    bool ok = _loadFromStartupFile();
    if (ok) return;

    _init();
}

StartupParams::StartupParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
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

void StartupParams::Reinit() { _init(); }

StartupParams::~StartupParams() {}

bool StartupParams::GetAutoStretch() const { return (0 != GetValueLong(_autoStretchTag, (long)false)); }

void StartupParams::SetAutoStretch(bool val) { SetValueLong(_autoStretchTag, "Enable Auto Stretch", val); }

int StartupParams::GetJpegQuality() const
{
    int quality = (int)GetValueDouble(_jpegQualityTag, 100.f);
    return quality;
}

void StartupParams::SetJpegQuality(int quality)
{
    string description = "Specify the quality of JPEG screen captures";
    SetValueDouble(_jpegQualityTag, description, quality);
}

long StartupParams::GetCacheMB() const
{
    long val = GetValueLong(_cacheMBTag, 1000);
    if (val < 0) val = 1000;
    return (val);
}

void StartupParams::SetCacheMB(long val)
{
    if (val < 0) val = 1000;
    SetValueLong(_cacheMBTag, "Set cache size", val);
}

long StartupParams::GetTextureSize() const
{
    long val = GetValueLong(_texSizeTag, 0);
    if (val < 0) val = 0;
    return (val);
}

void StartupParams::SetTextureSize(long val)
{
    if (val < 0) val = 0;
    SetValueLong(_texSizeTag, "Set graphic tex size", val);
}

void StartupParams::SetTexSizeEnable(bool val) { SetValueLong(_texSizeEnableTag, "toggle enable texture size", (long)val); }

bool StartupParams::GetTexSizeEnable() const { return (0 != GetValueLong(_texSizeEnableTag, (long)0)); }

void StartupParams::SetWinSizeLock(bool val) { SetValueLong(_winSizeLockTag, "toggle lock window size", (long)val); }

bool StartupParams::GetWinSizeLock() const { return (0 != GetValueLong(_winSizeLockTag, (long)false)); }

string StartupParams::GetSessionDir() const
{
    string dir = GetValueString(_sessionDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetSessionDir(string name) { SetValueString(_sessionDirTag, "set session directory", name); }

string StartupParams::GetMetadataDir() const
{
    string dir = GetValueString(_metadataDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetMetadataDir(string dir) { SetValueString(_metadataDirTag, "set metadata directory", dir); }

string StartupParams::GetImageDir() const
{
    string dir = GetValueString(_imageDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetImageDir(string dir) { SetValueString(_imageDirTag, "set image directory", dir); }

string StartupParams::GetTFDir() const
{
    string dir = GetValueString(_tfDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetTFDir(string dir) { SetValueString(_tfDirTag, "set trans function directory", dir); }

string StartupParams::GetFlowDir() const
{
    string dir = GetValueString(_flowDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetFlowDir(string dir) { SetValueString(_flowDirTag, "set flow save directory", dir); }

string StartupParams::GetPythonDir() const
{
    string dir = GetValueString(_pythonDirTag, string("."));
    if (dir == "~") { dir = QDir::homePath().toStdString(); }
    return (dir);
}

void StartupParams::SetPythonDir(string dir) { SetValueString(_pythonDirTag, "set python directory", dir); }

string StartupParams::GetCurrentPrefsPath() const
{
    string defaultv = "/tmp/.vapor3_prefs";
    return GetValueString(_currentPrefsPathTag, defaultv);
}

void StartupParams::SetCurrentPrefsPath(string pth) { SetValueString(_currentPrefsPathTag, "set current preference path", pth); }

size_t StartupParams::GetNumExecutionThreads() const
{
    long val = GetValueLong(_numExecutionThreads, 0);
    if (val < 0) val = 0;
    return ((size_t)val);
}

void StartupParams::SetNumExecutionThreads(size_t val) { SetValueLong(_numExecutionThreads, "Number of execution threads", 0); }

void StartupParams::SetFidelityDefault3D(long lodDef, long refDef)
{
    vector<long> val;
    val.push_back(lodDef);
    val.push_back(refDef);
    SetValueLongVec(_fidelityDefault3DTag, "Set fidelity 3D default", val);
}

void StartupParams::SetFidelityDefault2D(long lodDef, long refDef)
{
    vector<long> val;
    val.push_back(lodDef);
    val.push_back(refDef);
    SetValueLongVec(_fidelityDefault2DTag, "Set fidelity 2D default", val);
}

bool StartupParams::_loadFromStartupFile()
{
    XmlNode *node = GetNode();
    assert(node != NULL);

    bool enabled = MyBase::GetEnableErrMsg();
    MyBase::EnableErrMsg(false);

    XmlParser xmlparser;
    int       rc = xmlparser.LoadFromFile(node, _startupPath);
    bool      status = rc >= 0;

    MyBase::EnableErrMsg(enabled);

    return (status);
}

int StartupParams::SaveStartup() const
{
    ofstream fileout;
    string   s;

    fileout.open(_startupPath.c_str());
    if (!fileout) {
        MyBase::SetErrMsg("Unable to open output startup file \"_startupPath.c_str()\" : %M");
        return (-1);
    }

    const XmlNode *node = GetNode();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        MyBase::SetErrMsg("Unable to write output startup file \"_startupPath.c_str()\" : %M");
        return (-1);
    }

    fileout.close();
    return (0);
}

// Reset startup settings to initial state
void StartupParams::_init()
{
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

    SetFidelityDefault3D(4., 4.);
    SetFidelityDefault2D(2., 2.);

    vector<string> ppaths = {"palettes"};
    string         palettes = GetAppPath("VAPOR", "share", ppaths);
    SetTFDir(string(palettes));

    vector<string> ipaths = {"images"};
    string         images = GetAppPath("VAPOR", "share", ipaths);
    SetImageDir(string(images));

    vector<string> pypaths = {"python"};
    string         python = GetAppPath("VAPOR", "share", pypaths);
    SetPythonDir(string(python));
}

void StartupParams::SetWinSize(size_t width, size_t height)
{
    if (width < 400) width = 400;
    if (height < 400) height = 400;

    vector<long> val;
    val.push_back(width);
    val.push_back(height);
    SetValueLongVec(_winSizeTag, "Set window size", val);
}

void StartupParams::GetWinSize(size_t &width, size_t &height) const
{
    vector<long> defaultv;
    defaultv.push_back(1280);
    defaultv.push_back(1024);
    vector<long> val = GetValueLongVec(_winSizeTag, defaultv);
    if (val[0] < 400) val[0] = defaultv[0];
    if (val[1] < 400) val[1] = defaultv[1];
    width = val[0];
    height = val[1];
}
