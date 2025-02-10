#pragma once

#include "Updatable.h"
#include <QWidget>

class PImportData;

class ImportPanel : public QWidget, public Updatable {
    Q_OBJECT

    VAPoR::ControlExec *_ce;

    PImportData *_importer;

public:
    ImportPanel(VAPoR::ControlExec *ce);
    void Update() override;
};
