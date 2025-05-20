#pragma once

#include "Updatable.h"
#include <QWidget>

class PImportDataWidget;
class PProjectionStringWidget;
class DatasetImporter;

class ImportTab : public QWidget, public Updatable {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    PImportDataWidget *_importer;
    PProjectionStringWidget *_projectionWidget;

public:
    ImportTab(VAPoR::ControlExec *ce, DatasetImporter *di);
    void Update() override;
};
