//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		VizWinMgr.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Sept 2004
//
//	Description:  Implementation of VizWinMgr class
//		This class manages the VizWin visualizers
//		Its main function is to catch events from the visualizers and
//		to route them to the appropriate params class, and in reverse,
//		to route events from tab panels to the appropriate visualizer.
//
#ifdef WIN32
    #pragma warning(disable : 4251 4100)
#endif
#include <vapor/glutil.h>    // Must be included first!!!
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include "vapor/VAssert.h"
#include <qapplication.h>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include <vapor/DataStatus.h>
#include <vapor/STLUtils.h>
#include "QtVizWinGLContextManager.h"

#include <vapor/AnimationParams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/GUIStateParams.h>
#include <vapor/MouseModeParams.h>
#include <vapor/TrackBall.h>
#include "VizWin.h"
#include "VizWinMgr.h"
#include "ErrorReporter.h"
#include <common.h>

using namespace VAPoR;

VizWinMgr::VizWinMgr(ControlExec *ce)
: _controlExec(ce), visualizerGLContextManager(new QtVizWinGLContextManager(this))
{
    _mdiArea = this;
    _mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    _trackBall = new Trackball();

    _initialized = false;
}

VizWinMgr::~VizWinMgr()
{
    if (_trackBall) delete _trackBall;
}

class DisableFocusFilter: public QObject {
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::FocusIn)
            return true;
        return QObject::eventFilter(object, event);
    }
};

// All other tested methods for user-interaction window activation also fire from programmatic events
// - The original focus method had no way of truly distinguishing user actions.
//   Even when wrapping the operations in _safeMDIChange which disables focus signals, for some actions such as adding subwindows Qt will ignore this.
// - MDI active signals are also indistinguishable and unblockable for certain operations.
//   Also, while QMDISubWindow and QMDIArea both have a "window activated" signal, they behave a little differently, both have the same issues though.
// A potential solution would be to not have the active visualizer be a core part of the application state but this is currently deeply ingrained in the app.
class UserFocusEventFilter: public QObject {
    function<void(void)> _callback;
    static QWidget *_focusedWidget;
public:
    UserFocusEventFilter(const function<void(void)> &callback): _callback(callback) {}
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::FocusIn && ((QFocusEvent*)event)->reason() == Qt::MouseFocusReason)
            _callback();
        return QObject::eventFilter(object, event);
    }
};
void OnMDISubwindowUserFocus(QMdiSubWindow *w, const function<void(void)> &callback)
{
    w->widget()->setFocusPolicy(Qt::ClickFocus);
    w->widget()->installEventFilter(new UserFocusEventFilter(callback));
    w->installEventFilter(new UserFocusEventFilter(callback));
}

class CancelCloseEventFilter: public QObject {
    function<void(void)> _callback;
public:
    CancelCloseEventFilter(const function<void(void)> &callback): _callback(callback) {}
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::Close) {
            _callback();
            event->ignore();
            return true;
        }
        return QObject::eventFilter(object, event);
    }
};

void VizWinMgr::_attachVisualizer(string vizName)
{
    if (_vizWindow.find(vizName) != _vizWindow.end()) return;

    QString qvizname = QString::fromStdString(vizName);

    QGLFormat glFormat;
    glFormat.setVersion(4, 1);
    glFormat.setProfile(QGLFormat::CoreProfile);

    _vizWindow[vizName] = new VizWin(glFormat, this, qvizname, vizName, _controlExec, _trackBall);
    _vizWindow[vizName]->setWindowTitle(qvizname);
//    _vizWindow[vizName]->setAttribute(Qt::WA_ShowWithoutActivating); // Not honored in MDI view

    // Closing window will be handled by syncing with params
    _vizWindow[vizName]->installEventFilter(new CancelCloseEventFilter([this, vizName](){
        if (_controlExec->GetNumVisualizers() == 1)
            return;
        _controlExec->GetParamsMgr()->RemoveVisualizer(vizName);
    }));

    connect(_vizWindow[vizName], SIGNAL(EndNavigation(const string &)), this, SLOT(_syncViewpoints(const string &)));

    _vizMdiWin[vizName] = _mdiArea->addSubWindow(_vizWindow[vizName]);
    OnMDISubwindowUserFocus(_vizMdiWin[vizName], [this, vizName](){
        _setActiveViz(vizName);
    });

    GUIStateParams *p = _controlExec->GetParams<GUIStateParams>();
    string          prevActiveVizName = p->GetActiveVizName();

    _safeMDIChange([&](){ _vizWindow[vizName]->showMaximized(); });

    int numWins = _controlExec->GetNumVisualizers();
    // Tile if more than one visualizer:
    if (numWins > 1) FitSpace();

    // When we go from 1 to 2 windows, need to enable multiple
    // viz panels and signals.
    //
    if (numWins > 1) {
        emit enableMultiViz(true);
        _syncViewpoints(prevActiveVizName);
    }
}


void VizWinMgr::FitSpace() {
    _safeMDIChange([this](){
        _mdiArea->tileSubWindows();
    });
}


void VizWinMgr::_setActiveViz(string vizName)
{
    GUIStateParams *p = _controlExec->GetParams<GUIStateParams>();
    string          currentVizName = p->GetActiveVizName();
    if (currentVizName != vizName) {
        p->SetActiveVizName(vizName);
        emit(activateViz(QString::fromStdString(vizName)));

        if (vizName.empty()) return;

        // Need to cause a redraw in all windows if we are not in navigate mode,
        // So that the manips will change where they are drawn:

        if (p->GetMouseModeParams()->GetCurrentMouseMode() != MouseModeParams::GetNavigateModeName()) {
            map<string, VizWin *>::iterator it;
            for (it = _vizWindow.begin(); it != _vizWindow.end(); it++) { (it->second)->Render(false); }
        }
    }
}

void VizWinMgr::_syncViewpoints(string vizName)
{
    if (vizName.empty()) return;

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    bool enabled = paramsMgr->GetSaveStateEnabled();
    paramsMgr->SetSaveStateEnabled(false);

    ViewpointParams *currentVP = _controlExec->GetParamsMgr()->GetViewpointParams(vizName);

    vector<string> winNames = _getVisualizerNames();
    for (int i = 0; i < winNames.size(); i++) {
        if (winNames[i] != vizName) {
            ViewpointParams *vpParams = _controlExec->GetParamsMgr()->GetViewpointParams(winNames[i]);
            vpParams->SetModelViewMatrix(currentVP->GetModelViewMatrix());
            vpParams->SetRotationCenter(currentVP->GetRotationCenter());
        }
    }

    paramsMgr->SetSaveStateEnabled(enabled);
}

vector<string> VizWinMgr::_getVisualizerNames() const
{
    vector<string>                             names;
    std::map<string, VizWin *>::const_iterator itr = _vizWindow.begin();
    for (; itr != _vizWindow.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}


void VizWinMgr::_killViz(string vizName)
{
    VAssert(_vizWindow.find(vizName) != _vizWindow.end());

    _mdiArea->removeSubWindow(_vizMdiWin[vizName]);

    // This will trigger a closeEvent on VizWin, which will in turn
    // call vizAboutToDisappear
    //
    _vizWindow[vizName]->setEnabled(false);
    _vizWindow[vizName]->close();
}


void VizWinMgr::_removeVisualizer(const string &name)
{
    auto win = _vizWindow[name];
    win->makeCurrent();

    _controlExec->CleanupVisualizer(name, true);

    // disconnect all signals from window
    disconnect(win, 0, this, 0);

    // Remove the vizwin and the vizmdiwin
    _safeMDIChange([&](){ _mdiArea->removeSubWindow(_vizMdiWin[name]); });
    delete _vizWindow[name];
    _vizMdiWin.erase(name);
    _vizWindow.erase(name);
}

void VizWinMgr::_safeMDIChange(function<void(void)> f)
{
    // QMdiArea ignores attributes that disable focus signals launching by showing.
    // These conflict with active visualizer selection based on viz win selection.
    auto e = new DisableFocusFilter;
    for (auto &w : _vizWindow) {
        w.second->installEventFilter(e);
        w.second->paintOnResize = false;
    }

    f();

    for (auto &w : _vizWindow) {
        w.second->removeEventFilter(e);
        w.second->paintOnResize = true;
    }
}


void VizWinMgr::syncWithParams()
{
    vector<string> vizNames = _controlExec->GetParamsMgr()->GetVisualizerNames();

    vector<string> toRemove;
    for (auto it = _vizWindow.cbegin(); it != _vizWindow.cend(); ++it)
        if (!STLUtils::Contains(vizNames, it->first))
            toRemove.push_back(it->first);

    for (const auto &name : toRemove)
        _removeVisualizer(name);
    if (!toRemove.empty())
        FitSpace();

    for (int i = 0; i < vizNames.size(); i++) {
        if (_vizWindow.find(vizNames[i]) != _vizWindow.end()) continue;
        blockSignals(true);
        _attachVisualizer(vizNames[i]);
        blockSignals(false);
    }

    if (_controlExec->WasDataCacheDirty())
        setTrackballScale();
}

void VizWinMgr::Update(bool fast)
{
    // Certain actions queue multiple renders within Qt's event queue (e.g. changing the
    // renderer variable dimension). Normally, Qt will then process each of these events
    // sequentially and render multiple times. While not ideal, this just results in extra
    // renders and computation time. When using the progress bar, however, it sometimes
    // requests a redraw of the entire GUI at which point Qt processes queued events.
    // When you have pending render events, it will now try to initiate a new render
    // before the first has finished. This prevents that but it does not fix the
    // underlying issue.
    if (_insideRender) return;
    _insideRender = true;

    this->syncWithParams();

    map<string, VizWin *>::const_iterator it;
    for (it = _vizWindow.begin(); it != _vizWindow.end(); it++) { (it->second)->Render(fast); }

    _insideRender = false;
}

int VizWinMgr::EnableImageCapture(string filename, string winName)
{
    AnimationParams *ap = _controlExec->GetParams<AnimationParams>();
    //ap->SetValueString(AnimationParams::CaptureFileNameTag, filename);
    _vizWindow[winName]->makeCurrent();
    _vizWindow[winName]->_preRender();
    return _controlExec->EnableImageCapture(filename, winName);
    _vizWindow[winName]->_postRender();
}

void VizWinMgr::setTrackballScale()
{
    if (_controlExec->GetDataNames().size() == 0) return;

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    ParamsMgr * paramsMgr = _controlExec->GetParamsMgr();
    size_t      ts = _controlExec->GetParams<AnimationParams>()->GetCurrentTimestep();

    VAPoR::CoordType minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);

    double scale[3];
    scale[0] = scale[1] = scale[2] = max(maxExts[0] - minExts[0], (maxExts[1] - minExts[1]));
    _trackBall->SetScale(scale);
}
