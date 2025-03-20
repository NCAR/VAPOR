#pragma once

#include "Updatable.h"
#include <QWidget>

class PImportDataWidget;
class MainForm;

class ImportTab : public QWidget, public Updatable {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    PImportDataWidget *_importer;

public:
    ImportTab(VAPoR::ControlExec *ce, MainForm *mf);
    void Update() override;

public slots:
    void DataImported();

signals:
    void dataImported();
};
