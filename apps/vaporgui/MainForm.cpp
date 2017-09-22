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
#define MIN_WINDOW_WIDTH 700
#define MIN_WINDOW_HEIGHT 700

#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif
#include <vapor/glutil.h>	// Must be included first!!!
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <iostream>
#include <vapor/Version.h>
#include <vapor/DataMgr.h>
#include <vapor/ControlExecutive.h>
#include <vapor/GetAppPath.h>
#include <vapor/CFuncs.h>

#include "VizWin.h"
#include "VizSelectCombo.h"
#include "TabManager.h"
#include "ViewpointEventRouter.h"
#include "regioneventrouter.h"
#include "VizFeatureEventRouter.h"
#include "AnimationEventRouter.h"
#include "MappingFrame.h"
#include "BannerGUI.h"
#include "SeedMe.h"
#include "Statistics.h"
#include "Plot.h"
#include "ErrorReporter.h"
#include "MainForm.h"


//Following shortcuts are provided:
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


//The following are pixmaps that are used in gui:
#include "images/vapor-icon-32.xpm"
#include "images/cascade.xpm"
#include "images/tiles.xpm"
#include "images/wheel.xpm"

//#include "images/planes.xpm"
//#include "images/lightbulb.xpm"
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
//Initialize the singleton to 0:
MainForm* MainForm::_mainForm = 0;
ControlExec * MainForm::_controlExec = NULL;

namespace {
bool make_dataset_name(
	const vector <string> &currentPaths,
	const vector <string> &currentNames,
	const string &newPath,
	string &dataSetName
	
) {
	dataSetName.clear();

	assert(currentPaths.size() == currentNames.size());

	string volume, dir, file;
	Wasp::Splitpath(newPath, volume, dir, file, false);

	Wasp::StrRmWhiteSpace(file);

	// Remove any file extension
	//
	if (file.find(".") != std::string::npos) {
		file.erase(file.find_last_of("."), string::npos);
	}

	for (int i=0; i<currentNames.size(); i++) {
		if (currentNames[i] == file) {
			if (currentPaths[i] == newPath) {
				//
				// path and data set name already exist
				dataSetName = file;
				return(false);
			}
			else {
				file += "1";
				dataSetName = file;
				return(true);
			}
		}
	}
	dataSetName = file;
	return(true);
}
};

//Only the main program should call the constructor:
//
MainForm::MainForm(
	vector<QString> files, QApplication* app, QWidget* parent, const char*)
    : QMainWindow( parent)
{
	QString fileName("");
	setAttribute(Qt::WA_DeleteOnClose);
	_mainForm = this;
	_vizWinMgr = 0;
	_App = app;
	_capturingAnimationVizName = "";
	_interactiveRefinementSpin = 0;
	_modeStatusWidget = 0;
	
	setWindowIcon(QPixmap(vapor_icon___));

    //insert my qmdiArea:
    _mdiArea = new QMdiArea;
    _mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(_mdiArea);

    _banner = NULL;
	_seedMe = NULL;
	_stats = NULL;
	_plot = NULL;	
	_paramsStateChange = true;
   

    createActions();
    createMenus();
    

    //Now add a docking tabbed window on the left side.
    _tabDockWindow = new QDockWidget(this );
    addDockWidget(Qt::LeftDockWidgetArea, _tabDockWindow );
	_tabDockWindow->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
	//setup the tab widget

	// Register additional params with the ParamsMgr
	//
	vector <string> myParams;
	myParams.push_back(GUIStateParams::GetClassType());
	myParams.push_back(AppSettingsParams::GetClassType());
	myParams.push_back(StartupParams::GetClassType());
	myParams.push_back(AnimationParams::GetClassType());
	myParams.push_back(MiscParams::GetClassType());
	myParams.push_back(StatisticsParams::GetClassType());
	myParams.push_back(PlotParams::GetClassType());
	
	// Create the Control executive before the VizWinMgr. Disable
	// state saving until completely initalized
	//
	_controlExec = new ControlExec(myParams);
	_controlExec->SetSaveStateEnabled(false);

	_paramsMgr = _controlExec->GetParamsMgr();
	_paramsMgr->RegisterStateChangeFlag(&_paramsStateChange);

	StartupParams *sP = GetStartupParams();
	_controlExec->SetCacheSize(sP->GetCacheMB());
	_controlExec->SetNumThreads(sP->GetNumExecutionThreads());

	//MappingFrame::SetControlExec(_controlExec);
	BoxSliderFrame::SetControlExec(_controlExec);
	
	_tabMgr = TabManager::Create(this, _controlExec);
    _tabMgr->setMaximumWidth(600);
    _tabMgr->setUsesScrollButtons(true);
    //This is just large enough to show the whole width of flow tab, with a scrollbar
    //on right (on windows, linux, and irix)  Using default settings of
    //qtconfig (10 pt font)
    _tabMgr->setMinimumWidth(460);
    _tabMgr->setMinimumHeight(500);


    _tabDockWindow->setWidget(_tabMgr);

	_vizWinMgr = VizWinMgr::Create(_controlExec);
	_vizWinMgr->createAllDefaultTabs();
	
	
	createToolBars();	
	
	addMouseModes();
    (void)statusBar();
    _main_Menubar->adjustSize();
    languageChange();
	hookupSignals();   
	enableWidgets(false);

	//Now that the tabmgr and the viz mgr exist, hook up the tabs:
	
	

	//Load preferences at start, set preferences directory
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
	}
	else {
		sessionNew();
	}

	if (files.size()) {

		// Assume VAPOR VDC file. Need to deal with import cases!
		//
		if (files[0].endsWith(".nc")){
			loadData(files[0].toStdString());
		}
	}
	app->installEventFilter(this);

	_controlExec->SetSaveStateEnabled(true);

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

void MainForm::createToolBars(){
    // mouse mode toolbar:
    _modeToolBar = addToolBar("Mouse Modes"); 
	_modeToolBar->setParent(this);
	_modeToolBar->addWidget(new QLabel(" Modes: "));
	QString qws = QString("The mouse modes are used to enable various manipulation tools ")+
		"that can be used to control the location and position of objects in "+
		"the 3D scene, by dragging box-handles in the scene. "+
		"Select the desired mode from the pull-down menu,"+
		"or revert to the default (Navigation) by clicking the button";
	_modeToolBar->setWhatsThis(qws);
	//add mode buttons, left to right:
	_modeToolBar->addAction(_navigationAction);
	
	_modeCombo = new QComboBox(_modeToolBar);
	_modeCombo->setToolTip("Select the mouse mode to use in the visualizer");
	
	_modeToolBar->addWidget(_modeCombo);
	

	
	// Animation Toolbar:
	//
	_animationToolBar = addToolBar("animation control");
	_timeStepEditValidator = new QIntValidator(0,99999,_animationToolBar);
	_timeStepEdit = new QLineEdit(_animationToolBar);
	_timeStepEdit->setAlignment(Qt::AlignHCenter);
	_timeStepEdit->setMaximumWidth(40);
	_timeStepEdit->setToolTip( "Edit/Display current time step");
	_timeStepEdit->setValidator(_timeStepEditValidator);
	_animationToolBar->addWidget(_timeStepEdit);
	
	_animationToolBar->addAction(playBackwardAction);
	_animationToolBar->addAction(_stepBackAction);
	_animationToolBar->addAction(pauseAction);
	_animationToolBar->addAction(_stepForwardAction);
	_animationToolBar->addAction(playForwardAction);
	
	QString qat = QString("The animation toolbar enables control of the time steps ")+
		"in the current active visualizer.  Additional controls are available in"+
		"the animation tab ";
	_animationToolBar->setWhatsThis(qat);
	
	// Viz tool bar:
	//
	_vizToolBar = addToolBar("");

	//Add a QComboBox to toolbar to select window
	_windowSelector = new VizSelectCombo(this);
	_vizToolBar->addWidget(_windowSelector);

	_vizToolBar->addAction(_tileAction);
	_vizToolBar->addAction(_cascadeAction);
	_vizToolBar->addAction(_homeAction);
	_vizToolBar->addAction(_sethomeAction);
	_vizToolBar->addAction(_viewRegionAction);
	_vizToolBar->addAction(_viewAllAction);


	alignViewCombo = new QComboBox(_vizToolBar);
	alignViewCombo->addItem("Align View");
	alignViewCombo->addItem("Nearest axis");
	alignViewCombo->addItem("     + X ");
	alignViewCombo->addItem("     + Y ");
	alignViewCombo->addItem("     + Z ");
	alignViewCombo->addItem("     - X ");
	alignViewCombo->addItem("     - Y ");
	alignViewCombo->addItem("Default: - Z ");
	alignViewCombo->setToolTip( "Rotate view to an axis-aligned viewpoint,\ncentered on current rotation center.");
	
	_vizToolBar->addWidget(alignViewCombo);
	_interactiveRefinementSpin = new QSpinBox(_vizToolBar);
	_interactiveRefinementSpin->setPrefix(" Interactive Refinement: ");
	_interactiveRefinementSpin->setMinimumWidth(230);
	
	_interactiveRefinementSpin->setToolTip(
		"Specify minimum refinement level during mouse motion,\nused in DVR and Iso rendering");
	_interactiveRefinementSpin->setWhatsThis(
		QString("While the viewpoint is changing due to mouse-dragging ")+
		"in a visualizer, the refinement level used by the DVR "+
		"and Iso renderers is reduced to this interactive refinement level, "+
		"if it is less than the current refinement level of the renderer.");
	_interactiveRefinementSpin->setMinimum(0);
	_interactiveRefinementSpin->setMaximum(10);
	
	_vizToolBar->addWidget(_interactiveRefinementSpin);
}

void MainForm::hookupSignals() {

	// Slots on the MainForm
	//
	AnimationEventRouter* aRouter = (AnimationEventRouter*)
        _vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	connect(
		aRouter, SIGNAL(AnimationOnOffChanged(bool)), 
		this, SLOT( setAnimationOnOff(bool))
	);

	connect(
		aRouter, SIGNAL(AnimationDraw()), 
		this, SLOT( setAnimationDraw())
	);

	connect(
		_modeCombo, SIGNAL(currentIndexChanged(int)), 
		this, SLOT( modeChange(int))
	);
	connect( 
		_fileNew_SessionAction, SIGNAL( triggered() ),
		this, SLOT( sessionNew() ) 
	);
	connect( 
		_fileOpenAction, SIGNAL( triggered() ),
		this, SLOT( sessionOpen() ) 
	);
	connect( 
		_fileSaveAction, SIGNAL( triggered() ),
		this, SLOT( fileSave() ) 
	);
	connect( 
		_fileSaveAsAction, SIGNAL( triggered() ),
		this, SLOT( fileSaveAs() ) 
	);
	connect( 
		_fileExitAction, SIGNAL( triggered() ),
		this, SLOT( fileExit() ) 
	);
	connect(
		_editUndoAction, SIGNAL(triggered()),
		this, SLOT (undo())
	);
	connect(
		_editRedoAction, SIGNAL(triggered()),
		this, SLOT (redo())
	);
	connect(
		_editUndoRedoClearAction, SIGNAL(triggered()),
		this, SLOT(clear())
	);
    connect( 
		_helpAboutAction, SIGNAL( triggered() ),
		this, SLOT( helpAbout() ) 
	);
	connect( 
		_dataLoad_MetafileAction, SIGNAL( triggered() ),
		this, SLOT( loadData() ) 
	);
	connect( 
		_dataClose_MetafileAction, SIGNAL( triggered() ),
		this, SLOT( closeData() ) 
	);
	connect( 
		_dataImportWRF_Action, SIGNAL( triggered() ),
		this, SLOT( importWRFData() ) 
	);
	connect( 
		_dataImportCF_Action, SIGNAL( triggered() ),
		this, SLOT( importCFData() ) 
	);
	connect(
		_captureMenu, SIGNAL(aboutToShow()),
		this, SLOT(initCaptureMenu())
	);
	connect(
		_Edit, SIGNAL(aboutToShow()),
		this, SLOT (setupEditMenu())
	);
	connect(
		_statsAction, SIGNAL(triggered()),
		this, SLOT(launchStats())
	);
	connect(
		_plotAction, SIGNAL(triggered()),
		this, SLOT(launchPlotUtility())
	);
	connect(
		_seedMeAction, SIGNAL(triggered()),
		this, SLOT(launchSeedMe())
	);
	connect(
		_installCLIToolsAction, SIGNAL(triggered()),
		this, SLOT(installCLITools())
	);
	connect (
		_captureSingleJpegCaptureAction, SIGNAL(triggered()),
		this, SLOT (captureSingleJpeg())
	);
	connect( 
		_captureStartJpegCaptureAction, SIGNAL( triggered() ),
		this, SLOT( startAnimCapture() ) 
	);
	connect( 
		_captureEndJpegCaptureAction, SIGNAL( triggered() ),
		this, SLOT( endAnimCapture() ) 
	);
	connect (
		_navigationAction, SIGNAL(toggled(bool)),
		this, SLOT(setNavigate(bool))
	);
	connect (
		_interactiveRefinementSpin, SIGNAL(valueChanged(int)),
		this, SLOT(setInteractiveRefLevel(int))
	);
	connect (
		playForwardAction, SIGNAL(triggered()),
		this, SLOT(playForward())
	);
	connect (
		playBackwardAction, SIGNAL(triggered()),
		this, SLOT(playBackward())
	);
	connect (
		pauseAction, SIGNAL(triggered()),
		this, SLOT(pauseClick())
	);
	connect (
		_stepForwardAction, SIGNAL(triggered()),
		this, SLOT(stepForward())
	);
	connect (
		_stepBackAction, SIGNAL(triggered()),
		this, SLOT(stepBack())
	);
	connect (
		_timeStepEdit, SIGNAL(returnPressed()),
		this, SLOT(setTimestep())
	);
	connect (
		_webTabHelpMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(launchWebHelp(QAction*))
	);
	connect (
		_webBasicHelpMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(launchWebHelp(QAction*))
	);
	connect (
		_webPythonHelpMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(launchWebHelp(QAction*))
	);
	connect (
		_webPreferencesHelpMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(launchWebHelp(QAction*))
	);
	connect (
		_webVisualizationHelpMenu, SIGNAL(triggered(QAction*)),
		this, SLOT(launchWebHelp(QAction*))
	);
	connect (
		_tabMgr, SIGNAL(ActiveEventRouterChanged(string)),
		this, SLOT(setActiveEventRouter(string))
	);

	// Slots on the VizWinMgr
	//
	connect (
		_tileAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(fitSpace())
	);
	connect (
		_cascadeAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(cascade())
	);
	connect (
		_homeAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(home())
	);
	connect (
		_sethomeAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(sethome())
	);
	connect (
		_viewAllAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(viewAll())
	);
	connect (
		_viewRegionAction, SIGNAL(triggered()),
		_vizWinMgr, SLOT(viewRegion())
	);
	connect (
		alignViewCombo, SIGNAL(activated(int)),
		_vizWinMgr, SLOT(alignView(int))
	);
	connect (
		_windowSelector, SIGNAL(winActivated(const QString &)),
		_vizWinMgr, SLOT(winActivated(const QString &))
	);
	connect (
		_vizWinMgr, SIGNAL(newViz(const QString &)),
		_windowSelector, SLOT(addWindow(const QString&))
	);
	connect (
		_vizWinMgr, SIGNAL(removeViz(const QString &)),
		_windowSelector, SLOT(removeWindow(const QString &))
	);
	connect (
		_vizWinMgr, SIGNAL(activateViz(const QString &)),
		_windowSelector, SLOT(setWindowActive(const QString &))
	);
	connect (
		_windowSelector, SIGNAL(newWin()),
		_vizWinMgr, SLOT(LaunchVisualizer())
	);
}

void MainForm::createMenus(){
    // menubar
    _main_Menubar = menuBar();
    _File = menuBar()->addMenu(tr("File"));
	_File->addAction(_dataLoad_MetafileAction );
	_File->addAction(_dataClose_MetafileAction );
    _File->addAction(_dataImportWRF_Action);
    _File->addAction(_dataImportCF_Action);
    _File->addAction(_fileNew_SessionAction);
    _File->addAction(_fileOpenAction);
    _File->addAction(_fileSaveAction);
    _File->addAction(_fileSaveAsAction);
    _File->addAction(_fileExitAction);

    _Edit = menuBar()->addMenu(tr("Edit"));
	_Edit->addAction(_editUndoAction);
	_Edit->addAction(_editRedoAction);
	_Edit->addAction(_editUndoRedoClearAction);
	_Edit->addSeparator();

	_Data = menuBar()->addMenu(tr("Data"));
	_Data->addAction(_plotAction);
	_Data->addAction(_statsAction);

	//Note that the ordering of the following 4 is significant, so that image
	//capture actions correctly activate each other.
    _captureMenu = menuBar()->addMenu(tr("Capture"));
	_captureMenu->addAction(_captureSingleJpegCaptureAction);
	_captureMenu->addAction(_captureStartJpegCaptureAction);
	_captureMenu->addAction(_captureEndJpegCaptureAction);
	_captureMenu->addAction(_seedMeAction);
	
	_main_Menubar->addSeparator();
    _helpMenu = menuBar()->addMenu(tr("Help"));
    _helpMenu->addAction(_whatsThisAction);
    _helpMenu->addSeparator();
    _helpMenu->addAction(_helpAboutAction );
	_webBasicHelpMenu = new QMenu("Web Help: Basic capabilities of VAPOR GUI",this);
	_helpMenu->addMenu(_webBasicHelpMenu);
	_webPreferencesHelpMenu = new QMenu("Web Help: User Preferences",this);
	_helpMenu->addMenu(_webPreferencesHelpMenu);
	_webPythonHelpMenu = new QMenu("Web Help: Derived Variables",this);
	_helpMenu->addMenu(_webPythonHelpMenu);
	_webVisualizationHelpMenu = new QMenu("Web Help: Visualization",this);
	_helpMenu->addMenu(_webVisualizationHelpMenu);
	buildWebHelpMenus();
	_webTabHelpMenu = new QMenu("Web Help: About the current tab",this);
	_helpMenu->addMenu(_webTabHelpMenu);
#ifdef Darwin
	_helpMenu->addAction(_installCLIToolsAction);
#endif
}

void MainForm::createActions(){
    // first do actions for menu bar:
    
    _fileOpenAction = new QAction( this);
	_fileOpenAction->setEnabled(true);
    _fileSaveAction = new QAction( this );
	_fileSaveAction->setEnabled(true);
    _fileSaveAsAction = new QAction( this );
	_fileSaveAsAction->setEnabled(true);
    _fileExitAction = new QAction( this);
	
	_editUndoAction = new QAction(this);
	_editRedoAction = new QAction(this);
	_editUndoRedoClearAction = new QAction(this);
	
	_editUndoAction->setEnabled(true);
	_editRedoAction->setEnabled(false);
	_editUndoRedoClearAction->setEnabled(true);
    
	_whatsThisAction = QWhatsThis::createAction(this);

    _helpAboutAction = new QAction( this );
	_helpAboutAction->setEnabled(true);
    
    _dataLoad_MetafileAction = new QAction( this);
	_dataClose_MetafileAction = new QAction( this );
	_dataImportWRF_Action = new QAction( this );
	_dataImportCF_Action = new QAction( this );
	_fileNew_SessionAction = new QAction( this );
    
	_captureSingleJpegCaptureAction = new QAction(this);
	_captureStartJpegCaptureAction = new QAction( this );
	_captureEndJpegCaptureAction = new QAction( this);

	_seedMeAction = new QAction(this);
	_seedMeAction->setEnabled(false);
	_plotAction = new QAction(this);
	_plotAction->setEnabled(false);
	_statsAction = new QAction(this);
	_statsAction->setEnabled(false);

	_installCLIToolsAction = new QAction(this);

	//Then do the actions for the toolbars:
	//Create an exclusive action group for the mouse mode toolbar:
	_mouseModeActions = new QActionGroup(this);
	//Toolbar buttons:
	QPixmap* wheelIcon = new QPixmap(wheel);
	
	_navigationAction = new QAction(*wheelIcon,"Navigation Mode",_mouseModeActions);
	_navigationAction->setCheckable(true);
	_navigationAction->setChecked(true);


	//Actions for the viztoolbar:
	QPixmap* homeIcon = new QPixmap(home);
	_homeAction = new QAction(*homeIcon,QString(tr("Go to Home Viewpoint  ")), this);
	_homeAction->setShortcut(QKeySequence(tr("Ctrl+H")));
	_homeAction->setShortcut(Qt::CTRL+Qt::Key_H);
	QPixmap* sethomeIcon = new QPixmap(sethome);
	_sethomeAction = new QAction(*sethomeIcon, "Set Home Viewpoint", this);
	QPixmap* eyeIcon = new QPixmap(eye);
	_viewAllAction = new QAction(*eyeIcon,QString(tr("View All  ")), this);
	_viewAllAction->setShortcut(QKeySequence(tr("Ctrl+V")));
	_viewAllAction->setShortcut(Qt::CTRL+Qt::Key_V);
	QPixmap* magnifyIcon = new QPixmap(magnify);
	_viewRegionAction = new QAction(*magnifyIcon, "View Region", this);

	QPixmap* tileIcon = new QPixmap(tiles);
	_tileAction = new QAction(*tileIcon,QString(tr("Tile Windows  ")),this);
	_tileAction->setShortcut(Qt::CTRL+Qt::Key_T);

	QPixmap* cascadeIcon = new QPixmap(cascade);
	_cascadeAction = new QAction(*cascadeIcon, "Cascade Windows", this);

	//Create actions for each animation control button:
	QPixmap* playForwardIcon = new QPixmap(playforward);
	playForwardAction = new QAction(*playForwardIcon,QString(tr("Play Forward  ")), this);
	playForwardAction->setShortcut(Qt::CTRL+Qt::Key_P);
	playForwardAction->setCheckable(true);
	QPixmap* playBackwardIcon = new QPixmap(playreverse);
	playBackwardAction = new QAction(*playBackwardIcon,QString(tr("Play Backward  ")),this);
	playBackwardAction->setShortcut(Qt::CTRL+Qt::Key_B);
	playBackwardAction->setCheckable(true);
	QPixmap* pauseIcon = new QPixmap(pauseimage);
	pauseAction = new QAction(*pauseIcon,QString(tr("End animation and unsteady flow integration  ")), this);
	pauseAction->setShortcut(Qt::CTRL+Qt::Key_E);
	//pauseAction->setCheckable(true);
	QPixmap* stepForwardIcon = new QPixmap(stepfwd);
	_stepForwardAction = new QAction(*stepForwardIcon,QString(tr("Step forward  ")), this);
	_stepForwardAction->setShortcut(Qt::CTRL+Qt::Key_F);
	QPixmap* stepBackIcon = new QPixmap(stepback);
	_stepBackAction = new QAction(*stepBackIcon,"Step back",this);
}

//
//  Sets the strings of the subwidgets using the current
//  language.
//
void MainForm::languageChange()
{
	setWindowTitle( tr( "VAPoR:  NCAR Visualization and Analysis Platform for Research" ) );

    _fileNew_SessionAction->setText( tr( "New Session" ) );
    
	_fileNew_SessionAction->setToolTip("Restart the session with default settings");
	_fileNew_SessionAction->setShortcut( Qt::CTRL + Qt::Key_N );
    
    _fileOpenAction->setText( tr( "&Open Session" ) );
    _fileOpenAction->setShortcut( tr( "Ctrl+O" ) );
	_fileOpenAction->setToolTip("Launch a file open dialog to reopen a previously saved session file");
    
    _fileSaveAction->setText( tr( "&Save Session" ) );
    _fileSaveAction->setShortcut( tr( "Ctrl+S" ) );
	_fileSaveAction->setToolTip("Launch a file-save dialog to save the state of this session in current session file");
    _fileSaveAsAction->setText( tr( "Save Session As" ) );
    
	_fileSaveAsAction->setToolTip("Launch a file-save dialog to save the state of this session in another session file");
 
    _fileExitAction->setText( tr( "E&xit" ) );
	_editUndoAction->setText( tr( "&Undo" ) );
    _editUndoAction->setShortcut( tr( "Ctrl+Z" ) );
	_editUndoAction->setToolTip("Undo the most recent session state change");
	_editRedoAction->setText( tr( "&Redo" ) );
    _editRedoAction->setShortcut( tr( "Ctrl+Y" ) );
	_editRedoAction->setToolTip("Redo the last undone session state change");
	_editUndoRedoClearAction->setText( tr( "&Clear undo/redo" ) );
	_editUndoRedoClearAction->setToolTip("Clear the undo/redo queue");
    
	
    _helpAboutAction->setText( tr( "About VAPOR" ) );
    _helpAboutAction->setToolTip( tr( "Information about VAPOR" ) );

	_whatsThisAction->setText( tr( "Explain This" ) );
	_whatsThisAction->setToolTip(tr("Click here, then click over an object for context-sensitive help. "));

	_installCLIToolsAction->setText("Install CLI Tools");
	_installCLIToolsAction->setToolTip("Add VAPOR_HOME to environment and add current utilities location to path. Needs to updated if app bundle moved");
	
    _dataLoad_MetafileAction->setText( tr( "Open a VDC in Current Session" ) );
	_dataLoad_MetafileAction->setToolTip("Specify a VDC data set to be loaded in current session");
	_dataLoad_MetafileAction->setShortcut(tr("Ctrl+D"));
    _dataClose_MetafileAction->setText( tr( "Close a VDC in Current Session" ) );
	_dataClose_MetafileAction->setToolTip("Specify a VDC data set to close in current session");
	_dataImportWRF_Action->setText(tr("Import WRF-ARW files in current session"));
	_dataImportWRF_Action->setToolTip("Specify one or more WRF-ARW output files to import into the current session");
	_dataImportCF_Action->setText(tr("Import NetCDF CF files in current session"));
	_dataImportCF_Action->setToolTip("Specify one or more NetCDF Climate Forecast (CF) convention output files to import into the current session");
	_plotAction->setText("Plot Utility");
	_statsAction->setText("Data Statistics");
	_seedMeAction->setText("SeedMe Video Encoder");
	_seedMeAction->setToolTip("Launch the SeedMe application to create videos of your still-frames");
 
	_captureSingleJpegCaptureAction->setText( tr( "Single image capture" ) );
	_captureSingleJpegCaptureAction->setToolTip("Capture one image from current active visualizer");
	_captureStartJpegCaptureAction->setText( tr( "Begin image capture sequence " ) );
	_captureStartJpegCaptureAction->setToolTip("Begin saving jpeg image files rendered in current active visualizer");
	_captureEndJpegCaptureAction->setText( tr( "End image capture" ) );
	_captureEndJpegCaptureAction->setToolTip("End capture of image files in current active visualizer");


    _vizToolBar->setWindowTitle( tr( "VizTools" ) );
	_modeToolBar->setWindowTitle( tr( "Mouse Modes" ) );
   
}

void MainForm::sessionOpenHelper(string fileName) {

	// Clear out the current session:

	endAnimCapture();
	enableWidgets(false);

	_vizWinMgr->Shutdown();

	GUIStateParams *p = GetStateParams();
	vector <string> currentPaths, currentDataSets;
	p->GetOpenDataSets(currentPaths, currentDataSets);
	for (int i=0; i<currentDataSets.size(); i++) {
		_controlExec->CloseData(currentDataSets[i]);
	}

	if (! fileName.empty()) {
		int rc = _controlExec->LoadState(fileName);
		if (rc < 0) {
			MSG_ERR("Failed to restore session from file");
			_controlExec->LoadState();
		}
	}
	else {
		_controlExec->LoadState();
	}


	// ControlExec::LoadState invalidates params state
	//
	GUIStateParams *newP = GetStateParams();
	newP->SetCurrentSessionPath(fileName);

}

// Open session file
//
void MainForm::sessionOpen(QString qfileName)
{

	// This launches a panel that enables the
    // user to choose input session save files, then to
	// load that session
	//
	if (qfileName==""){	
		GUIStateParams *p = GetStateParams();
		string path = p->GetCurrentSessionPath();

		vector <string> files = myGetOpenFileNames(
			"Choose a VAPOR session file to restore a session", 
			path, "Vapor 3 Session Save Files (*.vs3)", false
		);
		if (files.empty()) return;

		qfileName = files[0].c_str();
	}

	
	//Make sure the name ends with .vs3
	if (! qfileName.endsWith(".vs3")){
		return;
	}
	string fileName = qfileName.toStdString();

	sessionOpenHelper(fileName);

	_vizWinMgr->Restart();
}


void MainForm::fileSave()
{
	
	GUIStateParams *p = GetStateParams();
	string path = p->GetCurrentSessionPath();

	if (_controlExec->SaveSession(path) < 0){
		MSG_ERR("Saving session file");
		return;
	}
}


void MainForm::fileSaveAs()
{

	GUIStateParams *p = GetStateParams();
	string defaultPath = p->GetCurrentSessionPath();
	
   	QString fileName = QFileDialog::getSaveFileName(
		this, "Choose the fileName to save the current session",
		defaultPath.c_str(),
		"Vapor 3 Session Files (*.vs3)"
	);
	string path = fileName.toStdString();

	if (_controlExec->SaveSession(path)){
		MSG_ERR("Saving session file");
		return;
	}

	// Save to use a default for fileSave()
	//
	p->SetCurrentSessionPath(path);
}



void MainForm::fileExit()
{
	close();
}

#ifdef	DEAD
void MainForm::closeEvent(QCloseEvent* ){
	MessageReporter::SetFullSilence(true);
	delete _controlExec;
}
#endif

void MainForm::undoRedoHelper(bool undo) {

	// Disable state saving
	//
	bool enabled = _controlExec->GetSaveStateEnabled();
	_controlExec->SetSaveStateEnabled(false);

	bool status;
	_vizWinMgr->Shutdown();

	if (undo) {
		status = _controlExec->Undo();
	}
	else {
		status = _controlExec->Redo();
	}
	if (! status) {
		cerr << "UNDO/REDO FAILED\n";
		return;
	}

	_vizWinMgr->Restart();

	// Ugh. Trackball isn't integrated with Params database so need to 
	// handle undo/redo manually. I.e. get modelview matrix params from
	// database and set them in the TrackBall
	//
	vector <string> winNames = _paramsMgr->GetVisualizerNames();
	for (int i=0; i<winNames.size(); i++) {
		ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winNames[i]);
		double pos[3], dir[3], up[3], center[3];
		vpParams->GetCameraPos(pos);
		vpParams->GetCameraViewDir(dir);
		vpParams->GetCameraUpVec(up);
		vpParams->GetRotationCenter(center);

		_vizWinMgr->SetTrackBall(pos, dir, up, center, true);
	}

	// Needed?
	//
	_tabMgr->Update();

	// Restore state saving
	//
	_controlExec->SetSaveStateEnabled(enabled);

}

void MainForm::undo() {
	if (! _controlExec->UndoSize()) return;
	undoRedoHelper(true);

	if (! _controlExec->UndoSize()) {
		_editUndoAction->setEnabled(false);
	}
	_editRedoAction->setEnabled(true);
}

void MainForm::redo() {
	if (! _controlExec->RedoSize()) return;
	undoRedoHelper(false);

	if (! _controlExec->RedoSize()) {
		_editRedoAction->setEnabled(false);
	}
	_editUndoAction->setEnabled(true);
}

void MainForm::clear(){
	_controlExec->UndoRedoClear();
	//_editUndoAction->setEnabled(false);
	_editRedoAction->setEnabled(true);
}



void MainForm::helpIndex()
{

}


void MainForm::helpContents()
{

}


void MainForm::helpAbout()
{
	std::string banner_file_name = "vapor_banner.png";
    if(_banner) delete _banner;
	std::string banner_text = 
		"Visualization and Analysis Platform for atmospheric, Oceanic and "
		"solar Research.\n\n"
        "Developed by the National Center for Atmospheric Research's (NCAR) \n"
        "Computational and Information Systems Lab. \n\n"
		"Boulder, Colorado 80305, U.S.A.\n"
		"Web site: http://www.vapor.ucar.edu\n"
        "Contact: vapor@ucar.edu\n"
        "Version: " + string(Version::GetVersionString().c_str());

        _banner = new BannerGUI(
		banner_file_name, -1, true, banner_text.c_str(), 
		"http://www.vapor.ucar.edu"
	);
}
void MainForm::batchSetup(){
    //Here we provide panel to setup batch runs
}

void MainForm::loadDataHelper(
	vector <string> files, string prompt, string filter, string format,
	bool multi
) {
	GUIStateParams *p = GetStateParams();
	vector <string> currentPaths, currentDataSets;
	p->GetOpenDataSets(currentPaths, currentDataSets);

	// This launches a panel that enables the
    // user to choose input data files, then to
	// create a datamanager using those files
    // or metafiles.  
	//
	if (files.empty()) {

		string defaultPath = currentPaths.size() ? 
			currentPaths[currentPaths.size()-1] : ".";

		files = myGetOpenFileNames(
			prompt, defaultPath, filter, multi
		);
	}
	
	if (files.empty()) return;

	// Generate a new data set name if needed (not re-opening the same 
	// file)
	//
	string dataSetName;
	bool newDataSet = make_dataset_name(
		currentPaths, currentDataSets, files[0], dataSetName
	);

	int rc = _controlExec->OpenData(files, dataSetName, format);
	if (rc<0) {
		MSG_ERR("Failed to load data");
		return;
	}

	if (newDataSet) {
		currentPaths.push_back(files[0]);
		currentDataSets.push_back(dataSetName);
		p->SetOpenDataSets(currentPaths, currentDataSets);
	}

	// Reinitialize all tabs
	//
	
	_vizWinMgr->viewAll();

	DataStatus* ds = _controlExec->getDataStatus();
	BoxSliderFrame::setDataStatus(ds);

	_vizWinMgr->ReinitRouters();

	enableWidgets(true);

	_timeStepEditValidator->setRange(0,ds->GetTimeCoordinates().size()-1);

	update();
	_tabMgr->Update();
} 

//Load data into current session
//If current session is at default then same as loadDefaultData
//
void MainForm::loadData(string fileName)
{

	vector <string> files;
	if (! fileName.empty()) {
		files.push_back(fileName);
	}
		
	loadDataHelper(
		files, "Choose the Master data File to load", 
		"Vapor VDC files (*.*)", "vdc", false
	);

}

void MainForm::closeData(string fileName) {
	cout << "how do we close a dataset?" << endl;
}
	
//import WRF data into current session
//
void MainForm::importWRFData()
{

	vector <string> files;
	loadDataHelper(
		files, "WRF-ARW NetCDF files", "", "wrf", true
	);
	
}

void MainForm::importCFData()
{

	vector <string> files;
	loadDataHelper(
		files, "NetCDF CF files", "", "cf", true
	);
	
}

vector <string> MainForm::myGetOpenFileNames(
	string prompt, string dir, string filter, bool multi
) {

	QString qPrompt(prompt.c_str());
	QString qDir(dir.c_str());
	QString qFilter(filter.c_str());

	vector <string> files;
	if (multi) {
		QStringList fileNames = QFileDialog::getOpenFileNames(
			this, qPrompt, qDir, qFilter
		);
		QStringList list = fileNames;
		QStringList::Iterator it = list.begin();
		while(it != list.end()) {
			files.push_back((*it).toStdString());
			++it;
		}
	}
	else {
		QString fileName = QFileDialog::getOpenFileName(
			this, qPrompt, qDir, qFilter
		);
		files.push_back(fileName.toStdString());
	}

	for (int i=0; i<files.size(); i++) {
		QFileInfo fInfo(files[i].c_str());
		if (! fInfo.isReadable() || ! fInfo.isFile()){
			MyBase::SetErrMsg("Load Data Error \n Invalid data set\n");
			MSG_ERR("Failed to load data");
			return(vector <string> ());
		}
	}
	return(files);
}

void MainForm::sessionNew(){

	sessionOpenHelper("");

	_vizWinMgr->LaunchVisualizer();

	QString sessionPath = QDir::homePath();
	assert(! sessionPath.isEmpty());  
	sessionPath.append("/VaporSaved.vs3");
	sessionPath = QDir::toNativeSeparators(sessionPath);
	string fileName = sessionPath.toStdString();


	GUIStateParams *p = GetStateParams();
	p->SetCurrentSessionPath(fileName);
}
	
	

//
// Respond to toolbar clicks:
// navigate mode.  Don't change tab menu
//
void MainForm::setNavigate(bool on)
{
#ifdef	DEAD
	//Only do something if this is an actual change of mode
	if (MouseModeParams::GetCurrentMouseMode() == MouseModeParams::navigateMode) return;
	if (on){
		MouseModeParams::SetCurrentMouseMode(MouseModeParams::navigateMode);
		_modeCombo->setCurrentIndex(0);
		
		if(_modeStatusWidget) {
			statusBar()->removeWidget(_modeStatusWidget);
			delete _modeStatusWidget;
		}
		_modeStatusWidget = new QLabel("Navigation Mode:  Use left mouse to rotate or spin-animate, right to zoom, middle to translate",this);
		statusBar()->addWidget(_modeStatusWidget,2);
	}	
#endif
}


void MainForm::setupEditMenu(){
	
	QString undoText("Undo ");
	QString redoText("Redo ");
	QString clearText("Clear Undo/Redo ");
	
	_editUndoAction->setText( undoText );
	_editRedoAction->setText( redoText );
	_editUndoRedoClearAction->setText( clearText );
}

// Enable or disable the Capture menu options:
//
void MainForm::initCaptureMenu(){
	
	GUIStateParams *p = GetStateParams();
	string vizName = p->GetActiveVizName();

	_captureSingleJpegCaptureAction->setText("Capture single image");
	_captureStartJpegCaptureAction->setText( "&Begin image capture sequence in " + QString::fromStdString(vizName));
	_captureEndJpegCaptureAction->setText("End image capture sequence in " + QString::fromStdString(vizName));
	//Disable every option if no viz, or if capturing in another viz
	if (vizName.empty() || (! _capturingAnimationVizName.empty() &&_capturingAnimationVizName != vizName)) {
		_captureStartJpegCaptureAction->setEnabled(false);
		_captureSingleJpegCaptureAction->setEnabled(false);
		_captureEndJpegCaptureAction->setEnabled( false);
		MSG_WARN("Animation capture is in progress in another visualizer");
	} else if (_capturingAnimationVizName == vizName) {// there is a visualizer, and it's capturing images
		
		_captureStartJpegCaptureAction->setEnabled(false);
		_captureSingleJpegCaptureAction->setEnabled(false);
		_captureEndJpegCaptureAction->setEnabled(true);
	} else { //valid viz, not capturing:
		_captureStartJpegCaptureAction->setEnabled(true);
		_captureSingleJpegCaptureAction->setEnabled(true);
		_captureEndJpegCaptureAction->setEnabled(false);
	}
}




void MainForm::setInteractiveRefLevel(int val){
	
}
void MainForm::setInteractiveRefinementSpin(int val){
	
}
	
void MainForm::pauseClick(){
	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->AnimationPause();
	update();
}

void MainForm::playForward(){
	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->AnimationPlayForward();
	update();
}

void MainForm::playBackward(){
	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->AnimationPlayReverse();
	update();
}

void MainForm::stepBack(){
	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->AnimationStepReverse();
	update();
}	

void MainForm::stepForward(){
	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->AnimationStepForward();
	update();
}	

void MainForm::setAnimationOnOff(bool on) {

	if (on) {
		enableAnimationWidgets(false);

		_App->removeEventFilter(this);
	}
	else  {
		playForwardAction->setChecked(false);
		playBackwardAction->setChecked(false);
		enableAnimationWidgets(true);
		_App->installEventFilter(this);
	}
}

void MainForm::setAnimationDraw() {
	_vizWinMgr->updateDirtyWindows();
	update();
}

//Respond to a change in the text in the animation toolbar
void MainForm::setTimestep(){
	int timestep = _timeStepEdit->text().toInt();

	AnimationEventRouter* aRouter = (AnimationEventRouter*) 
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->SetTimeStep(timestep);
	update();
}

void MainForm::enableKeyframing(bool ison){
	QPalette pal(_timeStepEdit->palette());
	_timeStepEdit->setEnabled(!ison);
}


#ifdef	DEAD
	// Get the current mode setting from MouseModeParams
	//
	MouseModeParams *p = GetStateParams()->GetMouseModeParams();
	string mode = p->GetCurrentMouseMode();

	for (int i=0; i<_modeCombo->count(); i++) {
		if (_modeCombo->itemText(i).toStdString() == mode) { 
			_modeCombo->setCurrentIndex(i);

			if (mode != MouseModeParams::GetNavigateModeName()) {
				 _navigationAction->setChecked(false);
			}
			else {
				_navigationAction->setChecked(true);
			}
		}
		break;
	}
#endif

void MainForm::showTab(const std::string& tag){
	_tabMgr->MoveToFront(tag);
	EventRouter* eRouter = _vizWinMgr->GetEventRouter(tag);
	eRouter->updateTab();
}

void MainForm::modeChange(int newmode){
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
	
#ifdef	DEAD
	showTab(MouseModeParams::getModeTag(newmode));
#endif

	
	if(_modeStatusWidget) {
		statusBar()->removeWidget(_modeStatusWidget);
		delete _modeStatusWidget;
	}

	_modeStatusWidget = new QLabel(
		QString::fromStdString(modeName)+" Mode: To modify box in scene, grab handle with left mouse to translate, right mouse to stretch",this); 

	statusBar()->addWidget(_modeStatusWidget,2);
}

void MainForm::showCitationReminder(){
	//First check if reminder is turned off:
	AppSettingsParams* aParams = GetAppSettingsParams();
	if (!aParams->GetCurrentShowCitation()) return;
	//Provide a customized message box
	QMessageBox msgBox;
	QString reminder("VAPOR is developed as an Open Source application by the National Center for Atmospheric Research ");
	reminder.append("under the sponsorship of the National Science Foundation.  ");
	reminder.append("Continued support from VAPOR is dependent on demonstrable evidence of the software's value to the scientific community.  ");
	reminder.append("You are free to use VAPOR as permitted under the terms and conditions of the licence.\n\n ");
	reminder.append("We kindly request that you cite VAPOR in your publications and presentations. ");
	reminder.append("Citation details can be found on the VAPOR website at: \n\n  http://www.vapor.ucar.edu/index.php?id=citation");
	msgBox.setText(reminder);
	msgBox.setInformativeText("This reminder can be silenced from the User Preferences panel");
		
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
		
	msgBox.exec();
	aParams->SetCurrentShowCitation(false);

}
void MainForm::addMouseModes(){
	MouseModeParams *p = GetStateParams()->GetMouseModeParams();
	vector <string> mouseModes = p->GetAllMouseModes();

	for (int i = 0; i<mouseModes.size(); i++){
		QString text = QString::fromStdString(mouseModes[i]);
		const char* const * xpmIcon = p->GetIcon(mouseModes[i]);
		QPixmap qp = QPixmap(xpmIcon);
		QIcon icon = QIcon(qp);
		addMode(text,icon);
	}
}
void MainForm::launchWebHelp(QAction* webAction){
	QVariant qv = webAction->data();
	QUrl myURL = qv.toUrl();
	bool success = QDesktopServices::openUrl(myURL);
	if (!success){
		MSG_ERR("Unable to launch Web browser for URL");
	}
}

void MainForm::buildWebTabHelpMenu(vector<QAction*>* actions){
	_webTabHelpMenu->clear();
	for (int i = 0; i< (*actions).size(); i++){
		if ((*actions)[i] != NULL) {
			_webTabHelpMenu->addAction((*actions)[i]);
		}
	}
}

void MainForm::buildWebTabHelpMenu(
	const vector <pair <string, string> > &help
) {
	_webTabHelpMenu->clear();

	for (int i=0; i<help.size(); i++) {
		string desc = help[i].first;
		string url = help[i].second;

		QAction* currAction = new QAction(
			QString(desc.c_str()), _webTabHelpMenu
		);

		QUrl myqurl(url.c_str());
		QVariant qv(myqurl);
		currAction->setData(qv);

		_webTabHelpMenu->addAction(currAction);
	}
}

void MainForm::buildWebHelpMenus(){
	//Basci web help
	const char* currText = "VAPOR Overview";
	const char* currURL = "http://www.vapor.ucar.edu/docs/vapor-overview/vapor-overview";
	QAction* currAction = new QAction(QString(currText),this);
	QUrl myqurl(currURL);
	QVariant qv(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "VAPOR GUI General Guide";
	currURL = "http://www.vapor.ucar.edu/docs/vaporgui-help";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "Obtaining help within VAPOR GUI";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/help-user-interface";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "Loading and Importing data";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/loading-and-importing-data";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "Undo and Redo";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/undo-and-redo-recent-actions";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "Sessions and Session files";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/sessions-and-session-files";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	currText = "Capturing images and flow lines";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/capturing-images-and-flow-lines";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webBasicHelpMenu->addAction(currAction);
	//Preferences Help
	currText = "User Preferences Overview";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Data Cache size";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#dataCacheSize";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Specifying window size";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#lockWindow";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Control message popups and logging";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/user-preferences#MessageLoggingPopups";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Changing user default settings";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/changing-default-tabbed-panel-settings";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Specifying default directories for reading and writing files";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/default-directories";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	currText = "Rearrange or hide tabs";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-documentation/rearrange-or-hide-tabs";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPreferencesHelpMenu->addAction(currAction);
	//Derived variables menu
	currText = "Derived variables overview";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/derived-variables";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	currText = "Defining new derived variables in Python";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/define-variables-python";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	currText = "Specifying a Python startup script";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/specify-python-startup-script";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	currText = "VAPOR python modules";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/vapor-python-modules";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	currText = "Viewing python script output text";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/python-script-output-text";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	currText = "Using IDL to derive variables in VAPOR GUI";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/using-idl-vapor-gui";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webPythonHelpMenu->addAction(currAction);
	//Visualization help
	currText = "VAPOR data visualization overview";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/data-visualization";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Visualizers (visualization windows)";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/visualizers-visualization-windows";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Auxiliary content in the scene";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/auxiliary-geometry-scene";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Navigation in the 3D scene";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/navigation-3d-scene";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Controlling multiple visualizers";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/multiple-visualizers";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Visualizer features";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/visualizer-features";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Manipulators and mouse modes";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-how-guide/manipulators-and-mouse-modes";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
	currText = "Coordinate systems";
	currURL = "http://www.vapor.ucar.edu/docs/vapor-gui-help/coordinate-systems-user-grid-latlon";
	currAction = new QAction(QString(currText),this);
	myqurl.setUrl(currURL);
	qv.setValue(myqurl);
	currAction->setData(qv);
	_webVisualizationHelpMenu->addAction(currAction);
}

void MainForm::loadStartingPrefs(){
	string prefPath;
	string preffile =
#ifdef WIN32
		"\\.vapor3_prefs";
#else
		"/.vapor3_prefs";
#endif
	char* pPath = getenv("VAPOR3_PREFS_DIR");
	if (!pPath)
		pPath = getenv("HOME");
	if (!pPath){
		vector<string> tpath;
		tpath.push_back("examples");
		tpath.push_back(preffile);
		prefPath = GetAppPath("VAPOR","share",tpath);
	} else {
		prefPath = string(pPath) + preffile;
	}

	//Make this path the default at startup:
	//
	StartupParams *sP = GetStartupParams();
	sP->SetCurrentPrefsPath(prefPath);

#ifdef	DEAD
	_controlExec->RestorePreferences(prefPath);
#endif
		
}

void MainForm::setActiveEventRouter(string type) {

	EventRouter *eRouter = _vizWinMgr->GetEventRouter(type);
	if (! eRouter) return;

	// Set up help for active tab
	//
	vector <pair <string, string> >help;
	eRouter->GetWebHelp(help);

	buildWebTabHelpMenu(help);

	eRouter->updateTab();
}


bool MainForm::event(QEvent* e){

	return QWidget::event(e);

}

bool MainForm::eventFilter(QObject *obj, QEvent *event) {

	assert(_controlExec && _vizWinMgr);

	switch(event->type()) {
	case (QEvent::MouseButtonPress):
	case (QEvent::MouseButtonRelease):
//	case (QEvent::MouseMove):
	case (QEvent::KeyRelease):

	// Not sure why Paint is needed. Who generates it?
	//
	//case (QEvent::Paint):

		_vizWinMgr->updateDirtyWindows();

		// Only update the GUI if the Params state has changed
		//
		if (_paramsStateChange) {
			_vizWinMgr->UpdateRouters();
			_paramsStateChange = false;
		}

		//update();

	break;
	default:
#ifdef	DEAD
		cout << "UNHANDLED EVENT TYPE " << event->type() << endl;
#endif
	break;
	}

	// Pass event on to target
	return(false);
}

void MainForm::update() {

	assert(_controlExec);

	AnimationParams* aParams = GetAnimationParams();
	size_t timestep = aParams->GetCurrentTimestep();

	_timeStepEdit->setText(QString::number((int) timestep));

#ifdef	DEAD
	// Get the current mode setting from MouseModeParams
	//
	MouseModeParams *p = GetStateParams()->GetMouseModeParams();
	string mode = p->GetCurrentMouseMode();

	for (int i=0; i<_modeCombo->count(); i++) {
		if (_modeCombo->itemText(i).toStdString() == mode) { 
			_modeCombo->setCurrentIndex(i);

			if (mode != MouseModeParams::GetNavigateModeName()) {
				 _navigationAction->setChecked(false);
			}
			else {
				_navigationAction->setChecked(true);
			}
		}
		break;
	}
#endif

}

void MainForm::enableWidgets(bool onOff) {
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
	alignViewCombo->setEnabled(onOff);
	_navigationAction->setEnabled(onOff);
	_Edit->setEnabled(onOff);
	_windowSelector->setEnabled(onOff);
	_vizWinMgr->setEnabled(onOff);
	_tabMgr->setEnabled(onOff);
	_statsAction->setEnabled(onOff);
	_plotAction->setEnabled(onOff);

	AnimationEventRouter* aRouter = (AnimationEventRouter*)
		_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

	aRouter->setEnabled(onOff);

	RegionEventRouter* rRouter = (RegionEventRouter*)
		_vizWinMgr->GetEventRouter(RegionEventRouter::GetClassType());

	rRouter->setEnabled(onOff);

	ViewpointEventRouter* vRouter = (ViewpointEventRouter*)
		_vizWinMgr->GetEventRouter(ViewpointEventRouter::GetClassType());

	vRouter->setEnabled(onOff);

	VizFeatureEventRouter* vfRouter = (VizFeatureEventRouter*)
		_vizWinMgr->GetEventRouter(VizFeatureEventRouter::GetClassType());

	vfRouter->setEnabled(onOff);
}

void MainForm::enableAnimationWidgets(bool on) {

	enableWidgets(on);
	if (on) { 
		playBackwardAction->setEnabled(true);
		_stepBackAction->setEnabled(true);
		_stepForwardAction->setEnabled(true);
		playForwardAction->setEnabled(true);
	}
	else {
		AnimationEventRouter* aRouter = (AnimationEventRouter*)
			_vizWinMgr->GetEventRouter(AnimationEventRouter::GetClassType());

		_vizWinMgr->setEnabled(true);
		aRouter->setEnabled(true);

		_animationToolBar->setEnabled(true);
		playBackwardAction->setEnabled(false);
		_stepBackAction->setEnabled(false);
		_stepForwardAction->setEnabled(false);
		playForwardAction->setEnabled(false);
	}
}


//Capture just one image
//Launch a file save dialog to specify the names
//Then put jpeg in it.
//
void MainForm::captureSingleJpeg() {
	showCitationReminder();
	StartupParams *startupP = GetStartupParams();
	string imageDir = startupP->GetImageDir();

	QFileDialog fileDialog(this,
		"Specify single image capture file name",
		imageDir.c_str(),
		"Jpeg or Tiff images (*.jpg *.tif)");
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.move(pos());
	fileDialog.resize(450,450);
	if (fileDialog.exec() != QDialog::Accepted) return;
	
	//Extract the path, and the root name, from the returned string.
	QStringList files = fileDialog.selectedFiles();
	if (files.isEmpty()) return;
	QString fn = files[0];
	//Extract the path, and the root name, from the returned string.
	QFileInfo* fileInfo = new QFileInfo(fn);
	QString suffix = fileInfo->suffix();
	if (suffix != "jpg" && suffix != "tif" ) {
		MSG_ERR("Image capture file name must end with .jpg or .tif");
		return;
	}

	string filepath = fileInfo->absoluteFilePath().toStdString();

	//Save the path for future captures
	startupP->SetImageDir(fileInfo->absolutePath().toStdString());

	//Turn on "image capture mode" in the current active visualizer
	GUIStateParams *p = GetStateParams();
	string vizName = p->GetActiveVizName();
	_controlExec->EnableImageCapture(filepath, vizName);
	
}

void MainForm::launchSeedMe(){
    if (_seedMe==NULL) _seedMe = new SeedMe;
    _seedMe->Initialize();
}

void MainForm::installCLITools(){
	vector<string> pths;
	string home = GetAppPath("VAPOR","home", pths, true);
	string path = home + "/utilities";

	home.erase(home.size() - strlen("Contents/"), strlen("Contents/"));

	QMessageBox box;
	box.addButton(QMessageBox::Ok);

	string profilePath = string(getenv("HOME")) + "/.profile";
	FILE *prof = fopen(profilePath.c_str(), "a");
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

void MainForm::launchStats(){
    if (!_stats) _stats = new Statistics(this);
//	DataStatus* ds = _controlExec->getDataStatus();
//	string dm = ds->GetDataMgrNames()[0];
//	DataMgr *dataMgr = ds->GetDataMgr(dm);
//	if (dataMgr){
//      _stats->initDataMgr(dataMgr);
//        _stats->showMe();
//    }
	if (_controlExec) {
		_stats->initControlExec(_controlExec);
	} 
	_stats->showMe();   
}

void MainForm::launchPlotUtility(){
    if (! _plot) _plot = new Plot(this);
    _plot->Initialize(_controlExec, _vizWinMgr);
}


//Begin capturing animation images.
//Launch a file save dialog to specify the names
//Then start file saving mode.
void MainForm::startAnimCapture() {
	showCitationReminder();
	StartupParams *startupP = GetStartupParams();
	string imageDir = startupP->GetImageDir();
	QFileDialog fileDialog(this,
		"Specify first file name for image capture sequence",
		imageDir.c_str(),
		"Jpeg or Tiff images (*.jpg *.tif )");
	fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	fileDialog.move(pos());
	fileDialog.resize(450,450);
	if (fileDialog.exec() != QDialog::Accepted) return;
	
	//Extract the path, and the root name, from the returned string.
	QStringList qsl = fileDialog.selectedFiles();
	if (qsl.isEmpty()) return;
	QString s = qsl[0];
	QFileInfo* fileInfo = new QFileInfo(s);

	QString suffix = fileInfo->suffix();
	if (suffix != "jpg" && suffix != "tif" && suffix != "tiff") suffix = "jpg";
	if (suffix == "tiff") suffix = "tif";
	//Save the path for future captures
	startupP->SetImageDir(fileInfo->absolutePath().toStdString());

	QString fileBaseName = fileInfo->baseName();
	//See if it ends with digits.  If not, append them
	int posn;
	for (posn = fileBaseName.length()-1; posn >=0; posn--){
		if (!fileBaseName.at(posn).isDigit()) break;
	}
	int startFileNum = 0;
	unsigned int lastDigitPos = posn+1;
	if (lastDigitPos < fileBaseName.length()) {
		startFileNum = fileBaseName.right(fileBaseName.length()-lastDigitPos).toInt();
		fileBaseName.truncate(lastDigitPos);
	}
	QString filePath = fileInfo->absolutePath() + "/" + fileBaseName;
	//Insert up to 4 zeros
	QString zeroes;
	if (startFileNum == 0) zeroes = "000";
	else {
		switch((int)log10((float)startFileNum)) {
		case (0):
			zeroes = "000";
			break;
		case (1):
			zeroes = "00";
			break;
		case (2):
			zeroes = "0";
			break;
		default:
			zeroes = "";
			break;
		}
	}
	filePath += zeroes;
	filePath += QString::number(startFileNum);
	filePath += ".";
	filePath += suffix;
	string fpath = filePath.toStdString();
	//Turn on "image capture mode" in the current active visualizer
	GUIStateParams *p = GetStateParams();
	string vizName = p->GetActiveVizName();
	_controlExec->EnableAnimationCapture(vizName, true, fpath);
	_capturingAnimationVizName = vizName;
	delete fileInfo;
}
void MainForm::endAnimCapture(){
	//Turn off capture mode for the current active visualizer (if it is on!)
	if (_capturingAnimationVizName.empty() ) return;
	GUIStateParams *p = GetStateParams();
	string vizName = p->GetActiveVizName();
	if (vizName != _capturingAnimationVizName){
		MSG_WARN("Terminating capture in non-active visualizer");
	}
	if (_controlExec->EnableAnimationCapture(_capturingAnimationVizName, false))
		MSG_WARN("Image Capture Warning;\nCurrent active visualizer is not capturing images");
	
	_capturingAnimationVizName = "";


}
