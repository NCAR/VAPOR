//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		AppSettingsParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2015
//
//	Description:	Defines the AppSettingsParams class.
//		This class supports parameters associted with the
//		AppSettings panel, describing the visual features in the visualizer
//
#ifndef APPSETTINGSPARAMS_H
#define APPSETTINGSPARAMS_H


#include <vector>
#include <vapor/ParamsBase.h>


//! \class AppSettingsParams
//! \ingroup Public_Params
//! \brief A class for describing visual features displayed in the visualizer.
//! \author Alan Norton
//! \version 3.0
//! \date    June 2015

//! The AppSettingsParams class controls various features displayed in the visualizers
//! There is a global AppSettingsParams, that 
//! is shared by all windows whose AppSettings is set to "global".  There is also
//! a local AppSettingsParams for each window, that users can select whenever there are multiple windows.
//! When local settings are used, they only affect one currently active visualizer.
//! The AppSettingsParams class also has several methods that are useful in setting up data requests from the DataMgr.
//!
class PARAMS_API AppSettingsParams : public VAPoR::ParamsBase {
	
public: 

 AppSettingsParams(
    VAPoR::ParamsBase::StateSave *ssave
 );

 AppSettingsParams(
     VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node
 );


 virtual ~AppSettingsParams();


	//! Pure virtual method on Params. Provide a short name suitable for use in the GUI
	//! \retval string name
	const std::string getShortName() {return _shortName;}

#ifdef	DEAD
	//! Copy the current settings to the saved settings
	//! Needed whenever user wants to make current settings default
	//! All of the settings in this Params have an additional "current" state
	//! The current state is what is shown in the tab panel
	void saveCurrentSettings();


	//! Copy the saved settings to the current settings.
	//! Needed whenever settings are loaded.
	void copySavedSettings();
#endif

	//! Restore default settings
	//! performed on user request.
	void restoreDefaults();

	//! Specify jpeg quality level
	//! \param[in] val quality level (1-99)
	void SetJpegQuality(int val) ;

	//! Obtain jpeg quality
	//! \return jpeg quality level
	long GetJpegQuality() const;

	//! Specify autosave interval
	//! \param[in] val autosave interval >= 1
	void SetAutosaveInterval(int val);

	//! Obtain autosave interval
	//! \return interval between autosaves
	long GetAutosaveInterval() const;
	
	bool GetAutosaveEnabled(){
		return (0 != GetValueLong(_autosaveEnabledTag,(long) true));
	}

	void SetAutosaveEnabled(bool onOff){
		SetValueLong(_autosaveEnabledTag, "toggle autosave enabled", (long)onOff);
	}

	bool GetShowWarning() const {
		return (0 != GetValueLong(_showDataWarningTag,(long) true));
	}
	void SetShowWarning(bool onOff){
		SetValueLong(_showDataWarningTag, "toggle warn data missing", (long)onOff);
	}

	bool GetUseLessAccurate() const {
		return (0 != GetValueLong(_useLessAccurateTag,(long) false));
	}
	void SetUseLessAccurate(bool onOff){
		SetValueLong(_useLessAccurateTag, "toggle use lower accuracy", (long)onOff);
	}

	bool GetTrackMouse() const {
		return (0 != GetValueLong(_trackMouseTag,(long) true));
	}

	void SetTrackMouse(bool onOff){
		SetValueLong(_trackMouseTag, "toggle track mouse", (long)onOff);
	}

	bool GetShowCitation() const {
		return (0 != GetValueLong(_showCitationTag,(long) true));
	}

	void SetShowCitation(bool onOff){
		SetValueLong(_showCitationTag, "toggle show citation info", (long)onOff);
	}

	void SetMessageSilence(bool val){
		SetValueLong(_messagesSilencedTag, "(Un)Silence messages",(long)val);
	}

	bool GetMessageSilence() const {
		return (0!= GetValueLong(_messagesSilencedTag,(long) false));
	}
	string GetAutosaveName(){
		return GetValueString(_autosaveFileNameTag, string("/tmp/vapor_autosave.vs3"));
	}

	void SetAutosaveName(string name){
		SetValueString(_autosaveFileNameTag,"set autosave file", name);
	}

	string GetLogFileName() const {
		return GetValueString(_logFileNameTag, string ("/tmp/vaporLog.txt"));
	}

	void SetLogFileName(string name){
		SetValueString(_logFileNameTag,"set log file", name);
	}

	bool GetLogfileEnabled() const {
		return (0!= GetValueLong(_logfileEnabledTag,(long) false));
	}

	void SetLogfileEnabled(bool val){
		SetValueLong(_logfileEnabledTag, "toggle logfile enabled",(long)val);
	}

	//! Specify current jpeg quality level
	//! \param[in] val quality level (1-99)
	void SetCurrentJpegQuality(int val);

	long GetCurrentJpegQuality() const ;

	//! Specify current autosave interval
	//! \param[in] val autosave interval >= 1
	void SetCurrentAutosaveInterval(int val);

	//! Obtain Current autosave interval
	//! \return interval between autosaves
	long GetCurrentAutosaveInterval() const;
	
	bool GetCurrentAutosaveEnabled() const {
		return (0 != GetValueLong(_currentAutosaveEnabledTag,(long) true));
	}

	void SetCurrentAutosaveEnabled(bool onOff){
		SetValueLong(
			_currentAutosaveEnabledTag,
			"Toggle current autosave enabled", (long)onOff
		);
	}

	bool GetCurrentShowWarning() const {
		return (0 != GetValueLong(_currentShowDataWarningTag, (long) true));
	}

	void SetCurrentShowWarning(bool onOff){
		SetValueLong(
			_currentShowDataWarningTag,
			"toggle current show warning",(long)onOff
		);
	}

	bool GetCurrentUseLessAccurate() const {
		return (0!= GetValueLong(_currentUseLessAccurateTag,(long) false));
	}

	void SetCurrentUseLessAccurate(bool onOff){
		SetValueLong(
			_currentUseLessAccurateTag,
			"toggle current use less accuracy",(long)onOff
		);
	}

	bool GetCurrentTrackMouse() const {
		return(0 != GetValueLong(_currentTrackMouseTag,(long) true));
	}

	void SetCurrentTrackMouse(bool onOff){
		SetValueLong(_currentTrackMouseTag,"toggle current track mouse",(long)onOff);
	}

	bool GetCurrentShowCitation() const {
		return (0 != GetValueLong(_currentShowCitationTag, (long) true));
	}

	void SetCurrentShowCitation(bool onOff){
		SetValueLong(_currentShowCitationTag,"toggle current show citation",(long)onOff);
	}

	string GetCurrentAutosaveName() const {
		return GetValueString(
			_currentAutosaveFileNameTag, string("/tmp/vapor_autosave.vs3")
		);
	}

	void SetCurrentAutosaveName(string name){
		SetValueString(_currentAutosaveFileNameTag,"set current autosave file", name);
	}

	string GetCurrentLogFileName() const {
		return GetValueString(_currentLogFileNameTag, string("/tmp/vaporLog.txt"));
	}
	void SetCurrentLogFileName(string name){
		SetValueString(_currentLogFileNameTag,"set current log file", name);
	}
	void SetCurrentMessageSilence(bool val){
		SetValueLong(_currentMessagesSilencedTag, "(Un)Silence current messages",(long)val);
	}

	bool GetCurrentMessageSilence() const {
		return (0!= GetValueLong(_currentMessagesSilencedTag,(long) false));
	}

	bool GetCurrentLogfileEnabled() const {
		return (0!= GetValueLong(_currentLogfileEnabledTag,(long) false));
	}

	void SetCurrentLogfileEnabled(bool val){
		SetValueLong(_currentLogfileEnabledTag, "toggle current logfile enabled",(long)val);
	}
	
 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return(m_classType);
 }
	
private:
	static const string m_classType;
	static const string _shortName;
	static const string _jpegQualityTag;
	static const string _autosaveEnabledTag;
	static const string _autosaveIntervalTag;
	static const string _showDataWarningTag;
	static const string _useLessAccurateTag;
	static const string _trackMouseTag;
	static const string _showCitationTag;
	static const string _logFileNameTag;
	static const string _logfileEnabledTag;
	static const string _autosaveFileNameTag;
	static const string _messagesSilencedTag;
	static const string _currentLogFileNameTag;
	static const string _currentLogfileEnabledTag;
	static const string _currentAutosaveFileNameTag;
	static const string _currentJpegQualityTag;
	static const string _currentAutosaveEnabledTag;
	static const string _currentAutosaveIntervalTag;
	static const string _currentShowDataWarningTag;
	static const string _currentUseLessAccurateTag;
	static const string _currentTrackMouseTag;
	static const string _currentShowCitationTag;
	static const string _currentMessagesSilencedTag;
	
	void _init();
	
};

#endif //APPSETTINGSPARAMS_H 
