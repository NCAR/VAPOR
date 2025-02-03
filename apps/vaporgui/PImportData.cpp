#include "PImportData.h"
//#include "VRadioButtons.h"
#include "VGroup.h"
#include <vapor/ParamsBase.h>
#include <vapor/ParamsMgr.h>
#include <vapor/GUIStateParams.h>
#include <QRadioButton>
#include "VLineItem.h"
#include "VPushButton.h"
#include "DatasetTypeLookup.h"
#include "PButton.h"
#include "PFileSelector.h"

PImportData::PImportData() : PWidget("", _group = new VGroup()) {
    std::vector<std::string> types = GetDatasetTypeDescriptions();
    for (auto type : types) {
        QRadioButton* rb = new QRadioButton(QString::fromStdString(type), this);
        rb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        _group->Add(new VLineItem("", rb));
        connect(rb, &QRadioButton::toggled, this, [this, rb](bool checked) {if (checked) radioButtonChecked(rb);});
    }
    
    //QHBoxWidget* importer = new QHBoxWidget();
    //VHBoxWidget* importer = new VHBoxWidget();
    //_group->Add(importer);

    _group->Add(new PButton("PImport Files", [](VAPoR::ParamsBase *p){} ));
    _selector = new PFileOpenSelector("", "Import Files" );
    _group->Add(_selector);

    VPushButton* pb = new VPushButton("Import Files");
    pb->layout()->setAlignment(Qt::AlignRight);
    _group->Add(pb);
}

// Need a universal way to import data from both here as well as MainForm.
//void PImportData::importDataset(const std::vector<string> &files, string format, DatasetExistsAction existsAction, string name)
//{
//    _paramsMgr->BeginSaveStateGroup("Import Dataset");
//    if (name.empty()) name = _getDataSetName(files[0], existsAction);
//    int rc = _controlExec->OpenData(files, name, format);
//    if (rc < 0) {
//        _paramsMgr->EndSaveStateGroup();
//        MSG_ERR("Failed to load data");
//        return;
//    }
//
//    auto gsp = _controlExec->GetParams<GUIStateParams>();
//    DataStatus *ds = _controlExec->GetDataStatus();
//
//    gsp->InsertOpenDateSet(name, format, files);
//    GetAnimationParams()->SetEndTimestep(ds->GetTimeCoordinates().size() - 1);
//
//    if (_sessionNewFlag) {
//        NavigationUtils::ViewAll(_controlExec);
//        NavigationUtils::SetHomeViewpoint(_controlExec);
//        gsp->SetProjectionString(ds->GetMapProjection());
//    }
//
//    _sessionNewFlag = false;
//    _paramsMgr->EndSaveStateGroup();
//}

void PImportData::updateGUI() const {
    _selector->Update(getParams());
    std::string type = dynamic_cast<GUIStateParams*>(getParams())->GetCurrentImportDataType();
    QList<QRadioButton*> rbs = _group->findChildren<QRadioButton*>();
    for (auto rb : rbs) {
        std::cout << "foo " << rb->text().toStdString() << std::endl;
        if (rb->text().toStdString() == type) rb->setChecked(true);
        else rb->setChecked(false);
    }
}

//void PImportData::radioButtonClicked(std::string& type) { 
void PImportData::radioButtonChecked(QRadioButton* rb) { 
    //std::cout << "type " << type << std::endl;
    //GUIStateParams* p = dynamic_cast<GUIStateParams *>(getParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    std::string type = rb->text().toStdString();
    std::cout << type << std::endl;
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(getParams());
    p->SetCurrentImportDataType(type);
}

#include "PSection.h"

PImportDataSection::PImportDataSection() : PWidgetWrapper(new PSection("Data", {new PImportData})) {}
