//************************************************************************
//															*
//			 Copyright (C)  2015										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		Settingseventrouter.cpp
//
//	Author:	Scott Pearse
//			Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Feb 2018
//
//	Description:	Implements the SettingsEventRouter class.
//		This class supports routing messages from the gui to the params
//		This is derived from the Settings Widget
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

#ifdef VAPOR3_0_0_ALPHA
    #include "images/fileopen.xpm"
#endif

#include <qlabel.h>
#include <QFileDialog>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "TabManager.h"

#include "SettingsEventRouter.h"
#include "vapor/ControlExecutive.h"
#include "EventRouter.h"
#include "SettingsParams.h"
#include "ErrorReporter.h"
#include "FileOperationChecker.h"

#include "QIntValidatorWithFixup.h"

using namespace VAPoR;

SettingsEventRouter::SettingsEventRouter(QWidget *parent, ControlExec *ce) : QWidget(parent), Ui_SettingsGUI(), EventRouter(ce, SettingsParams::GetClassType())
{
    setupUi(this);

    ParamsBase::StateSave *ss = new ParamsBase::StateSave;
    _defaultParams = new SettingsParams(ss, false);

    QIntValidatorWithFixup *numThreadsValidator = new QIntValidatorWithFixup(0, INT_MAX, _numThreadsEdit);
    _numThreadsEdit->setValidator(numThreadsValidator);

    QIntValidatorWithFixup *cacheSizeValidator = new QIntValidatorWithFixup(1000, INT_MAX, _cacheSizeEdit);
    _cacheSizeEdit->setValidator(cacheSizeValidator);

    QIntValidatorWithFixup *windowWidthValidator = new QIntValidatorWithFixup(800, 16000, _windowWidthEdit);
    _windowWidthEdit->setValidator(windowWidthValidator);

    QIntValidatorWithFixup *windowHeightValidator = new QIntValidatorWithFixup(600, 16000, _windowHeightEdit);
    _windowHeightEdit->setValidator(windowHeightValidator);

    QIntValidatorWithFixup *autoSaveValidator = new QIntValidatorWithFixup(1, 1000, _autoSaveIntervalEdit);
    _autoSaveIntervalEdit->setValidator(autoSaveValidator);
}

SettingsEventRouter::~SettingsEventRouter() {}

/**********************************************************
 * Whenever a new Settingstab is created it must be hooked up here
 ************************************************************/
void SettingsEventRouter::hookUpTab()
{
    connect(_autoStretchCheckbox, SIGNAL(toggled(bool)), this, SLOT(_enableAutoStretch(bool)));
    connect(_autoSaveCheckbox, SIGNAL(toggled(bool)), this, SLOT(_enableAutoSave(bool)));
    connect(_autoSaveIntervalEdit, SIGNAL(returnPressed()), this, SLOT(_changesPerSaveChanged()));
    connect(_autoSaveFileButton, SIGNAL(clicked()), this, SLOT(_chooseAutoSaveFile()));
    connect(_autoSaveFileEdit, SIGNAL(returnPressed()), this, SLOT(_autoSaveFileChanged()));
    connect(_cacheSizeEdit, SIGNAL(returnPressed()), this, SLOT(_cacheSizeChanged()));
    connect(_windowSizeCheckbox, SIGNAL(toggled(bool)), this, SLOT(_enableWinSize(bool)));
    connect(_windowWidthEdit, SIGNAL(returnPressed()), this, SLOT(_windowSizeChanged()));
    connect(_windowHeightEdit, SIGNAL(returnPressed()), this, SLOT(_windowSizeChanged()));
    connect(_numThreadsEdit, SIGNAL(returnPressed()), this, SLOT(_numThreadsChanged()));
    connect(_defaultButton, SIGNAL(clicked()), this, SLOT(_restoreDefaults()));
    connect(_sessionPathEdit, SIGNAL(returnPressed()), this, SLOT(_setSessionPath()));
    connect(_metadataPathEdit, SIGNAL(returnPressed()), this, SLOT(_setMetadataPath()));
    connect(_tfPathEdit, SIGNAL(returnPressed()), this, SLOT(_setTFPath()));
    connect(_flowPathEdit, SIGNAL(returnPressed()), this, SLOT(_setFlowPath()));
    connect(_pythonPathEdit, SIGNAL(returnPressed()), this, SLOT(_setPythonPath()));
    connect(_sessionPathButton, SIGNAL(clicked()), this, SLOT(_chooseSessionPath()));
    connect(_metadataPathButton, SIGNAL(clicked()), this, SLOT(_chooseMetadataPath()));
    connect(_tfPathButton, SIGNAL(clicked()), this, SLOT(_chooseTFPath()));
    connect(_flowPathButton, SIGNAL(clicked()), this, SLOT(_chooseFlowPath()));
    connect(_pythonPathButton, SIGNAL(clicked()), this, SLOT(_choosePythonPath()));
}

void SettingsEventRouter::_numThreadsChanged()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    size_t          numThreads = (size_t)_numThreadsEdit->text().toInt();
    sParams->SetNumThreads(numThreads);
    _saveSettings();
}

void SettingsEventRouter::_cacheSizeChanged()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    int             cacheSize = _cacheSizeEdit->text().toInt();

    sParams->SetCacheMB(cacheSize);
    _saveSettings();
}

void SettingsEventRouter::_enableWinSize(bool enabled)
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    sParams->SetWinSizeLock(enabled);
    _windowWidthEdit->setEnabled(enabled);
    _windowHeightEdit->setEnabled(enabled);
    _saveSettings();
}

void SettingsEventRouter::_windowSizeChanged()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    int             width = _windowWidthEdit->text().toInt();
    int             height = _windowHeightEdit->text().toInt();
    size_t          oldWidth, oldHeight;
    sParams->GetWinSize(oldWidth, oldHeight);
    sParams->SetWinSize(width, height);
    _saveSettings();
}

void SettingsEventRouter::_enableAutoStretch(bool enabled)
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    sParams->SetAutoStretchEnabled(enabled);
    _saveSettings();
}

void SettingsEventRouter::_enableAutoSave(bool enabled)
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    sParams->SetSessionAutoSaveEnabled(enabled);
    _saveSettings();
}

void SettingsEventRouter::_changesPerSaveChanged()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    size_t          changesPerSave = (size_t)_autoSaveIntervalEdit->text().toInt();
    sParams->SetChangesPerAutoSave(changesPerSave);
    _saveSettings();
}

bool SettingsEventRouter::_confirmFileExist(QString &qfilename)
{
    QFileInfo check_file(qfilename);
    if (check_file.exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        QString msg = "The following file exists.\n ";
        msg += qfilename;
        msg += "\n";
        msg += "Do you want to continue? You can choose \"No\" to go back and change the file name.";
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No)
            return false;
        else
            return true;
    } else
        return true;
}

void SettingsEventRouter::_chooseAutoSaveFile()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    QFileDialog fileDialog(_autoSaveFileButton, QString("Select auso-save VAPOR session file"), QString::fromStdString(sParams->GetAutoSaveSessionFile()), QString("Vapor 3 Session Files (*.vs3)"));
    fileDialog.setDefaultSuffix(QString("vs3"));
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QDialog::Accepted) return;
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty() || files.size() > 1) return;
    QString qfilename = files.first();
    if (!qfilename.endsWith(".vs3")) qfilename.append(".vs3");

    if (!_confirmFileExist(qfilename)) {
        _updateTab();
        return;
    }

    if (FileOperationChecker::FileGoodToWrite(qfilename)) {
        sParams->SetAutoSaveSessionFile(qfilename.toStdString());
        _saveSettings();
    } else {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        _updateTab();
    }
}

void SettingsEventRouter::_autoSaveFileChanged()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    QString         qfilename = _autoSaveFileEdit->text();
    if (!qfilename.endsWith(".vs3")) {
        qfilename.append(".vs3");
        _autoSaveFileEdit->setText(qfilename);
    }

    if (!_confirmFileExist(qfilename)) {
        _updateTab();
        return;
    }

    if (FileOperationChecker::FileGoodToWrite(qfilename)) {
        sParams->SetAutoSaveSessionFile(qfilename.toStdString());
        _saveSettings();
    } else {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        _updateTab();
    }
}

void SettingsEventRouter::_setSessionPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    _setFilePath(&SettingsParams::SetSessionDir, &SettingsParams::GetSessionDir, *sParams, _sessionPathEdit);
}

void SettingsEventRouter::_setMetadataPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    _setFilePath(&SettingsParams::SetMetadataDir, &SettingsParams::GetMetadataDir, *sParams, _metadataPathEdit);
}

void SettingsEventRouter::_setFlowPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    _setFilePath(&SettingsParams::SetFlowDir, &SettingsParams::GetFlowDir, *sParams, _flowPathEdit);
}

void SettingsEventRouter::_setPythonPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    _setFilePath(&SettingsParams::SetPythonDir, &SettingsParams::GetPythonDir, *sParams, _pythonPathEdit);
}

void SettingsEventRouter::_setTFPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    _setFilePath(&SettingsParams::SetTFDir, &SettingsParams::GetTFDir, *sParams, _tfPathEdit);
}

void SettingsEventRouter::_setFilePath(void (SettingsParams::*setFunc)(string), string (SettingsParams::*getFunc)() const, SettingsParams &sParams, QLineEdit *lineEdit)
{
    string  path = lineEdit->text().toStdString();
    QString qpath = QString::fromStdString(path);
    if (FileOperationChecker::DirectoryGoodToRead(qpath)) {
        (sParams.*setFunc)(path);
    } else {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        _updateTab();
        return;
    }
}

void SettingsEventRouter::_blockSignals(bool block)
{
    QList<QWidget *>                 widgetList = this->findChildren<QWidget *>();
    QList<QWidget *>::const_iterator widgetIter(widgetList.begin());
    QList<QWidget *>::const_iterator lastWidget(widgetList.end());

    while (widgetIter != lastWidget) {
        (*widgetIter)->blockSignals(block);
        ++widgetIter;
    }
}

void SettingsEventRouter::_updateGeneralSettings()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    bool autoStretchDomain = sParams->GetAutoStretchEnabled();
    _autoStretchCheckbox->setChecked(autoStretchDomain);

    bool autoSaveSession = sParams->GetSessionAutoSaveEnabled();
    _autoSaveCheckbox->setChecked(autoSaveSession);

    int changesPerSave = sParams->GetChangesPerAutoSave();
    _autoSaveIntervalEdit->setText(QString::number(changesPerSave));

    string autoSaveFile = sParams->GetAutoSaveSessionFile();
    _autoSaveFileEdit->setText(QString::fromStdString(autoSaveFile));
}

void SettingsEventRouter::_updateDirectoryPaths()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string sessionPath = sParams->GetSessionDir();
    _sessionPathEdit->setText(QString::fromStdString(sessionPath));

    string metaPath = sParams->GetMetadataDir();
    _metadataPathEdit->setText(QString::fromStdString(metaPath));

    _imagePathEdit->setText(QDir::homePath());

    string tfPath = sParams->GetTFDir();
    _tfPathEdit->setText(QString::fromStdString(tfPath));

    string flowPath = sParams->GetFlowDir();
    _flowPathEdit->setText(QString::fromStdString(flowPath));

    string pythonPath = sParams->GetPythonDir();
    _pythonPathEdit->setText(QString::fromStdString(pythonPath));
}

void SettingsEventRouter::_updateStartupSettings()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    QString numThreads = QString::number(sParams->GetNumThreads());
    _numThreadsEdit->setText(numThreads);

    QString cacheSize = QString::number(sParams->GetCacheMB());
    _cacheSizeEdit->setText(cacheSize);

    size_t w, h;
    sParams->GetWinSize(w, h);
    _windowWidthEdit->setText(QString::number(w));
    _windowHeightEdit->setText(QString::number(h));

    bool windowLockEnabled = sParams->GetWinSizeLock();
    _windowSizeCheckbox->setChecked(windowLockEnabled);
    _windowWidthEdit->setEnabled(windowLockEnabled);
    _windowHeightEdit->setEnabled(windowLockEnabled);

    bool autoStretchEnabled = sParams->GetAutoStretchEnabled();
    _autoStretchCheckbox->setChecked(autoStretchEnabled);
}

// Insert values from params into tab panel
//
void SettingsEventRouter::_updateTab()
{
    _blockSignals(true);
    _updateGeneralSettings();
    _updateStartupSettings();
    _updateDirectoryPaths();
    _blockSignals(false);
}

string SettingsEventRouter::_choosePathHelper(string current, string help)
{
    // Launch a file-chooser dialog, just choosing the directory
    QString dir;

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

void SettingsEventRouter::_chooseSessionPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string dir = _choosePathHelper(sParams->GetSessionDir(), "Choose the session file directory");

    if (!dir.empty()) {
        sParams->SetSessionDir(dir);
        _saveSettings();
    }
}

void SettingsEventRouter::_chooseMetadataPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string dir = _choosePathHelper(sParams->GetMetadataDir(), "Choose the data file directory");

    if (!dir.empty()) {
        sParams->SetMetadataDir(dir);
        _saveSettings();
    }
}

void SettingsEventRouter::_chooseTFPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string dir = _choosePathHelper(sParams->GetTFDir(), "Choose the Transfer Function file directory");

    if (!dir.empty()) {
        sParams->SetTFDir(dir);
        _saveSettings();
    }
}

void SettingsEventRouter::_chooseFlowPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string dir = _choosePathHelper(sParams->GetFlowDir(), "Choose the Flow file directory");

    if (!dir.empty()) {
        sParams->SetFlowDir(dir);
        _saveSettings();
    }
}

void SettingsEventRouter::_choosePythonPath()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    string dir = _choosePathHelper(sParams->GetPythonDir(), "Choose the Python script file directory");

    if (!dir.empty()) {
        sParams->SetPythonDir(dir);
        _saveSettings();
    }
}

void SettingsEventRouter::_winLockChanged(bool val)
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();
    sParams->SetWinSizeLock(val);
}

void SettingsEventRouter::_restoreDefaults()
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Restoring default Settings");

    SettingsParams *settingsParams;
    settingsParams = (SettingsParams *)paramsMgr->GetParams("SettingsParams");

    XmlNode *settingsNode = settingsParams->GetNode();
    XmlNode *parent = settingsNode->GetParent();

    SettingsParams *newParams = new SettingsParams(*_defaultParams);
    *settingsParams = *newParams;
    settingsParams->GetNode()->SetParent(parent);

    delete newParams;

    _saveSettings();
    _updateTab();

    paramsMgr->EndSaveStateGroup();
}

void SettingsEventRouter::GetWebHelp(vector<pair<string, string>> &help) const
{
    help.clear();

    help.push_back(make_pair("Overview of the Settings tab", "http://www.vapor.ucar.edu/docs/vapor-gui-help/startup-tab#SettingsOverview"));
}

void SettingsEventRouter::_saveSettings()
{
    SettingsParams *sParams = (SettingsParams *)GetActiveParams();

    int rc = sParams->SaveSettings();
    if (rc < 0) { MSG_ERR("Failed to save startup file"); }

    _updateTab();
}
