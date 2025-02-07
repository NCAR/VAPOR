#include "PImportData.h"
//#include "VRadioButtons.h"
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
#include "PRadioButton.h"
#include "PGroup.h"
#include "vapor/ControlExecutive.h"
#include "vapor/FileUtils.h"

//PImportData::PImportData() : PWidget("", _group = new PGroup()) {
//PImportData::PImportData() : PGroup() {
//PImportData::PImportData(VAPoR::ControlExec* ce) : PSection("Import Files"), _ce(ce) {
PImportData::PImportData(VAPoR::ControlExec* ce) : PWidget("", _group = new PGroup()), _ce(ce) {
    AlwaysEnable();
    _group->AlwaysEnable();
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    for (auto type : types) {
        PRadioButton* rb = new PRadioButton(GUIStateParams::SelectedImportDataTypeTag, type);
        _group->Add(rb);
        auto gsp = _ce->GetParams<GUIStateParams>();
        rb->AlwaysEnable();
    }
    
    _selector = new PFilesOpenSelector(GUIStateParams::SelectedImportDataFilesTag, "Import Files" );
    _selector->AlwaysEnable();
    connect(_selector, &PFilesOpenSelector::filesSelected, this, &PImportData::importDataset);
    _group->Add(_selector);

    //ParamsMgr* pm = _ce->GetParamsMgr();
    //auto gsp = _ce->GetParams<GUIStateParams>();
    //Update(gsp);
}

//void PImportData::Update() {
//    _selector->Update(getParams(), getParamsMgr(), getDataMgr);    
//}

// This is a wart.  We need a better/universal way to import data from both here as well as MainForm.
void PImportData::importDataset() {
    ParamsMgr* pm = _ce->GetParamsMgr();
    pm->BeginSaveStateGroup("Import Dataset");
    //if (name.empty()) name = _getDataSetName(files[0], existsAction);
    std::vector<std::string> files = getParams()->GetValueStringVec(GUIStateParams::SelectedImportDataFilesTag);
    std::string name = FileUtils::Basename(files[0]);
    std::string format = getParams()->GetValueString(GUIStateParams::SelectedImportDataTypeTag, "");
    format = DatasetTypeShortName(format);
    std::cout << "Open data " << files[0] << " " << name << " " << format << std::endl;
    int rc = _ce->OpenData(files, name, format);
    if (rc < 0) {
        pm->EndSaveStateGroup();
        //MSG_ERR("Failed to load data");
        std::cout << "Failed to load data" << std::endl;
        return;
    }

    auto gsp = _ce->GetParams<GUIStateParams>();
    DataStatus *ds = _ce->GetDataStatus();

    gsp->InsertOpenDataSet(name, format, files);
    //GetAnimationParams()->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);
    //pm->GetParams(AnimationParams::GetClassType())->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);
    auto ap = _ce->GetParams<AnimationParams>();
    ap->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);

    //if (_sessionNewFlag) {
    //    NavigationUtils::ViewAll(_controlExec);
    //    NavigationUtils::SetHomeViewpoint(_controlExec);
    //    gsp->SetProjectionString(ds->GetMapProjection());
    //}

    //_sessionNewFlag = false;
    pm->EndSaveStateGroup();
}

void PImportData::updateGUI() const {
    _group->Update(getParams());
    _selector->Update(getParams());
}

#include "PSection.h"

//PImportDataSection::PImportDataSection() : PWidgetWrapper(new PSection("Data", {new PImportData})) {}
