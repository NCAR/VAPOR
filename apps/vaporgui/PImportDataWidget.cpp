#include "DatasetTypeLookup.h"
#include "DatasetImporter.h"
#include "PImportDataWidget.h"
#include "PImportDataButton.h"
#include "PRadioButtons.h"
#include "PGroup.h"

#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"

PImportDataWidget::PImportDataWidget(VAPoR::ControlExec* ce, DatasetImporter* di) : PGroup() {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::ImportDataTypeTag, types);
    Add(prb);
    
    Add(new PImportDataButton(ce, di));
}
