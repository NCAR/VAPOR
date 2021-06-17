//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		SettingsParams.cpp
//
//	Author: Scott Pearse
//			Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Implements the SettingsParams class.
//		This class supports parameters associted with the
//		initial VAPOR settings settings
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
#include <vapor/ResourcePath.h>

#include "SettingsParams.h"

using namespace VAPoR;
using namespace Wasp;

const string SettingsParams::_classType = "SettingsParams";

const string SettingsParams::_shortName = "Settings";
const string SettingsParams::_cacheMBTag = "CacheMBs";
const string SettingsParams::_numThreadsTag = "NumThreads";
const string SettingsParams::_texSizeTag = "TexSize";
const string SettingsParams::_texSizeEnableTag = "TexSizeEnabled";
const string SettingsParams::_sessionDirTag = "SessionDir";
const string SettingsParams::_defaultSessionDirTag = "SessionDir";
const string SettingsParams::_metadataDirTag = "MetadataDir";
const string SettingsParams::_defaultMetadataDirTag = "MetadataDir";
const string SettingsParams::_tfDirTag = "TFDir";
const string SettingsParams::_defaultTfDirTag = "TFDir";
const string SettingsParams::_flowDirTag = "FlowDir";
const string SettingsParams::_defaultFlowDirTag = "FlowDir";
const string SettingsParams::_pythonDirTag = "PythonDir";
const string SettingsParams::_defaultPythonDirTag = "PythonDir";
const string SettingsParams::_currentPrefsPathTag = "CurrentPrefsPath";
const string SettingsParams::_fidelityDefault2DTag = "Fidelity2DDefault";
const string SettingsParams::_fidelityDefault3DTag = "Fidelity3DDefault";
const string SettingsParams::_autoStretchTag = "AutoStretch";
const string SettingsParams::_jpegQualityTag = "JpegImageQuality";
const string SettingsParams::_changesPerAutoSaveTag = "ChangesPerAutoSave";
const string SettingsParams::_autoSaveFileLocationTag = "AutoSaveFileLocation";
const string SettingsParams::_defaultAutoSaveFileTag = "DefaultAutoSaveFile";
const string SettingsParams::_sessionAutoSaveEnabledTag = "AutoSaveEnabled";
const string SettingsParams::_fontFileTag = "FontFile";
const string SettingsParams::_fontSizeTag = "FontSize";
const string SettingsParams::_dontShowIntelDriverWarningTag = "DontShowIntelDriverWarning";
const string SettingsParams::_settingsNeedsWriteTag = "SettingsNeedsWrite";

const string SettingsParams::UseAllCoresTag = "UseAllCoresTag";
const string SettingsParams::AutoCheckForUpdatesTag = "AutoCheckForUpdatesTag";

//
// Register class with object factory!!!
//
static ParamsRegistrar<SettingsParams> registrar(SettingsParams::GetClassType());

namespace {
string       SettingsFile = ".vapor3_settings";
const size_t defaultCacheSize = 0;
}    // namespace

SettingsParams::SettingsParams(ParamsBase::StateSave *ssave, bool loadFromFile) : ParamsBase(ssave, _classType)
{
    _settingsPath = QDir::homePath().toStdString();
    _settingsPath += QDir::separator().toLatin1();
    _settingsPath += SettingsFile;

    // Try to get settings params from .settings file
    //
    if (loadFromFile) {
        bool ok = LoadFromSettingsFile();
        if (ok) return;
    }

    Init();
}

SettingsParams::SettingsParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    _settingsPath = QDir::homePath().toStdString();
    _settingsPath += QDir::separator().toLatin1();
    _settingsPath += SettingsFile;

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != SettingsParams::GetClassType()) {
        node->SetTag(SettingsParams::GetClassType());

        // Try to get settings params from .settings file
        //
        bool ok = LoadFromSettingsFile();
        if (ok)
            return;
        else
            Init();
    }
}

SettingsParams::SettingsParams(const SettingsParams &rhs) : ParamsBase(new ParamsBase::StateSave, _classType)
{
    _settingsPath = QDir::homePath().toStdString();
    _settingsPath += QDir::separator().toLatin1();
    _settingsPath += SettingsFile;
    Init();
}

SettingsParams &SettingsParams::operator=(const SettingsParams &rhs)
{
    ParamsBase::operator=(rhs);

    _settingsPath = QDir::homePath().toStdString();
    _settingsPath += QDir::separator().toLatin1();
    _settingsPath += SettingsFile;

    return (*this);
}

void SettingsParams::Reinit() { Init(); }

SettingsParams::~SettingsParams() {}

void SettingsParams::_swapTildeWithHome(std::string &file) const
{
    size_t pos = 0;
    while ((pos = file.find("~", pos)) != std::string::npos) {
        file.replace(pos, 1, QDir::homePath().toStdString());
        pos += QDir::homePath().toStdString().length();
    }
}

long SettingsParams::GetCacheMB() const
{
    long val = GetValueLong(_cacheMBTag, 0);
    if (val < 0) val = defaultCacheSize;

    return (val);
}

void SettingsParams::SetCacheMB(long val)
{
    if (val < 0) val = defaultCacheSize;

    SetValueLong(_cacheMBTag, "Set cache size", val);
}

long SettingsParams::GetTextureSize() const
{
    long val = GetValueLong(_texSizeTag, 0);
    if (val < 0) val = 0;
    return (val);
}

void SettingsParams::SetTextureSize(long val)
{
    if (val < 0) val = 0;
    SetValueLong(_texSizeTag, "Set graphic tex size", val);
}

void SettingsParams::SetTexSizeEnable(bool val) { SetValueLong(_texSizeEnableTag, "toggle enable texture size", (long)val); }

bool SettingsParams::GetTexSizeEnable() const { return (0 != GetValueLong(_texSizeEnableTag, (long)0)); }

bool SettingsParams::GetAutoStretchEnabled() const { return (0 != GetValueLong(_autoStretchTag, (long)true)); }

void SettingsParams::SetAutoStretchEnabled(bool val) { SetValueLong(_autoStretchTag, "Enable Auto Stretch", val); }

int SettingsParams::GetJpegQuality() const
{
    int quality = (int)GetValueDouble(_jpegQualityTag, 100.f);
    return quality;
}

void SettingsParams::SetJpegQuality(int quality)
{
    string description = "Specify the quality of JPEG screen captures";
    SetValueDouble(_jpegQualityTag, description, quality);
}

bool SettingsParams::GetSessionAutoSaveEnabled() const
{
    double enabled = GetValueLong(_sessionAutoSaveEnabledTag, 1);
    if (enabled > 0)
        return true;
    else
        return false;
}

void SettingsParams::SetSessionAutoSaveEnabled(bool enabled)
{
    string description = "Enable/disable auto save of session files";
    SetValueLong(_sessionAutoSaveEnabledTag, description, (long)enabled);
}

int SettingsParams::GetChangesPerAutoSave() const
{
    int changes = (int)GetValueLong(_changesPerAutoSaveTag, 5);
    return changes;
}

void SettingsParams::SetChangesPerAutoSave(int count)
{
    if (count < 0) count = 5;
    string description = "User changes before auto saving session file";
    SetValueLong(_changesPerAutoSaveTag, description, count);
}

string SettingsParams::GetAutoSaveSessionFile() const
{
    string autoSaveDir = QDir::homePath().toStdString();
    string defaultFile = autoSaveDir + "/VaporAutoSave.vs3";

    string file = GetValueString(_autoSaveFileLocationTag, defaultFile);

    _swapTildeWithHome(file);

    return file;
}

void SettingsParams::SetAutoSaveSessionFile(string file)
{
    string description = "Session auto-save file location";
    SetValueString(_autoSaveFileLocationTag, description, file);
}

string SettingsParams::GetSessionDir() const
{
    string defaultDir = GetDefaultSessionDir();
    string dir = GetValueString(_sessionDirTag, defaultDir);
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetSessionDir(string name) { SetValueString(_sessionDirTag, "Set session directory", name); }

string SettingsParams::GetDefaultSessionDir() const
{
    string dir = GetValueString(_defaultSessionDirTag, string("."));
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetDefaultAutoSaveFile(string autoSaveFile)
{
    string description = "Set default auto-save session directory";
    SetValueString(_defaultAutoSaveFileTag, description, autoSaveFile);
}

void SettingsParams::SetDefaultSessionDir(string name)
{
    string description = "Set default session directory";
    SetValueString(_defaultSessionDirTag, description, name);
}

string SettingsParams::GetMetadataDir() const
{
    string defaultDir = GetDefaultMetadataDir();
    string dir = GetValueString(_metadataDirTag, defaultDir);
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetMetadataDir(string dir) { SetValueString(_metadataDirTag, "set metadata directory", dir); }

string SettingsParams::GetDefaultMetadataDir() const
{
    string dir = GetValueString(_defaultMetadataDirTag, string("."));
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetDefaultMetadataDir(string dir)
{
    string description = "set default metadata directory";
    SetValueString(_defaultMetadataDirTag, description, dir);
}

string SettingsParams::GetTFDir() const
{
    string defaultDir = GetDefaultTFDir();
    string dir = GetValueString(_tfDirTag, defaultDir);
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetTFDir(string dir) { SetValueString(_tfDirTag, "set trans function directory", dir); }

string SettingsParams::GetDefaultTFDir() const
{
    string dir = GetValueString(_defaultTfDirTag, string(""));
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetDefaultTFDir(string dir)
{
    string description = "set default trans function directory";
    SetValueString(_defaultTfDirTag, description, dir);
}

string SettingsParams::GetFlowDir() const
{
    string defaultDir = GetDefaultFlowDir();
    string dir = GetValueString(_flowDirTag, defaultDir);
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetFlowDir(string dir) { SetValueString(_flowDirTag, "set flow save directory", dir); }

string SettingsParams::GetDefaultFlowDir() const
{
    string dir = GetValueString(_defaultFlowDirTag, string("."));
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetDefaultFlowDir(string dir) { SetValueString(_defaultFlowDirTag, "set default flow save directory", dir); }

string SettingsParams::GetPythonDir() const
{
    string defaultDir = GetDefaultPythonDir();
    string dir = GetValueString(_pythonDirTag, defaultDir);
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetPythonDir(string dir) { SetValueString(_pythonDirTag, "set python directory", dir); }

string SettingsParams::GetDefaultPythonDir() const
{
    string dir = GetValueString(_defaultPythonDirTag, string("."));
    _swapTildeWithHome(dir);
    return (dir);
}

void SettingsParams::SetDefaultPythonDir(string dir) { SetValueString(_defaultPythonDirTag, "set default python directory", dir); }

string SettingsParams::GetCurrentPrefsPath() const
{
    string defaultv = "/tmp/.vapor3_prefs";
    return GetValueString(_currentPrefsPathTag, defaultv);
}

void SettingsParams::SetCurrentPrefsPath(string pth) { SetValueString(_currentPrefsPathTag, "set current preference path", pth); }

int SettingsParams::GetNumThreads() const
{
    if (GetValueLong(UseAllCoresTag, true)) { return 0; }
    long val = GetValueLong(_numThreadsTag, 0);
    val = val >= 0 ? val : 0;
    return ((int)val);
}

void SettingsParams::SetNumThreads(int val) { SetValueLong(_numThreadsTag, "Number of execution threads", val); }

int SettingsParams::GetFontSize() const { return 24; }

void SettingsParams::SetFontSize(int size) {}

string SettingsParams::GetFontFile() const { return ""; }

void SettingsParams::SetFontFile(string file) {}

bool SettingsParams::GetDontShowIntelDriverWarning() const { return GetValueLong(_dontShowIntelDriverWarningTag, false); }

void SettingsParams::SetDontShowIntelDriverWarning(bool b) { SetValueLong(_dontShowIntelDriverWarningTag, "Hide Intel driver warning", b); }

void SettingsParams::SetFidelityDefault3D(long lodDef, long refDef)
{
    vector<long> val;
    val.push_back(lodDef);
    val.push_back(refDef);
    SetValueLongVec(_fidelityDefault3DTag, "Set fidelity 3D default", val);
}

void SettingsParams::SetFidelityDefault2D(long lodDef, long refDef)
{
    vector<long> val;
    val.push_back(lodDef);
    val.push_back(refDef);
    SetValueLongVec(_fidelityDefault2DTag, "Set fidelity 2D default", val);
}

void SettingsParams::SetAutoCheckForUpdates(bool b) { SetValueLong(AutoCheckForUpdatesTag, "", b); }

bool SettingsParams::GetAutoCheckForUpdates() const { return GetValueLong(AutoCheckForUpdatesTag, true); }

bool SettingsParams::LoadFromSettingsFile()
{
    XmlNode *node = GetNode();
    VAssert(node != NULL);

    bool enabled = MyBase::GetEnableErrMsg();
    MyBase::EnableErrMsg(false);

    XmlParser xmlparser;
    int       rc = xmlparser.LoadFromFile(node, _settingsPath);
    bool      status = rc >= 0;

    MyBase::EnableErrMsg(enabled);

    return status;
}

int SettingsParams::SaveSettings() const
{
    ofstream fileout;
    string   s;

    fileout.open(_settingsPath.c_str());
    if (!fileout) {
        MyBase::SetErrMsg("Unable to open output settings file %s : %M", _settingsPath.c_str());
        return (-1);
    }

    const XmlNode *node = GetNode();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        MyBase::SetErrMsg("Unable to open output settings file %s : %M", _settingsPath.c_str());
        return (-1);
    }

    fileout.close();
    return (0);
}

// Reset settings settings to initial state
void SettingsParams::Init()
{
    SetSessionAutoSaveEnabled(true);
    SetChangesPerAutoSave(5);
    std::string homeDir = QDir::homePath().toStdString();
    SetAutoSaveSessionFile(homeDir + "/VaporAutoSave.vs3");

    SetAutoStretchEnabled(true);
    SetValueLong(UseAllCoresTag, "", true);
    SetValueLong(AutoCheckForUpdatesTag, "", true);
    SetNumThreads(4);
    SetCacheMB(defaultCacheSize);


    SetDefaultSessionDir(string(homeDir));
    SetDefaultMetadataDir(string(homeDir));
    SetDefaultFlowDir(string(homeDir));

    string palettes = GetSharePath("palettes");
    SetDefaultTFDir(string(palettes));

    string python = GetPythonDir();
    SetDefaultPythonDir(string(python));
}

std::string SettingsParams::GetSettingsPath() const { return _settingsPath; }
