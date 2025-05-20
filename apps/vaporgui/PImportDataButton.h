#pragma once

#include "PLineItem.h"

namespace VAPoR {
    class ControlExec;
}
class DatasetImporter;
class VHBoxWidget;
class VLabel;
class PButton;

class PImportDataButton : public PWidget {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    DatasetImporter *_di;
    VHBoxWidget *_hBox;
    VLabel *_fileLabel;
    PButton *_importButton;

    void _importDataset();

public:
    PImportDataButton(VAPoR::ControlExec* ce, DatasetImporter *di);

protected:
    void updateGUI() const override;
};
