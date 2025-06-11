#pragma once
#include <QTabWidget>
#include "Updatable.h"
#include "VaporFwd.h"
#include <common.h>

class ImportTab;

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    std::vector<Updatable *> _uTabs;
    ImportTab* _importTab;

public:
    LeftPanel(ControlExec *ce);
    void Update() override;
    void GoToRendererTab();
    void ConfigureEnabledState(bool enabled);

private:
    void tabChanged(int i);
};
