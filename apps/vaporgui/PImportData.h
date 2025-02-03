#pragma once

#include "PWidget.h"

class VGroup;
class QRadioButton;
class PFileOpenSelector;

//! \class PImportData
//! Creates VRadioButtons synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PImportData : public PWidget {
    Q_OBJECT

    VGroup *_group;
    PFileOpenSelector* _selector;

public:
    PImportData();

protected:
    void updateGUI() const override;

private slots:
    void radioButtonChecked(QRadioButton* rb);
};

#include "PWidgetWrapper.h"
class PImportDataSection : public PWidgetWrapper {
public:
    PImportDataSection();
};
