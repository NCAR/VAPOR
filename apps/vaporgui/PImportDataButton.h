#pragma once

#include "PLineItem.h"

namespace VAPoR {
    class ControlExec;
}

class DatasetImportController;
class VHBoxWidget;
class VLabel;
class PButton;

class PImportDataButton : public PWidget {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    DatasetImportController *_datasetImportController;
    VHBoxWidget *_hBox;
    VLabel *_fileLabel;
    PButton *_importButton;

    void _importDataset();

public:
    PImportDataButton(VAPoR::ControlExec* ce, DatasetImportController *datasetImportController);

protected:
    void updateGUI() const override;
};
