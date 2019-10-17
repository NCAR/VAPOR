//************************************************************************
//									*
//			 Copyright (C)  2004				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MainForm.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2004
//
//	Description:  Definition of MainForm class
//		This QT Main Window class supports all main window functionality
//		including menus, tab dialog, docking, visualizer window,
//		and some of the communication between these classes.
//		There is only one of these, it is created by the main program.
//		Other classes can use getInstance() to obtain it
//
#ifndef MAINFORM_H
#define MAINFORM_H

#include "vapor/VAssert.h"
#include <qvariant.h>
#include <qmainwindow.h>
#include <qstring.h>
#include <QPaintEvent>
#include <QActionGroup>
#include <QLabel>
#include <QComboBox>
#include <QIcon>
#include <QLineEdit>
#include <QWidgetAction>
#include <vapor/ControlExecutive.h>
#include "GUIStateParams.h"
#include "SettingsParams.h"
#include "AnimationParams.h"
//#include "MiscParams.h"
#include "TabManager.h"

class QApplication;
class QSpacerItem;
class QMenu;
class QToolBar;
class QWidget;
class QDesktopWidget;
class QMdiArea;
class QDockWindow;
class QLabel;
class QSpinBox;

class VizWindow;
class VizWinMgr;
class VizSelectCombo;

class BannerGUI;
class Statistics;
class Plot;
class PythonVariables;
class ErrorReporter;

class MainForm : public QMainWindow {
    Q_OBJECT

public:
    MainForm(vector<QString> files, QApplication *app, QWidget *parent = 0);
    ~MainForm();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static const QEvent::Type ParamsChangeEvent;
    static const QEvent::Type ParamsIntermediateChangeEvent;

    QMdiArea *    _mdiArea;
    QApplication *_App;

    // Animation actions
    //
    QAction *_playForwardAction;
    QAction *_playBackwardAction;
    QAction *_pauseAction;

    // Undo/redo actions
    //
    QAction *      _navigationAction;
    QAction *      _editUndoAction;
    QAction *      _editRedoAction;
    QLineEdit *    _timeStepEdit;
    QIntValidator *_timeStepEditValidator;

    QComboBox *_alignViewCombo;
    QComboBox *_modeCombo;
    QMenuBar * _main_Menubar;
    QMenu *    _File;
    QMenu *    _Edit;
    QMenu *    _Tools;
    QMenu *    _captureMenu;
    QMenu *    _helpMenu;

    QToolBar *_modeToolBar;
    QToolBar *_vizToolBar;
    QToolBar *_animationToolBar;

    QMenu *_webTabHelpMenu;
    QMenu *_webBasicHelpMenu;
    QMenu *_webPreferencesHelpMenu;
    QMenu *_webPythonHelpMenu;
    QMenu *_webVisualizationHelpMenu;

    // Submenus under the File menu:
    //
    QMenu *_dataMenu;
    QMenu *_closeVDCMenu;
    QMenu *_importMenu;
    QMenu *_sessionMenu;

    // File menu:
    //
    QAction *_fileOpenAction;
    QAction *_fileSaveAction;
    QAction *_fileSaveAsAction;
    QAction *_fileExitAction;

    // Help menu
    //
    QAction *_helpAboutAction;
    QAction *_whatsThisAction;
    QAction *_installCLIToolsAction;

    // Data menu
    //
    QAction *_dataImportWRF_Action;
    QAction *_dataImportCF_Action;
    QAction *_dataImportMPAS_Action;
    QAction *_dataLoad_MetafileAction;
    QAction *_dataClose_MetafileAction;
    QAction *_fileNew_SessionAction;
    QAction *_plotAction;
    QAction *_statsAction;
    QAction *_pythonAction;

    // Capture menu
    //
    QMenu *  _singleImageMenu;
    QAction *_captureSingleJpegAction;
    QAction *_captureSinglePngAction;
    QAction *_captureSingleTiffAction;

    QMenu *  _imageSequenceMenu;
    QAction *_captureJpegSequenceAction;
    QAction *_capturePngSequenceAction;
    QAction *_captureTiffSequenceAction;

    QAction *_captureEndImageAction;

    // Toolbars:
    //
    QActionGroup *_mouseModeActions;
    QAction *     _tileAction;
    QAction *     _cascadeAction;
    QAction *     _homeAction;
    QAction *     _sethomeAction;
    QAction *     _viewAllAction;
    QAction *     _viewRegionAction;
    QAction *     _stepForwardAction;
    QAction *     _stepBackAction;
    QSpinBox *    _interactiveRefinementSpin;
    QDockWidget * _tabDockWindow;

    Statistics *        _stats;
    Plot *              _plot;
    PythonVariables *   _pythonVariables;
    BannerGUI *         _banner;
    VizSelectCombo *    _windowSelector;
    QLabel *            _modeStatusWidget;
    VAPoR::ControlExec *_controlExec;
    VAPoR::ParamsMgr *  _paramsMgr;
    TabManager *        _tabMgr;
    VizWinMgr *         _vizWinMgr;
    string              _capturingAnimationVizName;

    bool _stateChangeFlag;
    bool _sessionNewFlag;
    bool _begForCitation;
    int  _eventsSinceLastSave;
    bool _buttonPressed;

    ErrorReporter *_errRep;

    // Zero out all member variables
    //
    void _initMembers();

    void _performSessionAutoSave();
    void _stateChangeCB();

    QMdiArea *getMDIArea() { return _mdiArea; }

    QApplication *getApp() { return _App; }

    void            setInteractiveRefinementSpin(int);
    void            enableKeyframing(bool onoff);
    VizSelectCombo *getWindowSelector() { return _windowSelector; }

    // Set the animation buttons in pause state,
    // don't trigger an event:
    //
    void setPause()
    {
        _playForwardAction->setChecked(false);
        _playBackwardAction->setChecked(false);
        _pauseAction->setChecked(true);
    }

    int addMode(QString &text, QIcon &icon)
    {
        _modeCombo->addItem(icon, text);
        return (_modeCombo->count() - 1);
    }

    //! Insert all the mouse modes into the modeCombo.
    void addMouseModes();
    void setMouseMode(int newMode) { _modeCombo->setCurrentIndex(newMode); }
    void showCitationReminder();
    void buildWebTabHelpMenu(std::vector<QAction *> *actions);
    void buildWebTabHelpMenu(const vector<pair<string, string>> &help);
    void buildWebHelpMenus();

    void stopAnimCapture(string vizName)
    {
        if (vizName == _capturingAnimationVizName) endAnimCapture();
    }

    GUIStateParams *GetStateParams() const
    {
        VAssert(_paramsMgr != NULL);
        return ((GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType()));
    }

    SettingsParams *GetSettingsParams() const
    {
        VAssert(_paramsMgr != NULL);
        return ((SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType()));
    }

    AnimationParams *GetAnimationParams() const
    {
        VAssert(_paramsMgr != NULL);
        return ((AnimationParams *)_paramsMgr->GetParams(AnimationParams::GetClassType()));
    }

    /*MiscParams *GetMiscParams() const {
    VAssert(_paramsMgr != NULL);
    return ((MiscParams *)
        _paramsMgr->GetParams(MiscParams::GetClassType())
    );
 }*/

    // Set the various widgets in the main window consistent with latest
    // params settings:
    //
    void                updateMenus();
    void                update();
    virtual void        undoRedoHelper(bool undo);
    static bool         doesQStringContainNonASCIICharacter(const QString &s);
    static int          checkQStringContainsNonASCIICharacter(const QString &s);
    std::vector<string> myGetOpenFileNames(string prompt, string dir, string filter, bool multi);

    void closeDataHelper(string dataSetName);

    bool openDataHelper(string dataSetName, string format, const vector<string> &files, const vector<string> &options = vector<string>());

    void         loadDataHelper(const std::vector<string> &files, string prompt, string filter, string format, bool multi, bool promptToReplaceExistingDataset = true);
    void         _createCaptureMenu();
    void         _createToolsMenu();
    void         _createEditMenu();
    void         _createFileMenu();
    void         _createHelpMenu();
    void         createMenus();
    void         hookupSignals();
    void         _createModeToolBar();
    void         _createAnimationToolBar();
    void         _createVizToolBar();
    void         createToolBars();
    virtual void sessionOpenHelper(string fileName);

    // Enable/Disable all the widgets that require data to be present
    //
    void enableWidgets(bool onOff);

    void enableAnimationWidgets(bool onOff);

    void _fileSaveHelper(string path);

    string _getDataSetName(string file, bool promptToReplaceExistingDataset = true);

private slots:
    void sessionOpen(QString qfileName = "");
    void fileSave();
    void fileSaveAs();
    void fileExit();
    void undo();
    void redo();
    void helpAbout();
    void loadData(string fileName = "");
    void closeData(string fileName = "");
    void importWRFData();
    void importCFData();
    void importMPASData();
    void sessionNew();
    void captureJpegSequence();
    void captureTiffSequence();
    void capturePngSequence();
    void startAnimCapture(string filter, string defaultSuffix);
    void endAnimCapture();
    void captureSingleImage(string filter, string defaultSuffix);
    void captureSingleJpeg();
    void captureSinglePng();
    void captureSingleTiff();
    void installCLITools();
    void launchStats();
    void launchPlotUtility();
    void launchPythonVariables();

    // animation toolbar:
    void _setTimeStep();

    void launchWebHelp(QAction *);
    void modeChange(int);
    void setInteractiveRefLevel(int);
    void loadStartingPrefs();

    void setActiveEventRouter(string type);

    void _setProj4String(string proj4String);

    void _setAnimationOnOff(bool onOff);
    void _setAnimationDraw();
};
#endif    // MAINFORM_H
