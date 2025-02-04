#pragma once

//#include "PWidget.h"
//#include "PGroup.h"
#include "PSection.h"

class VGroup;
class PGroup;
class QRadioButton;
class PFilesOpenSelector;
//class VAPoR::ControlExecutive;
//class ControlExecutive;
namespace VAPoR {
    class ControlExec;
}

//! \class PImportData
//! Creates VRadioButtons synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PImportData : public PWidget {
//class PImportData : public PGroup {
//class PImportData : public PSection {
    Q_OBJECT

    PGroup* _group;
    PFilesOpenSelector* _selector;
    void importDataset();
    VAPoR::ControlExec* _ce;

public:
    PImportData(VAPoR::ControlExec* ce);
    //void Update() override;

protected:
    void updateGUI() const override;

//private slots:
//    void radioButtonChecked(QRadioButton* rb);
};

//#include "PWidgetWrapper.h"
//class PImportDataSection : public PWidgetWrapper {
//public:
//    PImportDataSection();
//};
