#ifndef DVRSUBTABS_H
#define DVRSUBTABS_H

#include "ui_DVRAppearanceGUI.h"
#include "ui_DVRVariablesGUI.h"
#include "ui_DVRGeometryGUI.h"

#include <vapor/DVRParams.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class DVRVariablesSubtab : public QWidget, public Ui_DVRVariablesGUI {
    Q_OBJECT

public:
    DVRVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::SCALAR), (VariablesWidget::DimFlags)(VariablesWidget::THREED), (VariablesWidget::ColorFlags)(0));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class DVRAppearanceSubtab : public QWidget, public Ui_DVRAppearanceGUI {
    Q_OBJECT

public:
    DVRAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->Reinit((TFWidget::Flags)(0));

        _dvrParams = nullptr;

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

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(dataMgr, paramsMgr, rParams);
        _ColorbarWidget->Update(dataMgr, paramsMgr, rParams);

        _dvrParams = dynamic_cast<VAPoR::DVRParams *>(rParams);
        assert(_dvrParams);
        _lightingCheckBox->setChecked(_dvrParams->GetLighting());

        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        _ambientWidget->SetValue(coeffs[0]);
        _diffuseWidget->SetValue(coeffs[1]);
        _specularWidget->SetValue(coeffs[2]);
        _shininessWidget->SetValue(coeffs[3]);
    }

private slots:
    void on__lightingCheckBox_toggled(bool checked)
    {
        _dvrParams->SetLighting(checked);

        _ambientWidget->setEnabled(checked);
        _diffuseWidget->setEnabled(checked);
        _specularWidget->setEnabled(checked);
        _shininessWidget->setEnabled(checked);
    }

    void on__ambientWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[0] = value;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__diffuseWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[1] = value;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__specularWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[2] = value;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__shininessWidget_valueChanged(int value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[3] = double(value);
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__defaultLightingButton_clicked(bool checked)
    {
        std::vector<double> defaultLightingCoeffs(4);
        defaultLightingCoeffs[0] = 0.5;
        defaultLightingCoeffs[1] = 0.3;
        defaultLightingCoeffs[2] = 0.2;
        defaultLightingCoeffs[3] = 12.0;
        _dvrParams->SetLightingCoeffs(defaultLightingCoeffs);
    }

private:
    VAPoR::DVRParams *_dvrParams;

};    // End class DVRAppearanceSubtab

class DVRGeometrySubtab : public QWidget, public Ui_DVRGeometryGUI {
    Q_OBJECT

public:
    DVRGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit(GeometryWidget::THREED, GeometryWidget::MINMAX, GeometryWidget::SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

#endif    // DVRSUBTABS_H
