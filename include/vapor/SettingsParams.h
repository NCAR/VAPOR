//************************************************************************
//         *
//       Copyright (C)  2015    *
//     University Corporation for Atmospheric Research   *
//       All Rights Reserved    *
//         *
//************************************************************************/
//
// File:  SettingsParams.h
//
// Authors:
//   Scott Pearse
//   Alan Norton
//   National Center for Atmospheric Research
//   PO 3000, Boulder, Colorado
//
// Date:  February 2018
//
// Description: Defines the SettingsParams class.
//  This class supports parameters associted with the
//  Settings panel, describing the settings settings of the app.
//
#ifndef SETTINGSPARAMS_H
#define SETTINGSPARAMS_H

#include <vector>
#include <vapor/ParamsBase.h>

//! \class SettingsParams
//! \ingroup Public_Params
//! \brief A class for describing settings at settings.
//! \authors Scott Pearse, Alan Norton
//! \version 3.0
//! \date    February 2018

//! The SettingsParams class controls various features set when the application starts.
//! There is only a global SettingsParams, that
//! is used throughout the application
//!
class PARAMS_API SettingsParams : public VAPoR::ParamsBase {
public:
    SettingsParams(VAPoR::ParamsBase::StateSave *ssave, bool loadFromFile = true);
    SettingsParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);
    SettingsParams(const SettingsParams &rhs);
    SettingsParams &operator=(const SettingsParams &rhs);

    void Init();

    ~SettingsParams();

    int  GetNumThreads() const;
    void SetNumThreads(int num);

    long GetCacheMB() const;
    void SetCacheMB(long val);

    long GetTextureSize() const;
    void SetTextureSize(long val);
    void SetTexSizeEnable(bool val);
    bool GetTexSizeEnable() const;

    bool GetAutoStretchEnabled() const;
    void SetAutoStretchEnabled(bool val);

    int  GetJpegQuality() const;
    void SetJpegQuality(int quality);

    bool GetSessionAutoSaveEnabled() const;
    void SetSessionAutoSaveEnabled(bool enabled);

    int  GetChangesPerAutoSave() const;
    void SetChangesPerAutoSave(int changes);

    string GetAutoSaveSessionFile() const;
    void   SetAutoSaveSessionFile(string file);
    void   SetDefaultAutoSaveFile(string file);

    string GetSessionDir() const;
    void   SetSessionDir(string name);
    string GetDefaultSessionDir() const;
    void   SetDefaultSessionDir(string dir);

    string GetMetadataDir() const;
    void   SetMetadataDir(string dir);
    string GetDefaultMetadataDir() const;
    void   SetDefaultMetadataDir(string dir);

    string GetTFDir() const;
    void   SetTFDir(string dir);
    string GetDefaultTFDir() const;
    void   SetDefaultTFDir(string dir);

    string GetFlowDir() const;
    void   SetFlowDir(string dir);
    string GetDefaultFlowDir() const;
    void   SetDefaultFlowDir(string dir);

    string GetPythonDir() const;
    void   SetPythonDir(string dir);
    string GetDefaultPythonDir() const;
    void   SetDefaultPythonDir(string dir);

    int  GetFontSize() const;
    void SetFontSize(int size);

    string GetFontFile() const;
    void   SetFontFile(string file);

    bool GetDontShowIntelDriverWarning() const;
    void SetDontShowIntelDriverWarning(bool b);

    string GetCurrentPrefsPath() const;
    void   SetCurrentPrefsPath(string pth);

    void              Reinit();
    const std::string getShortName() { return _shortName; }

    void SetFidelityDefault3D(long lodDef, long refDef);
    void SetFidelityDefault2D(long lodDef, long refDef);

    static string GetClassType() { return (_classType); }

    int SaveSettings() const;

    static const string _sessionAutoSaveEnabledTag;

    std::string GetSettingsPath() const;

    void SetAutoCheckForUpdates(bool b);
    bool GetAutoCheckForUpdates() const;
    void SetAutoCheckForNotices(bool b);
    bool GetAutoCheckForNotices() const;
    void SetCasperCheckForVGL(bool b);
    bool GetCasperCheckForVGL() const;

    static const string UseAllCoresTag;
    static const string AutoCheckForUpdatesTag;
    static const string AutoCheckForNoticesTag;
    static const string CasperVGLCheck;

    bool LoadFromSettingsFile();
    void FindDefaultSettingsPath();

private:
    static const string _classType;
    static const string _shortName;
    static const string _numThreadsTag;
    static const string _cacheMBTag;
    static const string _texSizeTag;
    static const string _texSizeEnableTag;
    static const string _currentPrefsPathTag;
    static const string _sessionDirTag;
    static const string _defaultSessionDirTag;
    static const string _metadataDirTag;
    static const string _defaultMetadataDirTag;
    static const string _tfDirTag;
    static const string _defaultTfDirTag;
    static const string _flowDirTag;
    static const string _defaultFlowDirTag;
    static const string _pythonDirTag;
    static const string _defaultPythonDirTag;
    static const string _fidelityDefault2DTag;
    static const string _fidelityDefault3DTag;
    static const string _autoStretchTag;
    static const string _jpegQualityTag;
    static const string _changesPerAutoSaveTag;
    static const string _autoSaveFileLocationTag;
    static const string _defaultAutoSaveFileTag;
    static const string _fontFileTag;
    static const string _fontSizeTag;
    static const string _dontShowIntelDriverWarningTag;
    static const string _settingsNeedsWriteTag;

    void _swapTildeWithHome(std::string &file) const;

    string _settingsPath;
};

#endif    // SETTINGS_H
