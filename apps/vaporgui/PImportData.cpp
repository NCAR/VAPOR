#include "PImportData.h"
#include "VGroup.h"
#include <vapor/ParamsBase.h>
#include <vapor/ParamsMgr.h>
#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <QRadioButton>
#include "VLineItem.h"
#include "VPushButton.h"
#include "DatasetTypeLookup.h"
#include "PButton.h"
#include "PFileSelector.h"
#include "PRadioButtons.h"
#include "PGroup.h"
#include "MainForm.h"
#include "vapor/ControlExecutive.h"
#include "vapor/FileUtils.h"
#include "vapor/NavigationUtils.h"

//PImportData::PImportData(VAPoR::ControlExec* ce) : PWidget("", _group = new PGroup()), _ce(ce) {
PImportData::PImportData(VAPoR::ControlExec* ce, MainForm* mf) : PWidget("", _group = new PGroup()), _ce(ce), _mf(mf) {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    PRadioButtons* prb = new PRadioButtons(GUIStateParams::SelectedImportDataTypeTag, types);
    _group->Add(prb);
    //for (auto type : types) {
    //    PRadioButton* rb = new PRadioButton(GUIStateParams::SelectedImportDataTypeTag, type);
    //    _group->Add(rb);
    //}
    
    _selector = new PFilesOpenSelector(GUIStateParams::SelectedImportDataFilesTag, "Import Files" );
    connect(_selector, &PFilesOpenSelector::filesSelected, this, &PImportData::importDataset);
    _group->Add(_selector);
}

// This is a wart.  We need a better/universal way to import data from both here as well as MainForm.
void PImportData::importDataset() {
    std::vector<std::string> files = getParams()->GetValueStringVec(GUIStateParams::SelectedImportDataFilesTag);
    std::string              format = getParams()->GetValueString(GUIStateParams::SelectedImportDataTypeTag, "");
    format = DatasetTypeShortName(format);
    _mf->importDataset(files, format);
    emit dataImported();
    return;
}

void PImportData::updateGUI() const {
    _group->Update(getParams());
    _selector->Update(getParams());
}

#include "PSection.h"
