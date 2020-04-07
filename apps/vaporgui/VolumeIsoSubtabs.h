#pragma once

#include "ui_VolumeIsoAppearanceGUI.h"
#include "ui_VolumeIsoVariablesGUI.h"
#include "ui_VolumeIsoGeometryGUI.h"
#include "ui_VolumeIsoAnnotationGUI.h"

#include "vapor/VolumeIsoParams.h"
#include "TFEditorIsoSurface.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class PGroup;

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

private:
    PGroup *_pg;
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
