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
    // Annoying unreferenced formal parameter warning
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

namespace {
string StartupFile = ".vapor3_startup";
}

using namespace VAPoR;

StartupEventRouter::StartupEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_startupTab(), EventRouter(ce, StartupParams::GetClassType())
{
    setupUi(this);
    QPixmap *fileopenIcon = new QPixmap(fileopen);

    sessionPathButton->setIcon(QIcon(*fileopenIcon));
    metadataPathButton->setIcon(QIcon(*fileopenIcon));
    imagePathButton->setIcon(QIcon(*fileopenIcon));
    tfPathButton->setIcon(QIcon(*fileopenIcon));
    flowPathButton->setIcon(QIcon(*fileopenIcon));
    pythonPathButton->setIcon(QIcon(*fileopenIcon));
    setSettingsChanged(false);
    _settingsChanged = false;    // this is a quick hack
    _savedStartupParams = 0;
    _textChangedFlag = false;

    _startupPath = QDir::homePath().toStdString();
    _startupPath += QDir::separator().toAscii();
    _startupPath += StartupFile;
    loadDefaultsFromStartupFile();
}

StartupEventRouter::~StartupEventRouter()
{
    if (_savedStartupParams) delete _savedStartupParams;
}

/**********************************************************
 * Whenever a new Startuptab is created it must be hooked up here
 ************************************************************/
void StartupEventRouter::hookUpTab()
{
    connect(applyButton, SIGNAL(clicked()), this, SLOT(saveStartup()));

    // General Startup Settings
    //
    connect(cacheSizeEdit, SIGNAL(returnPressed()), this, SLOT(setStartupChanged()));
    connect(textureSizeEdit, SIGNAL(returnPressed()), this, SLOT(setStartupChanged()));
    connect(winWidthEdit, SIGNAL(returnPressed()), this, SLOT(setStartupChanged()));
    connect(winHeightEdit, SIGNAL(returnPressed()), this, SLOT(setStartupChanged()));

    // Default directories
    //
    connect(sessionPathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));
    connect(metadataPathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));
    connect(imagePathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));
    connect(tfPathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));
    connect(flowPathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));
    connect(pythonPathEdit, SIGNAL(returnPressed()), this, SLOT(setDirChanged()));

    connect(defaultButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
    connect(sessionPathButton, SIGNAL(clicked()), this, SLOT(chooseSessionPath()));
    connect(metadataPathButton, SIGNAL(clicked()), this, SLOT(chooseMetadataPath()));
    connect(imagePathButton, SIGNAL(clicked()), this, SLOT(chooseImagePath()));
    connect(tfPathButton, SIGNAL(clicked()), this, SLOT(chooseTFPath()));
    connect(flowPathButton, SIGNAL(clicked()), this, SLOT(chooseFlowPath()));
    connect(pythonPathButton, SIGNAL(clicked()), this, SLOT(choosePythonPath()));
    connect(sessionLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestSession()));
    connect(metadataLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestMetadata()));
    connect(imageLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestImage()));
    connect(tfLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestTF()));
    connect(flowLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestFlow()));
    connect(pythonLatestButton, SIGNAL(clicked()), this, SLOT(copyLatestPython()));
    connect(textureSizeCheckbox, SIGNAL(toggled(bool)), this, SLOT(changeTextureSize(bool)));
    connect(lockSizeCheckbox, SIGNAL(toggled(bool)), this, SLOT(winLockChanged(bool)));
    connect(autoStretchCheckbox, SIGNAL(toggled(bool)), this, SLOT(setAutoStretch(bool)));

    TabManager *tabMgr = TabManager::getInstance();
    connect(tabMgr, SIGNAL(tabLeft(int, int)), this, SLOT(tabChanged(int, int)));
}

void StartupEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Overview of the Startup tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/startup-tab#StartupOverview"));
}

/*********************************************************************************
 * Slots associated with StartupTab:
 *********************************************************************************/

void StartupEventRouter::saveStartup()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    ofstream fileout;
    string   s;

    fileout.open(_startupPath.c_str());
    if (!fileout) {
        MyBase::SetErrMsg("Unable to open output startup file \"_startupPath.c_str()\" : %M");
        MSG_ERR("File open fail");
        return;
    }

    const XmlNode *node = sParams->GetNode();
    XmlNode::streamOut(fileout, *node);
    if (fileout.bad()) {
        MyBase::SetErrMsg("Unable to write output startup file \"_startupPath.c_str()\" : %M");
        MSG_ERR("File write fail");
    }

    fileout.close();
}

void StartupEventRouter::setStartupChanged()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    paramsMgr->BeginSaveStateGroup("Startup settings changed");

    sParams->SetCacheMB(cacheSizeEdit->text().toInt());
    sParams->SetTextureSize(textureSizeEdit->text().toInt());
    sParams->SetWinSize(winWidthEdit->text().toInt(), winWidthEdit->text().toInt());

    paramsMgr->EndSaveStateGroup();
}

void StartupEventRouter::updateStartupChanged()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    XmlNode *node = sParams->GetNode();

    cacheSizeEdit->setText(QString::number(sParams->GetCacheMB()));
    textureSizeEdit->setText(QString::number(sParams->GetTextureSize()));

    size_t w, h;
    sParams->GetWinSize(w, h);
    winWidthEdit->setText(QString::number(w));
    winHeightEdit->setText(QString::number(h));
}

void StartupEventRouter::setDirChanged()
{
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

void StartupEventRouter::updateDirChanged()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    sessionPathEdit->setText(QString::fromStdString(sParams->GetSessionDir()));
    metadataPathEdit->setText(QString::fromStdString(sParams->GetMetadataDir()));
    imagePathEdit->setText(QString::fromStdString(sParams->GetImageDir()));
    tfPathEdit->setText(QString::fromStdString(sParams->GetTFDir()));
    flowPathEdit->setText(QString::fromStdString(sParams->GetFlowDir()));
    pythonPathEdit->setText(QString::fromStdString(sParams->GetPythonDir()));
}

void StartupEventRouter::confirmText() {}

void StartupEventRouter::_confirmText() { StartupParams *sParams = (StartupParams *)GetActiveParams(); }

void StartupEventRouter::startupReturnPressed(void) { confirmText(); }

// Insert values from params into tab panel
//
void StartupEventRouter::_updateTab()
{
    updateStartupChanged();
    updateDirChanged();

    return;

#ifdef DEAD
    cout << "StartupEventRouter::_updateTab() BLOCKED" << endl;
    return;
#endif

    StartupParams *sParams = (StartupParams *)GetActiveParams();

    cacheSizeEdit->setText(QString::number(sParams->GetCacheMB()));
    textureSizeEdit->setText(QString::number(sParams->GetTextureSize()));

    size_t w, h;
    sParams->GetWinSize(w, h);
    winWidthEdit->setText(QString::number(w));
    winHeightEdit->setText(QString::number(h));

    autoStretchCheckbox->setChecked(sParams->GetAutoStretch());
    lockSizeCheckbox->setChecked(sParams->GetWinSizeLock());
    textureSizeCheckbox->setChecked(sParams->GetTexSizeEnable());
    setSettingsChanged(false);
    adjustSize();
}

string StartupEventRouter::choosePathHelper(string current, string help)
{
    // Launch a file-chooser dialog, just choosing the directory
    QString        dir;
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

void StartupEventRouter::chooseSessionPath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetSessionDir(), "Choose the session file directory");

    if (!dir.empty()) { sParams->SetSessionDir(dir); }
}

void StartupEventRouter::chooseMetadataPath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetMetadataDir(), "Choose the data file directory");

    if (!dir.empty()) { sParams->SetMetadataDir(dir); }
}

void StartupEventRouter::chooseImagePath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetImageDir(), "Choose the image file directory");

    if (!dir.empty()) { sParams->SetImageDir(dir); }
}

void StartupEventRouter::chooseTFPath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetTFDir(), "Choose the Transfer Function file directory");

    if (!dir.empty()) { sParams->SetTFDir(dir); }
}

void StartupEventRouter::chooseFlowPath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetFlowDir(), "Choose the Flow file directory");

    if (!dir.empty()) { sParams->SetFlowDir(dir); }
}

void StartupEventRouter::choosePythonPath()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    string dir = choosePathHelper(sParams->GetPythonDir(), "Choose the Python script file directory");

    if (!dir.empty()) { sParams->SetPythonDir(dir); }
}

void StartupEventRouter::copyLatestSession()
{
    GUIStateParams *p = GetStateParams();
    string          latestPath = p->GetCurrentSessionPath();

    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetSessionDir()) return;
    setSettingsChanged(true);
    sParams->SetSessionDir(latestPath);
    sessionPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestMetadata()
{
    GUIStateParams *p = GetStateParams();

    vector<string> paths, names;
    p->GetOpenDataSets(paths, names);
    string latestPath = paths.size() ? paths[paths.size() - 1] : ".";

    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetMetadataDir()) return;
    setSettingsChanged(true);
    sParams->SetMetadataDir(latestPath);
    metadataPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestTF()
{
    GUIStateParams *p = GetStateParams();

    string latestPath = p->GetCurrentTFPath();
    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetTFDir()) return;
    setSettingsChanged(true);
    sParams->SetTFDir(latestPath);
    tfPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestImage()
{
    GUIStateParams *p = GetStateParams();
    string          latestPath = p->GetCurrentImagePath();

    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetImageDir()) return;
    setSettingsChanged(true);
    sParams->SetImageDir(latestPath);
    imagePathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestFlow()
{
    GUIStateParams *p = GetStateParams();
    string          latestPath = p->GetCurrentFlowPath();

    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetFlowDir()) return;
    setSettingsChanged(true);
    sParams->SetFlowDir(latestPath);
    flowPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::copyLatestPython()
{
    GUIStateParams *p = GetStateParams();
    string          latestPath = p->GetCurrentPythonPath();

    size_t pos = latestPath.find_last_of("\\/");
    if (pos != string::npos) latestPath = latestPath.substr(0, pos);
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    if (latestPath == sParams->GetPythonDir()) return;
    setSettingsChanged(true);
    sParams->SetPythonDir(latestPath);
    pythonPathEdit->setText(latestPath.c_str());
}
void StartupEventRouter::changeTextureSize(bool val)
{
    confirmText();
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    setSettingsChanged(true);
    sParams->SetTexSizeEnable(val);
}
void StartupEventRouter::winLockChanged(bool val)
{
    confirmText();
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    setSettingsChanged(true);
    sParams->SetWinSizeLock(val);
}
void StartupEventRouter::setAutoStretch(bool val)
{
    confirmText();
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    setSettingsChanged(true);
    sParams->SetAutoStretch(val);
}

void StartupEventRouter::restoreDefaults()
{
#ifdef DEAD
    StartupParams *sParams = (StartupParams *)GetActiveParams();
    setSettingsChanged(true);

    Command *cmd = Command::CaptureStart(sParams, "Restore defaults");
    sParams->restart();
    Command::CaptureEnd(cmd, sParams);
#endif
    updateTab();
}

void StartupEventRouter::tabChanged(int topIndex, int subIndex)
{
#ifdef DEAD
    if (topIndex != _topTabIndex) return;
    if (subIndex != _subTabIndex) return;
#endif

    if (_settingsChanged || _textChangedFlag) {
        // Tell user to save preferences
        QMessageBox msgBox;
        msgBox.setText("Startup settings have changed.");
        msgBox.setInformativeText("Do you want to save your settings?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save) {
            saveStartup();
        } else {
#ifdef DEAD
            // revert to saved settings
            assert(_savedStartupParams);
            StartupParams *sParams = (StartupParams *)_paramsMgr->GetDefaultParams(StartupParams::_startupParamsTag);
            assert(sParams);
            ParamNode *copyNode = _savedStartupParams->GetRootNode()->deepCopy();
            delete sParams->GetRootNode();
            sParams->SetRootParamNode(copyNode);
            delete _savedStartupParams;
            _savedStartupParams = 0;
#endif
        }
        _settingsChanged = false;
    }
}
void StartupEventRouter::setSettingsChanged(bool didchange)
{
#ifdef DEAD
    // Copy the params before the first change
    if (didchange && !_settingsChanged) {
        StartupParams *sParams = (StartupParams *)GetActiveParams();
        _savedStartupParams = (StartupParams *)sParams->deepCopy(sParams->GetRootNode()->deepCopy());
    }
    _settingsChanged = didchange;
#endif
}

void StartupEventRouter::loadDefaultsFromStartupFile()
{
    StartupParams *sParams = (StartupParams *)GetActiveParams();

    XmlNode *node = sParams->GetNode();
    assert(node != NULL);

    bool enabled = MyBase::GetEnableErrMsg();
    MyBase::EnableErrMsg(false);

    XmlParser xmlparser;
    int       rc = xmlparser.LoadFromFile(node, _startupPath);

    MyBase::EnableErrMsg(enabled);
}
