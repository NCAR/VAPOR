#pragma once
#include <QTabWidget>
#include "Updatable.h"
#include "VaporFwd.h"
#include <common.h>

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    ControlExec *_ce;
    std::vector<Updatable *> _uTabs;

public:
    LeftPanel(ControlExec *ce);
    void Update() override;

private:
    void tabChanged(int i);
};
