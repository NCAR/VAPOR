#include "DatasetTypeLookup.h"
#include "MainForm.h"
#include "PImportDataWidget.h"
#include "PImportDataButton.h"
#include "PRadioButtons.h"
#include "PGroup.h"

#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"

PImportDataWidget::PImportDataWidget(VAPoR::ControlExec* ce, MainForm* mf) : PGroup() {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::ImportDataTypeTag, types);
    Add(prb);
    
    Add(new PImportDataButton(ce, mf));
}
