/************************************************************************/
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
/************************************************************************/

//
//	File:		MainForm.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2004
//
//	Description:  Implementation of MainForm class
//		This QMainWindow class supports all main window functionality
//		including menus, tab dialog, docking, visualizer window,
//		and some of the communication between these classes
//
#define MIN_WINDOW_WIDTH  700
#define MIN_WINDOW_HEIGHT 700

#ifdef WIN32
    #pragma warning(disable : 4251 4100)
#endif
#include <vapor/glutil.h>    // Must be included first!!!
#include <iostream>
#include <fstream>
#include "vapor/VAssert.h"
#include <sstream>
#include <iostream>
#include <functional>
#include <cmath>

#include <QDesktopWidget>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMdiArea>
#include <QWhatsThis>
#include <QStatusBar>
#include <QDebug>
#include <QScreen>

#include <vapor/Version.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ControlExecutive.h>
#include <vapor/ResourcePath.h>
#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>
#include <vapor/STLUtils.h>
#include <vapor/Proj4API.h>

#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/DCMPAS.h>
#include <vapor/DCCF.h>

#include "VizWinMgr.h"
#include "VizSelectCombo.h"
#include "TabManager.h"
//#include "NavigationEventRouter.h"
//#include "AnnotationEventRouter.h"
//#include "AnimationEventRouter.h"
#include "BannerGUI.h"
#include "Statistics.h"
#include "PythonVariables.h"
#include "Plot.h"
#include "ErrorReporter.h"
#include "MainForm.h"
#include "FileOperationChecker.h"
#include "windowsUtils.h"
#include "ParamsWidgetDemo.h"
#include "AppSettingsMenu.h"

#include <QProgressDialog>
#include <QProgressBar>
#include <QToolButton>
#include <QStyle>
#include <vapor/Progress.h>
#include <vapor/OSPRay.h>

// Following shortcuts are provided:
// CTRL_N: new session
// CTRL_O: open session
// CTRL_S: Save current session
// CTRL_D: load data into current session
// CTRL_Z: Undo
// CTRL_Y: Redo
// CTRL_R: Region mode
// CTRL_2: 2D mode
// CTRL_I: Image mode
// CTRL_K: Rake mode
// CTRL_P: Play forward
// CTRL_B: Play backward
// CTRL_F: Step forward
// CTRL_E: Stop (End) play
// CTRL_T: Tile windows
// CTRL_V: View all
// CTRL_H: Home viewpoint

// The following are pixmaps that are used in gui:
#include "images/vapor-icon-32.xpm"
#include "images/cascade.xpm"
#include "images/tiles.xpm"

#include "images/home.xpm"
#include "images/sethome.xpm"
#include "images/eye.xpm"
#include "images/magnify.xpm"
#include "images/playreverse.xpm"
#include "images/playforward.xpm"
#include "images/pauseimage.xpm"
#include "images/stepfwd.xpm"
#include "images/stepback.xpm"

//
// TODO:
//
// Calls to launchStats() and launchPlot() will not work because
// we do not have a DataMgr to pass to them in 3.X.  We need to find
// a substitute.
//
//

/*
 *  Constructs a MainForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
using namespace std;
using namespace VAPoR;

const QEvent::Type MainForm::ParamsChangeEvent = (QEvent::Type)QEvent::registerEventType();
const QEvent::Type MainForm::ParamsIntermediateChangeEvent = (QEvent::Type)QEvent::registerEventType();
const std::string  MainForm::_documentationURL = "http://www.docs.vapor.ucar.edu";

namespace {

string makename(string file)
{
    QFileInfo qFileInfo(QString(file.c_str()));

    return (ControlExec::MakeStringConformant(qFileInfo.fileName().toStdString()));
}

};    // namespace

void MainForm::_initMembers()
{
    _mdiArea = NULL;
    _App = NULL;

    _playForwardAction = NULL;
    _playBackwardAction = NULL;
    _pauseAction = NULL;

    _editUndoAction = NULL;
    _editRedoAction = NULL;
    _appSettingsAction = NULL;
    _timeStepEdit = NULL;
    _timeStepEditValidator = NULL;

    _alignViewCombo = NULL;
    _main_Menubar = NULL;
    _File = NULL;
    _Edit = NULL;
    _Tools = NULL;
    _captureMenu = NULL;
    _helpMenu = NULL;

    _vizToolBar = NULL;
    _animationToolBar = NULL;

    _dataMenu = NULL;
    _closeVDCMenu = NULL;
    _importMenu = NULL;
    _sessionMenu = NULL;

    _fileOpenAction = NULL;
    _fileSaveAction = NULL;
    _fileSaveAsAction = NULL;
    _fileExitAction = NULL;
    _fileNew_SessionAction = NULL;

    _helpAboutAction = NULL;
    _whatsThisAction = NULL;
    _installCLIToolsAction = NULL;
    _webDocumentationAction = NULL;

    _dataImportWRF_Action = NULL;
    _dataImportCF_Action = NULL;
    _dataImportMPAS_Action = NULL;
    _dataLoad_MetafileAction = NULL;
    _dataClose_MetafileAction = NULL;
    _plotAction = NULL;
    _statsAction = NULL;
    _pythonAction = NULL;

    _singleImageMenu = NULL;
    _captureSingleJpegAction = NULL;
    _captureSinglePngAction = NULL;
    _captureSingleTiffAction = NULL;

    _imageSequenceMenu = NULL;
    _captureJpegSequenceAction = NULL;
    _capturePngSequenceAction = NULL;
    _captureTiffSequenceAction = NULL;

    _captureEndImageAction = NULL;

    _tileAction = NULL;
    _cascadeAction = NULL;
    _homeAction = NULL;
    _sethomeAction = NULL;
    _viewAllAction = NULL;
    _viewRegionAction = NULL;
    _stepForwardAction = NULL;
    _stepBackAction = NULL;
    _interactiveRefinementSpin = NULL;
    _tabDockWindow = NULL;

    _appSettingsMenu = nullptr;
    _stats = NULL;
    _plot = NULL;
    _pythonVariables = NULL;
    _banner = NULL;
    _windowSelector = NULL;
    _controlExec = NULL;
    _paramsMgr = NULL;
    _tabMgr = NULL;
    _vizWinMgr = NULL;

    _capturingAnimationVizName.clear();

    _stateChangeFlag = false;
    _sessionNewFlag = false;
    _begForCitation = false;
    _eventsSinceLastSave = 0;
    _buttonPressed = false;
}

class ProgressStatusBar : public QWidget {
    QLabel *      _titleLabel = new QLabel;
    QProgressBar *_progressBar = new QProgressBar;
    QToolButton * _cancelButton = new QToolButton;

    bool _canceled = false;

public:
    ProgressStatusBar()
    {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setMargin(4);
        setLayout(layout);

        _cancelButton->setIcon(_cancelButton->style()->standardIcon(QStyle::StandardPixmap::SP_DialogCancelButton));
        QObject::connect(_cancelButton, &QAbstractButton::clicked, this, [this]() {
            _canceled = true;
            Finish();
            SetTitle("Cancelled.");
        });

        QSizePolicy sp = _cancelButton->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        _cancelButton->setSizePolicy(sp);
        _cancelButton->setIconSize(_cancelButton->iconSize() * 0.7);
        _cancelButton->setToolTip("Cancel");

        layout->addWidget(_titleLabel);
        layout->addWidget(_progressBar);
        layout->addWidget(_cancelButton);

        Finish();
    }
    void SetTitle(const string &title) { _titleLabel->setText(QString::fromStdString(title)); }
    void SetTotal(long total) { _progressBar->setRange(0, total); }
    void SetCancelable(bool b) { _cancelButton->setEnabled(b); }
    void SetDone(long done) { _progressBar->setValue(done); }
    bool Cancelled() { return _canceled; }
    void StartTask(const string &title, long total, bool cancelable)
    {
        Reset();
        SetTitle(title);
        SetTotal(total);
        SetCancelable(cancelable);
        _progressBar->show();
        _cancelButton->show();
    }
    void Finish()
    {
        _progressBar->hide();
        _cancelButton->hide();
        SetTitle("");
    }
    void Reset()
    {
        _canceled = false;
        _progressBar->reset();
    }
    const QObject *GetCancelButtonObject() const { return _cancelButton; }
};

// Only the main program should call the constructor:
//
MainForm::MainForm(vector<QString> files, QApplication *app, QWidget *parent) : QMainWindow(parent)
{
    _initMembers();

    _App = app;
    _sessionNewFlag = true;
    _begForCitation = true;

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

    // insert my qmdiArea:
    //
    _mdiArea = new QMdiArea;
    _mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(_mdiArea);

    // Now add a docking tabbed window on the left side.
    //
    _tabDockWindow = new QDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, _tabDockWindow);
    _tabDockWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _tabDockWindow->setFeatures(QDockWidget::NoDockWidgetFeatures);

    // Register additional params with the ParamsMgr
    //
    vector<string> myParams;
    myParams.push_back(GUIStateParams::GetClassType());
    myParams.push_back(SettingsParams::GetClassType());
    myParams.push_back(AnimationParams::GetClassType());
    myParams.push_back(AnnotationParams::GetClassType());

    vector<string> myRenParams;
    myRenParams.push_back(StatisticsParams::GetClassType());
    myRenParams.push_back(PlotParams::GetClassType());

    // Force creation of the static error reporter, which registers
    // callback's with the MyBase error logger used by the vapor render
    // library.
    //
    _errRep = new ErrorReporter(this);

    // Create the Control executive before the VizWinMgr. Disable
    // state saving until completely initalized
    //
    _controlExec = new ControlExec(myParams, myRenParams);
    _controlExec->SetSaveStateEnabled(false);

    _paramsMgr = _controlExec->GetParamsMgr();
    _paramsMgr->RegisterStateChangeCB(std::bind(&MainForm::_stateChangeCB, this));
    _paramsMgr->RegisterIntermediateStateChangeCB(std::bind(&MainForm::_intermediateStateChangedCB, this));
    _paramsMgr->RegisterStateChangeFlag(&_stateChangeFlag);

    // Set Defaults from startup file
    //
    SettingsParams *sP = GetSettingsParams();
    _controlExec->SetCacheSize(sP->GetCacheMB());
    _controlExec->SetNumThreads(sP->GetNumThreads());

    _vizWinMgr = new VizWinMgr(this, _mdiArea, _controlExec);

    _tabMgr = new TabManager(this, _controlExec);
    _tabMgr->setUsesScrollButtons(true);

    int dpi = qApp->desktop()->logicalDpiX();
    if (dpi > 96)
        _tabMgr->setMinimumWidth(675);
    else
        _tabMgr->setMinimumWidth(460);

    _tabMgr->setMinimumHeight(500);

    QWidget *    sidebar = new QWidget;
    QVBoxLayout *sidebarLayout = new QVBoxLayout;
    sidebarLayout->setMargin(0);
    sidebarLayout->setSpacing(0);
    sidebar->setLayout(sidebarLayout);
    sidebarLayout->addWidget(_tabMgr);

    _status = new ProgressStatusBar;
    sidebarLayout->addWidget(_status);
    _status->hide();

    _tabDockWindow->setWidget(sidebar);

    createMenus();

    createToolBars();

    _createProgressWidget();

    //    (void)statusBar();
    _main_Menubar->adjustSize();
    hookupSignals();
    enableWidgets(false);

    // Load preferences at start, set preferences directory
    loadStartingPrefs();

    setUpdatesEnabled(true);

    //    show();

    // Command line options:
    //
    // - Session file
    // - Session file + data file
    // - data file
    //

    if (files.size() && files[0].endsWith(".vs3")) {
        bool loadDatasetsFromSession = files.size() == 1;
        sessionOpen(files[0], loadDatasetsFromSession);
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
        if (determineDatasetFormat(paths, &fmt)) {
            loadDataHelper("", paths, "", "", fmt, true, ReplaceFirst);
        } else {
            MSG_ERR("Could not determine dataset format for command line parameters");
        }

        _stateChangeCB();
    }

    app->installEventFilter(this);

    _controlExec->SetSaveStateEnabled(true);
    _controlExec->RebaseStateSave();
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
    startAnimCapture(baseFileWithTS);
    ap->SetStartTimestep(start);
    ap->SetEndTimestep(end);

    vpp->SetValueLong(vpp->UseCustomFramebufferTag, "", true);
    vpp->SetValueLong(vpp->CustomFramebufferWidthTag, "", width);
    vpp->SetValueLong(vpp->CustomFramebufferHeightTag, "", height);

    _tabMgr->AnimationPlayForward();
    _paramsMgr->EndSaveStateGroup();

    connect(_tabMgr, &TabManager::AnimationOnOffSignal, this, [this]() {
        endAnimCapture();
        close();
    });

    connect(_tabMgr, &TabManager::AnimationDrawSignal, this, [this]() { printf("Rendering timestep %li\n", GetAnimationParams()->GetCurrentTimestep()); });

    return 0;
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    if (_paramsWidgetDemo) { _paramsWidgetDemo->close(); }

    if (_banner) delete _banner;
    if (_controlExec) delete _controlExec;

    // This is required since if the user quits during the progressbar update,
    // qt will process the quit event and delete things, and then it will
    // return to the original event loop.
    // When Qt does recursive event loops like this, it has backend code that
    // prevents users from quiting.
    if (_insideMessedUpQtEventLoop) exit(0);

    // no need to delete child widgets, Qt does it all for us?? (see closeEvent)
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
    if (isDatasetValidFormat<VDCNetCDF>(paths))
        *fmt = "vdc";
    else if (isDatasetValidFormat<DCWRF>(paths))
        *fmt = "wrf";
    else if (isDatasetValidFormat<DCMPAS>(paths))
        *fmt = "mpas";
    else if (isDatasetValidFormat<DCCF>(paths))
        *fmt = "cf";
    else
        return false;
    return true;
}

void MainForm::_createAnimationToolBar()
{
    // Create actions for each animation control button:
    //
    QPixmap *playForwardIcon = new QPixmap(playforward);
    _playForwardAction = new QAction(*playForwardIcon, QString(tr("Play Forward  ")), this);
    _playForwardAction->setShortcut(Qt::CTRL + Qt::Key_P);
    _playForwardAction->setCheckable(true);

    QPixmap *playBackwardIcon = new QPixmap(playreverse);
    _playBackwardAction = new QAction(*playBackwardIcon, QString(tr("Play Backward  ")), this);
    _playBackwardAction->setShortcut(Qt::CTRL + Qt::Key_B);
    _playBackwardAction->setCheckable(true);

    QPixmap *pauseIcon = new QPixmap(pauseimage);
    _pauseAction = new QAction(*pauseIcon, QString(tr("End animation and unsteady flow integration  ")), this);
    _pauseAction->setShortcut(Qt::CTRL + Qt::Key_E);

    QPixmap *stepForwardIcon = new QPixmap(stepfwd);
    _stepForwardAction = new QAction(*stepForwardIcon, QString(tr("Step forward  ")), this);
    _stepForwardAction->setShortcut(Qt::CTRL + Qt::Key_F);

    QPixmap *stepBackIcon = new QPixmap(stepback);
    _stepBackAction = new QAction(*stepBackIcon, "Step back", this);

    _animationToolBar = addToolBar("animation control");
    _timeStepEditValidator = new QIntValidator(0, 99999, _animationToolBar);
    _timeStepEdit = new QLineEdit(_animationToolBar);
    _timeStepEdit->setAlignment(Qt::AlignHCenter);
    _timeStepEdit->setMaximumWidth(40);
    _timeStepEdit->setToolTip("Edit/Display current time step");
    _timeStepEdit->setValidator(_timeStepEditValidator);
    _animationToolBar->addWidget(_timeStepEdit);

    _animationToolBar->addAction(_playBackwardAction);
    _animationToolBar->addAction(_stepBackAction);
    _animationToolBar->addAction(_pauseAction);
    _animationToolBar->addAction(_stepForwardAction);
    _animationToolBar->addAction(_playForwardAction);

    QString qat = QString("The animation toolbar enables control of the time steps "
                          "in the current active visualizer.  Additional controls are "
                          "available in the animation tab ");

    _animationToolBar->setWhatsThis(qat);

    connect(_playForwardAction, SIGNAL(triggered()), _tabMgr, SLOT(AnimationPlayForward()));
    connect(_playBackwardAction, SIGNAL(triggered()), _tabMgr, SLOT(AnimationPlayBackward()));
    connect(_pauseAction, SIGNAL(triggered()), _tabMgr, SLOT(AnimationPause()));
    connect(_stepForwardAction, SIGNAL(triggered()), _tabMgr, SLOT(AnimationStepForward()));
    connect(_stepBackAction, SIGNAL(triggered()), _tabMgr, SLOT(AnimationStepBackward()));
    connect(_timeStepEdit, SIGNAL(returnPressed()), this, SLOT(_setTimeStep()));
}

void MainForm::_createVizToolBar()
{
    // Actions for the viztoolbar:
    QPixmap *homeIcon = new QPixmap(home);
    _homeAction = new QAction(*homeIcon, QString(tr("Go to Home Viewpoint  ")), this);
    _homeAction->setShortcut(QKeySequence(tr("Ctrl+H")));
    _homeAction->setShortcut(Qt::CTRL + Qt::Key_H);

    QPixmap *sethomeIcon = new QPixmap(sethome);
    _sethomeAction = new QAction(*sethomeIcon, "Set Home Viewpoint", this);

    QPixmap *eyeIcon = new QPixmap(eye);
    _viewAllAction = new QAction(*eyeIcon, QString(tr("View All  ")), this);
    _viewAllAction->setShortcut(QKeySequence(tr("Ctrl+V")));
    _viewAllAction->setShortcut(Qt::CTRL + Qt::Key_V);

    QPixmap *magnifyIcon = new QPixmap(magnify);
    _viewRegionAction = new QAction(*magnifyIcon, "View Region", this);

    QPixmap *tileIcon = new QPixmap(tiles);
    _tileAction = new QAction(*tileIcon, QString(tr("Tile Windows  ")), this);
    _tileAction->setShortcut(Qt::CTRL + Qt::Key_T);

    QPixmap *cascadeIcon = new QPixmap(cascade);
    _cascadeAction = new QAction(*cascadeIcon, "Cascade Windows", this);

    // Viz tool bar:
    //
    _vizToolBar = addToolBar("Viewpoint Toolbar");
    _vizToolBar->setWindowTitle(tr("VizTools"));
    QString vizHelpString = QString("The tools in the Viewpoint Toolbar help "
                                    "you with shortcuts that bookmark importation viewpoints in your "
                                    "scene, orient your viewpoint along axes, and configure your "
                                    "visualizers");
    _vizToolBar->setWhatsThis(vizHelpString);

    // Add a QComboBox to toolbar to select window
    //
    _windowSelector = new VizSelectCombo(this);
    _vizToolBar->addWidget(_windowSelector);

    _vizToolBar->addAction(_tileAction);
    _vizToolBar->addAction(_cascadeAction);
    _vizToolBar->addAction(_homeAction);
    _vizToolBar->addAction(_sethomeAction);
    _vizToolBar->addAction(_viewRegionAction);
    _vizToolBar->addAction(_viewAllAction);

    _alignViewCombo = new QComboBox(_vizToolBar);
    _alignViewCombo->addItem("Align View");
    _alignViewCombo->addItem("Nearest axis");
    _alignViewCombo->addItem("     + X ");
    _alignViewCombo->addItem("     + Y ");
    _alignViewCombo->addItem("     + Z ");
    _alignViewCombo->addItem("     - X ");
    _alignViewCombo->addItem("     - Y ");
    _alignViewCombo->addItem("Default: - Z ");
    _alignViewCombo->setToolTip("Rotate view to an axis-aligned viewpoint,\ncentered on current "
                                "rotation center.");

    _vizToolBar->addWidget(_alignViewCombo);

    _interactiveRefinementSpin = new QSpinBox(_vizToolBar);
    _interactiveRefinementSpin->setPrefix(" Interactive Refinement: ");
    _interactiveRefinementSpin->setMinimumWidth(230);

    _interactiveRefinementSpin->setToolTip("Specify minimum refinement level during mouse motion,\nused "
                                           "in DVR and Iso rendering");
    _interactiveRefinementSpin->setWhatsThis(QString("While the viewpoint is changing due to mouse-dragging "
                                                     "in a visualizer, the refinement level used by the DVR "
                                                     "and Iso renderers is reduced to this interactive refinement level, "
                                                     "if it is less than the current refinement level of the renderer."));
    _interactiveRefinementSpin->setMinimum(0);
    _interactiveRefinementSpin->setMaximum(10);

    _vizToolBar->addWidget(_interactiveRefinementSpin);

    connect(_homeAction, SIGNAL(triggered()), _tabMgr, SLOT(UseHomeViewpoint()));
    connect(_viewAllAction, SIGNAL(triggered()), _tabMgr, SLOT(ViewAll()));
    connect(_sethomeAction, SIGNAL(triggered()), _tabMgr, SLOT(SetHomeViewpoint()));
    connect(_alignViewCombo, SIGNAL(activated(int)), _tabMgr, SLOT(AlignView(int)));
    connect(_viewRegionAction, SIGNAL(triggered()), _tabMgr, SLOT(CenterSubRegion()));
    connect(_tileAction, SIGNAL(triggered()), _vizWinMgr, SLOT(FitSpace()));
    connect(_cascadeAction, SIGNAL(triggered()), _vizWinMgr, SLOT(Cascade()));
    connect(_interactiveRefinementSpin, SIGNAL(valueChanged(int)), this, SLOT(setInteractiveRefLevel(int)));
}

void MainForm::createToolBars()
{
    _createAnimationToolBar();
    _createVizToolBar();
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

void MainForm::hookupSignals()
{
    connect(_tabMgr, SIGNAL(AnimationOnOffSignal(bool)), this, SLOT(_setAnimationOnOff(bool)));

    connect(_tabMgr, SIGNAL(AnimationDrawSignal()), this, SLOT(_setAnimationDraw()));

    connect(_tabMgr, SIGNAL(ActiveEventRouterChanged(string)), this, SLOT(setActiveEventRouter(string)));

    // Signals on the VizWinMgr
    //
    connect(_windowSelector, SIGNAL(winActivated(const QString &)), _vizWinMgr, SLOT(SetWinActive(const QString &)));
    connect(_vizWinMgr, SIGNAL(newViz(const QString &)), _windowSelector, SLOT(AddWindow(const QString &)));
    connect(_vizWinMgr, SIGNAL(removeViz(const QString &)), _windowSelector, SLOT(RemoveWindow(const QString &)));
    connect(_vizWinMgr, SIGNAL(activateViz(const QString &)), _windowSelector, SLOT(SetWindowActive(const QString &)));
    connect(_windowSelector, SIGNAL(newWin()), _vizWinMgr, SLOT(LaunchVisualizer()));

    connect(_vizWinMgr, SIGNAL(activateViz(const QString &)), _tabMgr, SLOT(SetActiveViz(const QString &)));

    connect(_tabMgr, SIGNAL(Proj4StringChanged(string)), this, SLOT(_setProj4String(string)));
    connect(_vizWinMgr, SIGNAL(removeViz(const QString &)), _tabMgr, SLOT(SetActiveViz(const QString &)));
}

void MainForm::_createFileMenu()
{
    // Actions
    //
    _dataLoad_MetafileAction = new QAction(this);
    _dataLoad_MetafileAction->setText(tr("Open V&DC"));
    _dataLoad_MetafileAction->setToolTip("Specify a VDC data set to be loaded in current session");
    _dataLoad_MetafileAction->setShortcut(tr("Ctrl+D"));

    _dataClose_MetafileAction = new QAction(this);
    _dataClose_MetafileAction->setText(tr("Close VDC"));
    _dataClose_MetafileAction->setToolTip("Specify a VDC data set to close in current session");

    _dataImportWRF_Action = new QAction(this);
    _dataImportWRF_Action->setText(tr("WRF-ARW"));
    _dataImportWRF_Action->setToolTip("Specify one or more WRF-ARW output files to import into "
                                      "the current session");

    _dataImportCF_Action = new QAction(this);
    _dataImportCF_Action->setText(tr("NetCDF-CF"));
    _dataImportCF_Action->setToolTip("Specify one or more NetCDF Climate Forecast (CF) convention "
                                     "output files to import into the current session");

    _dataImportMPAS_Action = new QAction(this);
    _dataImportMPAS_Action->setText(tr("MPAS"));
    _dataImportMPAS_Action->setToolTip("Specify one or more MPAS output files to import into the "
                                       "current session");

    _fileOpenAction = new QAction(this);
    _fileOpenAction->setEnabled(true);
    _fileSaveAction = new QAction(this);
    _fileSaveAction->setEnabled(true);
    _fileSaveAsAction = new QAction(this);
    _fileSaveAsAction->setEnabled(true);
    _fileExitAction = new QAction(this);
    _fileNew_SessionAction = new QAction(this);

    _fileNew_SessionAction->setText(tr("&New Session"));

    _fileNew_SessionAction->setToolTip("Restart the session with default settings");
    _fileNew_SessionAction->setShortcut(Qt::CTRL + Qt::Key_N);

    _fileOpenAction->setText(tr("&Open Session"));
    _fileOpenAction->setShortcut(tr("Ctrl+O"));
    _fileOpenAction->setToolTip("Launch a file open dialog to reopen a previously saved session file");

    _fileSaveAction->setText(tr("&Save Session"));
    _fileSaveAction->setShortcut(tr("Ctrl+S"));
    _fileSaveAction->setToolTip("Launch a file-save dialog to save the state of this session in "
                                "current session file");
    _fileSaveAsAction->setText(tr("Save Session As..."));

    _fileSaveAsAction->setToolTip("Launch a file-save dialog to save the state of this session in "
                                  "another session file");

    _fileExitAction->setText(tr("E&xit"));

    _File = menuBar()->addMenu(tr("File"));

    _File->addAction(_dataLoad_MetafileAction);
    _closeVDCMenu = _File->addMenu("Close VDC");

    // closeVDCMenu items added dynamically in updateMenus()
    //

    _importMenu = _File->addMenu("Import");
    _importMenu->addAction(_dataImportWRF_Action);
    _importMenu->addAction(_dataImportCF_Action);
    _importMenu->addAction(_dataImportMPAS_Action);
    _File->addSeparator();

    // _File->addAction(createTextSeparator(" Session"));
    //
    _File->addAction(_fileNew_SessionAction);
    _File->addAction(_fileOpenAction);
    _File->addAction(_fileSaveAction);
    _File->addAction(_fileSaveAsAction);
    _File->addAction(_fileExitAction);

    connect(_dataLoad_MetafileAction, SIGNAL(triggered()), this, SLOT(loadData()));
    connect(_dataImportWRF_Action, SIGNAL(triggered()), this, SLOT(importWRFData()));
    connect(_dataImportCF_Action, SIGNAL(triggered()), this, SLOT(importCFData()));
    connect(_dataImportMPAS_Action, SIGNAL(triggered()), this, SLOT(importMPASData()));

    connect(_fileNew_SessionAction, SIGNAL(triggered()), this, SLOT(sessionNew()));
    connect(_fileOpenAction, SIGNAL(triggered()), this, SLOT(sessionOpen()));
    connect(_fileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));
    connect(_fileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    connect(_fileExitAction, SIGNAL(triggered()), this, SLOT(fileExit()));
}

void MainForm::_createEditMenu()
{
    _editUndoAction = new QAction(this);
    _editUndoAction->setText(tr("&Undo"));
    _editUndoAction->setShortcut(tr("Ctrl+Z"));
    _editUndoAction->setToolTip("Undo the most recent session state change");
    _editUndoAction->setEnabled(true);

    _editRedoAction = new QAction(this);
    _editRedoAction->setText(tr("&Redo"));
    _editRedoAction->setShortcut(tr("Ctrl+Y"));
    _editRedoAction->setToolTip("Redo the last undone session state change");
    _editRedoAction->setEnabled(false);


    _Edit = menuBar()->addMenu(tr("Edit"));
    _Edit->addAction(_editUndoAction);
    _Edit->addAction(_editRedoAction);

    _Edit->addSeparator();
    _appSettingsMenu = new AppSettingsMenu(this);
    _Edit->addAction("Preferences", _appSettingsMenu, &QDialog::open);

    connect(_editUndoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(_editRedoAction, SIGNAL(triggered()), this, SLOT(redo()));
}

void MainForm::_createToolsMenu()
{
    _plotAction = new QAction(this);
    _plotAction->setText("Plot Utility");
    _plotAction->setEnabled(false);

    _statsAction = new QAction(this);
    _statsAction->setText("Data Statistics");
    _statsAction->setEnabled(false);

    _pythonAction = new QAction(this);
    _pythonAction->setText("Python Variables");
    _pythonAction->setEnabled(false);

    _installCLIToolsAction = new QAction(this);
    _installCLIToolsAction->setText("Install Command Line Tools");
    _installCLIToolsAction->setToolTip("Add VAPOR_HOME to environment and add current utilities "
                                       "location to path. Needs to updated if app bundle moved");

    _Tools = menuBar()->addMenu(tr("Tools"));
    _Tools->addAction(_plotAction);
    _Tools->addAction(_statsAction);
    _Tools->addAction(_pythonAction);
#ifdef WIN32
    #define ADD_INSTALL_CLI_TOOLS_ACTION 1
#endif
#ifdef Darwin
    #define ADD_INSTALL_CLI_TOOLS_ACTION 1
#endif
#ifdef ADD_INSTALL_CLI_TOOLS_ACTION

    _Tools->addSeparator();

    _Tools->addAction(_installCLIToolsAction);

    connect(_installCLIToolsAction, SIGNAL(triggered()), this, SLOT(installCLITools()));

#endif

    connect(_statsAction, SIGNAL(triggered()), this, SLOT(launchStats()));
    connect(_plotAction, SIGNAL(triggered()), this, SLOT(launchPlotUtility()));
    connect(_pythonAction, SIGNAL(triggered()), this, SLOT(launchPythonVariables()));
}

void MainForm::_createCaptureMenu()
{
    _captureSingleJpegAction = new QAction(this);
    _captureSingleJpegAction->setText(tr("JPEG"));
    _captureSingleJpegAction->setToolTip("Capture one JPEG from current active visualizer");
    _captureSinglePngAction = new QAction(this);
    _captureSinglePngAction->setText(tr("PNG"));
    _captureSinglePngAction->setToolTip("Capture one PNG from current active visualizer");
    _captureSingleTiffAction = new QAction(this);
    _captureSingleTiffAction->setText(tr("TIFF"));
    _captureSingleTiffAction->setToolTip("Capture one TIFF from current active visualizer");

    _captureJpegSequenceAction = new QAction(this);
    _captureJpegSequenceAction->setText(tr("JPEG"));
    _captureJpegSequenceAction->setToolTip("Begin saving JPEG image files rendered in current active visualizer");
    _capturePngSequenceAction = new QAction(this);
    _capturePngSequenceAction->setText(tr("PNG"));
    _capturePngSequenceAction->setToolTip("Begin saving PNG image files rendered in current active visualizer");
    _captureTiffSequenceAction = new QAction(this);
    _captureTiffSequenceAction->setText(tr("TIFF"));
    _captureTiffSequenceAction->setToolTip("Begin saving TIFF image files rendered in current active visualizer");

    _captureEndImageAction = new QAction(this);
    _captureEndImageAction->setText(tr("End image capture"));
    _captureEndImageAction->setToolTip("End capture of image files in current active visualizer");
    _captureEndImageAction->setEnabled(false);

    // Note that the ordering of the following 4 is significant, so that image
    // capture actions correctly activate each other.
    //
    _captureMenu = menuBar()->addMenu(tr("Capture"));
    _singleImageMenu = _captureMenu->addMenu(tr("Single image"));
    //_singleImageMenu->addAction(_captureSingleJpegAction);
    _singleImageMenu->addAction(_captureSinglePngAction);
    _singleImageMenu->addAction(_captureSingleTiffAction);
    //_captureMenu->addMenu("Single image");
    _imageSequenceMenu = _captureMenu->addMenu(tr("Image sequence"));
    //_imageSequenceMenu->addAction(_captureJpegSequenceAction);
    _imageSequenceMenu->addAction(_capturePngSequenceAction);
    _imageSequenceMenu->addAction(_captureTiffSequenceAction);
    //_captureMenu->addAction(_captureStartJpegAction);
    _captureMenu->addAction(_captureEndImageAction);

    connect(_captureSingleJpegAction, SIGNAL(triggered()), this, SLOT(captureSingleJpeg()));
    connect(_captureSinglePngAction, SIGNAL(triggered()), this, SLOT(captureSinglePng()));
    connect(_captureSingleTiffAction, SIGNAL(triggered()), this, SLOT(captureSingleTiff()));

    connect(_captureJpegSequenceAction, SIGNAL(triggered()), this, SLOT(captureJpegSequence()));
    connect(_capturePngSequenceAction, SIGNAL(triggered()), this, SLOT(capturePngSequence()));
    connect(_captureTiffSequenceAction, SIGNAL(triggered()), this, SLOT(captureTiffSequence()));

    connect(_captureEndImageAction, SIGNAL(triggered()), this, SLOT(endAnimCapture()));
}

void MainForm::_createHelpMenu()
{
    _main_Menubar->addSeparator();

    _whatsThisAction = QWhatsThis::createAction(this);
    _whatsThisAction->setText(tr("Explain This"));
    _whatsThisAction->setToolTip(tr("Click here, then click over an object for context-sensitive help."));

    _helpAboutAction = new QAction(this);
    _helpAboutAction->setText(tr("About VAPOR"));
    _helpAboutAction->setToolTip(tr("Information about VAPOR"));
    _helpAboutAction->setEnabled(true);

    _webDocumentationAction = new QAction(this);
    _webDocumentationAction->setText("Online documentation");
    _webDocumentationAction->setEnabled(true);

    _helpMenu = menuBar()->addMenu(tr("Help"));
    _helpMenu->addAction(_webDocumentationAction);
    _helpMenu->addSeparator();
    _helpMenu->addAction(_whatsThisAction);
    _helpMenu->addAction(_helpAboutAction);

    connect(_helpAboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(_webDocumentationAction, &QAction::triggered, this, &MainForm::launchWebDocs);
}

void MainForm::_createDeveloperMenu()
{
    _paramsWidgetDemo = new ParamsWidgetDemo;
    //    _paramsWidgetDemo->setWindowFlags(_paramsWidgetDemo->windowFlags() & ~Qt::WA_QuitOnClose);

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
    // menubar
    //
    _main_Menubar = menuBar();

    _createFileMenu();
    _createEditMenu();
    _createToolsMenu();
    _createCaptureMenu();
    _createHelpMenu();
#ifndef NDEBUG
    _createDeveloperMenu();
#endif
}

void MainForm::sessionOpenHelper(string fileName, bool loadDatasets)
{
    // Clear out the current session:

    enableWidgets(false);

    _vizWinMgr->Shutdown();
    _tabMgr->Shutdown();

    // Close any open data sets
    //
    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();
    for (int i = 0; i < dataSetNames.size(); i++) { closeDataHelper(dataSetNames[i]); }

    if (!fileName.empty()) {
        int rc = _controlExec->LoadState(fileName);
        if (rc < 0) {
            MSG_ERR("Failed to restore session from file");
            _controlExec->LoadState();
        }
    } else {
        _controlExec->LoadState();
    }

    // Ugh. Load state will of course set open data sets in database
    //
    GUIStateParams *newP = GetStateParams();
    dataSetNames = newP->GetOpenDataSetNames();

    if (loadDatasets) {
        for (int i = 0; i < dataSetNames.size(); i++) {
            string         name = dataSetNames[i];
            vector<string> paths = newP->GetOpenDataSetPaths(name);
            if (std::all_of(paths.begin(), paths.end(), [](string path) { return FileUtils::Exists(path); })) {
                loadDataHelper(name, paths, "", "", newP->GetOpenDataSetFormat(name), true, DatasetExistsAction::AddNew);
            } else {
                newP->RemoveOpenDateSet(name);

                string err = "This session links to the dataset " + name + " which was not found. Please open this dataset if it is in a different location";

                string details;
                for (const auto &path : paths)
                    if (!FileUtils::Exists(path)) details += "\"" + path + "\" not found.\n";

                ErrorReporter::GetInstance()->Report(err, ErrorReporter::Warning, details);
            }
        }
    } else {
        for (auto name : dataSetNames) newP->RemoveOpenDateSet(name);
    }

    _vizWinMgr->Restart();
    _tabMgr->Restart();

    // Close data can't be undone
    //
    _controlExec->UndoRedoClear();
}

void MainForm::sessionOpen(QString qfileName, bool loadDatasets)
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

    // This launches a panel that enables the
    // user to choose input session save files, then to
    // load that session
    //
    if (qfileName == "") {
        SettingsParams *sP = GetSettingsParams();
        string          dir = sP->GetSessionDir();

        vector<string> files = myGetOpenFileNames("Choose a VAPOR session file to restore a session", dir, "Vapor 3 Session Save Files (*.vs3)", false);
        if (files.empty()) return;

        qfileName = files[0].c_str();
    }

    // Make sure the name ends with .vs3
    if (!qfileName.endsWith(".vs3")) { return; }

    if (!FileOperationChecker::FileGoodToRead(qfileName)) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return;
    }

    _paramsMgr->BeginSaveStateGroup("Load state");

    string fileName = qfileName.toStdString();
    _sessionNewFlag = false;
    sessionOpenHelper(fileName, loadDatasets);

    GUIStateParams *p = GetStateParams();
    p->SetCurrentSessionFile(fileName);

    _stateChangeFlag = false;

    GUIStateParams *state = GetStateParams();
    vector<string>  openDataSetNames = state->GetOpenDataSetNames();
    string          vizWin = state->GetActiveVizName();
    string          activeRendererType;
    string          activeRendererName;
    string          activeDataSetName;
    state->GetActiveRenderer(vizWin, activeRendererType, activeRendererName);
    _controlExec->RenderLookup(activeRendererName, vizWin, activeDataSetName, activeRendererType);

    if (STLUtils::Contains(openDataSetNames, activeDataSetName)) {
        _tabMgr->SetActiveRenderer(vizWin, activeRendererType, activeRendererName);
    } else {
        _tabMgr->HideRenderWidgets();
    }

    _paramsMgr->EndSaveStateGroup();

    // Session load can't currently be undone
    //
    _controlExec->UndoRedoClear();

    _stateChangeCB();
}

void MainForm::_fileSaveHelper(string path)
{
    if (path.empty()) {
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

        path = files[0].toStdString();
    }

    if (!FileOperationChecker::FileGoodToWrite(QString::fromStdString(path))) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        return;
    }

    if (_controlExec->SaveSession(path) < 0) {
        MSG_ERR("Saving session file failed");
        return;
    }

    GUIStateParams *p = GetStateParams();
    p->SetCurrentSessionFile(path);

    _stateChangeFlag = false;
}

void MainForm::fileSave()
{
    GUIStateParams *p = GetStateParams();
    string          path = p->GetCurrentSessionFile();

    _fileSaveHelper(path);
}

void MainForm::fileSaveAs() { _fileSaveHelper(""); }

void MainForm::fileExit() { close(); }

void MainForm::_stateChangeCB()
{
    if (_paramsEventQueued) return;
    _paramsEventQueued = true;

    // Generate an application event whenever state changes
    //
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

void MainForm::undoRedoHelper(bool undo)
{
    // Disable state saving
    //
    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    bool visualizerEvent = false;
    if (undo) {
        visualizerEvent = ((_paramsMgr->GetTopUndoDesc() == _controlExec->GetRemoveVisualizerUndoTag()) || (_paramsMgr->GetTopUndoDesc() == _controlExec->GetNewVisualizerUndoTag()));
    } else {
        visualizerEvent = ((_paramsMgr->GetTopRedoDesc() == _controlExec->GetRemoveVisualizerUndoTag()) || (_paramsMgr->GetTopRedoDesc() == _controlExec->GetNewVisualizerUndoTag()));
    }

    // Visualizer create/destroy undo/redo events require special
    // handling.
    //
    if (visualizerEvent) { _vizWinMgr->Shutdown(); }
    _tabMgr->Shutdown();

    bool status = true;
    if (undo) {
        status = _controlExec->Undo();
    } else {
        status = _controlExec->Redo();
    }
    if (!status) {
        MSG_ERR("Undo/Redo failed");
        _controlExec->SetSaveStateEnabled(enabled);
        return;
    }

    if (visualizerEvent) { _vizWinMgr->Restart(); }
    _tabMgr->Restart();

    // Restore state saving
    //
    _controlExec->SetSaveStateEnabled(enabled);
    _stateChangeCB();
}

void MainForm::undo()
{
    if (!_controlExec->UndoSize()) return;
    undoRedoHelper(true);
}

void MainForm::redo()
{
    if (!_controlExec->RedoSize()) return;
    undoRedoHelper(false);
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

// Close a data set and remove from database
//
void MainForm::closeDataHelper(string dataSetName)
{
    GUIStateParams *p = GetStateParams();

    p->RemoveOpenDateSet(dataSetName);

    _controlExec->CloseData(dataSetName);

    // Close data can't be undone
    //
    _controlExec->UndoRedoClear();
}

// Open a data set and remove from database. If data with same name
// already exists close it first.
//
bool MainForm::openDataHelper(string dataSetName, string format, const vector<string> &files, const vector<string> &options)
{
    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();

    // Open the data set
    //
    int rc = _controlExec->OpenData(files, options, dataSetName, format);
    if (rc < 0) {
        MSG_ERR("Failed to load data");
        return (false);
        ;
    }

    DataStatus *ds = _controlExec->GetDataStatus();
    p->SetProjectionString(ds->GetMapProjection());

    p->InsertOpenDateSet(dataSetName, format, files);

    _tabMgr->LoadDataNotify(dataSetName);

    // Opening data is not an undoable event :-(
    //
    _controlExec->UndoRedoClear();
    return (true);
}

void MainForm::loadDataHelper(string dataSetName, const vector<string> &files, string prompt, string filter, string format, bool multi, DatasetExistsAction existsAction)
{
    vector<string> myFiles = files;

    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();

    // This launches a panel that enables the
    // user to choose input data files, then to
    // create a datamanager using those files
    // or metafiles.
    //
    if (myFiles.empty()) {
        string defaultPath = ".";
        if (dataSetNames.size()) {
            string lastData = dataSetNames[dataSetNames.size() - 1];
            defaultPath = p->GetOpenDataSetPaths(lastData)[0];
        } else {
            SettingsParams *sP = GetSettingsParams();
            defaultPath = sP->GetMetadataDir();
        }

        myFiles = myGetOpenFileNames(prompt, defaultPath, filter, multi);
    }

    if (myFiles.empty()) return;

    // Generate data set name
    //
    if (dataSetName.empty()) { dataSetName = _getDataSetName(myFiles[0], existsAction); }
    if (dataSetName.empty()) return;

    vector<string> options = {"-project_to_pcs", "-vertical_xform"};

    if (!p->GetProjectionString().empty()) {
        options.push_back("-proj4");
        options.push_back(p->GetProjectionString());
    }

    bool status = openDataHelper(dataSetName, format, myFiles, options);
    if (!status) { return; }

    // Reinitialize all tabs
    //

    if (_sessionNewFlag) {
        _tabMgr->ViewAll();
        _tabMgr->SetHomeViewpoint();

        _sessionNewFlag = false;
    }

    DataStatus *ds = _controlExec->GetDataStatus();

    _tabMgr->Update();
    _vizWinMgr->Reinit();
    _tabMgr->Reinit();

    enableWidgets(true);

    _timeStepEditValidator->setRange(0, ds->GetTimeCoordinates().size() - 1);

    // Opening data is not an undoable event :-(
    //
    _controlExec->UndoRedoClear();
}

// Load data into current session
// If current session is at default then same as loadDefaultData
//
void MainForm::loadData(string fileName)
{
    vector<string> files;
    if (!fileName.empty()) { files.push_back(fileName); }

    loadDataHelper("", files, "Choose the Master data File to load", "Vapor VDC files (*.nc *.vdc)", "vdc", false);

    _tabMgr->adjustSize();
    _tabDockWindow->adjustSize();
}

void MainForm::closeData(string fileName)
{
    QAction *a = (QAction *)sender();

    string dataSetName = a->text().toStdString();

    closeDataHelper(dataSetName);

    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();

    if (dataSetNames.size() == 0) { enableWidgets(false); }

    _tabMgr->Update();
    _vizWinMgr->Reinit();

    // Close data can't be undone
    //
    _controlExec->UndoRedoClear();

    if (!_controlExec->GetDataNames().size()) { sessionNew(); }
}

// import WRF data into current session
//
void MainForm::importWRFData()
{
    vector<string> files;
    loadDataHelper("", files, "WRF-ARW NetCDF files", "", "wrf", true);
}

void MainForm::importCFData()
{
    vector<string> files;
    loadDataHelper("", files, "NetCDF CF files", "", "cf", true);
}

void MainForm::importMPASData()
{
    vector<string> files;
    loadDataHelper("", files, "MPAS files", "", "mpas", true);
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

vector<string> MainForm::myGetOpenFileNames(string prompt, string dir, string filter, bool multi)
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
                if (checkQStringContainsNonASCIICharacter(*it) < 0) return vector<string>();
                files.push_back((*it).toStdString());
            }
            ++it;
        }
    } else {
        QString fileName = QFileDialog::getOpenFileName(this, qPrompt, qDir, qFilter);
        if (!fileName.isNull()) {
            if (checkQStringContainsNonASCIICharacter(fileName) < 0) return vector<string>();
            files.push_back(fileName.toStdString());
        }
    }

    for (int i = 0; i < files.size(); i++) {
        QFileInfo fInfo(files[i].c_str());
        if (!fInfo.isReadable() || !fInfo.isFile()) {
            MyBase::SetErrMsg("Load Data Error \n Invalid data set\n");
            MSG_ERR("Failed to load data");
            return (vector<string>());
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

    _paramsMgr->BeginSaveStateGroup("Load state");

    sessionOpenHelper("");

    _vizWinMgr->LaunchVisualizer();

    _paramsMgr->EndSaveStateGroup();

    // Session load can't currently be undone
    //
    _controlExec->UndoRedoClear();

    _stateChangeFlag = false;
    _sessionNewFlag = true;
}

void MainForm::setInteractiveRefLevel(int val) {}
void MainForm::setInteractiveRefinementSpin(int val) {}

void MainForm::_setAnimationOnOff(bool on)
{
    if (on) {
        enableAnimationWidgets(false);

        _App->removeEventFilter(this);
    } else {
        _playForwardAction->setChecked(false);
        _playBackwardAction->setChecked(false);
        enableAnimationWidgets(true);
        _App->installEventFilter(this);

        // Generate an event and force an update
        //
        QEvent *event = new QEvent(ParamsChangeEvent);
        QApplication::postEvent(this, event);
    }
}

void MainForm::_setAnimationDraw()
{
    _tabMgr->Update();
    _vizWinMgr->Update(false);

    AnimationParams *aParams = GetAnimationParams();
    size_t           timestep = aParams->GetCurrentTimestep();

    _timeStepEdit->setText(QString::number((int)timestep));
}

void MainForm::enableKeyframing(bool ison)
{
    QPalette pal(_timeStepEdit->palette());
    _timeStepEdit->setEnabled(!ison);
}

void MainForm::showCitationReminder()
{
    // Disable citation reminder in Debug build
#ifndef NDEBUG
    return;
#endif
    if (!_begForCitation) return;

    _begForCitation = false;
    // Provide a customized message box
    QMessageBox msgBox;
    QString     reminder("VAPOR is developed as an Open Source application by NCAR, ");
    reminder.append("under the sponsorship of the National Science Foundation.\n\n");
    reminder.append("We depend on evidence of the software's value to the scientific community.  ");
    reminder.append("You are free to use VAPOR as permitted under the terms and conditions of the licence.\n\n ");
    reminder.append("Please cite VAPOR in your publications and presentations. ");
    reminder.append("Citation details:\n    http://www.vapor.ucar.edu/citation");
    msgBox.setText(reminder);

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.exec();
}

void MainForm::launchWebDocs() const
{
    bool success = QDesktopServices::openUrl(QString::fromStdString(_documentationURL));
    if (!success) { MSG_ERR("Unable to launch Web browser for URL"); }
}

void MainForm::loadStartingPrefs()
{
    string prefPath;
    string preffile =
#ifdef WIN32
        "\\.vapor3_prefs";
#else
        "/.vapor3_prefs";
#endif
    char *pPath = getenv("VAPOR3_PREFS_DIR");
    if (!pPath) pPath = getenv("HOME");
    if (!pPath) {
        vector<string> tpath;
        tpath.push_back("examples");
        tpath.push_back(preffile);
        prefPath = GetSharePath("examples/" + preffile);
    } else {
        prefPath = string(pPath) + preffile;
    }

    // Make this path the default at startup:
    //
    SettingsParams *sP = GetSettingsParams();
    sP->SetCurrentPrefsPath(prefPath);

#ifdef VAPOR3_0_0_ALPHA
    _controlExec->RestorePreferences(prefPath);
#endif
}

void MainForm::setActiveEventRouter(string type)
{
    // Set up help for active tab
    //
    vector<pair<string, string>> help;
    _tabMgr->GetWebHelp(type, help);
}

void MainForm::_setProj4String(string proj4String)
{
    GUIStateParams *p = GetStateParams();

    DataStatus *ds = _controlExec->GetDataStatus();
    string      currentString = ds->GetMapProjection();

    if (proj4String == currentString) return;

    Proj4API proj;
    int      err = proj.Initialize("", proj4String);
    if (err < 0) {
        MSG_ERR("Invalid proj4 string");
        return;
    }

    _App->removeEventFilter(this);

    vector<string> dataSets = p->GetOpenDataSetNames();

    _paramsMgr->BeginSaveStateGroup("Set proj4 string");

    // Close and re-open all data with new
    // proj4 string
    //
    map<int, vector<string>> filesMap;
    map<int, string>         formatsMap;
    for (int i = 0; i < dataSets.size(); i++) {
        // Save list of files and format before close so we can re-open
        //
        filesMap[i] = p->GetOpenDataSetPaths(dataSets[i]);
        formatsMap[i] = p->GetOpenDataSetFormat(dataSets[i]);

        closeDataHelper(dataSets[i]);
    }

    vector<string> options = {"-project_to_pcs", "-vertical_xform"};
    if (!proj4String.empty()) {
        options.push_back("-proj4");
        options.push_back(proj4String);
    };

    for (int i = 0; i < dataSets.size(); i++) { (void)openDataHelper(dataSets[i], formatsMap[i], filesMap[i], options); }

    _paramsMgr->EndSaveStateGroup();

    // Map projection changes can't currently be undone
    //
    _controlExec->UndoRedoClear();

    _App->installEventFilter(this);
}

bool MainForm::eventFilter(QObject *obj, QEvent *event)
{
    VAssert(_controlExec && _vizWinMgr);

    if (_insideMessedUpQtEventLoop) {
        // Prevent menu item actions from running
        if (event->type() == QEvent::MetaCall) return true;
        // Prevent queued ParamsChangedEvents from recursively running
        if (event->type() == ParamsChangeEvent) {
            _paramsEventQueued = false;
            return true;
        }
        if (event->type() == ParamsIntermediateChangeEvent) {
            _paramsEventQueued = false;
            return true;
        }
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

    // Only update the GUI if the Params state has changed
    //
    if (event->type() == ParamsChangeEvent) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (_stats) { _stats->Update(); }
        if (_plot) { _plot->Update(); }
        if (_pythonVariables) { _pythonVariables->Update(); }
        if (_appSettingsMenu) { _appSettingsMenu->Update(GetSettingsParams()); }

        setUpdatesEnabled(false);
        _tabMgr->Update();

#ifndef NDEBUG
        _paramsWidgetDemo->Update(GetStateParams(), _paramsMgr);
#endif
        setUpdatesEnabled(true);

        // force visualizer redraw
        //
        if (_animationCapture == false) {
            menuBar()->setEnabled(false);
            _progressEnabled = true;
            _vizWinMgr->Update(false);
            _progressEnabled = false;
            menuBar()->setEnabled(true);
        }

        update();

        QApplication::restoreOverrideCursor();

        _paramsEventQueued = false;
        return (false);
    }

    if (event->type() == ParamsIntermediateChangeEvent) {
        // Rendering the GUI becomes a bottleneck
        //        _tabMgr->Update();

        // force visualizer redraw
        //
        _progressEnabled = true;
        _vizWinMgr->Update(true);
        _progressEnabled = false;

#ifndef NDEBUG
        _paramsWidgetDemo->Update(GetStateParams(), _paramsMgr);
#endif

        //        update();

        _paramsEventQueued = false;
        return (false);
    }

    // Pass event on to target
    return (false);
}

void MainForm::updateMenus()
{
    GUIStateParams *p = GetStateParams();

    // Close menu
    //
    _closeVDCMenu->clear();
    vector<string> dataSetNames = p->GetOpenDataSetNames();
    int            size = dataSetNames.size();
    if (size < 1)
        _closeVDCMenu->setEnabled(false);
    else {
        _closeVDCMenu->setEnabled(true);
        for (int i = 0; i < dataSetNames.size(); i++) {
            // Add menu option to close the dataset in the File menu
            //
            QAction *closeAction = new QAction(QString::fromStdString(dataSetNames[i]), _closeVDCMenu);
            _closeVDCMenu->addAction(closeAction);

            connect(closeAction, SIGNAL(triggered()), this, SLOT(closeData()));
        }
    }

    _editUndoAction->setEnabled((bool)_controlExec->UndoSize());
    _editRedoAction->setEnabled((bool)_controlExec->RedoSize());

    // Turn off jpeg and png capture if we're in MapOrthographic mode
    ViewpointParams *VPP;
    VPP = _paramsMgr->GetViewpointParams(GetStateParams()->GetActiveVizName());

    // If there are no visualizers (can only happen if user deletes them)
    // then there are no ViewpointParams. See GitHub issue #1636
    //
    if (!VPP) return;

    if (VPP->GetProjectionType() == ViewpointParams::MapOrthographic) {
        if (_captureSingleJpegAction->isEnabled()) _captureSingleJpegAction->setEnabled(false);
        if (_captureSinglePngAction->isEnabled()) _captureSinglePngAction->setEnabled(false);
        if (_captureJpegSequenceAction->isEnabled()) _captureJpegSequenceAction->setEnabled(false);
        if (_capturePngSequenceAction->isEnabled()) _capturePngSequenceAction->setEnabled(false);
    } else {
        if (!_captureSingleJpegAction->isEnabled()) _captureSingleJpegAction->setEnabled(true);
        if (!_captureSinglePngAction->isEnabled()) _captureSinglePngAction->setEnabled(true);
        if (!_captureJpegSequenceAction->isEnabled()) _captureJpegSequenceAction->setEnabled(true);
        if (!_capturePngSequenceAction->isEnabled()) _capturePngSequenceAction->setEnabled(true);
    }
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

void MainForm::update()
{
    VAssert(_controlExec);

    AnimationParams *aParams = GetAnimationParams();
    size_t           timestep = aParams->GetCurrentTimestep();

    _timeStepEdit->setText(QString::number((int)timestep));

    updateMenus();

    _performSessionAutoSave();
}

void MainForm::enableWidgets(bool onOff)
{
    _captureMenu->setEnabled(onOff);
    _timeStepEdit->setEnabled(onOff);
    _animationToolBar->setEnabled(onOff);
    _tileAction->setEnabled(onOff);
    _cascadeAction->setEnabled(onOff);
    _homeAction->setEnabled(onOff);
    _sethomeAction->setEnabled(onOff);
    _viewAllAction->setEnabled(onOff);
    _viewRegionAction->setEnabled(onOff);
    //	_stepForwardAction->setEnabled(onOff);
    //	_stepBackAction->setEnabled(onOff);
    _interactiveRefinementSpin->setEnabled(onOff);
    _alignViewCombo->setEnabled(onOff);
    _editUndoAction->setEnabled(onOff);
    _editRedoAction->setEnabled(onOff);
    _windowSelector->setEnabled(onOff);
    _tabMgr->setEnabled(onOff);
    _statsAction->setEnabled(onOff);
    _plotAction->setEnabled(onOff);
    _pythonAction->setEnabled(onOff);

    _tabMgr->EnableRouters(onOff);
}

void MainForm::enableAnimationWidgets(bool on)
{
    enableWidgets(on);
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

void MainForm::captureSingleJpeg()
{
    string filter = "JPG (*.jpg *.jpeg)";
    string defaultSuffix = ".jpg";
    captureSingleImage(filter, defaultSuffix);
}

void MainForm::captureSinglePng()
{
    string filter = "PNG (*.png)";
    string defaultSuffix = ".png";
    captureSingleImage(filter, defaultSuffix);
}

void MainForm::captureSingleTiff()
{
    string filter = "TIFF (*.tif *.tiff)";
    string defaultSuffix = ".tiff";
    captureSingleImage(filter, defaultSuffix);
}

void MainForm::captureSingleImage(string filter, string defaultSuffix)
{
    showCitationReminder();
    auto imageDir = QDir::homePath();

    QFileDialog fileDialog(this, "Specify single image capture file name", imageDir, QString::fromStdString(filter));

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.move(pos());
    fileDialog.resize(450, 450);
    QStringList mimeTypeFilters;
    if (fileDialog.exec() != QDialog::Accepted) return;

    // Extract the path, and the root name, from the returned string.
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty()) return;
    QString fn = files[0];

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

void MainForm::installCLITools()
{
    QMessageBox box;
    box.addButton(QMessageBox::Ok);

#ifdef Darwin
    string home = GetResourcePath("");
    string binPath = home + "/MacOS";
    home.erase(home.size() - strlen("Contents/"), strlen("Contents/"));

    vector<string> profilePaths = {
        string(getenv("HOME")) + "/.profile",
        string(getenv("HOME")) + "/.zshrc",
    };
    bool success = true;

    for (auto profilePath : profilePaths) {
        FILE *prof = fopen(profilePath.c_str(), "a");
        success &= (bool)prof;
        if (prof) {
            fprintf(prof, "\n\n");
            fprintf(prof, "export VAPOR_HOME=\"%s\"\n", home.c_str());
            fprintf(prof, "export PATH=\"%s:$PATH\"\n", binPath.c_str());
            fclose(prof);
        }
    }

    if (success) {
        box.setText("Environmental variables set in ~/.profile and ~/.zshrc");
        box.setInformativeText("Please log out and log back in for changes to take effect.");
        box.setIcon(QMessageBox::Information);
    } else {
        box.setText("Unable to set environmental variables");
        box.setIcon(QMessageBox::Critical);
    }
#endif

#ifdef WIN32
    HKEY   key;
    long   error;
    long   errorClose;
    bool   pathWasModified = false;
    string home = GetResourcePath("");

    error = Windows_OpenRegistry(WINDOWS_HKEY_CURRENT_USER, "Environment", key);
    if (error == WINDOWS_SUCCESS) {
        string path;
        error = Windows_GetRegistryString(key, "Path", path, "");
        if (error == WINDOWS_ERROR_FILE_NOT_FOUND) {
            error = WINDOWS_SUCCESS;
            path = "";
        }
        if (error == WINDOWS_SUCCESS) {
            bool   alreadyExists = false;
            size_t index;
            if (path.find(";" + home + ";") != std::string::npos)
                alreadyExists = true;
            else if ((index = path.find(";" + home)) != std::string::npos && index + home.length() + 1 == path.length())
                alreadyExists = true;
            else if ((index = path.find(home + ";")) != std::string::npos && index == 0)
                alreadyExists = true;
            else if (path == home)
                alreadyExists = true;

            if (!alreadyExists) {
                if (path.length() > 0) path += ";";
                path += home;
                error = Windows_SetRegistryString(key, "Path", path);
                if (error == WINDOWS_SUCCESS) pathWasModified = true;
            }
        }
        errorClose = Windows_CloseRegistry(key);
    }

    if (error == WINDOWS_SUCCESS && errorClose == WINDOWS_SUCCESS) {
        // This tells windows to re-load the environment variables
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, NULL, (LPARAM) "Environment");

        box.setIcon(QMessageBox::Information);
        if (pathWasModified)
            box.setText("Vapor conversion utilities were added to your path");
        else
            box.setText("Your path is properly configured");
    } else {
        box.setIcon(QMessageBox::Critical);
        box.setText("Unable to set environmental variables");
        string errString = "";
        if (error != WINDOWS_SUCCESS) errString += Windows_GetErrorString(error) + "\n";
        if (errorClose != WINDOWS_SUCCESS) errString += "CloseRegistry: " + Windows_GetErrorString(errorClose);
        box.setInformativeText(QString::fromStdString(errString));
    }
#endif

    box.exec();
}

void MainForm::launchStats()
{
    if (!_stats) {
        _stats = new Statistics(this);
        connect(_stats, &QDialog::finished, this, &MainForm::_statsClosed);
    }
    if (_controlExec) { _stats->initControlExec(_controlExec); }
    _stats->showMe();
}

void MainForm::_statsClosed()
{
    if (_stats != nullptr) {
        delete _stats;
        _stats = nullptr;
    }
}

void MainForm::launchPlotUtility()
{
    if (!_plot) {
        VAssert(_controlExec->GetDataStatus());
        VAssert(_controlExec->GetParamsMgr());
        _plot = new Plot(_controlExec->GetDataStatus(), _controlExec->GetParamsMgr(), this);
        connect(_plot, &QDialog::finished, this, &MainForm::_plotClosed);
    }
    _plot->Update();
    _plot->open();
}

void MainForm::_plotClosed()
{
    if (_plot != nullptr) {
        delete _plot;
        _plot = nullptr;
    }
}

void MainForm::launchPythonVariables()
{
    if (!_pythonVariables) {
        _pythonVariables = new PythonVariables(this);
    }
    if (_controlExec) { _pythonVariables->InitControlExec(_controlExec); }
    _pythonVariables->ShowMe();
}

void MainForm::_setTimeStep()
{
    _paramsMgr->BeginSaveStateGroup("Change Timestep");
    int ts = _timeStepEdit->text().toInt();
    _tabMgr->AnimationSetTimestep(ts);
    _paramsMgr->EndSaveStateGroup();
}

void MainForm::captureJpegSequence()
{
    string filter = "JPG (*.jpg *.jpeg)";
    string defaultSuffix = "jpg";
    selectAnimCatureOutput(filter, defaultSuffix);
}

void MainForm::capturePngSequence()
{
    string filter = "PNG (*.png)";
    string defaultSuffix = "png";
    selectAnimCatureOutput(filter, defaultSuffix);
}

void MainForm::captureTiffSequence()
{
    string filter = "TIFF (*.tif *.tiff)";
    string defaultSuffix = "tiff";
    selectAnimCatureOutput(filter, defaultSuffix);
}

// Begin capturing animation images.
// Launch a file save dialog to specify the names
// Then start file saving mode.
void MainForm::selectAnimCatureOutput(string filter, string defaultSuffix)
{
    showCitationReminder();
    auto imageDir = QDir::homePath();

    QFileDialog fileDialog(this, "Specify image sequence file name", imageDir, QString::fromStdString(filter));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QDialog::Accepted) return;

    // Extract the path, and the root name, from the returned string.
    QStringList qsl = fileDialog.selectedFiles();
    if (qsl.isEmpty()) return;
    QString fileName = qsl[0];

    startAnimCapture(fileName.toStdString(), defaultSuffix);
}

void MainForm::startAnimCapture(string baseFile, string defaultSuffix)
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
        if (msgBox.exec() == QMessageBox::No) { return; }
    }

    // Turn on "image capture mode" in the current active visualizer
    _animationCapture = true;
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    _controlExec->EnableAnimationCapture(vizName, true, fpath);
    _capturingAnimationVizName = vizName;

    _captureEndImageAction->setEnabled(true);
    _imageSequenceMenu->setEnabled(false);
    _captureJpegSequenceAction->setEnabled(false);
    _capturePngSequenceAction->setEnabled(false);
    _captureTiffSequenceAction->setEnabled(false);
    _singleImageMenu->setEnabled(false);
    _captureSingleJpegAction->setEnabled(false);
    _captureSinglePngAction->setEnabled(false);
    _captureSingleTiffAction->setEnabled(false);
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

    _captureEndImageAction->setEnabled(false);

    _imageSequenceMenu->setEnabled(true);
    _captureJpegSequenceAction->setEnabled(true);
    _capturePngSequenceAction->setEnabled(true);
    _captureTiffSequenceAction->setEnabled(true);

    _singleImageMenu->setEnabled(true);
    _captureSingleJpegAction->setEnabled(true);
    _captureSinglePngAction->setEnabled(true);
    _captureSingleTiffAction->setEnabled(true);
}

string MainForm::_getDataSetName(string file, DatasetExistsAction existsAction)
{
    vector<string> names = _controlExec->GetDataNames();
    if (names.empty() || existsAction == AddNew) {
        return (makename(file));
    } else if (existsAction == ReplaceFirst) {
        return names[0];
    }

    string newSession = "New Dataset";

    QStringList items;
    items << tr(newSession.c_str());
    for (int i = 0; i < names.size(); i++) { items << tr(names[i].c_str()); }

    bool    ok;
    QString item = QInputDialog::getItem(this, tr("Load Data"), tr("Load as new dataset or replace existing"), items, 0, false, &ok);
    if (!ok || item.isEmpty()) return ("");

    string dataSetName = item.toStdString();

    if (dataSetName == newSession) { dataSetName = makename(file); }

    return (dataSetName);
}
