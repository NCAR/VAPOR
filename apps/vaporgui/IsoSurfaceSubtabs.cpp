#include "IsoSurfaceSubtabs.h"

IsoSurfaceAppearanceSubtab::IsoSurfaceAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    _TFWidget->Reinit((TFFlags)(0));

    _params = nullptr;

    // Set up lighting parameter widgets
    _ambientWidget->SetLabel(QString::fromAscii("Ambient   "));
    _ambientWidget->SetDecimals(2);
    _ambientWidget->SetExtents(0.0, 1.0);
    _ambientWidget->SetIntType(false);

    _diffuseWidget->SetLabel(QString::fromAscii("Diffuse     "));
    _diffuseWidget->SetDecimals(2);
    _diffuseWidget->SetExtents(0.0, 1.0);
    _diffuseWidget->SetIntType(false);

    _specularWidget->SetLabel(QString::fromAscii("Specular  "));
    _specularWidget->SetDecimals(2);
    _specularWidget->SetExtents(0.0, 1.0);
    _specularWidget->SetIntType(false);

    _shininessWidget->SetLabel(QString::fromAscii("Shininess "));
    _shininessWidget->SetExtents(1.0, 100.0);
    _shininessWidget->SetIntType(true);

    // Set up iso value widgets
    _isoWidget0->SetLabel(QString::fromAscii("Value 1"));
    _isoWidget0->SetDecimals(4);
    _isoWidget0->SetIntType(false);
    _isoWidget0->setEnabled(true);

    _isoWidget1->SetLabel(QString::fromAscii("Value 2"));
    _isoWidget1->SetDecimals(4);
    _isoWidget1->SetIntType(false);
    _isoWidget1->setEnabled(false);

    _isoWidget2->SetLabel(QString::fromAscii("Value 3"));
    _isoWidget2->SetDecimals(4);
    _isoWidget2->SetIntType(false);
    _isoWidget2->setEnabled(false);

    _isoWidget3->SetLabel(QString::fromAscii("Value 4"));
    _isoWidget3->SetDecimals(4);
    _isoWidget3->SetIntType(false);
    _isoWidget3->setEnabled(false);
}

void IsoSurfaceAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params)
{
    _TFWidget->Update(dataMgr, paramsMgr, params);

    _params = dynamic_cast<VAPoR::IsoSurfaceParams *>(params);
    assert(_params);
    _lightingCheckBox->setChecked(_params->GetLighting());

    // Retrieve lighting coefficients
    std::vector<double> coeffs = _params->GetLightingCoeffs();
    _ambientWidget->SetValue(coeffs[0]);
    _diffuseWidget->SetValue(coeffs[1]);
    _specularWidget->SetValue(coeffs[2]);
    _shininessWidget->SetValue(coeffs[3]);

    // Get the value range
    float               valueRange[2];
    std::vector<double> extMin, extMax;
    params->GetBox()->GetExtents(extMin, extMax);
    VAPoR::Grid *grid = dataMgr->GetVariable(params->GetCurrentTimestep(), params->GetVariableName(), params->GetRefinementLevel(), params->GetCompressionLevel(), extMin, extMax);
    grid->GetRange(valueRange);
    delete grid;

    // Retrieve Iso Values
    std::vector<bool>   enableFlags = _params->GetEnabledIsoValueFlags();
    std::vector<double> isoValues = _params->GetIsoValues();
    _isoWidget0->setEnabled(enableFlags[0]);    // update 1st widget
    if (enableFlags[0]) {
        _isoWidget0->SetExtents(valueRange[0], valueRange[1]);
        _isoWidget0->SetValue(isoValues[0]);
    }
    _isoWidget1->setEnabled(enableFlags[1]);    // update 2nd widget
    if (enableFlags[1]) {
        _isoWidget1->SetExtents(valueRange[0], valueRange[1]);
        _isoWidget1->SetValue(isoValues[1]);
    }
    _isoWidget2->setEnabled(enableFlags[2]);    // update 3rd widget
    if (enableFlags[2]) {
        _isoWidget2->SetExtents(valueRange[0], valueRange[1]);
        _isoWidget2->SetValue(isoValues[2]);
    }
    _isoWidget3->setEnabled(enableFlags[3]);    // update 4th widget
    if (enableFlags[3]) {
        _isoWidget3->SetExtents(valueRange[0], valueRange[1]);
        _isoWidget3->SetValue(isoValues[3]);
    }
}

void IsoSurfaceAppearanceSubtab::on__lightingCheckBox_toggled(bool checked)
{
    _params->SetLighting(checked);

    _ambientWidget->setEnabled(checked);
    _diffuseWidget->setEnabled(checked);
    _specularWidget->setEnabled(checked);
    _shininessWidget->setEnabled(checked);
}

void IsoSurfaceAppearanceSubtab::on__ambientWidget_valueChanged(double value)
{
    std::vector<double> coeffs = _params->GetLightingCoeffs();
    coeffs[0] = value;
    _params->SetLightingCoeffs(coeffs);
}

void IsoSurfaceAppearanceSubtab::on__diffuseWidget_valueChanged(double value)
{
    std::vector<double> coeffs = _params->GetLightingCoeffs();
    coeffs[1] = value;
    _params->SetLightingCoeffs(coeffs);
}

void IsoSurfaceAppearanceSubtab::on__specularWidget_valueChanged(double value)
{
    std::vector<double> coeffs = _params->GetLightingCoeffs();
    coeffs[2] = value;
    _params->SetLightingCoeffs(coeffs);
}

void IsoSurfaceAppearanceSubtab::on__shininessWidget_valueChanged(int value)
{
    std::vector<double> coeffs = _params->GetLightingCoeffs();
    coeffs[3] = double(value);
    _params->SetLightingCoeffs(coeffs);
}

void IsoSurfaceAppearanceSubtab::on__defaultLightingButton_clicked(bool checked)
{
    std::vector<double> defaultLightingCoeffs(4);
    defaultLightingCoeffs[0] = 0.5;
    defaultLightingCoeffs[1] = 0.3;
    defaultLightingCoeffs[2] = 0.2;
    defaultLightingCoeffs[3] = 12.0;
    _params->SetLightingCoeffs(defaultLightingCoeffs);
}

void IsoSurfaceAppearanceSubtab::on__isoValueCheckbox0_toggled(bool checked)
{
    std::vector<bool> enabled = _params->GetEnabledIsoValueFlags();
    enabled[0] = checked;
    _params->SetEnabledIsoValueFlags(enabled);
}

void IsoSurfaceAppearanceSubtab::on__isoValueCheckbox1_toggled(bool checked)
{
    std::vector<bool> enabled = _params->GetEnabledIsoValueFlags();
    enabled[1] = checked;
    _params->SetEnabledIsoValueFlags(enabled);
}

void IsoSurfaceAppearanceSubtab::on__isoValueCheckbox2_toggled(bool checked)
{
    std::vector<bool> enabled = _params->GetEnabledIsoValueFlags();
    enabled[2] = checked;
    _params->SetEnabledIsoValueFlags(enabled);
}

void IsoSurfaceAppearanceSubtab::on__isoValueCheckbox3_toggled(bool checked)
{
    std::vector<bool> enabled = _params->GetEnabledIsoValueFlags();
    enabled[3] = checked;
    _params->SetEnabledIsoValueFlags(enabled);
}

void IsoSurfaceAppearanceSubtab::on__isoWidget0_valueChanged(double val)
{
    std::vector<double> vals = _params->GetIsoValues();
    vals[0] = val;
    _params->SetIsoValues(vals);
}

void IsoSurfaceAppearanceSubtab::on__isoWidget1_valueChanged(double val)
{
    std::vector<double> vals = _params->GetIsoValues();
    vals[1] = val;
    _params->SetIsoValues(vals);
}

void IsoSurfaceAppearanceSubtab::on__isoWidget2_valueChanged(double val)
{
    std::vector<double> vals = _params->GetIsoValues();
    vals[2] = val;
    _params->SetIsoValues(vals);
}

void IsoSurfaceAppearanceSubtab::on__isoWidget3_valueChanged(double val)
{
    std::vector<double> vals = _params->GetIsoValues();
    vals[3] = val;
    _params->SetIsoValues(vals);
}
