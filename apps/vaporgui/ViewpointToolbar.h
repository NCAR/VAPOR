#pragma once
#include <QToolBar>
#include "common.h"
#include "Updatable.h"

class VizWinMgr;
class PVisualizerSelector;

class ViewpointToolbar : public QToolBar, public Updatable {
    Q_OBJECT
    ControlExec * const _ce;
    VizWinMgr * const _vizWinMgr;
    PVisualizerSelector *_visualizerSelector;

public:
    ViewpointToolbar(QWidget *parent, ControlExec *ce, VizWinMgr *vwm);
    void Update() override;
};
