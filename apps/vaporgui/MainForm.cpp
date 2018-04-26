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
#include <cassert>
#include <sstream>
#include <iostream>
#include <functional>
#include <cmath>
#include <QDesktopWidget>
#include <vapor/Version.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ControlExecutive.h>
#include <vapor/GetAppPath.h>
#include <vapor/CFuncs.h>

#include "VizWinMgr.h"
#include "VizSelectCombo.h"
#include "TabManager.h"
//#include "NavigationEventRouter.h"
//#include "AnnotationEventRouter.h"
//#include "AnimationEventRouter.h"
#include "MappingFrame.h"
#include "BannerGUI.h"
#include "SeedMe.h"
#include "Statistics.h"
#include "Plot.h"
#include "ErrorReporter.h"
#include "MainForm.h"
#include "FileOperationChecker.h"

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
#include "images/wheel.xpm"

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

QEvent::Type MainForm::ParamsChangeEvent::_customEventType = QEvent::None;

namespace {

string makename(string file)
{
    QFileInfo qFileInfo(QString(file.c_str()));

    return (ControlExec::MakeStringConformant(qFileInfo.fileName().toStdString()));
}

string concatpath(string s1, string s2)
{
    string s;
    if (!s1.empty()) {
        s = s1 + "/" + s2;
    } else {
        s = s2;
    }
    return (QDir::toNativeSeparators(s.c_str()).toStdString());
}
};    // namespace

void MainForm::_initMembers()
{
    _mdiArea = NULL;
    _App = NULL;

    _playForwardAction = NULL;
    _playBackwardAction = NULL;
    _pauseAction = NULL;

    _navigationAction = NULL;
    _editUndoAction = NULL;
    _editRedoAction = NULL;
    _editUndoRedoClearAction = NULL;
    _timeStepEdit = NULL;
    _timeStepEditValidator = NULL;

    _alignViewCombo = NULL;
    _modeCombo = NULL;
    _main_Menubar = NULL;
    _File = NULL;
    _Edit = NULL;
    _Tools = NULL;
    _captureMenu = NULL;
    _helpMenu = NULL;

    _modeToolBar = NULL;
    _vizToolBar = NULL;
    _animationToolBar = NULL;

    _webTabHelpMenu = NULL;
    _webBasicHelpMenu = NULL;
    _webPreferencesHelpMenu = NULL;
    _webPythonHelpMenu = NULL;
    _webVisualizationHelpMenu = NULL;

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

    _dataImportWRF_Action = NULL;
    _dataImportCF_Action = NULL;
    _dataImportMPAS_Action = NULL;
    _dataLoad_MetafileAction = NULL;
    _dataClose_MetafileAction = NULL;
    _plotAction = NULL;
    _statsAction = NULL;

    _captureStartJpegCaptureAction = NULL;
    _captureEndJpegCaptureAction = NULL;
    _captureSingleJpegCaptureAction = NULL;
    _seedMeAction = NULL;

    _mouseModeActions = NULL;
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

    _stats = NULL;
    _plot = NULL;
    SeedMe *_seedMe = NULL;
    _banner = NULL;
    _windowSelector = NULL;
    _modeStatusWidget = NULL;
    _controlExec = NULL;
    _paramsMgr = NULL;
    _tabMgr = NULL;
    _vizWinMgr = NULL;

    _capturingAnimationVizName.clear();

    _stateChangeFlag = false;
    _sessionNewFlag = false;
    _begForCitation = false;
    _eventsSinceLastSave = 0;
}

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
    QSize screenSize = QDesktopWidget().availableGeometry().size();
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
    _tabDockWindow->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

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
    ErrorReporter::GetInstance();

    // Create the Control executive before the VizWinMgr. Disable
    // state saving until completely initalized
    //
    _controlExec = new ControlExec(myParams, myRenParams);
    _controlExec->SetSaveStateEnabled(false);

    _paramsMgr = _controlExec->GetParamsMgr();
    _paramsMgr->RegisterStateChangeCB(std::bind(&MainForm::_stateChangeCB, this));
    _paramsMgr->RegisterStateChangeFlag(&_stateChangeFlag);

    // Set Defaults from startup file
    //
    SettingsParams *sP = GetSettingsParams();
    _controlExec->SetCacheSize(sP->GetCacheMB());
    _controlExec->SetNumThreads(sP->GetNumThreads());

    bool lockSize = sP->GetWinSizeLock();
    if (lockSize) {
        size_t width, height;
        sP->GetWinSize(width, height);
        setFixedSize(QSize(width, height));
    }

    _vizWinMgr = new VizWinMgr(this, _mdiArea, _controlExec);

    _tabMgr = new TabManager(this, _controlExec);
    _tabMgr->setMaximumWidth(600);
    _tabMgr->setUsesScrollButtons(true);

    _tabMgr->setMinimumWidth(460);
    _tabMgr->setMinimumHeight(500);

    _tabDockWindow->setWidget(_tabMgr);

    createMenus();

    createToolBars();

    addMouseModes();
    (void)statusBar();
    _main_Menubar->adjustSize();
    hookupSignals();
    enableWidgets(false);

    // Load preferences at start, set preferences directory
    loadStartingPrefs();

    setUpdatesEnabled(true);
    show();

    // Handle four initialization cases:
    //
    // 1. No files
    // 2. Session file
    // 3. Session file + data file(s)
    // 4. Data file(s)
    //
    if (files.size() && files[0].endsWith(".vs3")) {
        sessionOpen(files[0]);
        files.erase(files.begin());
    } else {
        sessionNew();
    }

    if (files.size()) {
        // Assume VAPOR VDC file. Need to deal with import cases!
        //
        if (files[0].endsWith(".nc")) { loadData(files[0].toStdString()); }
    }
    app->installEventFilter(this);

    _controlExec->SetSaveStateEnabled(true);
    _controlExec->RebaseStateSave();
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    if (_modeStatusWidget) delete _modeStatusWidget;
    if (_banner) delete _banner;
    if (_controlExec) delete _controlExec;

    // no need to delete child widgets, Qt does it all for us?? (see closeEvent)
}

void MainForm::_createModeToolBar()
{
    _mouseModeActions = new QActionGroup(this);
    QPixmap *wheelIcon = new QPixmap(wheel);

    _navigationAction = new QAction(*wheelIcon, "Navigation Mode", _mouseModeActions);

    _navigationAction->setCheckable(true);
    _navigationAction->setChecked(true);

    // mouse mode toolbar:
    //
    _modeToolBar = addToolBar("Mouse Modes");
    _modeToolBar->setWindowTitle(tr("Mouse Modes"));
    _modeToolBar->setParent(this);
    _modeToolBar->addWidget(new QLabel(" Modes: "));
    QString qws = QString("The mouse modes are used to enable various manipulation tools ") + "that can be used to control the location and position of objects in "
                + "the 3D scene, by dragging box-handles in the scene. " + "Select the desired mode from the pull-down menu," + "or revert to the default (Navigation) by clicking the button";

    _modeToolBar->setWhatsThis(qws);

    // add mode buttons, left to right:
    //
    _modeToolBar->addAction(_navigationAction);

    _modeCombo = new QComboBox(_modeToolBar);
    _modeCombo->setToolTip("Select the mouse mode to use in the visualizer");

    _modeToolBar->addWidget(_modeCombo);

    connect(_navigationAction, SIGNAL(toggled(bool)), this, SLOT(setNavigate(bool)));
    connect(_modeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChange(int)));
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
    _createModeToolBar();
    _createAnimationToolBar();
    _createVizToolBar();
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

    _editUndoRedoClearAction = new QAction(this);
    _editUndoRedoClearAction->setEnabled(true);
    _editUndoRedoClearAction->setText(tr("&Clear undo/redo"));
    _editUndoRedoClearAction->setToolTip("Clear the undo/redo queue");
    _editUndoRedoClearAction->setEnabled(true);

    _Edit = menuBar()->addMenu(tr("Edit"));
    _Edit->addAction(_editUndoAction);
    _Edit->addAction(_editRedoAction);
    _Edit->addAction(_editUndoRedoClearAction);
    _Edit->addSeparator();

    connect(_editUndoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(_editRedoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(_editUndoRedoClearAction, SIGNAL(triggered()), this, SLOT(clear()));
}

void MainForm::_createToolsMenu()
{
    _plotAction = new QAction(this);
    _plotAction->setText("Plot Utility");
    _plotAction->setEnabled(false);

    _statsAction = new QAction(this);
    _statsAction->setText("Data Statistics");
    _statsAction->setEnabled(false);

    _Tools = menuBar()->addMenu(tr("Tools"));
    _Tools->addAction(_plotAction);
    _Tools->addAction(_statsAction);

    connect(_statsAction, SIGNAL(triggered()), this, SLOT(launchStats()));
    connect(_plotAction, SIGNAL(triggered()), this, SLOT(launchPlotUtility()));
}

void MainForm::_createCaptureMenu()
{
    _captureSingleJpegCaptureAction = new QAction(this);
    _captureSingleJpegCaptureAction->setText(tr("Single image capture"));
    _captureSingleJpegCaptureAction->setToolTip("Capture one image from current active visualizer");

    _captureStartJpegCaptureAction = new QAction(this);
    _captureStartJpegCaptureAction->setText(tr("Begin image capture sequence "));
    _captureStartJpegCaptureAction->setToolTip("Begin saving jpeg image files rendered in current active visualizer");

    _captureEndJpegCaptureAction = new QAction(this);
    _captureEndJpegCaptureAction->setText(tr("End image capture"));
    _captureEndJpegCaptureAction->setToolTip("End capture of image files in current active visualizer");
    _captureEndJpegCaptureAction->setEnabled(false);

    _seedMeAction = new QAction(this);
    _seedMeAction->setText("SeedMe Video Encoder");
    _seedMeAction->setToolTip("Launch the SeedMe application to create videos of your still-frames");
    _seedMeAction->setEnabled(false);

    // Note that the ordering of the following 4 is significant, so that image
    // capture actions correctly activate each other.
    //
    _captureMenu = menuBar()->addMenu(tr("Capture"));
    _captureMenu->addAction(_captureSingleJpegCaptureAction);
    _captureMenu->addAction(_captureStartJpegCaptureAction);
    _captureMenu->addAction(_captureEndJpegCaptureAction);
    _captureMenu->addAction(_seedMeAction);

    connect(_captureSingleJpegCaptureAction, SIGNAL(triggered()), this, SLOT(captureSingleJpeg()));
    connect(_captureStartJpegCaptureAction, SIGNAL(triggered()), this, SLOT(startAnimCapture()));
    connect(_captureEndJpegCaptureAction, SIGNAL(triggered()), this, SLOT(endAnimCapture()));
    connect(_seedMeAction, SIGNAL(triggered()), this, SLOT(launchSeedMe()));
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

    _installCLIToolsAction = new QAction(this);
    _installCLIToolsAction->setText("Install CLI Tools");
    _installCLIToolsAction->setToolTip("Add VAPOR_HOME to environment and add current utilities "
                                       "location to path. Needs to updated if app bundle moved");

    _helpMenu = menuBar()->addMenu(tr("Help"));
    _helpMenu->addAction(_whatsThisAction);
    _helpMenu->addSeparator();
    _helpMenu->addAction(_helpAboutAction);
    _webBasicHelpMenu = new QMenu("Web Help: Basic capabilities of VAPOR GUI", this);
    _helpMenu->addMenu(_webBasicHelpMenu);
    _webPreferencesHelpMenu = new QMenu("Web Help: User Preferences", this);
    _helpMenu->addMenu(_webPreferencesHelpMenu);
    _webPythonHelpMenu = new QMenu("Web Help: Derived Variables", this);
    _helpMenu->addMenu(_webPythonHelpMenu);
    _webVisualizationHelpMenu = new QMenu("Web Help: Visualization", this);
    _helpMenu->addMenu(_webVisualizationHelpMenu);
    buildWebHelpMenus();
    _webTabHelpMenu = new QMenu("Web Help: About the current tab", this);
    _helpMenu->addMenu(_webTabHelpMenu);
#ifdef Darwin
    _helpMenu->addAction(_installCLIToolsAction);
#endif

    connect(_helpAboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(_webTabHelpMenu, SIGNAL(triggered(QAction *)), this, SLOT(launchWebHelp(QAction *)));
    connect(_webBasicHelpMenu, SIGNAL(triggered(QAction *)), this, SLOT(launchWebHelp(QAction *)));
    connect(_webPythonHelpMenu, SIGNAL(triggered(QAction *)), this, SLOT(launchWebHelp(QAction *)));
    connect(_webPreferencesHelpMenu, SIGNAL(triggered(QAction *)), this, SLOT(launchWebHelp(QAction *)));
    connect(_webVisualizationHelpMenu, SIGNAL(triggered(QAction *)), this, SLOT(launchWebHelp(QAction *)));

    connect(_installCLIToolsAction, SIGNAL(triggered()), this, SLOT(installCLITools()));
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
}

void MainForm::sessionOpenHelper(string fileName)
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

    for (int i = 0; i < dataSetNames.size(); i++) { newP->RemoveOpenDateSet(dataSetNames[i]); }

    _vizWinMgr->Restart();
    _tabMgr->Restart();
}

// Open session file
//
void MainForm::sessionOpen(QString qfileName)
{
    if (_stateChangeFlag) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        msgBox.setText("The current session settings are not saved. Do you want to continue? \nYou can choose \"No\" now to go back and save the current session.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return; }
    }

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

    string fileName = qfileName.toStdString();
    sessionOpenHelper(fileName);

    GUIStateParams *p = GetStateParams();
    p->SetCurrentSessionFile(fileName);

    _stateChangeFlag = false;
    _sessionNewFlag = false;
}

void MainForm::_fileSaveHelper(string path)
{
    if (path.empty()) {
        SettingsParams *sP = GetSettingsParams();
        string          dir = sP->GetSessionDir();

        QString fileName;
        fileName = QFileDialog::getSaveFileName(this, tr("Save VAPOR session file"), tr(dir.c_str()), "Vapor 3 Session Save Files (*.vs3)");
        path = fileName.toStdString();
    }
    if (path.empty()) return;

    if (!FileOperationChecker::FileGoodToWrite(path)) {
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
    // Generate an application event whenever state changes
    //
    ParamsChangeEvent *event = new ParamsChangeEvent();
    QApplication::postEvent(this, event);

    _eventsSinceLastSave++;
}

void MainForm::undoRedoHelper(bool undo)
{
    // Disable state saving
    //
    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    bool status;
    _vizWinMgr->Shutdown();
    _tabMgr->Shutdown();

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

    _vizWinMgr->Restart();
    _tabMgr->Restart();

    // Restore state saving
    //
    _controlExec->SetSaveStateEnabled(enabled);
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

void MainForm::clear() { _controlExec->UndoRedoClear(); }

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
                            + string(Version::GetVersionString().c_str());

    _banner = new BannerGUI(this, banner_file_name, -1, true, banner_text.c_str(), "http://www.vapor.ucar.edu");
}

// Close a data set and remove from database
//
void MainForm::closeDataHelper(string dataSetName)
{
    GUIStateParams *p = GetStateParams();

    p->RemoveOpenDateSet(dataSetName);

    _controlExec->CloseData(dataSetName);
}

// Open a data set and remove from database. If data with same name
// already exists close it first.
//
bool MainForm::openDataHelper(string dataSetName, string format, const vector<string> &files, const vector<string> &options)
{
    GUIStateParams *p = GetStateParams();
    vector<string>  dataSetNames = p->GetOpenDataSetNames();

#ifdef DEAD
    // If data set with this name already exists, close it
    //
    for (int i = 0; i < dataSetNames.size(); i++) {
        if (dataSetNames[i] == dataSetName) { closeDataHelper(dataSetName); }
    }
#endif

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

    return (true);
}

void MainForm::loadDataHelper(const vector<string> &files, string prompt, string filter, string format, bool multi)
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
    string dataSetName = _getDataSetName(myFiles[0]);
    if (dataSetName.empty()) return;

    vector<string> options = {"-project_to_pcs"};
    bool           status = openDataHelper(dataSetName, format, myFiles, options);
    if (!status) return;

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
}

// Load data into current session
// If current session is at default then same as loadDefaultData
//
void MainForm::loadData(string fileName)
{
    vector<string> files;
    if (!fileName.empty()) { files.push_back(fileName); }

    loadDataHelper(files, "Choose the Master data File to load", "Vapor VDC files (*.*)", "vdc", false);
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
}

// import WRF data into current session
//
void MainForm::importWRFData()
{
    vector<string> files;
    loadDataHelper(files, "WRF-ARW NetCDF files", "", "wrf", true);
}

void MainForm::importCFData()
{
    vector<string> files;
    loadDataHelper(files, "NetCDF CF files", "", "cf", true);
}

void MainForm::importMPASData()
{
    vector<string> files;
    loadDataHelper(files, "MPAS files", "", "mpas", true);
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
            if (!it->isNull()) files.push_back((*it).toStdString());
            ++it;
        }
    } else {
        QString fileName = QFileDialog::getOpenFileName(this, qPrompt, qDir, qFilter);
        if (!fileName.isNull()) files.push_back(fileName.toStdString());
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
    if (_stateChangeFlag) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        msgBox.setText("The current session settings are not saved. Do you want to continue? \nYou can choose \"No\" now to go back and save the current session.");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return; }
    }

    sessionOpenHelper("");

    _vizWinMgr->LaunchVisualizer();

    _stateChangeFlag = false;
    _sessionNewFlag = true;
}

//
// Respond to toolbar clicks:
// navigate mode.  Don't change tab menu
//
void MainForm::setNavigate(bool on)
{
#ifdef DEAD
    // Only do something if this is an actual change of mode
    if (MouseModeParams::GetCurrentMouseMode() == MouseModeParams::navigateMode) return;
    if (on) {
        MouseModeParams::SetCurrentMouseMode(MouseModeParams::navigateMode);
        _modeCombo->setCurrentIndex(0);

        if (_modeStatusWidget) {
            statusBar()->removeWidget(_modeStatusWidget);
            delete _modeStatusWidget;
        }
        _modeStatusWidget = new QLabel("Navigation Mode:  Use left mouse to rotate or spin-animate, right to zoom, middle to translate", this);
        statusBar()->addWidget(_modeStatusWidget, 2);
    }
#endif
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
        ParamsChangeEvent *event = new ParamsChangeEvent();
        QApplication::postEvent(this, event);
    }
}

void MainForm::_setAnimationDraw()
{
    _tabMgr->Update();
    _vizWinMgr->Update();
}

void MainForm::enableKeyframing(bool ison)
{
    QPalette pal(_timeStepEdit->palette());
    _timeStepEdit->setEnabled(!ison);
}

void MainForm::modeChange(int newmode)
{
    string modeName = _modeCombo->itemText(newmode).toStdString();

    // Get the current mode setting from MouseModeParams
    //
    MouseModeParams *p = GetStateParams()->GetMouseModeParams();
    p->SetCurrentMouseMode(modeName);

    if (modeName == MouseModeParams::GetNavigateModeName()) {
        _navigationAction->setChecked(true);
        return;
    }

    _navigationAction->setChecked(false);

    if (_modeStatusWidget) {
        statusBar()->removeWidget(_modeStatusWidget);
        delete _modeStatusWidget;
    }

    _modeStatusWidget = new QLabel(QString::fromStdString(modeName) + " Mode: To modify box in scene, grab handle with left mouse to translate, right mouse to stretch", this);

    statusBar()->addWidget(_modeStatusWidget, 2);
}

void MainForm::showCitationReminder()
{
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
void MainForm::addMouseModes()
{
    MouseModeParams *p = GetStateParams()->GetMouseModeParams();
    vector<string>   mouseModes = p->GetAllMouseModes();

    for (int i = 0; i < mouseModes.size(); i++) {
        QString            text = QString::fromStdString(mouseModes[i]);
        const char *const *xpmIcon = p->GetIcon(mouseModes[i]);
        QPixmap            qp = QPixmap(xpmIcon);
        QIcon              icon = QIcon(qp);
        addMode(text, icon);
    }
}
void MainForm::launchWebHelp(QAction *webAction)
{
    QVariant qv = webAction->data();
    QUrl     myURL = qv.toUrl();
    bool     success = QDesktopServices::openUrl(myURL);
    if (!success) { MSG_ERR("Unable to launch Web browser for URL"); }
}

void MainForm::buildWebTabHelpMenu(vector<QAction *> *actions)
{
    _webTabHelpMenu->clear();
    for (int i = 0; i < (*actions).size(); i++) {
        if ((*actions)[i] != NULL) { _webTabHelpMenu->addAction((*actions)[i]); }
    }
}

void MainForm::buildWebTabHelpMenu(const vector<pair<string, string>> &help)
{
    _webTabHelpMenu->clear();

    for (int i = 0; i < help.size(); i++) {
        string desc = help[i].first;
        string url = help[i].second;

        QAction *currAction = new QAction(QString(desc.c_str()), _webTabHelpMenu);

        QUrl     myqurl(url.c_str());
        QVariant qv(myqurl);
        currAction->setData(qv);

        _webTabHelpMenu->addAction(currAction);
    }
}

void MainForm::buildWebHelpMenus()
{
    // Basci web help
    const char *currText = "VAPOR Overview";
    const char *currURL = "http://www.vapor.ucar.edu/docs/vapor-overview/vapor-overview";
    QAction *   currAction = new QAction(QString(currText), this);
    QUrl        myqurl(currURL);
    QVariant    qv(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "VAPOR GUI General Guide";
    currURL = "http://www.vapor.ucar.edu/docs/vaporgui-help";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "Obtaining help within VAPOR GUI";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/help-user-interface";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "Loading and Importing data";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/loading-and-importing-data";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "Undo and Redo";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/undo-and-redo-recent-actions";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "Sessions and Session files";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/sessions-and-session-files";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    currText = "Capturing images and flow lines";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/capturing-images-and-flow-lines";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webBasicHelpMenu->addAction(currAction);
    // Preferences Help
    currText = "User Preferences Overview";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Data Cache size";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#dataCacheSize";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Specifying window size";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#lockWindow";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Control message popups and logging";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#MessageLoggingPopups";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Changing user default settings";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/changing-default-tabbed-panel-settings";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Specifying default directories for reading and writing files";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/default-directories";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    currText = "Rearrange or hide tabs";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/rearrange-or-hide-tabs";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPreferencesHelpMenu->addAction(currAction);
    // Derived variables menu
    currText = "Derived variables overview";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/derived-variables";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    currText = "Defining new derived variables in Python";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/define-variables-python";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    currText = "Specifying a Python startup script";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/specify-python-startup-script";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    currText = "VAPOR python modules";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/vapor-python-modules";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    currText = "Viewing python script output text";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/python-script-output-text";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    currText = "Using IDL to derive variables in VAPOR GUI";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/using-idl-vapor-gui";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webPythonHelpMenu->addAction(currAction);
    // Visualization help
    currText = "VAPOR data visualization overview";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/data-visualization";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Visualizers (visualization windows)";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/visualizers-visualization-windows";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Auxiliary content in the scene";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/auxiliary-geometry-scene";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Navigation in the 3D scene";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/navigation-3d-scene";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Controlling multiple visualizers";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/multiple-visualizers";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Visualizer features";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/visualizer-features";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Manipulators and mouse modes";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/manipulators-and-mouse-modes";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
    currText = "Coordinate systems";
    currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/coordinate-systems-user-grid-latlon";
    currAction = new QAction(QString(currText), this);
    myqurl.setUrl(currURL);
    qv.setValue(myqurl);
    currAction->setData(qv);
    _webVisualizationHelpMenu->addAction(currAction);
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
        prefPath = GetAppPath("VAPOR", "share", tpath);
    } else {
        prefPath = string(pPath) + preffile;
    }

    // Make this path the default at startup:
    //
    SettingsParams *sP = GetSettingsParams();
    sP->SetCurrentPrefsPath(prefPath);

#ifdef DEAD
    _controlExec->RestorePreferences(prefPath);
#endif
}

void MainForm::setActiveEventRouter(string type)
{
    // Set up help for active tab
    //
    vector<pair<string, string>> help;
    _tabMgr->GetWebHelp(type, help);

    buildWebTabHelpMenu(help);
}

void MainForm::_setProj4String(string proj4String)
{
    GUIStateParams *p = GetStateParams();

    DataStatus *ds = _controlExec->GetDataStatus();
    string      currentString = ds->GetMapProjection();

    if (proj4String == currentString) return;

    _App->removeEventFilter(this);

    vector<string> dataSets = p->GetOpenDataSetNames();

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

    vector<string> options = {"-project_to_pcs"};
    if (!proj4String.empty()) {
        options.push_back("-proj4");
        options.push_back(proj4String);
    };

    for (int i = 0; i < dataSets.size(); i++) { (void)openDataHelper(dataSets[i], formatsMap[i], filesMap[i], options); }

    _App->installEventFilter(this);
}

bool MainForm::event(QEvent *e) { return QWidget::event(e); }

bool MainForm::eventFilter(QObject *obj, QEvent *event)
{
    assert(_controlExec && _vizWinMgr);

    // Only update the GUI if the Params state has changed
    //
    if (event->type() == ParamsChangeEvent::type()) {
        ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        if (_stats) { _stats->Update(); }
        if (_plot) { _plot->Update(); }

        _tabMgr->Update();
        _vizWinMgr->Update();

        update();

        return (false);
    }

    // Most other events result in a redraw. Not sure if this
    // is necessary
    //
    switch (event->type()) {
    case (QEvent::MouseButtonPress):
    case (QEvent::MouseButtonRelease):
    case (QEvent::MouseMove):
        //	case (QEvent::KeyRelease):

        // Not sure why Paint is needed. Who generates it?
        //
        // case (QEvent::Paint):

        _vizWinMgr->Update();

        break;
    default: break;
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
        _paramsMgr->SaveToFile(autoSaveFile);
        _eventsSinceLastSave = 0;
    }
}

void MainForm::update()
{
    assert(_controlExec);

    AnimationParams *aParams = GetAnimationParams();
    size_t           timestep = aParams->GetCurrentTimestep();

    _timeStepEdit->setText(QString::number((int)timestep));

    updateMenus();

    _performSessionAutoSave();
}

void MainForm::enableWidgets(bool onOff)
{
    _modeCombo->setEnabled(onOff);
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
    _navigationAction->setEnabled(onOff);
    _Edit->setEnabled(onOff);
    _windowSelector->setEnabled(onOff);
    _tabMgr->setEnabled(onOff);
    _statsAction->setEnabled(onOff);
    _plotAction->setEnabled(onOff);
    //	_seedMeAction->setEnabled(onOff);

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

// Capture just one image
// Launch a file save dialog to specify the names
//
void MainForm::captureSingleJpeg()
{
    showCitationReminder();
    SettingsParams *sP = GetSettingsParams();
    string          imageDir = sP->GetImageDir();
    if (imageDir == "") imageDir = sP->GetDefaultImageDir();

    QFileDialog fileDialog(this, "Specify single image capture file name", imageDir.c_str(), "PNG or JPEG images (*.png *.jpg *.jpeg)");
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.move(pos());
    fileDialog.resize(450, 450);
    if (fileDialog.exec() != QDialog::Accepted) return;

    // Extract the path, and the root name, from the returned string.
    QStringList files = fileDialog.selectedFiles();
    if (files.isEmpty()) return;
    QString fn = files[0];
    // Extract the path, and the root name, from the returned string.
    QFileInfo *fileInfo = new QFileInfo(fn);
    QString    suffix = fileInfo->suffix();
    if (suffix != "jpg" && suffix != "jpeg" && suffix != "png") {
        MSG_ERR("Image capture file name must end with .png or .jpg or .jpeg");
        return;
    }

    string filepath = fileInfo->absoluteFilePath().toStdString();

    // Save the path for future captures
    sP->SetImageDir(filepath);

    // Turn on "image capture mode" in the current active visualizer
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    _controlExec->EnableImageCapture(filepath, vizName);

    delete fileInfo;
}

void MainForm::launchSeedMe()
{
    if (_seedMe == NULL) _seedMe = new VAPoR::SeedMe;
    _seedMe->Initialize();
}

void MainForm::installCLITools()
{
    vector<string> pths;
    string         home = GetAppPath("VAPOR", "home", pths, true);
    string         path = home + "/MacOS";

    home.erase(home.size() - strlen("Contents/"), strlen("Contents/"));

    QMessageBox box;
    box.addButton(QMessageBox::Ok);

    string profilePath = string(getenv("HOME")) + "/.profile";
    FILE * prof = fopen(profilePath.c_str(), "a");
    if (prof) {
        fprintf(prof, "\n");
        fprintf(prof, "export VAPOR_HOME=\"%s\"\n", home.c_str());
        fprintf(prof, "export PATH=\"%s:$PATH\"\n", path.c_str());
        fclose(prof);

        box.setText("Environmental variables set in ~/.profile");
        box.setInformativeText("Please log out and log back in for changes to take effect.");
        box.setIcon(QMessageBox::Information);
    } else {
        box.setText("Unable to set environmental variables");
        box.setIcon(QMessageBox::Critical);
    }
    box.exec();
}

void MainForm::launchStats()
{
    if (!_stats) _stats = new Statistics(this);
    if (_controlExec) { _stats->initControlExec(_controlExec); }
    _stats->showMe();
}

void MainForm::launchPlotUtility()
{
    if (!_plot) {
        assert(_controlExec->GetDataStatus());
        assert(_controlExec->GetParamsMgr());
        _plot = new Plot(_controlExec->GetDataStatus(), _controlExec->GetParamsMgr(), this);
    } else {
        _plot->show();
        _plot->activateWindow();
    }
}

// Begin capturing animation images.
// Launch a file save dialog to specify the names
// Then start file saving mode.
void MainForm::startAnimCapture()
{
    showCitationReminder();
    SettingsParams *sP = GetSettingsParams();
    string          imageDir = sP->GetImageDir();
    if (imageDir == "") imageDir = sP->GetDefaultImageDir();

    QFileDialog fileDialog(this, "Specify the base file name for image capture sequence", imageDir.c_str(), "PNG or JPEG images (*.png *.jpg *.jpeg )");
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.move(pos());
    fileDialog.resize(450, 450);
    if (fileDialog.exec() != QDialog::Accepted) return;

    // Extract the path, and the root name, from the returned string.
    QStringList qsl = fileDialog.selectedFiles();
    if (qsl.isEmpty()) return;
    QFileInfo *fileInfo = new QFileInfo(qsl[0]);

    QString suffix = fileInfo->suffix();
    if (suffix != "png" && suffix != "jpg" && suffix != "jpeg") {
        MSG_ERR("Image capture file name must end with .png or .jpg or .jpeg");
        return;
    }
    // Save the path for future captures
    sP->SetImageDir(fileInfo->absolutePath().toStdString());
    QString fileBaseName = fileInfo->baseName();

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

    QString filePath = fileInfo->absolutePath() + "/" + fileBaseName;
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
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    _controlExec->EnableAnimationCapture(vizName, true, fpath);
    _capturingAnimationVizName = vizName;

    _captureEndJpegCaptureAction->setEnabled(true);
    _captureStartJpegCaptureAction->setEnabled(false);
    _captureSingleJpegCaptureAction->setEnabled(false);

    delete fileInfo;
}

void MainForm::endAnimCapture()
{
    // Turn off capture mode for the current active visualizer (if it is on!)
    if (_capturingAnimationVizName.empty()) return;
    GUIStateParams *p = GetStateParams();
    string          vizName = p->GetActiveVizName();
    if (vizName != _capturingAnimationVizName) { MSG_WARN("Terminating capture in non-active visualizer"); }
    if (_controlExec->EnableAnimationCapture(_capturingAnimationVizName, false)) MSG_WARN("Image Capture Warning;\nCurrent active visualizer is not capturing images");

    _capturingAnimationVizName = "";

    _captureEndJpegCaptureAction->setEnabled(false);
    _captureStartJpegCaptureAction->setEnabled(true);
    _captureSingleJpegCaptureAction->setEnabled(true);
}

string MainForm::_getDataSetName(string file)
{
    vector<string> names = _controlExec->GetDataNames();
    if (names.empty()) { return (makename(file)); }

    string newSession = "New session";

    QStringList items;
    items << tr(newSession.c_str());
    for (int i = 0; i < names.size(); i++) { items << tr(names[i].c_str()); }

    bool    ok;
    QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Load data into session:"), items, 0, false, &ok);
    if (!ok || item.isEmpty()) return ("");

    string dataSetName = item.toStdString();

    if (dataSetName == newSession) { dataSetName = makename(file); }

    return (dataSetName);
}
