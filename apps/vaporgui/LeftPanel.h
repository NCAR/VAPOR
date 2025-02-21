#pragma once
#include <QTabWidget>
#include "Updatable.h"
#include "VaporFwd.h"
#include <common.h>

class ImportTab;
class TestPanel;
class MainForm;

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    ControlExec *_ce;
    MainForm *_mf;
    std::vector<Updatable *> _uTabs;
    ImportTab* _importTab;

public:
    LeftPanel(ControlExec *ce, MainForm* mf);
    void Update() override;
    void UpdateImportMenu();

private:
    void tabChanged(int i);
    void _goToRendererTab();
};
