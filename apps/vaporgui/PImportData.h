#pragma once

//#include "PWidget.h"
//#include "PGroup.h"
#include "PSection.h"

namespace VAPoR {
    class ControlExec;
}
class VGroup;
class PGroup;
class QRadioButton;
class PFilesOpenSelector;

//! \class PImportData
//! Creates VRadioButtons synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PImportData : public PWidget {
//class PImportData : public PGroup {
//class PImportData : public PSection {
    Q_OBJECT

    PGroup*             _group;
    VSection*           _section;
    PFilesOpenSelector* _selector;
    VAPoR::ControlExec* _ce;
    void importDataset();

public:
    PImportData(VAPoR::ControlExec* ce);
    //void Update() override;

protected:
    void updateGUI() const override;

signals:
    void dataImported();

//private slots:
//    void radioButtonChecked(QRadioButton* rb);
};

//#include "PWidgetWrapper.h"
//class PImportDataSection : public PWidgetWrapper {
//public:
//    PImportDataSection();
//};
