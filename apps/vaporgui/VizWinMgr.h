#pragma once

#include <vapor/ControlExecutive.h>
#include <QMdiArea>
#include <vapor/common.h>
#include <vapor/ParamsMgr.h>
#include "VaporFwd.h"

class VizWin;
class Trackball;
class QtVizWinGLContextManager;

class VizWinMgr : public QMdiArea {
    Q_OBJECT

    VAPoR::ControlExec *_controlExec;

public:
    VizWinMgr(VAPoR::ControlExec *ce);
    ~VizWinMgr();

    void Update(bool fast);

    void SetTrackBall(const double posvec[3], const double dirvec[3], const double upvec[3], const double centerRot[3], bool perspective);
    int EnableImageCapture(string filename, string winName);

    VizWin *Get(const std::string &name) { return _vizWindow[name]; }

    QtVizWinGLContextManager * const visualizerGLContextManager;

public slots:
    void FitSpace();

private slots:

    void _setActiveViz(string winName);
    void _syncViewpoints(string winName);

signals:
    void enableMultiViz(bool onOff);
    void activateViz(const QString &);

private:
    std::map<string, VizWin *>        _vizWindow;
    std::map<string, QMdiSubWindow *> _vizMdiWin;

    QMdiArea *_mdiArea;

    Trackball *         _trackBall;
    bool                _initialized;
    bool                _insideRender = false;

    void syncWithParams();

    void _attachVisualizer(string vizName);

    void _removeVisualizer(const string &name);
    void _safeMDIChange(function<void(void)> f);

    vector<string> _getVisualizerNames() const;
    void           _killViz(string vizName);
    void setTrackballScale();
};
