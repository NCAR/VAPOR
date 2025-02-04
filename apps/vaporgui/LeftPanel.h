#pragma once
#include <QTabWidget>
#include "Updatable.h"
#include "VaporFwd.h"
#include <common.h>

class ImportPanel;
class TestPanel;

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    ControlExec *_ce;
    std::vector<Updatable *> _uTabs;
    ImportPanel* _importPanel;
    TestPanel* _testPanel;

public:
    LeftPanel(ControlExec *ce);
    void Update() override;
    void UpdateImportPanel();

private:
    void tabChanged(int i);
};
