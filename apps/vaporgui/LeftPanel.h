#pragma once

#include <QTabWidget>
#include <common.h>

#include "Updatable.h"
#include "VaporFwd.h"

class ImportTab;
class CaptureController;
class DatasetImportController;

class LeftPanel : public QTabWidget, public Updatable {
    Q_OBJECT
    std::vector<Updatable *> _uTabs;
    ImportTab* _importTab;

public:
    LeftPanel(ControlExec *ce, CaptureController *captureController, DatasetImportController *datasetImportController);
    void Update() override;
    void GoToRendererTab();
    void ConfigureEnabledState(bool enabled);

private:
    void tabChanged(int i);
};
