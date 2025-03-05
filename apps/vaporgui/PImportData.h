#pragma once

#include "PSection.h"

namespace VAPoR {
    class ControlExec;
}
class VGroup;
class PGroup;
class QRadioButton;
class PFilesOpenSelector;
class MainForm;

//! \class PImportData
//! Creates VRadioButtons synced with the paramsdatabase using the PWidget interface.
//! Allows user to select a set of files to import.
//! \copydoc PWidget

class PImportData : public PWidget {
    Q_OBJECT

    PGroup*             _group;
    VSection*           _section;
    PFilesOpenSelector* _selector;
    VAPoR::ControlExec* _ce;
    MainForm* _mf;
    void importDataset();

public:
    PImportData(VAPoR::ControlExec* ce, MainForm *mf);

protected:
    void updateGUI() const override;

signals:
    void dataImported();

};
