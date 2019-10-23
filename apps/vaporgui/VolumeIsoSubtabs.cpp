#include "VolumeIsoSubtabs.h"
#include "VolumeSubtabs.h"

using namespace VAPoR;

void VolumeIsoVariablesSubtab::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *params)
{
    VolumeIsoParams *vp = dynamic_cast<VolumeIsoParams *>(params);
    _isoParams = vp;
    VAssert(vp);
    // TODO volume
    // long mode = _isoParams->GetCastingMode();
    // _castingModeComboBox->setCurrentIndex( mode - 1 );

    _variablesWidget->Update(dataMgr, paramsMgr, params);
}

VolumeIsoAppearanceSubtab::VolumeIsoAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    ((QVBoxLayout *)layout())->insertWidget(0, _tfe = new TFEditorIsoSurface);

    _params = nullptr;

    _samplingRateComboBox->blockSignals(true);
    vector<float> samplingOptions = VolumeParams::GetSamplingRateMultiples();
    for (double rate : samplingOptions) _samplingRateComboBox->addItem(VolumeAppearanceSubtab::GetQStringForSamplingRate(rate));
    _samplingRateComboBox->blockSignals(false);

    // Set up lighting parameter widgets
    _ambientWidget->SetLabel(QString::fromAscii("Ambient"));
    _ambientWidget->SetDecimals(2);
    _ambientWidget->SetExtents(0.0, 1.0);
    _ambientWidget->SetIntType(false);

    _diffuseWidget->SetLabel(QString::fromAscii("Diffuse"));
    _diffuseWidget->SetDecimals(2);
    _diffuseWidget->SetExtents(0.0, 1.0);
    _diffuseWidget->SetIntType(false);

    _specularWidget->SetLabel(QString::fromAscii("Specular"));
    _specularWidget->SetDecimals(2);
    _specularWidget->SetExtents(0.0, 1.0);
    _specularWidget->SetIntType(false);

    _shininessWidget->SetLabel(QString::fromAscii("Shininess"));
    _shininessWidget->SetExtents(1.0, 100.0);
    _shininessWidget->SetIntType(true);
}

void VolumeIsoAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params)
{
    _tfe->Update(dataMgr, paramsMgr, params);

    _params = dynamic_cast<VAPoR::VolumeIsoParams *>(params);
    VAssert(_params);

    _castingModeComboBox->blockSignals(true);
    string algorithm = _params->GetAlgorithm();
    int    index = _castingModeComboBox->findText(QString::fromStdString(algorithm));

    if (index == -1) {
        _castingModeComboBox->clear();
        const vector<string> algorithms = VolumeParams::GetAlgorithmNames(VolumeParams::Type::Iso);
        for (const string &s : algorithms) _castingModeComboBox->addItem(QString::fromStdString(s));

        index = _castingModeComboBox->findText(QString::fromStdString(algorithm));
    }
    _castingModeComboBox->setCurrentIndex(index);
    _castingModeComboBox->blockSignals(false);

    _samplingRateComboBox->blockSignals(true);
    _samplingRateComboBox->setCurrentIndex(_samplingRateComboBox->findText(VolumeAppearanceSubtab::GetQStringForSamplingRate(_params->GetSamplingMultiplier())));
    _samplingRateComboBox->blockSignals(false);

    _lightingCheckBox->setChecked(_params->GetLightingEnabled());

    _ambientWidget->SetValue(_params->GetPhongAmbient());
    _diffuseWidget->SetValue(_params->GetPhongDiffuse());
    _specularWidget->SetValue(_params->GetPhongSpecular());
    _shininessWidget->SetValue(_params->GetPhongShininess());
}

void VolumeIsoAppearanceSubtab::on__castingModeComboBox_currentIndexChanged(const QString &text)
{
    if (!text.isEmpty()) {
        _params->SetAlgorithm(text.toStdString());
        _params->SetAlgorithmWasManuallySetByUser(true);
    }
}

void VolumeIsoAppearanceSubtab::on__samplingRateComboBox_currentIndexChanged(const QString &text)
{
    if (!text.isEmpty()) { _params->SetSamplingMultiplier(VolumeAppearanceSubtab::GetSamplingRateForQString(text)); }
}

void VolumeIsoAppearanceSubtab::on__lightingCheckBox_toggled(bool checked)
{
    _params->SetLightingEnabled(checked);

    _ambientWidget->setEnabled(checked);
    _diffuseWidget->setEnabled(checked);
    _specularWidget->setEnabled(checked);
    _shininessWidget->setEnabled(checked);
}

void VolumeIsoAppearanceSubtab::on__ambientWidget_valueChanged(double value) { _params->SetPhongAmbient(value); }

void VolumeIsoAppearanceSubtab::on__diffuseWidget_valueChanged(double value) { _params->SetPhongDiffuse(value); }

void VolumeIsoAppearanceSubtab::on__specularWidget_valueChanged(double value) { _params->SetPhongSpecular(value); }

void VolumeIsoAppearanceSubtab::on__shininessWidget_valueChanged(int value) { _params->SetPhongShininess(value); }

void VolumeIsoAppearanceSubtab::on__defaultLightingButton_clicked(bool checked)
{
    _params->SetPhongAmbient(_params->GetDefaultPhongAmbient());
    _params->SetPhongDiffuse(_params->GetDefaultPhongDiffuse());
    _params->SetPhongSpecular(_params->GetDefaultPhongSpecular());
    _params->SetPhongShininess(_params->GetDefaultPhongShininess());
}
