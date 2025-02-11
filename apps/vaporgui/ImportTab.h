#pragma once

#include "Updatable.h"
#include <QWidget>

class PImportData;

class ImportTab : public QWidget, public Updatable {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    PImportData *_importer;

public:
    ImportTab(VAPoR::ControlExec *ce);
    void Update() override;

public slots:
    void DataImported();

signals:
    void dataImported();
};
