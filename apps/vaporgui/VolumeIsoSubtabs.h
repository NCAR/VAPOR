#pragma once

#include "ui_VolumeIsoAppearanceGUI.h"
#include "ui_VolumeIsoVariablesGUI.h"
#include "ui_VolumeIsoGeometryGUI.h"
#include "ui_VolumeIsoAnnotationGUI.h"

#include "vapor/VolumeIsoParams.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class VolumeIsoVariablesSubtab : public QWidget, public Ui_VolumeIsoVariablesGUI {
    Q_OBJECT

public:
    VolumeIsoVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR | COLOR), (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params);

private slots:

private:
    VAPoR::VolumeIsoParams *_isoParams;
};

class VolumeIsoAppearanceSubtab : public QWidget, public Ui_VolumeIsoAppearanceGUI {
    Q_OBJECT

public:
    VolumeIsoAppearanceSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params);

private slots:
    void on__castingModeComboBox_currentIndexChanged(const QString &text);
    void on__samplingRateComboBox_currentIndexChanged(const QString &text);

    void on__lightingCheckBox_toggled(bool checked);
    void on__ambientWidget_valueChanged(double value);
    void on__diffuseWidget_valueChanged(double value);
    void on__specularWidget_valueChanged(double value);
    void on__shininessWidget_valueChanged(int value);
    void on__defaultLightingButton_clicked(bool checked);

    void on__isoValueCheckbox0_toggled(bool);
    void on__isoValueCheckbox1_toggled(bool);
    void on__isoValueCheckbox2_toggled(bool);
    void on__isoValueCheckbox3_toggled(bool);

    void on__isoWidget0_valueChanged(double);
    void on__isoWidget1_valueChanged(double);
    void on__isoWidget2_valueChanged(double);
    void on__isoWidget3_valueChanged(double);

private:
    VAPoR::VolumeIsoParams *_params;
};

class VolumeIsoGeometrySubtab : public QWidget, public Ui_VolumeIsoGeometryGUI {
    Q_OBJECT

public:
    VolumeIsoGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((DimFlags)THREED, (VariableFlags)SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

class VolumeIsoAnnotationSubtab : public QWidget, public Ui_VolumeIsoAnnotationGUI {
public:
    VolumeIsoAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};
