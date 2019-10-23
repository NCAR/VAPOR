#include "VolumeSubtabs.h"
#include "TFEditor.h"
#include "QSliderEdit.h"

using namespace VAPoR;

void VolumeVariablesSubtab::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *params)
{
    VolumeParams *vp = dynamic_cast<VolumeParams *>(params);
    _volumeParams = vp;
    VAssert(vp);

    _variablesWidget->Update(dataMgr, paramsMgr, params);
}

VolumeAppearanceSubtab::VolumeAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    verticalLayout->insertWidget(0, _tfe = new TFEditor);

    _densitySlider = new QSliderEdit();
    _densitySlider->SetLabel("Volume Density");
    _densitySlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(_densitySlider, SIGNAL(valueChanged(double)), this, SLOT(_densitySlider_valueChanged(double)));
    _raytracingFrame->layout()->addWidget(_densitySlider);

    _params = nullptr;

    _samplingRateComboBox->blockSignals(true);
    vector<float> samplingOptions = VolumeParams::GetSamplingRateMultiples();
    for (double rate : samplingOptions) _samplingRateComboBox->addItem(GetQStringForSamplingRate(rate));
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

void VolumeAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    VAPoR::VolumeParams *vp = dynamic_cast<VolumeParams *>(rParams);
    _params = vp;
    VAssert(vp);

    _tfe->Update(dataMgr, paramsMgr, rParams);

    // ---------------------------
    // Raytracing Parameters
    // ---------------------------

    string algorithm = vp->GetAlgorithm();
    int    index = _castingModeComboBox->findText(QString::fromStdString(algorithm));

    _castingModeComboBox->blockSignals(true);
    if (index == -1) {
        _castingModeComboBox->clear();
        const vector<string> algorithms = VolumeParams::GetAlgorithmNames(VolumeParams::Type::DVR);
        for (const string &s : algorithms) _castingModeComboBox->addItem(QString::fromStdString(s));

        index = _castingModeComboBox->findText(QString::fromStdString(algorithm));
    }
    _castingModeComboBox->setCurrentIndex(index);
    _castingModeComboBox->blockSignals(false);

    _samplingRateComboBox->blockSignals(true);
    _samplingRateComboBox->setCurrentIndex(_samplingRateComboBox->findText(GetQStringForSamplingRate(_params->GetSamplingMultiplier())));
    _samplingRateComboBox->blockSignals(false);

    MapperFunction *tf = vp->GetMapperFunc(vp->GetVariableName());
    float           opacityValue = powf(tf->getOpacityScale(), 1 / 4.f);
    _densitySlider->SetValue(opacityValue);

    // ---------------------------
    // Lighting Parameters
    // ---------------------------

    _lightingCheckBox->setChecked(_params->GetLightingEnabled());
    _ambientWidget->SetValue(_params->GetPhongAmbient());
    _diffuseWidget->SetValue(_params->GetPhongDiffuse());
    _specularWidget->SetValue(_params->GetPhongSpecular());
    _shininessWidget->SetValue(_params->GetPhongShininess());
}

void VolumeAppearanceSubtab::_densitySlider_valueChanged(double v)
{
    MapperFunction *tf = _params->GetMapperFunc(_params->GetVariableName());
    tf->setOpacityScale(powf(v, 4));
}

void VolumeAppearanceSubtab::on__castingModeComboBox_currentIndexChanged(const QString &text)
{
    if (!text.isEmpty()) {
        _params->SetAlgorithm(text.toStdString());
        _params->SetAlgorithmWasManuallySetByUser(true);
    }
}

void VolumeAppearanceSubtab::on__samplingRateComboBox_currentIndexChanged(const QString &text)
{
    if (!text.isEmpty()) { _params->SetSamplingMultiplier(GetSamplingRateForQString(text)); }
}

void VolumeAppearanceSubtab::on__lightingCheckBox_toggled(bool checked)
{
    _params->SetLightingEnabled(checked);

    _ambientWidget->setEnabled(checked);
    _diffuseWidget->setEnabled(checked);
    _specularWidget->setEnabled(checked);
    _shininessWidget->setEnabled(checked);
}

void VolumeAppearanceSubtab::on__ambientWidget_valueChanged(double value) { _params->SetPhongAmbient(value); }

void VolumeAppearanceSubtab::on__diffuseWidget_valueChanged(double value) { _params->SetPhongDiffuse(value); }

void VolumeAppearanceSubtab::on__specularWidget_valueChanged(double value) { _params->SetPhongSpecular(value); }

void VolumeAppearanceSubtab::on__shininessWidget_valueChanged(int value) { _params->SetPhongShininess(value); }

void VolumeAppearanceSubtab::on__defaultLightingButton_clicked(bool checked)
{
    _params->SetPhongAmbient(_params->GetDefaultPhongAmbient());
    _params->SetPhongDiffuse(_params->GetDefaultPhongDiffuse());
    _params->SetPhongSpecular(_params->GetDefaultPhongSpecular());
    _params->SetPhongShininess(_params->GetDefaultPhongShininess());
}

QString VolumeAppearanceSubtab::GetQStringForSamplingRate(const float rate)
{
    char buf[16];
    sprintf(buf, "%gx", rate);
    return QString(buf);
}

float VolumeAppearanceSubtab::GetSamplingRateForQString(const QString &str)
{
    double rate;
    sscanf(str.toStdString().c_str(), "%lgx", &rate);
    return rate;
}
