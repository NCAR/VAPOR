#include "VolumeSubtabs.h"

using namespace VAPoR;

void VolumeVariablesSubtab::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *params) {
    VolumeParams *vp = dynamic_cast<VolumeParams *>(params);
    _volumeParams = vp;
    assert(vp);

    string algorithm = vp->GetAlgorithm();
    int index = _castingModeComboBox->findText(QString::fromStdString(algorithm));

    if (index == -1) {
        _castingModeComboBox->clear();
        const vector<string> algorithms = VolumeParams::GetAlgorithmNames(VolumeParams::Type::DVR);
        for (const string &s : algorithms)
            _castingModeComboBox->addItem(QString::fromStdString(s));

        index = _castingModeComboBox->findText(QString::fromStdString(algorithm));
    }

    _castingModeComboBox->setCurrentIndex(index);

    _variablesWidget->Update(dataMgr, paramsMgr, params);
}

void VolumeVariablesSubtab::on__castingModeComboBox_currentIndexChanged(const QString &text) {
    if (!text.isEmpty()) {
        _volumeParams->SetAlgorithm(text.toStdString());
        _volumeParams->SetAlgorithmWasManuallySetByUser(true);
    }
}

void VolumeAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) {
    VAPoR::VolumeParams *vp = (VAPoR::VolumeParams *)rParams;
    _params = vp;

    _TFWidget->Update(dataMgr, paramsMgr, rParams);

    _lightingCheckBox->setChecked(_params->GetLightingEnabled());
    _ambientWidget->SetValue(_params->GetPhongAmbient());
    _diffuseWidget->SetValue(_params->GetPhongDiffuse());
    _specularWidget->SetValue(_params->GetPhongSpecular());
    _shininessWidget->SetValue(_params->GetPhongShininess());
}

void VolumeAppearanceSubtab::on__lightingCheckBox_toggled(bool checked) {
    _params->SetLightingEnabled(checked);

    _ambientWidget->setEnabled(checked);
    _diffuseWidget->setEnabled(checked);
    _specularWidget->setEnabled(checked);
    _shininessWidget->setEnabled(checked);
}

void VolumeAppearanceSubtab::on__ambientWidget_valueChanged(double value) {
    _params->SetPhongAmbient(value);
}

void VolumeAppearanceSubtab::on__diffuseWidget_valueChanged(double value) {
    _params->SetPhongDiffuse(value);
}

void VolumeAppearanceSubtab::on__specularWidget_valueChanged(double value) {
    _params->SetPhongSpecular(value);
}

void VolumeAppearanceSubtab::on__shininessWidget_valueChanged(int value) {
    _params->SetPhongShininess(value);
}

void VolumeAppearanceSubtab::on__defaultLightingButton_clicked(bool checked) {
    _params->SetPhongAmbient(_params->GetDefaultPhongAmbient());
    _params->SetPhongDiffuse(_params->GetDefaultPhongDiffuse());
    _params->SetPhongSpecular(_params->GetDefaultPhongSpecular());
    _params->SetPhongShininess(_params->GetDefaultPhongShininess());
}
