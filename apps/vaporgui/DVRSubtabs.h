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

        /* Note: slider ranges are set using QTCreator, and should be modified there ! */
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(dataMgr, paramsMgr, rParams);
        _ColorbarWidget->Update(dataMgr, paramsMgr, rParams);

        _dvrParams = dynamic_cast<VAPoR::DVRParams *>(rParams);
        assert(_dvrParams);
        _lightingCheckBox->setChecked(_dvrParams->GetLighting());

        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        _ambientSlider->setValue(int(coeffs[0] * 100.0));
        _diffuseSlider->setValue(int(coeffs[1] * 100.0));
        _specularSlider->setValue(int(coeffs[2] * 100.0));
        _shininessSlider->setValue(int(coeffs[3]));
    }

private slots:
    void on__lightingCheckBox_toggled(bool checked) { _dvrParams->SetLighting(checked); }

    void on__ambientSlider_valueChanged(int value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[0] = double(value) / 100.0;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__diffuseSlider_valueChanged(int value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[1] = double(value) / 100.0;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__specularSlider_valueChanged(int value)
    {
        std::vector<double> coeffs = _dvrParams->GetLightingCoeffs();
        coeffs[2] = double(value) / 100.0;
        _dvrParams->SetLightingCoeffs(coeffs);
    }

    void on__shininessSlider_valueChanged(int value)
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
};

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
