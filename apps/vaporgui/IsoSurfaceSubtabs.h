#ifndef ISOSURFACESUBTABS_H
#define ISOSURFACESUBTABS_H

#include "ui_IsoSurfaceAppearanceGUI.h"
#include "ui_IsoSurfaceVariablesGUI.h"
#include "ui_IsoSurfaceGeometryGUI.h"
#include "ui_IsoSurfaceAnnotationGUI.h"

#include "vapor/IsoSurfaceParams.h"

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
        _variablesWidget->Reinit((VariableFlags)(SCALAR | COLOR), (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class IsoSurfaceAppearanceSubtab : public QWidget, public Ui_IsoSurfaceAppearanceGUI {
    Q_OBJECT

public:
    IsoSurfaceAppearanceSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *params);

private slots:
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
