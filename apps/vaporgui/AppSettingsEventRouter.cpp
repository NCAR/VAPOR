//************************************************************************
//															*
//		     Copyright (C)  2015										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//***********************************************************************/
//
//	File:		AppSettingsEventRouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the AppSettingsEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the appSettings Widget
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 4996 )
#endif
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include "AppSettingsParams.h"
#include "StartupParams.h"
#include "qcolordialog.h"
#include "../images/fileopen.xpm"
#include "MessageReporter.h"

#include <qlabel.h>
#include <qfiledialog.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "AppSettingsEventRouter.h"

#include "vapor/ControlExecutive.h"
#include "EventRouter.h"

using namespace VAPoR;


AppSettingsEventRouter::AppSettingsEventRouter(
    QWidget *parent, ControlExec *ce
) : QWidget(parent),
    Ui_appSettingsTab(),
    EventRouter(ce, AppSettingsParams::GetClassType())
{
	setupUi(this);


	QPixmap* fileopenIcon = new QPixmap(fileopen);
	autosavePathButton->setIcon(QIcon(*fileopenIcon));
	logFilePathButton->setIcon(QIcon(*fileopenIcon));
	_settingsChanged = false;
}


AppSettingsEventRouter::~AppSettingsEventRouter(){
	
}
/**********************************************************
 * Whenever a new appSettingstab is created it must be hooked up here
 ************************************************************/
void AppSettingsEventRouter::hookUpTab()
{
	connect (jpegQualityEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setAppSettingsTextChanged(const QString&)));
	connect (jpegQualityEdit, SIGNAL( returnPressed()), this, SLOT(appSettingsReturnPressed()));
	connect (autosaveIntervalEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setAppSettingsTextChanged(const QString&)));
	connect (autosaveIntervalEdit, SIGNAL( returnPressed()), this, SLOT(appSettingsReturnPressed()));
	connect (saveButton, SIGNAL(clicked()), this, SLOT(save()));
	connect (logFilePathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setAppSettingsTextChanged(const QString&)));
	connect (logFilePathEdit, SIGNAL( returnPressed()), this, SLOT(appSettingsReturnPressed()));
	connect (autosaveFilenameEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setAppSettingsTextChanged(const QString&)));
	connect (autosaveFilenameEdit, SIGNAL( returnPressed()), this, SLOT(appSettingsReturnPressed()));
	connect (logFilePathButton, SIGNAL(clicked()), this, SLOT(chooseLogFilePath()));
	connect (autosavePathButton, SIGNAL(clicked()), this, SLOT(chooseAutoSaveFilename()));
	connect (defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
	connect (unsilenceButton, SIGNAL(clicked()), this, SLOT(unsilenceMessages()));
	connect (autosaveCheckbox, SIGNAL(toggled(bool)),this, SLOT(setAutoSave(bool)));
	connect (logfileCheckbox, SIGNAL(toggled(bool)), this, SLOT(enableLogfile(bool)));
	connect (warnMissingDataCheckbox, SIGNAL(toggled(bool)),this, SLOT(warningChanged(bool)));
	connect (lowerAccuracyCheckbox, SIGNAL(toggled(bool)),this, SLOT(lowerAccuracyChanged(bool)));
	connect (trackMouseCheckbox, SIGNAL(toggled(bool)),this, SLOT(trackMouseChanged(bool)));
	connect (noShowCitationCheckbox, SIGNAL(toggled(bool)),this, SLOT(setNoCitation(bool)));
	connect (silenceCheckbox, SIGNAL(toggled(bool)),this, SLOT(silenceAllMessages(bool)));

}


void AppSettingsEventRouter::GetWebHelp(
    vector <pair <string, string> > &help
) const {
    help.clear();

	help.push_back(make_pair(
		"Overview of the AppSettings tab",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/appSettings-tab#AppSettingsOverview"
	));

}

/*********************************************************************************
 * Slots associated with AppSettingsTab:
 *********************************************************************************/

void AppSettingsEventRouter::
setAppSettingsTextChanged(const QString& ){
	SetTextChanged(true);
}
void AppSettingsEventRouter::confirmText(){
	if (!_textChangedFlag) return;
	AppSettingsParams* vParams = (AppSettingsParams*)GetActiveParams();

	
	_confirmText();
	
	SetTextChanged(false);

#ifdef	DEAD
	vParams->Validate(2);
	
#endif
	
}
void AppSettingsEventRouter::_confirmText(){
	AppSettingsParams* aParams = (AppSettingsParams*) GetActiveParams();

#ifdef	DEAD
	if (logFilePathEdit->text().toStdString() != aParams->GetCurrentLogFileName()){
		aParams->SetCurrentLogFileName(logFilePathEdit->text().toStdString());
		int rc = m_controlExec->OpenLogfile(aParams->GetCurrentLogFileName());
		if (!rc && aParams->GetCurrentLogfileEnabled()) MessageReporter::setDiagMsgCB(true);
		else MessageReporter::setDiagMsgCB(false);
	}
#endif
	aParams->SetCurrentAutosaveName(autosaveFilenameEdit->text().toStdString());
	aParams->SetCurrentAutosaveInterval(autosaveIntervalEdit->text().toInt());
	aParams->SetCurrentJpegQuality(jpegQualityEdit->text().toInt());
	_settingsChanged = true;
}
void AppSettingsEventRouter::
appSettingsReturnPressed(void){
	confirmText();
}

//Insert values from params into tab panel
//
void AppSettingsEventRouter::_updateTab(){

    // Disable signals. 
    //
    bool oldState = this->blockSignals(true);
	
	AppSettingsParams* aParams = (AppSettingsParams*) GetActiveParams();

	autosaveIntervalEdit->setText(QString::number(aParams->GetCurrentAutosaveInterval()));
	logFilePathEdit->setText(QString::fromStdString(aParams->GetLogFileName()));
	autosaveFilenameEdit->setText(QString::fromStdString(aParams->GetAutosaveName()));
	jpegQualityEdit->setText(QString::number(aParams->GetCurrentJpegQuality()));
	autosaveCheckbox->setChecked(aParams->GetCurrentAutosaveEnabled());
	warnMissingDataCheckbox->setChecked(aParams->GetCurrentShowWarning());
	lowerAccuracyCheckbox->setChecked(aParams->GetCurrentUseLessAccurate());
	trackMouseCheckbox->setChecked(aParams->GetCurrentTrackMouse());
	noShowCitationCheckbox->setChecked(!aParams->GetCurrentShowCitation());
	silenceCheckbox->setChecked(aParams->GetCurrentMessageSilence());
	logfileCheckbox->setChecked(aParams->GetCurrentLogfileEnabled());
	adjustSize();

    // Enable signals. 
    //
    this->blockSignals(oldState);
}

void AppSettingsEventRouter::setNoCitation(bool val){
	confirmText();
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentShowCitation(!val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::warningChanged(bool val){
	confirmText();
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentShowWarning(val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::trackMouseChanged(bool val){
	confirmText();
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentTrackMouse(val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::lowerAccuracyChanged(bool val){
	confirmText();
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentUseLessAccurate(val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::setAutoSave(bool val){
	confirmText();
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentAutosaveEnabled(val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::save(){
	
#ifdef	DEAD
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	//Copy the current App Settings to the Params for saving
	aParams->saveCurrentSettings();
	//save to default prefs file
	StartupParams* sParams = (StartupParams*)_paramsMgr->GetDefaultParams(StartupParams::_startupParamsTag);
	assert(sParams);
	int rc = _controlExec->SavePreferences(sParams->GetCurrentPrefsPath());
	if (!rc) _settingsChanged = false;
#endif
}

void AppSettingsEventRouter::restoreDefaults(){
	
#ifdef	DEAD
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	Command* cmd = Command::CaptureStart(aParams, "restore defaults");
	aParams->restart();
	Command::CaptureEnd(cmd, aParams);
	updateTab();
	_settingsChanged = true;
#endif
}
void AppSettingsEventRouter::chooseAutoSaveFilename(){
	//Launch a file-chooser dialog
	AppSettingsParams* sParams = (AppSettingsParams*)GetActiveParams();
    QString s = QFileDialog::getSaveFileName(this,
                "Choose a filename for autosaved session files", 
		sParams->GetAutosaveName().c_str(),
		"Vapor Session File (*.vs3)");
	if (s != ""){
		autosaveFilenameEdit->setText(s);
		string autosaveName = s.toStdString();
		sParams->SetAutosaveName(autosaveName);
		_settingsChanged = true;
	}
}
void AppSettingsEventRouter::chooseLogFilePath(){
#ifdef	DEAD
	//Launch a file-chooser dialog
	AppSettingsParams* sParams = (AppSettingsParams*)GetActiveParams();
    QString s = QFileDialog::getSaveFileName(this,
        "Choose a filename for log messages", 
		sParams->GetLogFileName().c_str(),
		"Text (*.txt)");
	if (s != ""){
		logFilePathEdit->setText(s);
		string logFileName = s.toStdString();
		sParams->SetCurrentLogFileName(logFileName);
		_settingsChanged = true;
		int rc = m_controlExec->OpenLogfile(sParams->GetCurrentLogFileName());
		if (!rc && sParams->GetCurrentLogfileEnabled()) MessageReporter::setDiagMsgCB(true);
		else MessageReporter::setDiagMsgCB(false);
	}
#endif
}
void AppSettingsEventRouter::unsilenceMessages(){
	AppSettingsParams* sParams = (AppSettingsParams*)GetActiveParams();
	sParams->SetCurrentMessageSilence(false);
	if (silenceCheckbox->isChecked())
		silenceCheckbox->setChecked(false);
	MessageReporter::unSilenceAll();
	_settingsChanged = true;
}
void AppSettingsEventRouter::silenceAllMessages(bool val){
	AppSettingsParams* sParams = (AppSettingsParams*)GetActiveParams();
	//Set current silence, so as to have an immediate effect
	sParams->SetCurrentMessageSilence(val);
	_settingsChanged = true;
}
void AppSettingsEventRouter::enableLogfile(bool val){
	AppSettingsParams* aParams = (AppSettingsParams*)GetActiveParams();
	aParams->SetCurrentLogfileEnabled(val);
	if (val) {
#ifdef	DEAD
		int rc = _controlExec->OpenLogfile(aParams->GetCurrentLogFileName());
		if (!rc) MessageReporter::setDiagMsgCB(true);
#endif
	}
	else  {
#ifdef	DEAD
		_controlExec->OpenLogfile("");
#endif
		MessageReporter::setDiagMsgCB(false);
	}
	_settingsChanged = true;
}
