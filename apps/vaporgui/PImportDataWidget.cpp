#include "DatasetTypeLookup.h"
#include "MainForm.h"
#include "PImportDataWidget.h"
#include "PImportDataLineItem.h"
#include "PRadioButtons.h"
#include "PGroup.h"

#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"

PImportDataWidget::PImportDataWidget(VAPoR::ControlExec* ce, MainForm* mf) : PWidget("", _group = new PGroup()), _ce(ce), _mf(mf) {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::ImportDataTypeTag, types);
    _group->Add(prb);
    
    _group->Add(new PImportDataLineItem(ce, mf));
}

void PImportDataWidget::updateGUI() const {
    _group->Update(getParams());
}
