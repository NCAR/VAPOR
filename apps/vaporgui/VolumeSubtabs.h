#pragma once

#include "ui_VolumeAppearanceGUI.h"
#include "ui_VolumeVariablesGUI.h"
#include "ui_VolumeGeometryGUI.h"
#include "ui_VolumeAnnotationGUI.h"
#include "Flags.h"
#include <vapor/MapperFunction.h>
#include <vapor/VolumeParams.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class VolumeVariablesSubtab : public QWidget, public Ui_VolumeVariablesGUI {

    Q_OBJECT

  public:
    VolumeVariablesSubtab(QWidget *parent) {
        setupUi(this);
        _variablesWidget->Reinit(
            (VariableFlags)(SCALAR),
            (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

  private slots:
    void on__castingModeComboBox_currentIndexChanged(const QString &text);

  private:
    VAPoR::VolumeParams *_volumeParams;
};

class VolumeAppearanceSubtab : public QWidget, public Ui_VolumeAppearanceGUI {

    Q_OBJECT

  public:
    VolumeAppearanceSubtab(QWidget *parent) {
        setupUi(this);
        _TFWidget->SetOpacityIntegrated(true);
        _TFWidget->Reinit((TFFlags)(SAMPLING));

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
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

  private slots:
    void on__lightingCheckBox_toggled(bool checked);
    void on__ambientWidget_valueChanged(double value);
    void on__diffuseWidget_valueChanged(double value);
    void on__specularWidget_valueChanged(double value);
    void on__shininessWidget_valueChanged(int value);
    void on__defaultLightingButton_clicked(bool checked);

  private:
    VAPoR::VolumeParams *_params;
};

class VolumeGeometrySubtab : public QWidget, public Ui_VolumeGeometryGUI {

    Q_OBJECT

  public:
    VolumeGeometrySubtab(QWidget *parent) {
        setupUi(this);
        _geometryWidget->Reinit(
            (DimFlags)THREED,
            (VariableFlags)SCALAR);
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

class VolumeAnnotationSubtab : public QWidget, public Ui_VolumeAnnotationGUI {

    Q_OBJECT

  public:
    VolumeAnnotationSubtab(QWidget *parent) {
        setupUi(this);
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _colorbarWidget->Update(dataMgr, paramsMgr, rParams);
    }
};
