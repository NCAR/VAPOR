#pragma once
#include <QTabWidget>
#include "Updatable.h"
#include "VaporFwd.h"
#include <common.h>

class ImportTab;
class MainForm;

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    ControlExec *_ce;
    std::vector<Updatable *> _uTabs;
    ImportTab* _importTab;

public:
    LeftPanel(ControlExec *ce, MainForm* mf);
    void Update() override;
    void GoToRendererTab();
    void ConfigureEnabledState(bool enabled);

private:
    void tabChanged(int i);
};
