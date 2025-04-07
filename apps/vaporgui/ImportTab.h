#pragma once

#include "Updatable.h"
#include <QWidget>

class PImportDataWidget;
class PProjectionStringWidget;
class MainForm;

class ImportTab : public QWidget, public Updatable {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    PImportDataWidget *_importer;
    PProjectionStringWidget *_projectionWidget;

public:
    ImportTab(VAPoR::ControlExec *ce, MainForm *mf);
    void Update() override;
};
