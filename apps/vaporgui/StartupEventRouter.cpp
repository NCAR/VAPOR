//************************************************************************
//															*
//		     Copyright (C)  2015										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		Startupeventrouter.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:	Implements the StartupEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the Startup Widget
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
#include "qcolordialog.h"

#include "../images/fileopen.xpm"
#include <qlabel.h>
#include <QFileDialog>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "TabManager.h"

#include "StartupEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"
#include "vapor/DataStatus.h"
#include "StartupParams.h"


using namespace VAPoR;

StartupEventRouter::StartupEventRouter(
	QWidget *parent, ControlExec *ce
) : QWidget(parent), 
	Ui_startupTab(), 
	EventRouter(ce, StartupParams::GetClassType())
{

	setupUi(this);
	QPixmap* fileopenIcon = new QPixmap(fileopen);

	sessionPathButton->setIcon(QIcon(*fileopenIcon));
	metadataPathButton->setIcon(QIcon(*fileopenIcon));
	imagePathButton->setIcon(QIcon(*fileopenIcon));
	tfPathButton->setIcon(QIcon(*fileopenIcon));
	flowPathButton->setIcon(QIcon(*fileopenIcon));
	pythonPathButton->setIcon(QIcon(*fileopenIcon));
	setSettingsChanged(false);
	_savedStartupParams = 0;
	_textChangedFlag = false;
}


StartupEventRouter::~StartupEventRouter(){
	if (_savedStartupParams) delete _savedStartupParams;
}
/**********************************************************
 * Whenever a new Startuptab is created it must be hooked up here
 ************************************************************/
void
StartupEventRouter::hookUpTab()
{
	connect (cacheSizeEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (cacheSizeEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (textureSizeEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (textureSizeEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (winWidthEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (winWidthEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (winHeightEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (winHeightEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (sessionPathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (sessionPathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (metadataPathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (metadataPathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (imagePathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (imagePathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (tfPathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (tfPathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (flowPathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (flowPathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (pythonPathEdit, SIGNAL( textChanged(const QString&) ), this, SLOT(setStartupTextChanged(const QString&)));
	connect (pythonPathEdit, SIGNAL( returnPressed()), this, SLOT(startupReturnPressed()));
	connect (applyButton, SIGNAL(clicked()), this, SLOT(apply()));
	connect (defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
	connect (sessionPathButton, SIGNAL(clicked()), this, SLOT(chooseSessionPath()));
	connect (metadataPathButton, SIGNAL(clicked()), this, SLOT(chooseMetadataPath()));
	connect (imagePathButton, SIGNAL(clicked()), this, SLOT(chooseImagePath()));
	connect (tfPathButton, SIGNAL(clicked()), this, SLOT(chooseTFPath()));
	connect (flowPathButton, SIGNAL(clicked()), this, SLOT(chooseFlowPath()));
	connect (pythonPathButton, SIGNAL(clicked()), this, SLOT(choosePythonPath()));
	connect (sessionLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestSession()));
	connect (metadataLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestMetadata()));
	connect (imageLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestImage()));
	connect (tfLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestTF()));
	connect (flowLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestFlow()));
	connect (pythonLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestPython()));
	connect (textureSizeCheckbox, SIGNAL(toggled(bool)),this, SLOT(changeTextureSize(bool)));
	connect (lockSizeCheckbox, SIGNAL(toggled(bool)),this, SLOT(winLockChanged(bool)));
	connect (autoStretchCheckbox, SIGNAL(toggled(bool)), this, SLOT(setAutoStretch(bool)));
	
	TabManager* tabMgr = TabManager::getInstance();
	connect(tabMgr, SIGNAL(tabLeft(int,int)), this, SLOT(tabChanged(int,int)));

}

void StartupEventRouter::GetWebHelp(
    vector <pair <string, string> > &help
) const {
    help.clear();

    help.push_back(make_pair(
		"Overview of the Startup tab",
		"http://www.vapor.ucar.edu/docs/vapor-gui-help/startup-tab#StartupOverview"
    ));
}

/*********************************************************************************
 * Slots associated with StartupTab:
 *********************************************************************************/

void StartupEventRouter::
setStartupTextChanged(const QString& ){
	SetTextChanged(true);
}
void StartupEventRouter::confirmText(){
	if (!_textChangedFlag) return;
	setSettingsChanged(true);

	
	_confirmText();
	
	SetTextChanged(false);

#ifdef	DEAD
	vParams->Validate(2);
#endif
	
}

void StartupEventRouter::_confirmText(){
	StartupParams* sParams = (StartupParams *) GetActiveParams();
	
	sParams->SetCacheMB(cacheSizeEdit->text().toInt());
	sParams->SetTextureSize(textureSizeEdit->text().toInt());
	sParams->SetWinSize(
		winWidthEdit->text().toInt(), winWidthEdit->text().toInt()
	);
	sParams->SetSessionDir(sessionPathEdit->text().toStdString());
	sParams->SetMetadataDir(metadataPathEdit->text().toStdString());
	sParams->SetImageDir(imagePathEdit->text().toStdString());
	sParams->SetFlowDir(flowPathEdit->text().toStdString());
	sParams->SetPythonDir(pythonPathEdit->text().toStdString());
	sParams->SetTFDir(tfPathEdit->text().toStdString());
	
}

void StartupEventRouter::
startupReturnPressed(void){
	confirmText();
}

//Insert values from params into tab panel
//
void StartupEventRouter::_updateTab(){
#ifdef	DEAD
cout << "StartupEventRouter::_updateTab() BLOCKED" << endl;
return;
#endif

	StartupParams* sParams = (StartupParams*) GetActiveParams();
	
	cacheSizeEdit->setText(QString::number(sParams->GetCacheMB()));
	textureSizeEdit->setText(QString::number(sParams->GetTextureSize()));
	
	size_t w, h;
	sParams->GetWinSize(w,h);
	winWidthEdit->setText(QString::number(w));
	winHeightEdit->setText(QString::number(h));
	sessionPathEdit->setText(QString::fromStdString(sParams->GetSessionDir()));
	metadataPathEdit->setText(QString::fromStdString(sParams->GetMetadataDir()));
	imagePathEdit->setText(QString::fromStdString(sParams->GetImageDir()));
	tfPathEdit->setText(QString::fromStdString(sParams->GetTFDir()));
	flowPathEdit->setText(QString::fromStdString(sParams->GetFlowDir()));
	pythonPathEdit->setText(QString::fromStdString(sParams->GetPythonDir()));
	autoStretchCheckbox->setChecked(sParams->GetAutoStretch());
	lockSizeCheckbox->setChecked(sParams->GetWinSizeLock());
	textureSizeCheckbox->setChecked(sParams->GetTexSizeEnable());
	setSettingsChanged(false);
	adjustSize();
}


void StartupEventRouter::chooseSessionPath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetSessionDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetSessionDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the session file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		sessionPathEdit->setText(s);
		string sessionDir = s.toStdString();
		sParams->SetSessionDir(sessionDir);
	}
}

void StartupEventRouter::chooseMetadataPath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetMetadataDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetMetadataDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the metadata file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		metadataPathEdit->setText(s);
		string metadataDir = s.toStdString();
		sParams->SetMetadataDir(metadataDir);
		
	}
}

void StartupEventRouter::chooseImagePath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetImageDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetImageDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the image file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		imagePathEdit->setText(s);
		string imageDir = s.toStdString();
		sParams->SetImageDir(imageDir);
		
	}
}
void StartupEventRouter::chooseTFPath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetTFDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetTFDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the transfer function file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		tfPathEdit->setText(s);
		string tfDir = s.toStdString();
		sParams->SetTFDir(tfDir);
		
	}
}
void StartupEventRouter::chooseFlowPath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetFlowDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetFlowDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the flow file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		flowPathEdit->setText(s);
		string flowDir = s.toStdString();
		sParams->SetFlowDir(flowDir);
	}
}
void StartupEventRouter::choosePythonPath(){
	//Launch a file-chooser dialog, just choosing the directory
	QString dir;
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (sParams->GetPythonDir() == ".") dir = QDir::currentPath();
	else dir = sParams->GetPythonDir().c_str();
	QString s = QFileDialog::getExistingDirectory(this,
            	"Choose the python file directory",
		dir);
	if (s != "") {
		setSettingsChanged(true);
		pythonPathEdit->setText(s);
		string pythonDir = s.toStdString();
		sParams->SetPythonDir(pythonDir);
	}
}
void StartupEventRouter::copyLatestSession(){
	GUIStateParams *p = GetStateParams(); 
	string latestPath = p->GetCurrentSessionPath();

	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetSessionDir()) return;
	setSettingsChanged(true);
	sParams->SetSessionDir(latestPath);
	sessionPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestMetadata(){
	GUIStateParams *p = GetStateParams(); 
	string latestPath = p->GetCurrentDataPath();

	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetMetadataDir()) return;
	setSettingsChanged(true);
	sParams->SetMetadataDir(latestPath);
	metadataPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestTF(){
	GUIStateParams *p = GetStateParams(); 

	string latestPath = p->GetCurrentTFPath();
	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetTFDir()) return;
	setSettingsChanged(true);
	sParams->SetTFDir(latestPath);
	tfPathEdit->setText(latestPath.c_str());
	
}
void StartupEventRouter::copyLatestImage(){
	GUIStateParams *p = GetStateParams(); 
	string latestPath = p->GetCurrentImagePath();

	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetImageDir()) return;
	setSettingsChanged(true);
	sParams->SetImageDir(latestPath);
	imagePathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestFlow(){
	GUIStateParams *p = GetStateParams(); 
	string latestPath = p->GetCurrentFlowPath();

	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetFlowDir()) return;
	setSettingsChanged(true);
	sParams->SetFlowDir(latestPath);
	flowPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestPython(){
	GUIStateParams *p = GetStateParams(); 
	string latestPath = p->GetCurrentPythonPath();

	size_t pos = latestPath.find_last_of("\\/");
	if(pos != string::npos) latestPath = latestPath.substr(0, pos);
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	if (latestPath == sParams->GetPythonDir()) return;
	setSettingsChanged(true);
	sParams->SetPythonDir(latestPath);
	pythonPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::changeTextureSize(bool val){	
	confirmText();
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	setSettingsChanged(true);
	sParams->SetTexSizeEnable(val);
}
void StartupEventRouter::winLockChanged(bool val){
	confirmText();
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	setSettingsChanged(true);
	sParams->SetWinSizeLock(val);
} 
void StartupEventRouter::setAutoStretch(bool val){
	confirmText();
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	setSettingsChanged(true);
	sParams->SetAutoStretch(val);
} 
void StartupEventRouter::apply(){
	//Just save to current file
	confirmText();
#ifdef	DEAD
	StartupParams* sParams = (StartupParams*)GetActiveParams();

	int rc = m_controlExec->SavePreferences(sParams->GetCurrentPrefsPath());
	if (!rc) setSettingsChanged(false);  //settings were saved to file
#endif
	
	return;
}

void StartupEventRouter::restoreDefaults(){
#ifdef	DEAD
	StartupParams* sParams = (StartupParams*)GetActiveParams();
	setSettingsChanged(true);

	Command* cmd = Command::CaptureStart(sParams, "Restore defaults");
	sParams->restart();
	Command::CaptureEnd(cmd, sParams);
#endif
	updateTab();
}
void StartupEventRouter::tabChanged(int topIndex, int subIndex){
#ifdef	DEAD
	if (topIndex != _topTabIndex) return;
	if (subIndex != _subTabIndex) return;
#endif
	
	if (_settingsChanged || _textChangedFlag) {
		//Tell user to save preferences
		QMessageBox msgBox;
		msgBox.setText("Startup settings have changed.");
		msgBox.setInformativeText("Do you want to save your settings?");
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
		msgBox.setDefaultButton(QMessageBox::Save);
		int ret = msgBox.exec();
		if (ret == QMessageBox::Save){
			apply();
		} else {
#ifdef	DEAD
			//revert to saved settings
			assert(_savedStartupParams);
			StartupParams* sParams = (StartupParams*)_paramsMgr->GetDefaultParams(StartupParams::_startupParamsTag);
			assert(sParams);
			ParamNode* copyNode = _savedStartupParams->GetRootNode()->deepCopy();
			delete sParams->GetRootNode();
			sParams->SetRootParamNode(copyNode);
			delete _savedStartupParams;
			_savedStartupParams = 0;
#endif
		
		}
		_settingsChanged = false;
	}
}
void StartupEventRouter::setSettingsChanged(bool didchange){
#ifdef	DEAD
	// Copy the params before the first change
	if (didchange && !_settingsChanged){
		StartupParams* sParams = (StartupParams*)GetActiveParams();
		_savedStartupParams = (StartupParams*)sParams->deepCopy(sParams->GetRootNode()->deepCopy());
	}
	_settingsChanged = didchange;
#endif
}
