#include "DatasetTypeLookup.h"
#include "MainForm.h"
#include "PImportData.h"
#include "PFileSelector.h"
#include "PRadioButtons.h"
#include "PGroup.h"
#include "vapor/ControlExecutive.h"
#include "vapor/GUIStateParams.h"

PImportData::PImportData(VAPoR::ControlExec* ce, MainForm* mf) : PWidget("", _group = new PGroup()), _ce(ce), _mf(mf) {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::SelectedImportDataTypeTag, types);
    _group->Add(prb);
    
    _selector = new PFilesOpenSelector(GUIStateParams::SelectedImportDataFilesTag, "Import Files" );
    connect(_selector, &PFilesOpenSelector::filesSelected, this, &PImportData::importDataset);
    _group->Add(_selector);
}

void PImportData::importDataset() {
    std::vector<std::string> files = getParams()->GetValueStringVec(GUIStateParams::SelectedImportDataFilesTag);
    std::string              format = getParams()->GetValueString(GUIStateParams::SelectedImportDataTypeTag, "");
    format = DatasetTypeShortName(format);
    _mf->ImportDataset(files, format);
    emit dataImported();
}

void PImportData::updateGUI() const {
    _group->Update(getParams());
    _selector->Update(getParams());
}
