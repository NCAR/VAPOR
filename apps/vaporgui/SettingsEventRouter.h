//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		SettingsEventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November, 2015
//
//	Description:	Defines the SettingsEventRouter class.
//		This class handles events for the Settings params
//
#ifndef SETTINGSEVENTROUTER_H
#define SETTINGSEVENTROUTER_H


#include <qobject.h>
#include "EventRouter.h"
#include <vapor/MyBase.h>
#include "ui_SettingsGUI.h"


QT_USE_NAMESPACE

namespace VAPoR {
	class ControlExec;
}

class SettingsEventRouter : public QWidget, public Ui_SettingsGUI, public EventRouter {

	Q_OBJECT

public: 
	SettingsEventRouter(
		QWidget *parent, VAPoR::ControlExec *ce
	);

	virtual ~SettingsEventRouter();

	//! For the SettingsEventRouter, we must override confirmText method on base class,
	//! so that text changes issue Command::CaptureStart and Command::CaptureEnd,
	//! Connect signals and slots from tab
	virtual void hookUpTab();

	virtual void GetWebHelp(
		std::vector <std::pair <string, string>> &help
	) const;
	
	//! Ignore wheel event in tab (to avoid confusion)
	virtual void wheelEvent(QWheelEvent*) {}

 // Get static string identifier for this router class
 //
 static string GetClassType() {
	return("SettingsEventRouter");
 }
 string GetType() const {return GetClassType(); }


protected:
 virtual void _updateTab();
 virtual void _confirmText() {}
	
private slots:

	void _enableAutoStretch(bool enabled);
	void _enableAutoSave(bool enabled);
	void _changesPerSaveChanged();
	void _autoSaveFileChanged();
	void _chooseAutoSaveFile();

	void _numThreadsChanged();
	void _cacheSizeChanged();
	void _enableWinSize(bool enabled);
	void _windowSizeChanged();
	
	void _saveSettings();
	void _setSessionPath();
	void _setMetadataPath();
	void _setTFPath();
	void _setFlowPath();
	void _setPythonPath();

	void _chooseSessionPath();
	void _chooseMetadataPath();
	void _chooseTFPath();
	void _chooseFlowPath();
	void _choosePythonPath();
	void _winLockChanged(bool val); 
	void _restoreDefaults();

private:
	void _setFilePath(void (SettingsParams::*setFunc)(string),
		string (SettingsParams::*getFunc)() const,
		SettingsParams &sParams,
		QLineEdit* lineEdit);
	void _blockSignals(bool block);

	void _updateGeneralSettings();
	void _updateStartupSettings();
	void _updateDirectoryPaths();

    // Checks if a file exists on the disk, and warns the user if it does.
    // Returns true if 1) there is no such file on disk; and
    //                 2) the user says YES to overwrite.
    // Returns false if the users says NO to overwrite.
    //
    bool _confirmFileExist( QString& filename );

	SettingsParams* _defaultParams;

	string _choosePathHelper(string current, string help);
};

#endif //SETTINGSEVENTROUTER_H 
