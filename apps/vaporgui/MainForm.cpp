#include <vapor/glutil.h>    // Must be included first!!!
#include "vapor/VAssert.h"
#include <functional>
#include <cmath>
#include <memory>
#ifndef _WIN32
    #include <unistd.h>
    #include "windowsUtils.h"
#endif

#include <QDesktopWidget>
#include <QDockWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QInputDialog>
#include <QWhatsThis>
#include <QStatusBar>
#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QVBoxLayout>

#include <vapor/Version.h>
#include <vapor/ControlExecutive.h>
#include <vapor/ResourcePath.h>
#include <vapor/FileUtils.h>
#include <vapor/STLUtils.h>
#include <vapor/Proj4API.h>

#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/DCMPAS.h>
#include <vapor/DCP.h>
#include <vapor/DCCF.h>
#include <vapor/DCBOV.h>
#include <vapor/DCUGRID.h>

#include "VizWinMgr.h"
#include "BannerGUI.h"
#include "Statistics.h"
#include "PythonVariables.h"
#include "PProjectionStringWidget.h"
#include "VProjectionStringFrame.h"
#include "Plot.h"
#include "ErrorReporter.h"
#include "MainForm.h"
#include "FileOperationChecker.h"
#include "ParamsWidgetDemo.h"
#include "AppSettingsMenu.h"
#include "CheckForUpdateUI.h"
#include "NoticeBoard.h"
#include "PVisualizerSelector.h"
#include "QtVizWinGLContextManager.h"
#include "ProgressStatusBar.h"
#include "LeftPanel.h"
#include "CLIToolInstaller.h"
#include "Updatable.h"
#include "CitationReminder.h"
#include "BookmarkManager.h"
#include "UCloseVDCMenu.h"
#include "PTimestepInput.h"
#include "NcarCasperUtils.h"
#include "ViewpointToolbar.h"
#include "DatasetTypeLookup.h"

#include <QStyle>
#include <vapor/Progress.h>
#include <vapor/OSPRay.h>

#include <vapor/XmlNode.h>
#include <vapor/NavigationUtils.h>


#include "images/vapor-icon-32.xpm"
#include "images/playreverse.xpm"
#include "images/playforward.xpm"
#include "images/pauseimage.xpm"
#include "images/stepfwd.xpm"
#include "images/stepback.xpm"


using namespace std;
using namespace VAPoR;

const QEvent::Type MainForm::ParamsChangeEvent = (QEvent::Type)QEvent::registerEventType();
const QEvent::Type MainForm::ParamsIntermediateChangeEvent = (QEvent::Type)QEvent::registerEventType();
MainForm *MainForm::_instance = nullptr;


MainForm::MainForm(vector<QString> files, QApplication *app, bool interactive, string filesType, QWidget *parent) : QMainWindow(parent)
{
    _App = app;
    _sessionNewFlag = true;
    _begForCitation = true;

    assert(!_instance);
    _instance = this;

    setWindowTitle(tr("VAPoR:  NCAR Visualization and Analysis Platform for Research"));

    setAttribute(Qt::WA_DeleteOnClose);

    // For vertical screens, reverse aspect ratio for window size
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect    screenSize = screen->geometry();

    if (screenSize.width() < screenSize.height()) {
        resize(screenSize.width() * .7, screenSize.width() * .7 * screenSize.width() / (float)screenSize.height());
    } else {
        resize(screenSize.width() * .7, screenSize.height() * .7);
    }

    setWindowIcon(QPixmap(vapor_icon___));

    auto sideDockWidgetArea = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, sideDockWidgetArea);
    sideDockWidgetArea->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    sideDockWidgetArea->setFeatures(QDockWidget::NoDockWidgetFeatures);

    vector<string> myParams;
    myParams.push_back(GUIStateParams::GetClassType());
    myParams.push_back(SettingsParams::GetClassType());
    myParams.push_back(AnimationParams::GetClassType());
    myParams.push_back(AnnotationParams::GetClassType());

    vector<string> myRenParams;
    myRenParams.push_back(StatisticsParams::GetClassType());
    myRenParams.push_back(PlotParams::GetClassType());

    _paramsMgr = new ParamsMgr(myParams, myRenParams);
    _paramsMgr->RegisterStateChangeCB(std::bind(&MainForm::_stateChangeCB, this));
    _paramsMgr->RegisterIntermediateStateChangeCB(std::bind(&MainForm::_intermediateStateChangedCB, this));
    _paramsMgr->RegisterStateChangeFlag(&_stateChangeFlag);

    _controlExec = new ControlExec(_paramsMgr);
    _controlExec->SetSaveStateEnabled(false);

    _appSettingsMenu = new AppSettingsMenu(this);

    _vizWinMgr = new VizWinMgr(_controlExec);
    _controlExec->SetVisualizerGLContextManager(_vizWinMgr->visualizerGLContextManager);
    setCentralWidget(_vizWinMgr);
    _dependOnLoadedData_insert(_vizWinMgr);

    _animationController = new AnimationController(_controlExec);
    connect(_animationController, SIGNAL(AnimationOnOffSignal(bool)), this, SLOT(_setAnimationOnOff(bool)));
    connect(_animationController, &AnimationController::AnimationDrawSignal, [this]() {
        setUpdatesEnabled(false);
        _controlExec->SyncWithParams();
        updateUI();
        setUpdatesEnabled(true);

        Render(false, true);
    });

    _leftPanel = new LeftPanel(_controlExec, this);
    const int dpi = qApp->desktop()->logicalDpiX();
    _leftPanel->setMinimumWidth(dpi > 96 ? 675 : 460);
    _leftPanel->setMinimumHeight(500);
    _dependOnLoadedData_insert(_leftPanel);
    _updatableElements.insert(_leftPanel);

    _status = new ProgressStatusBar;
    _status->hide();

    sideDockWidgetArea->setWidget(new VGroup({_leftPanel, _status}));
    // Only this specific resize method works for dock widgets, all other resize methods are noops
    resizeDocks({sideDockWidgetArea}, {_leftPanel->minimumWidth()}, Qt::Horizontal);

    createMenus();
    createToolBars();
    _createProgressWidget();

    GetSettingsParams()->FindDefaultSettingsPath();

    setUpdatesEnabled(true);

    // Command line options:
    //
    // - Session file
    // - Session file + data file
    // - data file
    //

    if (files.size() && files[0].endsWith(".vs3")) {
        bool loadDatasetsFromSession = files.size() == 1;
        openSession(files[0].toStdString(), loadDatasetsFromSession);
        files.erase(files.begin());

        if (!loadDatasetsFromSession && _controlExec->GetDataNames().size() > 1) {
            fprintf(stderr, "Cannot replace dataset from command line in session which contains multiple open datasets.");
            exit(1);
        }
    } else {
        sessionNew();
    }

    if (files.size()) {
        vector<string> paths;
        for (auto &f : files) paths.push_back(f.toStdString());

        string fmt;

        if (filesType == "auto") {
            if (!determineDatasetFormat(paths, &fmt)) {
                fmt = "";
                MSG_ERR("Could not determine dataset format for command line parameters");
            }
        } else {
            fmt = filesType;
        }

        if (!fmt.empty())
            ImportDataset(paths, fmt, ReplaceFirst);
    }

    app->installEventFilter(this);

    _controlExec->SetSaveStateEnabled(true);
    _controlExec->RebaseStateSave();
    _paramsMgr->TriggerManualStateChangeEvent("Init");
    _stateChangeFlag = false;

    if (interactive && GetSettingsParams()->GetAutoCheckForUpdates()) CheckForAndShowUpdate(_controlExec);
    if (interactive && GetSettingsParams()->GetAutoCheckForNotices()) NoticeBoard::CheckForAndShowNotices(_controlExec);
    NcarCasperUtils::CheckForCasperVGL(_controlExec);
}


int MainForm::RenderAndExit(int start, int end, const std::string &baseFile, int width, int height)
{
    if (start == 0 && end == 0) end = INT_MAX;
    start = std::max(0, start);

    if (_sessionNewFlag) {
        fprintf(stderr, "No session loaded\n");
        return -1;
    }

    QString   dir = QString::fromStdString(FileUtils::Dirname(baseFile));
    QFileInfo dirInfo(dir);
    if (!dirInfo.isWritable()) {
        fprintf(stderr, "Do not have write permissions\n");
        return -1;
    }

    auto baseFileWithTS = FileUtils::RemoveExtension(baseFile) + "-" + to_string(start) + "." + FileUtils::Extension(baseFile);

    auto ap = GetAnimationParams();
    auto vpp = _paramsMgr->GetViewpointParams(GetStateParams()->GetActiveVizName());

    _paramsMgr->BeginSaveStateGroup("test");
    StartAnimCapture(baseFileWithTS);
    ap->SetStartTimestep(start);
    ap->SetEndTimestep(end);

    vpp->SetValueLong(vpp->UseCustomFramebufferTag, "", true);
    vpp->SetValueLong(vpp->CustomFramebufferWidthTag, "", width);
    vpp->SetValueLong(vpp->CustomFramebufferHeightTag, "", height);

    _animationController->AnimationPlayForward();
    _paramsMgr->EndSaveStateGroup();

    connect(_animationController, &AnimationController::AnimationOnOffSignal, this, [this]() {
        endAnimCapture();
        close();
    });

    connect(_animationController, &AnimationController::AnimationDrawSignal, this, [this]() { printf("Rendering timestep %li\n", GetAnimationParams()->GetCurrentTimestep()); });

    return 0;
}

MainForm::~MainForm()
{
    QApplication::closeAllWindows();

    if (_banner) delete _banner;
    if (_controlExec) delete _controlExec;

    // This is required since if the user quits during the progressbar update,
    // qt will process the quit event and delete things, and then it will
    // return to the original event loop.
    // When Qt does recursive event loops like this, it has backend code that
    // prevents users from quiting.
    if (_insideMessedUpQtEventLoop) exit(0);
}

template<class T> bool MainForm::isDatasetValidFormat(const std::vector<std::string> &paths) const
{
    T    dc;
    bool errReportingEnabled = Wasp::MyBase::EnableErrMsg(false);
    int  ret = dc.Initialize(paths);
    Wasp::MyBase::EnableErrMsg(errReportingEnabled);
    return ret == 0;
}

bool MainForm::determineDatasetFormat(const std::vector<std::string> &paths, std::string *fmt) const
{
    vector<pair<string, bool>> formats = {
        {"vdc", isDatasetValidFormat<VDCNetCDF>(paths)}, {"wrf", isDatasetValidFormat<DCWRF>(paths)}, {"mpas", isDatasetValidFormat<DCMPAS>(paths)}, {"dcp", isDatasetValidFormat<DCP>(paths)},
        {"ugrid", isDatasetValidFormat<DCUGRID>(paths)}, {"cf", isDatasetValidFormat<DCCF>(paths)},   {"bov", isDatasetValidFormat<DCBOV>(paths)},
    };

    int nOk = 0;
    for (auto &f : formats) {
        if (f.second) {
            nOk++;
            *fmt = f.first;
        }
    }
    if (nOk == 1) return true;
    MyBase::SetErrMsg("Unable to confidently determine the dataset format. Please load it manually in the GUI");
    return false;
}


void MainForm::_createAnimationToolBar()
{
    _animationToolBar = addToolBar("Animation Control");

    _timeStepEdit = (new PTimestepInput(_controlExec))->SetTooltip("Edit/Display current time step");
    _dependOnLoadedData_insert(_timeStepEdit);
    _animationToolBar->addWidget(_timeStepEdit);
    _dependOnLoadedData_insert(_animationToolBar);
    _guiStateParamsUpdatableElements.insert(_timeStepEdit);

    _playBackwardAction = _animationToolBar->addAction(QPixmap(playreverse), "Play Backward", _animationController, SLOT(AnimationPlayReverse()));
    _stepBackAction = _animationToolBar->addAction(QPixmap(stepback), "Step Backward", _animationController, SLOT(AnimationStepReverse()));
    _pauseAction = _animationToolBar->addAction(QPixmap(pauseimage), "Pause", _animationController, SLOT(AnimationPause()));
    _stepForwardAction = _animationToolBar->addAction(QPixmap(stepfwd), "Step Forward", _animationController, SLOT(AnimationStepForward()));
    _playForwardAction = _animationToolBar->addAction(QPixmap(playforward), "Play Forward", _animationController, SLOT(AnimationPlayForward()));

    _playForwardAction->setCheckable(true);
    _playBackwardAction->setCheckable(true);
}

void MainForm::createToolBars()
{
    _createAnimationToolBar();

    auto vt = new ViewpointToolbar(this, _controlExec, _vizWinMgr);
    _dependOnLoadedData_insert(vt);
    _updatableElements.insert(vt);
    addToolBar(vt);
}

void MainForm::_createProgressWidget()
{
#define MAX_UPDATES_PER_SEC    10
#define MILLIS_BETWEEN_UPDATES (1000 / MAX_UPDATES_PER_SEC)
    _progressLastUpdateTime = std::chrono::system_clock::now();

    Progress::Update_t update = [this](long done, bool *cancelled) {
        if (!_progressEnabled) return;

        // Limit updates per second for perfomance reasons since this is on the same
        // thread as the calculation
        auto now = std::chrono::system_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(now - _progressLastUpdateTime);
        if (duration.count() < MILLIS_BETWEEN_UPDATES || _insideMessedUpQtEventLoop) return;
        _progressLastUpdateTime = now;

        // Qt will clear the currently bound framebuffer for some reason
        bool insideOpenGL = isOpenGLContextActive();
        if (insideOpenGL && _progressSavedFB < 0) {
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_progressSavedFB);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        _status->SetDone(done);
        _disableUserInputForAllExcept = _status->GetCancelButtonObject();
        _insideMessedUpQtEventLoop = true;
        QCoreApplication::processEvents();
        _disableUserInputForAllExcept = nullptr;
        _insideMessedUpQtEventLoop = false;
        *cancelled = _status->Cancelled();

        if (insideOpenGL && _progressSavedFB >= 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, _progressSavedFB);
            _progressSavedFB = -1;
        }
    };

    Progress::Start_t start = [this, update](const std::string &name, long total, bool cancelable) {
        if (!_progressEnabled) return;

        _status->StartTask(name, total, cancelable);
        update(0, &cancelable);
    };

    Progress::Finish_t finish = [this, update]() {
        if (!_progressEnabled) return;

        _status->Finish();
        bool b;
        update(0, &b);
    };

    Progress::SetHandlers(start, update, finish);
    _status->show();
    if (_progressEnabledMenuItem) _progressEnabledMenuItem->setChecked(true);
}

void MainForm::_disableProgressWidget()
{
    Progress::Finish();
    Progress::SetHandlers([](const string &, long, bool) {}, [](long, bool *) {}, []() {});
    _status->hide();
    if (_progressEnabledMenuItem) _progressEnabledMenuItem->setChecked(false);
}


void MainForm::_createFileMenu()
{
    auto fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction("New Session", this, &MainForm::sessionNew, QKeySequence("Ctrl+N"));
    fileMenu->addAction("Open Session", this, &MainForm::showOpenSessionGUI, QKeySequence("Ctrl+O"));
    fileMenu->addAction("Save Session", this, qOverload<>(&MainForm::SaveSession), QKeySequence("Ctrl+S"));
    fileMenu->addAction("Save Session As...", this, &MainForm::SaveSessionAs);

    fileMenu->addSeparator();
    auto importMenu = fileMenu->addMenu("Import Dataset");
    _updatableElements.insert(new UCloseVDCMenu(fileMenu, _controlExec));

    auto addImport = [this, importMenu](const auto &fmt, const auto &label) { importMenu->addAction(label, this, [this, fmt]() { showImportDatasetGUI(fmt); }); };
    addImport("vdc", "VDC");
    addImport("wrf", "WRF-ARW");
    addImport("cf", "NetCDF-CF");
    addImport("mpas", "MPAS");
    addImport("bov", "Brick of Values (BOV)");
    addImport("dcp", "Data Collection Particles (DCP)");
    addImport("ugrid", "Unstructured Grid (UGRID)");

    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, SLOT(close()));
}

void MainForm::_createEditMenu()
{
    auto editMenu = menuBar()->addMenu(tr("Edit"));

    _editUndoAction = editMenu->addAction("Undo", this, [this](){ _paramsMgr->Undo(); }, QKeySequence("Ctrl+Z"));
    _editRedoAction = editMenu->addAction("Redo", this, [this](){ _paramsMgr->Redo(); });
#ifdef WIN32
    _editRedoAction->setShortcut(QKeySequence("Ctrl+Y"));
#else
    _editRedoAction->setShortcut(QKeySequence("Ctrl+Shift+Z"));
#endif

    editMenu->addSeparator();
    editMenu->addAction("Preferences", _appSettingsMenu, &QDialog::open);

    editMenu->addSeparator();
    auto bm = new BookmarkManager(this, _controlExec, _vizWinMgr);
    bm->RegisterToMenu(editMenu);
    _updatableElements.insert(bm);
}

void MainForm::_createToolsMenu()
{
    auto toolMenu = menuBar()->addMenu("Tools");
    _dependOnLoadedData_insert(toolMenu->addAction("Plot Utility", this, SLOT(launchPlotUtility())));
    _dependOnLoadedData_insert(toolMenu->addAction("Data Statistics", this, SLOT(launchStats())));
    _dependOnLoadedData_insert(toolMenu->addAction("Python Variables", this, SLOT(launchPythonVariables())));
    _dependOnLoadedData_insert(toolMenu->addAction("Dataset Projection", this, SLOT(launchProjectionFrame())));

#ifdef WIN32
    #define ADD_INSTALL_CLI_TOOLS_ACTION 1
#endif
#ifdef Darwin
    #define ADD_INSTALL_CLI_TOOLS_ACTION 1
#endif
#ifdef ADD_INSTALL_CLI_TOOLS_ACTION
    toolMenu->addSeparator();
    auto installCLIToolsAction = toolMenu->addAction("Install Command Line Tools", [](){ CLIToolInstaller::Install(); });
    installCLIToolsAction->setToolTip("Add VAPOR_HOME to environment and add current utilities "
                                       "location to path. Needs to updated if app bundle moved");
#endif
}

void MainForm::_createHelpMenu()
{
    auto helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction("Online Documentation", [](){
        bool success = QDesktopServices::openUrl(QString::fromStdString("https://ncar.github.io/VaporDocumentationWebsite/"));
        if (!success) { MSG_ERR("Unable to launch Web browser for URL"); }
    });
    helpMenu->addSeparator();
    helpMenu->addAction("About VAPOR", this, &MainForm::helpAbout);
}

void MainForm::_createDeveloperMenu()
{
    _paramsWidgetDemo = new ParamsWidgetDemo;
    _guiStateParamsUpdatableElements.insert(_paramsWidgetDemo);

    _developerMenu = menuBar()->addMenu("Developer");
    _developerMenu->addAction("Show PWidget Demo", _paramsWidgetDemo, &QWidget::show);

    QAction *enableProgress = new QAction(QString("Enable Progress Bar"), nullptr);
    enableProgress->setCheckable(true);
    QObject::connect(enableProgress, &QAction::toggled, [this](bool checked) {
        if (checked)
            _createProgressWidget();
        else
            _disableProgressWidget();
    });
    _developerMenu->addAction(enableProgress);
    _progressEnabledMenuItem = enableProgress;

#ifdef BUILD_OSPRAY
    _developerMenu->addAction(QString::fromStdString("OSPRay " + VOSP::Version()))->setDisabled(true);
#endif
}

void MainForm::createMenus()
{
    _createFileMenu();
    _createEditMenu();
    _createToolsMenu();

    _captureMenu = menuBar()->addMenu(tr("Capture"));
    QAction* a = new QAction("Image capture controls have been moved to the Export Tab", _captureMenu);
    a->setEnabled(false);
    _captureMenu->addAction(a);

    _createHelpMenu();
#ifndef NDEBUG
    _createDeveloperMenu();
#endif
    menuBar()->adjustSize();
}


void MainForm::openSession(const string &path, bool loadData)
{
    closeAllParamsDatasets();
    closeProjectionFrame();

    auto datasetConflictAction = loadData
        ? ControlExec::LoadStateRelAndAbsPathsExistAction::Ask
        : ControlExec::LoadStateRelAndAbsPathsExistAction::LoadAbs;

retryLoad:
    try {
        int rc = _controlExec->LoadState(path, datasetConflictAction);
        if (rc < 0) {
            MSG_ERR("Failed to restore session from file");
            _controlExec->LoadState();
            return;
        }
    } catch(ControlExec::RelAndAbsPathsExistException const &e) {
        auto a = showSelectRelVAbsDataLoadGUI(e);
        if (!a) {
            sessionNew();
            return;
        }
        datasetConflictAction = a.value();
        goto retryLoad;
    }

    auto gsp = GetStateParams();
    GetSettingsParams()->LoadFromSettingsFile();
    gsp->SetCurrentSessionFile(path);
    if (loadData)
        checkSessionDatasetsExist();
    else
        for (auto name : gsp->GetOpenDataSetNames()) gsp->RemoveOpenDataSet(name);

    _paramsMgr->UndoRedoClear();
    _sessionNewFlag = false;
    _stateChangeFlag = false;
    _paramsMgr->TriggerManualStateChangeEvent("Session Opened");

    _leftPanel->GoToRendererTab();
}


void MainForm::showOpenSessionGUI()
{
    if (_stateChangeFlag) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        msgBox.setText("The current session settings are not saved. Do you want to continue? \nYou can choose \"No\" now to go back and save the current session.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return; }
    }

    string sessionDir = GetStateParams()->GetCurrentSessionFile();
    if (sessionDir.empty())
        sessionDir = GetSettingsParams()->GetSessionDir();

    const vector<string> files = getUserFileSelection("Choose a VAPOR session file to restore a session", sessionDir, "Vapor 3 Session Save Files (*.vs3)", false);
    if (files.empty()) return;
    const string path = files[0];
    if (!FileOperationChecker::FileGoodToRead(path.c_str())) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return;
    }

    openSession(path);
}


optional<ControlExec::LoadStateRelAndAbsPathsExistAction> MainForm::showSelectRelVAbsDataLoadGUI(const ControlExec::RelAndAbsPathsExistException &e)
{
    QStringList items({QString::fromStdString(e.AbsolutePath), QString::fromStdString(e.RelativePath)});
    bool ok;
    QString item = QInputDialog::getItem(this, tr("Load Data"), tr("Multiple dataset options found. Select which to load."), items, 0, false, &ok);
    if (!ok || item.isEmpty()) return {};
    if (item.toStdString() == e.RelativePath) return ControlExec::LoadStateRelAndAbsPathsExistAction::LoadRel;
    if (item.toStdString() == e.AbsolutePath) return ControlExec::LoadStateRelAndAbsPathsExistAction::LoadAbs;
    return {};
}


void MainForm::checkSessionDatasetsExist()
{
    GUIStateParams *sp = GetStateParams();

    for (const auto & dataset : sp->GetOpenDataSetNames()) {
        vector<string> paths = sp->GetOpenDataSetPaths(dataset);
        if (!std::all_of(paths.begin(), paths.end(), [](string path) { return FileUtils::Exists(path); })) {
            sp->RemoveOpenDataSet(dataset);

            string err = "This session links to the dataset " + dataset + " which was not found. Please open this dataset if it is in a different location";
            string details;
            for (const auto &path : paths)
                if (!FileUtils::Exists(path)) details += "\"" + path + "\" not found.\n";

            ErrorReporter::GetInstance()->Report(err, ErrorReporter::Warning, details);
        }
    }
}


void MainForm::closeAllParamsDatasets()
{
    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();
    for (int i = 0; i < dataSetNames.size(); i++) { _controlExec->CloseData(dataSetNames[i]); }
}


void MainForm::SaveSession()
{
    const auto currentSessionPath = GetStateParams()->GetCurrentSessionFile();
    if (currentSessionPath.empty())
        SaveSessionAs();
    else
        SaveSession(currentSessionPath);
}


void MainForm::SaveSession(string path)
{
    assert(!path.empty());

    if (!FileOperationChecker::FileGoodToWrite(QString::fromStdString(path))) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return;
    }

    if (_controlExec->SaveSession(path) < 0) {
        MSG_ERR("Saving session file failed");
        return;
    }

    GUIStateParams *p = GetStateParams();
    _paramsMgr->PushSaveStateEnabled(false);
    p->SetCurrentSessionFile(path);
    _paramsMgr->PopSaveStateEnabled();

    _stateChangeFlag = false;
}


void MainForm::SaveSessionAs()
{
    SettingsParams *sP = GetSettingsParams();
    GUIStateParams *guiStateParams = GetStateParams();

    string dir;
    if (!guiStateParams->GetCurrentSessionFile().empty())
        dir = guiStateParams->GetCurrentSessionFile();
    else
        dir = sP->GetSessionDir();

    QFileDialog fileDialog(this, "Save VAPOR session file", QString::fromStdString(dir), QString("Vapor 3 Session Save file (*.vs3)"));

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDefaultSuffix(QString("vs3"));
    if (fileDialog.exec() != QDialog::Accepted) return;

    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty() || files.size() > 1) return;

    SaveSession(files[0].toStdString());
}


void MainForm::_stateChangeCB()
{
    if (_paramsEventQueued) return;
    _paramsEventQueued = true;

    QEvent *event = new QEvent(ParamsChangeEvent);
    QApplication::postEvent(this, event);

    _eventsSinceLastSave++;
}

void MainForm::_intermediateStateChangedCB()
{
    if (_paramsEventQueued) return;
    _paramsEventQueued = true;

    QEvent *event = new QEvent(ParamsIntermediateChangeEvent);
    QApplication::postEvent(this, event);
}


void MainForm::helpAbout()
{
    std::string banner_file_name = "vapor_banner.png";
    if (_banner) delete _banner;
    std::string banner_text = "Visualization and Analysis Platform for atmospheric, Oceanic and "
                              "solar Research.\n\n"
                              "Developed by the National Center for Atmospheric Research's (NCAR) \n"
                              "Computational and Information Systems Lab. \n\n"
                              "Boulder, Colorado 80305, U.S.A.\n"
                              "Web site: http://www.vapor.ucar.edu\n"
                              "Contact: vapor@ucar.edu\n"
                              "Version: "
                            + string(Version::GetFullVersionString().c_str());

    _banner = new BannerGUI(this, banner_file_name, -1, true, banner_text.c_str(), "http://www.vapor.ucar.edu");
}


void MainForm::ImportDataset(const std::vector<string> &files, string format, DatasetExistsAction existsAction, string name)
{
    _paramsMgr->BeginSaveStateGroup("Import Dataset");
    if (name.empty()) name = _getDataSetName(files[0], existsAction);
    int rc = _controlExec->OpenData(files, name, format);
    if (rc < 0) {
        _paramsMgr->EndSaveStateGroup();
        MSG_ERR("Failed to load data");
        return;
    }

    auto gsp = _controlExec->GetParams<GUIStateParams>();
    gsp->InsertOpenDataSet(name, format, files);

    DataStatus *ds = _controlExec->GetDataStatus();
    GetAnimationParams()->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);

    if (_sessionNewFlag) {
        NavigationUtils::ViewAll(_controlExec);
        NavigationUtils::SetHomeViewpoint(_controlExec);
        gsp->SetProjectionString(ds->GetMapProjection());
    }

    _sessionNewFlag = false;
    _paramsMgr->EndSaveStateGroup();

    _leftPanel->GoToRendererTab();
}

void MainForm::showImportDatasetGUI(string format)
{
    static vector<pair<string, string>> prompts = GetDatasets();

    string defaultPath;
    auto openDatasets = GetStateParams()->GetOpenDataSetNames();
    if (!openDatasets.empty())
        defaultPath = GetStateParams()->GetOpenDataSetPaths(openDatasets.back())[0];
    else
        defaultPath = GetSettingsParams()->GetMetadataDir();

    auto files = getUserFileSelection(DatasetTypeDescriptiveName(format), defaultPath, "", format!="vdc");
    if (files.empty()) return;

    ImportDataset(files, format, DatasetExistsAction::Prompt);
}


bool MainForm::doesQStringContainNonASCIICharacter(const QString &s)
{
    for (int i = 0; i < s.length(); i++)
        if (s.at(i).unicode() > 127) return true;
    return false;
}

int MainForm::checkQStringContainsNonASCIICharacter(const QString &s)
{
    if (doesQStringContainNonASCIICharacter(s)) {
#ifdef WIN32
        MyBase::SetErrMsg("Windows will convert a colon (common in WRF timestamps) to a non-ASCII dot character. This needs to be renamed.\n");
#endif
        MyBase::SetErrMsg("Vapor does not support paths with non-ASCII characters.\n");
        MSG_ERR("Non ASCII Character in path");
        return -1;
    }
    return 0;
}

vector<string> MainForm::getUserFileSelection(string prompt, string dir, string filter, bool multi)
{
    QString qPrompt(prompt.c_str());
    QString qDir(dir.c_str());
    QString qFilter(filter.c_str());

    vector<string> files;
    if (multi) {
        QStringList           fileNames = QFileDialog::getOpenFileNames(this, qPrompt, qDir, qFilter);
        QStringList           list = fileNames;
        QStringList::Iterator it = list.begin();
        while (it != list.end()) {
            if (!it->isNull()) {
                if (checkQStringContainsNonASCIICharacter(*it) < 0)
                    return {};
                files.push_back((*it).toStdString());
            }
            ++it;
        }
    } else {
        QString fileName = QFileDialog::getOpenFileName(this, qPrompt, qDir, qFilter);
        if (!fileName.isNull()) {
            if (checkQStringContainsNonASCIICharacter(fileName) < 0)
                return {};
            files.push_back(fileName.toStdString());
        }
    }

    for (int i = 0; i < files.size(); i++) {
        QFileInfo fInfo(files[i].c_str());
        if (!fInfo.isReadable() || !fInfo.isFile()) {
            MyBase::SetErrMsg("File %s not readable", files[i].c_str());
            MSG_ERR("Invalid file selection");
            return {};
        }
    }
    return (files);
}

void MainForm::sessionNew()
{
    // Disable "Are you sure?" popup in debug build
#ifdef NDEBUG
    if (_stateChangeFlag) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        msgBox.setText("The current session settings are not saved. Do you want to continue? \nYou can choose \"No\" now to go back and save the current session.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return; }
    }
#endif

    closeProjectionFrame();
    _controlExec->LoadState();
    GetStateParams()->SetActiveVizName(_paramsMgr->CreateVisualizerParamsInstance());
    _paramsMgr->UndoRedoClear();

    _stateChangeFlag = false;
    _sessionNewFlag = true;
}

void MainForm::_setAnimationOnOff(bool on)
{
    if (on) {
        enableAnimationWidgets(false);
//        _App->removeEventFilter(this);
    } else {
        _playForwardAction->setChecked(false);
        _playBackwardAction->setChecked(false);
        enableAnimationWidgets(true);
//        _App->installEventFilter(this);
    }
}

void MainForm::showCitationReminder()
{
    // Disable citation reminder in Debug build
#ifndef NDEBUG
    return;
#endif
    if (!_begForCitation) return;
    _begForCitation = false;
    CitationReminder::Show();
}

void MainForm::Render(bool fast, bool skipSync)
{
    bool wasMenuBarEnabled = menuBar()->isEnabled();
    bool wasProgressEnabled = _progressEnabled;
    menuBar()->setEnabled(false);
    _progressEnabled = true;

    if (!skipSync) {
        setUpdatesEnabled(false);
        _controlExec->SyncWithParams();
        setUpdatesEnabled(true);
    }

    _vizWinMgr->Update(fast);
    _progressEnabled = wasProgressEnabled;
    menuBar()->setEnabled(wasMenuBarEnabled);

    auto ap = GetAnimationParams();
    if (_animationCapture == true && ap->GetCurrentTimestep() == ap->GetEndTimestep() && !ap->GetPlayBackwards()) {
        endAnimCapture();
    }
}

bool MainForm::eventFilter(QObject *obj, QEvent *event)
{
    VAssert(_controlExec && _vizWinMgr);

    if (event->type() == ParamsChangeEvent || event->type() == ParamsIntermediateChangeEvent) {
        _controlExec->EnforceDefaultAppState();
        _paramsEventQueued = false;
    }

    if (_insideMessedUpQtEventLoop) {
        // Prevent menu item actions from running
        if (event->type() == QEvent::MetaCall) return true;
        // Prevent queued ParamsChangedEvents from recursively running
        if (event->type() == ParamsChangeEvent || event->type() == ParamsIntermediateChangeEvent)
            return true;
        // Prevent user input for all widgets except the cancel button. This is essentially
        // the same behavior as we had before because the application would
        // freeze during a render so all user input was essentially blocked.
        // Since the events are processed top-down, we need to check to check
        // if this is not only the cancel button, but any of its parents
        if (_disableUserInputForAllExcept != nullptr && obj->isWidgetType()) {
            if (dynamic_cast<QInputEvent *>(event)) {
                const QObject *test = _disableUserInputForAllExcept;
                do {
                    if (obj == test) return false;    // Approve input
                } while (test = test->parent());
                return true;    // Reject input
            }
        }
    }

    if (event->type() == ParamsChangeEvent) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        setUpdatesEnabled(false);
        _controlExec->SyncWithParams();
        updateUI();
        setUpdatesEnabled(true);

        Render(false, true);

        QApplication::restoreOverrideCursor();
        return true;
    }

    if (event->type() == ParamsIntermediateChangeEvent) {
        // Rendering the GUI becomes a bottleneck and generally should not be affected by intermediate changes
        // updateUI();
#ifndef NDEBUG
        // Normally GUI doesn't get intermediate changes but this is for testing only
        _paramsWidgetDemo->Update(GetStateParams(), _paramsMgr);
#endif

        Render(true);

        return true;
    }

    return false;
}

void MainForm::updateMenus()
{
    _editUndoAction->setEnabled((bool)_paramsMgr->UndoSize());
    _editRedoAction->setEnabled((bool)_paramsMgr->RedoSize());
}

void MainForm::_performSessionAutoSave()
{
    if (_paramsMgr == NULL) return;

    SettingsParams *sParams = GetSettingsParams();
    if (sParams == NULL) return;

    int eventCountForAutoSave = sParams->GetChangesPerAutoSave();

    if (eventCountForAutoSave == 0) return;
    if (!sParams->GetSessionAutoSaveEnabled()) return;

    if (_eventsSinceLastSave >= eventCountForAutoSave) {
        string autoSaveFile = sParams->GetAutoSaveSessionFile();
        int    rc = _paramsMgr->SaveToFile(autoSaveFile);
        if (rc < 0) { MSG_ERR("Unable to write settings file " + autoSaveFile); }
        _eventsSinceLastSave = 0;
    }
}

void MainForm::updateUI()
{
    VAssert(_controlExec);

    _widgetsEnabled = !GetStateParams()->GetOpenDataSetNames().empty();

    for (auto &e : _dependOnLoadedData) e->setEnabled(_widgetsEnabled);

    for (const auto &e : _updatableElements) 
        e->Update();

    auto sp= _widgetsEnabled ? GetStateParams() : nullptr;
    for (const auto &e : _guiStateParamsUpdatableElements)
        e->Update(sp, _paramsMgr);


    _timeStepEdit->Update(GetStateParams()); // TODO this needs special handling for animation playback
    updateMenus();
    _appSettingsMenu->Update(GetSettingsParams());

    _performSessionAutoSave();
}

void MainForm::enableAnimationWidgets(bool on)
{
    if (on) {
        _playBackwardAction->setEnabled(true);
        _stepBackAction->setEnabled(true);
        _stepForwardAction->setEnabled(true);
        _playForwardAction->setEnabled(true);
    } else {
        _animationToolBar->setEnabled(true);
        _playBackwardAction->setEnabled(false);
        _stepBackAction->setEnabled(false);
        _stepForwardAction->setEnabled(false);
        _playForwardAction->setEnabled(false);
    }
}


void MainForm::CaptureSingleImage(string filter, string defaultSuffix)
{
    showCitationReminder();

    std::string imageDir = GetAnimationParams()->GetValueString(AnimationParams::CaptureFileDirTag, "");
    if (imageDir.empty()) imageDir = QDir::homePath().toStdString();

    QFileDialog fileDialog(this, "Specify single image capture file name", QString::fromStdString(imageDir), QString::fromStdString(filter));

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.move(pos());
    fileDialog.resize(450, 450);
    QStringList mimeTypeFilters;
    if (fileDialog.exec() != QDialog::Accepted) return;

    // Extract the path, and the root name, from the returned string.
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty()) return;
    QString fn = files[0];
    auto ap = GetAnimationParams();
    ap->SetValueString(AnimationParams::CaptureFileNameTag, "Capture file name", FileUtils::Basename(fn.toStdString()));
    ap->SetValueString(AnimationParams::CaptureFileDirTag, "Capture file directory", FileUtils::Dirname(fn.toStdString()));

    QFileInfo fileInfo(fn);
    QString   suffix = fileInfo.suffix();
    if (suffix == "") {
        fn += QString::fromStdString(defaultSuffix);
        fileInfo.setFile(fn);
    }

    string file = fileInfo.absoluteFilePath().toStdString();
    string filepath = fileInfo.path().toStdString();

    // Turn on "image capture mode" in the current active visualizer
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    int             success = _vizWinMgr->EnableImageCapture(file, vizName);

    if (success < 0) MSG_ERR("Error capturing image");
}


void MainForm::launchStats()
{
    if (!_stats) {
        _stats = new Statistics(this);
        _updatableElements.insert(_stats);
        connect(_stats, &QDialog::finished, this, &MainForm::_statsClosed);
    }
    if (_controlExec) { _stats->initControlExec(_controlExec); }
    _stats->showMe();
}

void MainForm::_statsClosed()
{
    _updatableElements.erase(_stats);
    delete _stats;
    _stats = nullptr;
}

void MainForm::launchPlotUtility()
{
    if (!_plot) {
        _plot = new Plot(_controlExec->GetDataStatus(), _controlExec->GetParamsMgr(), this);
        _updatableElements.insert(_plot);
        connect(_plot, &QDialog::finished, this, &MainForm::_plotClosed);
    }
    _plot->Update();
    _plot->open();
}

void MainForm::_plotClosed()
{
    _updatableElements.erase(_plot);
    delete _plot;
    _plot = nullptr;
}

void MainForm::launchPythonVariables()
{
    if (!_pythonVariables){
        _pythonVariables = new PythonVariables(this);
        _updatableElements.insert(_pythonVariables);
    }
    _pythonVariables->InitControlExec(_controlExec);
    _pythonVariables->ShowMe();
}

void MainForm::launchProjectionFrame()
{
    if (_projectionFrame == nullptr){
        _projectionFrame = new VProjectionStringFrame(new PProjectionStringWidget(_controlExec));
        connect(_projectionFrame, &VProjectionStringFrame::closed, this, &MainForm::closeProjectionFrame);
        _projectionFrame->adjustSize();
        _projectionFrame->Update(GetStateParams());
        _guiStateParamsUpdatableElements.insert(_projectionFrame);
    }
    _projectionFrame->raise();
}

void MainForm::closeProjectionFrame() {
    if (_projectionFrame != nullptr) {
        _guiStateParamsUpdatableElements.erase(_projectionFrame);
        _projectionFrame->close();
        _projectionFrame = nullptr;
    }
}

void MainForm::AnimationPlayForward() const {
    _animationController->AnimationPlayForward();
}

bool MainForm::StartAnimCapture(string baseFile, string defaultSuffix)
{
    QString   fileName = QString::fromStdString(baseFile);
    QFileInfo fileInfo = QFileInfo(fileName);

    QString suffix = fileInfo.suffix();
    if (suffix != "" || suffix != "jpg" || suffix != "jpeg" || suffix != "tif" || suffix != "tiff" || suffix != "png") {}
    if (suffix == "") {
        suffix = QString::fromStdString(defaultSuffix);
        fileName += QString::fromStdString(defaultSuffix);
    } else {
        fileName = fileInfo.absolutePath() + "/" + fileInfo.baseName();
    }

    QString fileBaseName = fileInfo.baseName();

    int posn;
    for (posn = fileBaseName.length() - 1; posn >= 0; posn--) {
        if (!fileBaseName.at(posn).isDigit()) break;
    }
    int startFileNum = 0;

    unsigned int lastDigitPos = posn + 1;
    if (lastDigitPos < fileBaseName.length()) {
        startFileNum = fileBaseName.right(fileBaseName.length() - lastDigitPos).toInt();
        fileBaseName.truncate(lastDigitPos);
    }

    QString filePath = fileInfo.absolutePath() + "/" + fileBaseName;
    // Insert up to 4 zeros
    QString zeroes;
    if (startFileNum == 0)
        zeroes = "000";
    else {
        switch ((int)log10((float)startFileNum)) {
        case (0): zeroes = "000"; break;
        case (1): zeroes = "00"; break;
        case (2): zeroes = "0"; break;
        default: zeroes = ""; break;
        }
    }
    filePath += zeroes;
    filePath += QString::number(startFileNum);
    filePath += ".";
    filePath += suffix;
    string fpath = filePath.toStdString();

    // Check if the numbered file exists.
    QFileInfo check_file(filePath);
    if (check_file.exists()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        QString msg = "The following numbered file exists.\n ";
        msg += filePath;
        msg += "\n";
        msg += "Do you want to continue? You can choose \"No\" to go back and change the file name.";
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return false; }
    }

    // Turn on "image capture mode" in the current active visualizer
    _animationCapture = true;
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    _controlExec->EnableAnimationCapture(vizName, true, fpath);
    _capturingAnimationVizName = vizName;

    return true;
}

void MainForm::endAnimCapture()
{
    // Turn off capture mode for the current active visualizer (if it is on!)
    if (_capturingAnimationVizName.empty()) return;
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    if (vizName != _capturingAnimationVizName) { MSG_WARN("Terminating capture in non-active visualizer"); }
    if (_controlExec->EnableAnimationCapture(_capturingAnimationVizName, false)) MSG_WARN("Image Capture Warning;\nCurrent active visualizer is not capturing images");

    _animationCapture = false;

    _capturingAnimationVizName = "";
}

string MainForm::_getDataSetName(string file, DatasetExistsAction existsAction)
{
    vector<string> names = _paramsMgr->GetDataMgrNames();
    if (names.empty() || existsAction == AddNew)
        return ControlExec::MakeStringConformant(FileUtils::Basename(file));
    else if (existsAction == ReplaceFirst)
        return names[0];

    string newSession = "New Dataset";

    QStringList items;
    items << tr(newSession.c_str());
    for (int i = 0; i < names.size(); i++)
        items << tr(names[i].c_str());

    bool    ok;
    QString item = QInputDialog::getItem(this, tr("Load Data"), tr("Load as new dataset or replace existing"), items, 0, false, &ok);
    if (!ok || item.isEmpty())
        return "";

    string dataSetName = item.toStdString();

    if (dataSetName == newSession)
        dataSetName = ControlExec::MakeStringConformant(FileUtils::Basename(file));

    return dataSetName;
}
