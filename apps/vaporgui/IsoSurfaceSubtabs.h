#ifndef ISOSURFACESUBTABS_H
#define ISOSURFACESUBTABS_H

#include "ui_IsoSurfaceAppearanceGUI.h"
#include "ui_IsoSurfaceVariablesGUI.h"
#include "ui_IsoSurfaceGeometryGUI.h"
#include "ui_IsoSurfaceAnnotationGUI.h"

#include <vapor/IsoSurfaceParams.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class IsoSurfaceVariablesSubtab : public QWidget, public Ui_IsoSurfaceVariablesGUI {
    Q_OBJECT

public:
    IsoSurfaceVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR), (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class IsoSurfaceAppearanceSubtab : public QWidget, public Ui_IsoSurfaceAppearanceGUI {
    Q_OBJECT

public:
    IsoSurfaceAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->Reinit((TFFlags)(0));

        _params = nullptr;

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
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(dataMgr, paramsMgr, rParams);

        _params = dynamic_cast<VAPoR::IsoSurfaceParams *>(rParams);
        assert(_params);
        _lightingCheckBox->setChecked(_params->GetLighting());

        std::vector<double> coeffs = _params->GetLightingCoeffs();
        _ambientWidget->SetValue(coeffs[0]);
        _diffuseWidget->SetValue(coeffs[1]);
        _specularWidget->SetValue(coeffs[2]);
        _shininessWidget->SetValue(coeffs[3]);
    }

private slots:
    void on__lightingCheckBox_toggled(bool checked)
    {
        _params->SetLighting(checked);

        _ambientWidget->setEnabled(checked);
        _diffuseWidget->setEnabled(checked);
        _specularWidget->setEnabled(checked);
        _shininessWidget->setEnabled(checked);
    }

    void on__ambientWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _params->GetLightingCoeffs();
        coeffs[0] = value;
        _params->SetLightingCoeffs(coeffs);
    }

    void on__diffuseWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _params->GetLightingCoeffs();
        coeffs[1] = value;
        _params->SetLightingCoeffs(coeffs);
    }

    void on__specularWidget_valueChanged(double value)
    {
        std::vector<double> coeffs = _params->GetLightingCoeffs();
        coeffs[2] = value;
        _params->SetLightingCoeffs(coeffs);
    }

    void on__shininessWidget_valueChanged(int value)
    {
        std::vector<double> coeffs = _params->GetLightingCoeffs();
        coeffs[3] = double(value);
        _params->SetLightingCoeffs(coeffs);
    }

    void on__defaultLightingButton_clicked(bool checked)
    {
        std::vector<double> defaultLightingCoeffs(4);
        defaultLightingCoeffs[0] = 0.5;
        defaultLightingCoeffs[1] = 0.3;
        defaultLightingCoeffs[2] = 0.2;
        defaultLightingCoeffs[3] = 12.0;
        _params->SetLightingCoeffs(defaultLightingCoeffs);
    }

private:
    VAPoR::IsoSurfaceParams *_params;
};

class IsoSurfaceGeometrySubtab : public QWidget, public Ui_IsoSurfaceGeometryGUI {
    Q_OBJECT

public:
    IsoSurfaceGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((DimFlags)THREED, (GeometryFlags)MINMAX, (VariableFlags)SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

class IsoSurfaceAnnotationSubtab : public QWidget, public Ui_IsoSurfaceAnnotationGUI {
public:
    IsoSurfaceAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};

#endif
