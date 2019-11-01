#pragma once

#include "vapor/ModelParams.h"
#include "ui_ModelVariablesGUI.h"
#include "ui_ModelGeometryGUI.h"
#include "RangeCombos.h"
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class SpacingCombo;
class TFEditor;
class VSection;
class ParamsWidget;

class ModelVariablesSubtab : public QWidget, public Ui_ModelVariablesGUI {
    Q_OBJECT

public:
    ModelVariablesSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private:
    VSection *             _modelSettings;
    vector<ParamsWidget *> _pw;

    void addPW(ParamsWidget *w);
};

class ModelGeometrySubtab : public QWidget, public Ui_ModelGeometryGUI {
    Q_OBJECT

public:
    ModelGeometrySubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _transformTable->Update(rParams->GetTransform()); }
};
