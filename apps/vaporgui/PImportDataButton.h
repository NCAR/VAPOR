#pragma once

#include "PLineItem.h"

namespace VAPoR {
    class ControlExec;
}
class MainForm;
class VHBoxWidget;
class VLabel;
class PButton;

class PImportDataButton : public PWidget {
    Q_OBJECT

    VAPoR::ControlExec *_ce;
    MainForm *_mf;
    VHBoxWidget *_hBox;
    VLabel *_fileLabel;

    void _importDataset();

public:
    PImportDataButton(VAPoR::ControlExec* ce, MainForm *mf);

protected:
    void updateGUI() const override;
};
