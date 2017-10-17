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

#include <cassert>
#include <qvariant.h>
#include <qmainwindow.h>
#include <qstring.h>
#include <QPaintEvent>
#include <QActionGroup>
#include <QLabel>
#include <QComboBox>
#include <QIcon>
#include <vapor/ControlExecutive.h>
#include "GUIStateParams.h"
#include "AppSettingsParams.h"
#include "StartupParams.h"
#include "AnimationParams.h"
#include "MiscParams.h"

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
class QLineEdit;

//class SeedMe;
class VizWindow;
class VizWinMgr;
class TabManager;
class VizSelectCombo;

class BannerGUI;
class Statistics;
class Plot;

namespace VAPoR {
class SeedMe;
}

class MainForm : public QMainWindow {
    Q_OBJECT

  public:
    //Note this is a singleton class.  Only the main program will create it.
    static MainForm *getInstance() {
        if (!_mainForm)
            assert(0);
        return _mainForm;
    }
    MainForm(vector<QString> files, QApplication *app, QWidget *parent = 0, const char *name = 0);
    ~MainForm();

    void showTab(const std::string &tag);
    QMdiArea *getMDIArea() { return _mdiArea; }

    QApplication *getApp() { return _App; }

    void setInteractiveRefinementSpin(int);
    void enableKeyframing(bool onoff);
    VizSelectCombo *getWindowSelector() { return _windowSelector; }

    //Following are public so accessible from animation tab:
    QAction *playForwardAction;
    QAction *playBackwardAction;
    QAction *pauseAction;
    QComboBox *alignViewCombo;
    //Set the animation buttons in pause state,
    //don't trigger an event:
    void setPause() {
        playForwardAction->setChecked(false);
        playBackwardAction->setChecked(false);
        pauseAction->setChecked(true);
    }
    int addMode(QString &text, QIcon &icon) {
        _modeCombo->addItem(icon, text);
        return (_modeCombo->count() - 1);
    }
    //! Insert all the mouse modes into the modeCombo.
    void addMouseModes();
    void setMouseMode(int newMode) { _modeCombo->setCurrentIndex(newMode); }
    void showCitationReminder();
    void buildWebTabHelpMenu(std::vector<QAction *> *actions);
    void buildWebTabHelpMenu(
        const vector<pair<string, string>> &help);
    void buildWebHelpMenus();

    bool event(QEvent *);
    void stopAnimCapture(string vizName) {
        if (vizName == _capturingAnimationVizName)
            endAnimCapture();
    }

    GUIStateParams *GetStateParams() const {
        assert(_paramsMgr != NULL);
        return ((GUIStateParams *)
                    _paramsMgr->GetParams(GUIStateParams::GetClassType()));
    }

    AppSettingsParams *GetAppSettingsParams() const {
        assert(_paramsMgr != NULL);
        return ((AppSettingsParams *)
                    _paramsMgr->GetParams(AppSettingsParams::GetClassType()));
    }

    StartupParams *GetStartupParams() const {
        assert(_paramsMgr != NULL);
        return ((StartupParams *)
                    _paramsMgr->GetParams(StartupParams::GetClassType()));
    }

    AnimationParams *GetAnimationParams() const {
        assert(_paramsMgr != NULL);
        return ((AnimationParams *)
                    _paramsMgr->GetParams(AnimationParams::GetClassType()));
    }

    MiscParams *GetMiscParams() const {
        assert(_paramsMgr != NULL);
        return ((MiscParams *)
                    _paramsMgr->GetParams(MiscParams::GetClassType()));
    }

  protected:
    bool eventFilter(QObject *obj, QEvent *event);

  private:
    class ParamsChangeEvent : public QEvent {
      public:
        ParamsChangeEvent() : QEvent(ParamsChangeEvent::type()) {}

        virtual ~ParamsChangeEvent() {}

        static QEvent::Type type() {
            if (_customEventType == QEvent::None) { // Register
                int generatedType = QEvent::registerEventType();
                _customEventType = static_cast<QEvent::Type>(generatedType);
            }
            return _customEventType;
        }

      private:
        static QEvent::Type _customEventType;
    };

    void _stateChangeCB();

    // Set the various widgets in the main window consistent with latest
    // params settings:
    //
    void update();
    virtual void undoRedoHelper(bool undo);
    std::vector<string> myGetOpenFileNames(
        string prompt, string dir, string filter, bool multi);
    void loadDataHelper(
        std::vector<string> files, string prompt, string filter, string format,
        bool multi);
    void createActions();
    void createMenus();
    void hookupSignals();
    void createToolBars();
    virtual void sessionOpenHelper(string fileName);

    // Enable/Disable all the widgets that require data to be present
    //
    void enableWidgets(bool onOff);

    void enableAnimationWidgets(bool onOff);

    //following are accessed during undo/redo
    QAction *_navigationAction;
    QAction *_editUndoAction;
    QAction *_editRedoAction;
    QAction *_editUndoRedoClearAction;
    QLineEdit *_timeStepEdit;
    QIntValidator *_timeStepEditValidator;

    QComboBox *_modeCombo;
    QMenuBar *_main_Menubar;
    QMenu *_File;
    QMenu *_Edit;
    QMenu *_Tools;
    QMenu *_captureMenu;
    QMenu *_helpMenu;
    QToolBar *_modeToolBar;
    QToolBar *_vizToolBar;
    QToolBar *_animationToolBar;
    QMenu *_webTabHelpMenu;
    QMenu *_webBasicHelpMenu;
    QMenu *_webPreferencesHelpMenu;
    QMenu *_webPythonHelpMenu;
    QMenu *_webVisualizationHelpMenu;

    // Submenus under the File menu:
    QMenu *_dataMenu;
    QMenu *_closeVDCMenu;
    QMenu *_importMenu;
    QMenu *_sessionMenu;

    //File menu:
    QAction *_fileOpenAction;
    QAction *_fileSaveAction;
    QAction *_fileSaveAsAction;
    QAction *_fileExitAction;

    //Help menu
    QAction *_helpContentsAction;
    QAction *_helpIndexAction;
    QAction *_helpAboutAction;
    QAction *_whatsThisAction;
    QAction *_installCLIToolsAction;

    //Data menu
    QAction *_dataImportWRF_Action;
    QAction *_dataImportCF_Action;
    QAction *_dataImportMPAS_Action;
    QAction *_dataLoad_MetafileAction;
    QAction *_dataClose_MetafileAction;
    QAction *_fileNew_SessionAction;
    QAction *_plotAction;
    QAction *_statsAction;

    // Capture menu
    QAction *_captureStartJpegCaptureAction;
    QAction *_captureEndJpegCaptureAction;
    QAction *_captureSingleJpegCaptureAction;
    QAction *_seedMeAction;

    //Toolbars:
    QActionGroup *_mouseModeActions;
    QAction *_tileAction;
    QAction *_cascadeAction;
    QAction *_homeAction;
    QAction *_sethomeAction;
    QAction *_viewAllAction;
    QAction *_viewRegionAction;
    QAction *_stepForwardAction;
    QAction *_stepBackAction;
    QSpinBox *_interactiveRefinementSpin;
    QDockWidget *_tabDockWindow;

    Statistics *_stats;
    Plot *_plot;
    VAPoR::SeedMe *_seedMe;
    BannerGUI *_banner;
    static MainForm *_mainForm;
    QMdiArea *_mdiArea;
    QApplication *_App;
    VizSelectCombo *_windowSelector;
    QLabel *_modeStatusWidget;
    static VAPoR::ControlExec *_controlExec;
    VAPoR::ParamsMgr *_paramsMgr;
    TabManager *_tabMgr;
    VizWinMgr *_vizWinMgr;
    string _capturingAnimationVizName;

  private slots:
    void sessionOpen(QString qfileName = "");
    void fileSave();
    void fileSaveAs();
    void fileExit();
    void undo();
    void redo();
    void clear();
    void helpIndex();
    void helpContents();
    void helpAbout();
    void loadData(string fileName = "");
    void closeData(string fileName = "");
    void importWRFData();
    void importCFData();
    void importMPASData();
    void sessionNew();
    void startAnimCapture();
    void endAnimCapture();
    void captureSingleJpeg();
    void launchSeedMe();
    void installCLITools();
    void launchStats();
    void launchPlotUtility();
    void batchSetup();

    //Set navigate mode
    void setNavigate(bool);

    //animation toolbar:
    void playForward();
    void playBackward();
    void pauseClick();
    void stepForward();
    void stepBack();
    void setAnimationOnOff(bool);
    void setAnimationDraw();
    void setTimestep();

    void launchWebHelp(QAction *);
    void modeChange(int);
    void languageChange();
    void initCaptureMenu();
    void setupEditMenu();
    void setInteractiveRefLevel(int);
    void loadStartingPrefs();

    void setActiveEventRouter(string type);

    // Change viewpoint to the current home viewpoint
    void goHome();

    // Move camera in or out to make entire volume visible
    void viewAll();

    // Set the current home viewpoint based on current viewpoint
    void setHome();

    // Align the camera to a specified axis
    // param[in] axis 1,2, or 3.
    void alignView(int axis);

    //! Move camera in or out to make current region visible
    void viewRegion();
};
#endif // MAINFORM_H
