#pragma once

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
#include <set>
#include <optional>
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
#include <vapor/SettingsParams.h>
#include <vapor/AnimationParams.h>
#include "AnimationController.h"
#include "PWidgetsFwd.h"
#include "QEnableable.h"

class QApplication;
class QSpacerItem;
class QMenu;
class QToolBar;
class QWidget;
class QMdiArea;
class QLabel;
class QSpinBox;
class ProgressStatusBar;
class QTimer;
class QDialog;

class VizWinMgr;
class Updatable;
class ParamsUpdatable;

class BannerGUI;
class Statistics;
class Plot;
class PythonVariables;
class VProjectionStringFrame;
class ErrorReporter;
class ParamsWidgetDemo;
class AppSettingsMenu;
class CaptureController;

class LeftPanel;

using namespace VAPoR;
using std::optional;

class MainForm : public QMainWindow {
    Q_OBJECT

public:
    MainForm(vector<QString> files, QApplication *app, bool interactive = true, string filesType = "auto", QWidget *parent = 0);
    ~MainForm();

    int RenderAndExit(int start, int end, const std::string &baseFile, int width, int height);
    static QWidget* Instance() { assert(_instance); return _instance; }

    // Needed for Export/Capture widget
    void AnimationPlayForward() const;

protected:
    void Render(bool fast=false, bool skipSync=false);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static const QEvent::Type ParamsChangeEvent;
    static const QEvent::Type ParamsIntermediateChangeEvent;

    QApplication *_App;
    static MainForm *_instance;

    QAction *_playForwardAction;
    QAction *_playBackwardAction;
    QAction *_pauseAction;

    QAction * _editUndoAction;
    QAction * _editRedoAction;
    PWidget * _timeStepEdit;

    QMenu *    _captureMenu;
    QMenu *    _developerMenu;

    QToolBar *_animationToolBar;

    QAction *_stepForwardAction;
    QAction *_stepBackAction;

    int      _progressSavedFB = -1;
    bool     _progressEnabled = false;
    QAction *_progressEnabledMenuItem = nullptr;

    ProgressStatusBar *                                _status = nullptr;
    std::chrono::time_point<std::chrono::system_clock> _progressLastUpdateTime;
    const QObject *                                    _disableUserInputForAllExcept = nullptr;
    bool                                               _insideMessedUpQtEventLoop = false;

    LeftPanel           *_leftPanel;
    Statistics *        _stats = nullptr;
    Plot *              _plot = nullptr;
    PythonVariables *   _pythonVariables = nullptr;
    VProjectionStringFrame* _projectionFrame = nullptr;
    AppSettingsMenu *   _appSettingsMenu = nullptr;
    BannerGUI *         _banner = nullptr;
    std::set<Updatable *> _updatableElements;
    std::set<ParamsUpdatable *> _guiStateParamsUpdatableElements;
    std::set<std::unique_ptr<QEnableableI>> _dependOnLoadedData;
    template<typename T> void _dependOnLoadedData_insert(T *o) { _dependOnLoadedData.insert(std::make_unique<QEnableable<T>>(o)); }

    VAPoR::ControlExec *_controlExec;
    VAPoR::ParamsMgr *  _paramsMgr;
    CaptureController *_captureController;
    AnimationController *_animationController;
    VizWinMgr *         _vizWinMgr;
    string              _capturingAnimationVizName;

    bool _stateChangeFlag = false;
    bool _sessionNewFlag = false;
    bool _begForCitation = false;
    int  _eventsSinceLastSave = 0;
    bool _paramsEventQueued = false;
    bool _widgetsEnabled;

    ParamsWidgetDemo *_paramsWidgetDemo = nullptr;

    void _performSessionAutoSave();
    void _stateChangeCB();
    void _intermediateStateChangedCB();

    QApplication *getApp() { return _App; }

    void setPause()
    {
        _playForwardAction->setChecked(false);
        _playBackwardAction->setChecked(false);
        _pauseAction->setChecked(true);
    }

    void showCitationReminder();

    GUIStateParams *GetStateParams() const { return ((GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType())); }
    SettingsParams *GetSettingsParams() const { return ((SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType())); }
    AnimationParams *GetAnimationParams() const { return ((AnimationParams *)_paramsMgr->GetParams(AnimationParams::GetClassType())); }

    void                updateMenus();
    void                updateUI();
    static bool         doesQStringContainNonASCIICharacter(const QString &s);
    static int          checkQStringContainsNonASCIICharacter(const QString &s);
    std::vector<string> getUserFileSelection(string prompt, string dir, string filter, bool multi);

    void showImportDatasetGUI(string format);
    void openSession(const string &path, bool loadData=true);
    void showOpenSessionGUI();
    optional<ControlExec::LoadStateRelAndAbsPathsExistAction> showSelectRelVAbsDataLoadGUI(const ControlExec::RelAndAbsPathsExistException &e);
    void checkSessionDatasetsExist();
    void         _createToolsMenu();
    void         _createEditMenu();
    void         _createFileMenu();
    void         _createHelpMenu();
    void         _createDeveloperMenu();
    void         createMenus();
    void         _createAnimationToolBar();
    void         createToolBars();
    void         _createProgressWidget();
    void         _disableProgressWidget();
    void         closeAllParamsDatasets();

    template<class T> bool isDatasetValidFormat(const std::vector<std::string> &paths) const;
    bool                   determineDatasetFormat(const std::vector<std::string> &paths, std::string *fmt) const;

    bool isOpenGLContextActive() const;
    void enableAnimationWidgets(bool onOff);
    void SaveSession();
    void SaveSession(string path);
    void SaveSessionAs();

private slots:
    void _plotClosed();
    void _statsClosed();
    void closeProjectionFrame();
    void helpAbout();
    void sessionNew();
    void launchStats();
    void launchPlotUtility();
    void launchPythonVariables();
    void launchProjectionFrame();
    void _setAnimationOnOff(bool onOff);
};
