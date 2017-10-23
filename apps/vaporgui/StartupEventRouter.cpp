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
#pragma warning(disable : 4100 4996)
#endif
#include <qdesktopwidget.h>
#include <qrect.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include "GL/glew.h"
#include "qcolordialog.h"

#include "images/fileopen.xpm"
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
#include "StartupParams.h"
#include "ErrorReporter.h"

using namespace VAPoR;

StartupEventRouter::StartupEventRouter(
    QWidget *parent, ControlExec *ce) : QWidget(parent),
                                        Ui_startupTab(),
                                        EventRouter(ce, StartupParams::GetClassType()) {

    setupUi(this);
    QPixmap *fileopenIcon = new QPixmap(fileopen);

    sessionPathButton->setIcon(QIcon(*fileopenIcon));
    metadataPathButton->setIcon(QIcon(*fileopenIcon));
    imagePathButton->setIcon(QIcon(*fileopenIcon));
    tfPathButton->setIcon(QIcon(*fileopenIcon));
    flowPathButton->setIcon(QIcon(*fileopenIcon));
    pythonPathButton->setIcon(QIcon(*fileopenIcon));
}

StartupEventRouter::~StartupEventRouter() {
}

/**********************************************************
 * Whenever a new Startuptab is created it must be hooked up here
 ************************************************************/
void StartupEventRouter::hookUpTab() {

    // Save and restore
    //
    connect(
        applyButton, SIGNAL(clicked()),
        this, SLOT(saveStartup()));
    connect(
        defaultButton, SIGNAL(clicked()),
        this, SLOT(restoreDefaults()));

    // General Startup Settings
    //
    connect(
        cacheSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(setStartupChanged()));
    connect(
        textureSizeEdit, SIGNAL(returnPressed()),
        this, SLOT(setStartupChanged()));
    connect(
        winWidthEdit, SIGNAL(returnPressed()),
        this, SLOT(setStartupChanged()));
    connect(
        winHeightEdit, SIGNAL(returnPressed()),
        this, SLOT(setStartupChanged()));
    connect(
        textureSizeCheckbox, SIGNAL(toggled(bool)),
        this, SLOT(changeTextureSize(bool)));
    connect(
        lockSizeCheckbox, SIGNAL(toggled(bool)),
        this, SLOT(winLockChanged(bool)));
    connect(
        autoStretchCheckbox, SIGNAL(toggled(bool)),
        this, SLOT(setAutoStretch(bool)));

    // Default directories
    //
    connect(
        sessionPathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));
    connect(
        metadataPathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));
    connect(
        imagePathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));
    connect(
        tfPathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));
    connect(
        flowPathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));
    connect(
        pythonPathEdit, SIGNAL(returnPressed()),
        this, SLOT(setDirChanged()));

    connect(
        sessionPathButton, SIGNAL(clicked()),
        this, SLOT(chooseSessionPath()));
    connect(
        metadataPathButton, SIGNAL(clicked()),
        this, SLOT(chooseMetadataPath()));
    connect(
        imagePathButton, SIGNAL(clicked()),
        this, SLOT(chooseImagePath()));
    connect(
        tfPathButton, SIGNAL(clicked()),
        this, SLOT(chooseTFPath()));
    connect(
        flowPathButton, SIGNAL(clicked()),
        this, SLOT(chooseFlowPath()));
    connect(
        pythonPathButton, SIGNAL(clicked()),
        this, SLOT(choosePythonPath()));
    connect(
        sessionLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestSession()));
    connect(
        metadataLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestMetadata()));
    connect(
        imageLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestImage()));
    connect(
        tfLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestTF()));
    connect(
        flowLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestFlow()));
    connect(
        pythonLatestButton, SIGNAL(clicked()),
        this, SLOT(copyLatestPython()));
}

void StartupEventRouter::GetWebHelp(
    vector<pair<string, string>> &help) const {
    help.clear();

    help.push_back(make_pair(
        "Overview of the Startup tab",
        "http://www.vapor.ucar.edu/docs/vapor-gui-help/startup-tab#StartupOverview"));
}

/*********************************************************************************
 * Slots associated with StartupTab:
 *********************************************************************************/

void StartupEventRouter::saveStartup() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    int rc = sParams->SaveStartup();
    if (rc < 0) {
        MSG_ERR("Failed to save startup file");
    }
}

void StartupEventRouter::setStartupChanged() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    paramsMgr->BeginSaveStateGroup("Startup settings changed");

    sParams->SetCacheMB(cacheSizeEdit->text().toInt());
    sParams->SetTextureSize(textureSizeEdit->text().toInt());
    sParams->SetWinSize(
        winWidthEdit->text().toInt(), winWidthEdit->text().toInt());

    paramsMgr->EndSaveStateGroup();
}

void StartupEventRouter::updateStartupChanged() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    cacheSizeEdit->setText(QString::number(sParams->GetCacheMB()));
    textureSizeEdit->setText(QString::number(sParams->GetTextureSize()));
    textureSizeCheckbox->setChecked(sParams->GetTexSizeEnable());

    size_t w, h;
    sParams->GetWinSize(w, h);
    winWidthEdit->setText(QString::number(w));
    winHeightEdit->setText(QString::number(h));
    lockSizeCheckbox->setChecked(sParams->GetWinSizeLock());

    autoStretchCheckbox->setChecked(sParams->GetAutoStretch());
}

void StartupEventRouter::setDirChanged() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    paramsMgr->BeginSaveStateGroup("Startup directory");

    sParams->SetSessionDir(sessionPathEdit->text().toStdString());
    sParams->SetMetadataDir(metadataPathEdit->text().toStdString());
    sParams->SetImageDir(imagePathEdit->text().toStdString());
    sParams->SetFlowDir(flowPathEdit->text().toStdString());
    sParams->SetPythonDir(pythonPathEdit->text().toStdString());
    sParams->SetTFDir(tfPathEdit->text().toStdString());

    paramsMgr->EndSaveStateGroup();
}

void StartupEventRouter::updateDirChanged() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sessionPathEdit->setText(QString::fromStdString(sParams->GetSessionDir()));
    metadataPathEdit->setText(QString::fromStdString(sParams->GetMetadataDir()));
    imagePathEdit->setText(QString::fromStdString(sParams->GetImageDir()));
    tfPathEdit->setText(QString::fromStdString(sParams->GetTFDir()));
    flowPathEdit->setText(QString::fromStdString(sParams->GetFlowDir()));
    pythonPathEdit->setText(QString::fromStdString(sParams->GetPythonDir()));
}

//Insert values from params into tab panel
//
void StartupEventRouter::_updateTab() {
    updateStartupChanged();
    updateDirChanged();
}

string StartupEventRouter::choosePathHelper(string current, string help) {

    //Launch a file-chooser dialog, just choosing the directory
    QString dir;
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    if (current == ".") {
        dir = QDir::currentPath();
    } else if (current == "~") {
        dir = QDir::homePath();
    } else {
        dir = current.c_str();
    }

    QString s = QFileDialog::getExistingDirectory(this, help.c_str(), dir);

    return (s.toStdString());
}

void StartupEventRouter::chooseSessionPath() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetSessionDir(), "Choose the session file directory");

    if (!dir.empty()) {
        sParams->SetSessionDir(dir);
    }
}

void StartupEventRouter::chooseMetadataPath() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetMetadataDir(), "Choose the data file directory");

    if (!dir.empty()) {
        sParams->SetMetadataDir(dir);
    }
}

void StartupEventRouter::chooseImagePath() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetImageDir(), "Choose the image file directory");

    if (!dir.empty()) {
        sParams->SetImageDir(dir);
    }
}

void StartupEventRouter::chooseTFPath() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetTFDir(), "Choose the Transfer Function file directory");

    if (!dir.empty()) {
        sParams->SetTFDir(dir);
    }
}

void StartupEventRouter::chooseFlowPath() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetFlowDir(), "Choose the Flow file directory");

    if (!dir.empty()) {
        sParams->SetFlowDir(dir);
    }
}

void StartupEventRouter::choosePythonPath() {

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(
        sParams->GetPythonDir(), "Choose the Python script file directory");

    if (!dir.empty()) {
        sParams->SetPythonDir(dir);
    }
}

void StartupEventRouter::copyLatestSession() {
    GUIStateParams *p = GetStateParams();
    string latestPath = p->GetCurrentSessionPath();

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetSessionDir(path);
    sessionPathEdit->setText(path.c_str());
}

void StartupEventRouter::copyLatestMetadata() {
    GUIStateParams *p = GetStateParams();
    vector<string> paths, names;
    p->GetOpenDataSets(paths, names);

    string latestPath = paths.size() ? paths[paths.size() - 1] : ".";

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetMetadataDir(path);
    metadataPathEdit->setText(path.c_str());
}

void StartupEventRouter::copyLatestTF() {
    GUIStateParams *p = GetStateParams();
    string latestPath = p->GetCurrentTFPath();

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetTFDir(path);
    tfPathEdit->setText(path.c_str());
}

void StartupEventRouter::copyLatestImage() {
    GUIStateParams *p = GetStateParams();
    string latestPath = p->GetCurrentImagePath();

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetImageDir(path);
    imagePathEdit->setText(path.c_str());
}

void StartupEventRouter::copyLatestFlow() {
    GUIStateParams *p = GetStateParams();
    string latestPath = p->GetCurrentFlowPath();

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetFlowDir(path);
    flowPathEdit->setText(path.c_str());
}

void StartupEventRouter::copyLatestPython() {
    GUIStateParams *p = GetStateParams();
    string latestPath = p->GetCurrentPythonPath();

    QFileInfo qFileInfo(QString(latestPath.c_str()));

    string path = qFileInfo.path().toStdString();

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->SetPythonDir(path);
    pythonPathEdit->setText(path.c_str());
}

void StartupEventRouter::changeTextureSize(bool val) {
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    sParams->SetTexSizeEnable(val);
}

void StartupEventRouter::winLockChanged(bool val) {
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    sParams->SetWinSizeLock(val);
}

void StartupEventRouter::setAutoStretch(bool val) {
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    sParams->SetAutoStretch(val);
}

void StartupEventRouter::restoreDefaults() {
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sParams->Reinit();
}
