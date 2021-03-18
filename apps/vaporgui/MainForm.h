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
#include <chrono>
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
class ProgressStatusBar;
class QTimer;

class VizWindow;
class VizWinMgr;
class VizSelectCombo;

class BannerGUI;
class Statistics;
class Plot;
class PythonVariables;
class ErrorReporter;
class ParamsWidgetDemo;
class AppSettingsMenu;

class MainForm : public QMainWindow {
    Q_OBJECT

public:
    MainForm(vector<QString> files, QApplication *app, QWidget *parent = 0);
    ~MainForm();

    int RenderAndExit(int start, int end, const std::string &baseFile, int width, int height);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static const QEvent::Type ParamsChangeEvent;
    static const QEvent::Type ParamsIntermediateChangeEvent;
    static const std::string  _documentationURL;

    QMdiArea *    _mdiArea;
    QApplication *_App;

    // Animation actions
    //
    QAction *_playForwardAction;
    QAction *_playBackwardAction;
    QAction *_pauseAction;

    // Undo/redo actions
    //
    QAction *      _editUndoAction;
    QAction *      _editRedoAction;
    QAction *      _appSettingsAction;
    QLineEdit *    _timeStepEdit;
    QIntValidator *_timeStepEditValidator;

    QComboBox *_alignViewCombo;
    QMenuBar * _main_Menubar;
    QMenu *    _File;
    QMenu *    _Edit;
    QMenu *    _Tools;
    QMenu *    _captureMenu;
    QMenu *    _helpMenu;
    QMenu *    _developerMenu;

    QToolBar *_vizToolBar;
    QToolBar *_animationToolBar;

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
    QAction *_webDocumentationAction;

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

    bool     _animationCapture = false;
    int      _progressSavedFB = -1;
    bool     _progressEnabled = false;
    bool     _needToReenableProgressAfterAnimation = false;
    QAction *_progressEnabledMenuItem = nullptr;

    ProgressStatusBar *                                _status = nullptr;
    std::chrono::time_point<std::chrono::system_clock> _progressLastUpdateTime;
    const QObject *                                    _disableUserInputForAllExcept = nullptr;
    bool                                               _insideMessedUpQtEventLoop = false;

    Statistics *        _stats;
    Plot *              _plot;
    PythonVariables *   _pythonVariables;
    AppSettingsMenu *   _appSettingsMenu;
    BannerGUI *         _banner;
    VizSelectCombo *    _windowSelector;
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
    bool _paramsEventQueued = false;

    ErrorReporter *_errRep;

    ParamsWidgetDemo *_paramsWidgetDemo = nullptr;

    // Zero out all member variables
    //
    void _initMembers();

    void _performSessionAutoSave();
    void _stateChangeCB();
    void _intermediateStateChangedCB();

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

    void showCitationReminder();

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

    enum DatasetExistsAction { Prompt, AddNew, ReplaceFirst };
    void         loadDataHelper(const std::vector<string> &files, string prompt, string filter, string format, bool multi, DatasetExistsAction existsAction = Prompt);
    void         _createCaptureMenu();
    void         _createToolsMenu();
    void         _createEditMenu();
    void         _createFileMenu();
    void         _createHelpMenu();
    void         _createDeveloperMenu();
    void         createMenus();
    void         hookupSignals();
    void         _createAnimationToolBar();
    void         _createVizToolBar();
    void         createToolBars();
    void         _createProgressWidget();
    void         _disableProgressWidget();
    virtual void sessionOpenHelper(string fileName, bool loadDatasets = true);

    template<class T> bool isDatasetValidFormat(const std::vector<std::string> &paths) const;
    bool                   determineDatasetFormat(const std::vector<std::string> &paths, std::string *fmt) const;

    bool isOpenGLContextActive() const;

    // Enable/Disable all the widgets that require data to be present
    //
    void enableWidgets(bool onOff);

    void enableAnimationWidgets(bool onOff);

    void _fileSaveHelper(string path);

    string _getDataSetName(string file, DatasetExistsAction existsAction = Prompt);

private slots:
    void _plotClosed();
    void _statsClosed();
    void sessionOpen(QString qfileName = "", bool loadDatasets = true);
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
    void selectAnimCatureOutput(string filter, string defaultSuffix);
    void startAnimCapture(string baseFile, string defaultSuffix = "tiff");
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

    void launchWebDocs() const;
    void setInteractiveRefLevel(int);
    void loadStartingPrefs();

    void setActiveEventRouter(string type);

    void _setProj4String(string proj4String);

    void _setAnimationOnOff(bool onOff);
    void _setAnimationDraw();
};
#endif    // MAINFORM_H
