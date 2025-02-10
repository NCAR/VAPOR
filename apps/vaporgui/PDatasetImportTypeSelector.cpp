#include "PDatasetImportTypeSelector.h"
#include "VRadioButtons.h"
#include <vapor/ParamsBase.h>
#include <vapor/ParamsMgr.h>
#include <vapor/GUIStateParams.h>

//PDatasetImportTypeSelector::PDatasetImportTypeSelector(std::vector<std::string>& types) : PWidget("", _vRadioButtons= new VRadioButtons) {
PDatasetImportTypeSelector::PDatasetImportTypeSelector(std::vector<std::string>& types) : PWidget("", _group = new VGroup() {
    for (auto type : types) {
        QRadioButton* rb = new QRadioButton(QString::fromStdString(type), this);
        _group->Add(new VLineItem("", rb));
        connect(rb, &QRadioButton::ValueChanged, this, &PDatasetImportTypeSelector::radioButtonChecked(rb));
    }
    //_vRadioButtons->SetValues(types);
    //connect(_vRadioButtons, &VRadioButtons::ValueChanged, this, &PDatasetImportTypeSelector::radioButtonChecked);
}

void PDatasetImportTypeSelector::updateGUI() const {
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(getParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    //std::vector<std::string> types GUIStateParams->GetImportableDataTypes();
    std::string type = p->GetCurrentImportDataType();
    //_vRadioButtons->SetValues(types);
    _vRadioButtons->SetValue(type);
    //bool on = getParamsLong();

    //_vcheckbox->SetValue(on);
}

//void PDatasetImportTypeSelector::radioButtonClicked(std::string& type) { 
void PDatasetImportTypeSelector::radioButtonChecked(QRadioButton* rb) { 
    //std::cout << "type " << type << std::endl;
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(getParamsMgr()->GetParams(GUIStateParams::GetClassType()));
}
