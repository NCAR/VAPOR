#pragma once

#include "PWidget.h"

class VRadioButtons;

//! \class PDatasetImportTypeSelector
//! Creates VRadioButtons synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PDatasetImportTypeSelector : public PWidget {
    Q_OBJECT

    VRadioButtons *_vRadioButtons;

public:
    PDatasetImportTypeSelector(std::vector<std::string>& types);

protected:
    void updateGUI() const override;

private slots:
    void radioButtonChecked();
};
