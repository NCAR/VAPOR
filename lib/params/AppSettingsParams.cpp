//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AppSettingsparams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2015
//
//	Description:	Implements the AppSettingsParams class.
//		This class supports application settings
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>

#include "vapor/AppSettingsParams.h"

using namespace VAPoR;

const string AppSettingsParams::m_classType = "ApplicationSettings";

const string AppSettingsParams::_shortName = "AppSettings";
const string AppSettingsParams::_currentJpegQualityTag = "CurrentJpegQuality";
const string AppSettingsParams::_currentAutosaveEnabledTag = "CurrentAutosaveEnabled";
const string AppSettingsParams::_currentAutosaveIntervalTag = "CurrentAutosaveInterval";
const string AppSettingsParams::_currentShowDataWarningTag = "CurrentWarnMissingData";
const string AppSettingsParams::_currentUseLessAccurateTag = "CurrentUseLessAccurateData";
const string AppSettingsParams::_currentTrackMouseTag = "CurrentTrackMouse";
const string AppSettingsParams::_currentShowCitationTag = "CurrentShowCitation";
const string AppSettingsParams::_currentLogFileNameTag = "CurrentLogfileName";
const string AppSettingsParams::_currentAutosaveFileNameTag = "CurrentAutosaveName";
const string AppSettingsParams::_currentMessagesSilencedTag = "CurrentMessageSilence";
const string AppSettingsParams::_currentLogfileEnabledTag = "CurrentLogfileEnabled";
const string AppSettingsParams::_logFileNameTag = "LogfileName";
const string AppSettingsParams::_logfileEnabledTag = "LogfileEnabled";
const string AppSettingsParams::_autosaveFileNameTag = "AutosaveName";
const string AppSettingsParams::_jpegQualityTag = "JpegQuality";
const string AppSettingsParams::_autosaveEnabledTag = "AutosaveEnabled";
const string AppSettingsParams::_autosaveIntervalTag = "AutosaveInterval";
const string AppSettingsParams::_showDataWarningTag = "WarnMissingData";
const string AppSettingsParams::_useLessAccurateTag = "UseLessAccurateData";
const string AppSettingsParams::_trackMouseTag = "TrackMouse";
const string AppSettingsParams::_showCitationTag = "ShowCitation";
const string AppSettingsParams::_messagesSilencedTag = "MessageSilence";

static ParamsRegistrar<AppSettingsParams> registrar(AppSettingsParams::GetClassType());

// Reset AppSettings settings to initial state
void AppSettingsParams::_init()
{
    SetJpegQuality(99);
    SetAutosaveInterval(10);
    SetAutosaveEnabled(true);
    SetShowWarning(false);
    SetUseLessAccurate(true);
    SetTrackMouse(true);
    SetShowCitation(true);
    SetLogFileName(string("/tmp/logfile.txt"));
    SetAutosaveName(string("/tmp/autosave.vs3"));
    SetMessageSilence(false);
    SetLogfileEnabled(false);
}

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
AppSettingsParams::AppSettingsParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, m_classType) { _init(); }

AppSettingsParams::AppSettingsParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != AppSettingsParams::GetClassType()) {
        node->SetTag(AppSettingsParams::GetClassType());
        _init();
    }
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
AppSettingsParams::~AppSettingsParams() { MyBase::SetDiagMsg("AppSettingsParams::~AppSettingsParams() this=%p", this); }

void AppSettingsParams::restoreDefaults() { _init(); }

#ifdef DEAD
void AppSettingsParams::saveCurrentSettings()
{
    // This copying should not be saved as a command;
    // It cannot be undone.  Copy all the current settings to the permanent settings.
    SetJpegQuality(GetCurrentJpegQuality());
    SetAutosaveInterval(GetCurrentAutosaveInterval());
    SetAutosaveEnabled(GetCurrentAutosaveEnabled());
    SetShowWarning(GetCurrentShowWarning());
    SetUseLessAccurate(GetCurrentUseLessAccurate());
    SetTrackMouse(GetCurrentTrackMouse());
    SetShowCitation(GetCurrentShowCitation());
    SetMessageSilence(GetCurrentMessageSilence());
    SetLogFileName(GetCurrentLogFileName());
    SetAutosaveName(GetCurrentAutosaveName());
    SetLogfileEnabled(GetCurrentLogfileEnabled());
}

void AppSettingsParams::copySavedSettings()
{
    SetCurrentJpegQuality(GetJpegQuality());
    SetCurrentAutosaveInterval(GetAutosaveInterval());
    SetCurrentAutosaveEnabled(GetAutosaveEnabled());
    SetCurrentShowWarning(GetShowWarning());
    SetCurrentUseLessAccurate(GetUseLessAccurate());
    SetCurrentTrackMouse(GetTrackMouse());
    SetCurrentShowCitation(GetShowCitation());
    SetCurrentMessageSilence(GetMessageSilence());
    SetCurrentLogfileEnabled(GetLogfileEnabled());
}

#endif

void AppSettingsParams::SetJpegQuality(int val)
{
    if (val < 1 || val > 99) val = 99;
    SetValueLong(_jpegQualityTag, "Set jpeg quality", val);
}

long AppSettingsParams::GetJpegQuality() const
{
    int defaultv = 99;
    int val = GetValueLong(_jpegQualityTag, defaultv);
    if (val < 1 || val > 99) val = defaultv;
    return (val);
}

void AppSettingsParams::SetAutosaveInterval(int val)
{
    if (val < 1) val = 1;
    SetValueLong(_autosaveIntervalTag, "Set autosave interval", val);
}

long AppSettingsParams::GetAutosaveInterval() const
{
    int defaultv = 5;
    int val = GetValueLong(_autosaveIntervalTag, defaultv);
    if (val < 1) val = defaultv;
    return (val);
}

void AppSettingsParams::SetCurrentJpegQuality(int val)
{
    if (val < 1 || val > 99) val = 99;
    SetValueLong(_currentJpegQualityTag, "Set current jpeg quality", val);
}

long AppSettingsParams::GetCurrentJpegQuality() const
{
    int defaultv = 99;
    int val = GetValueLong(_currentJpegQualityTag, defaultv);
    if (val < 1 || val > 99) val = defaultv;
    return (val);
}

void AppSettingsParams::SetCurrentAutosaveInterval(int val)
{
    if (val < 1) val = 1;
    SetValueLong(_currentAutosaveIntervalTag, "Set current autosave interval", val);
}

long AppSettingsParams::GetCurrentAutosaveInterval() const
{
    int defaultv = 5;
    int val = GetValueLong(_currentAutosaveIntervalTag, defaultv);
    if (val < 1) val = defaultv;
    return (val);
}
