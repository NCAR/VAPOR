#pragma once

#include "ui_VolumeAppearanceGUI.h"
#include "ui_VolumeVariablesGUI.h"
#include "ui_VolumeGeometryGUI.h"
#include "ui_VolumeAnnotationGUI.h"
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class VolumeVariablesSubtab : public QWidget, public Ui_VolumeVariablesGUI {
    Q_OBJECT

public:
    VolumeVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR), (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class VolumeAppearanceSubtab : public QWidget, public Ui_VolumeAppearanceGUI {
    Q_OBJECT

public:
    VolumeAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->SetOpacityIntegrated(true);
        _TFWidget->Reinit((TFFlags)(CONSTANT_COLOR));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _TFWidget->Update(dataMgr, paramsMgr, rParams); }
};

class VolumeGeometrySubtab : public QWidget, public Ui_VolumeGeometryGUI {
    Q_OBJECT

public:
    VolumeGeometrySubtab(QWidget *parent)
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

class VolumeAnnotationSubtab : public QWidget, public Ui_VolumeAnnotationGUI {
    Q_OBJECT

public:
    VolumeAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};
