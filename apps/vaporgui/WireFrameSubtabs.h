#ifndef WIREFRAMESUBTABS_H
#define WIREFRAMESUBTABS_H

#include "ui_WireFrameAppearanceGUI.h"
#include "ui_WireFrameVariablesGUI.h"
#include "ui_WireFrameGeometryGUI.h"
#include "ui_WireFrameAnnotationGUI.h"
#include "Flags.h"
#include "TFEditor.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class WireFrameVariablesSubtab : public QWidget, public Ui_WireFrameVariablesGUI {
    Q_OBJECT

public:
    WireFrameVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR | HEIGHT), (DimFlags)(THREED | TWOD));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class WireFrameAppearanceSubtab : public QWidget, public Ui_WireFrameAppearanceGUI {
    Q_OBJECT

    TFEditor *_tfe;

public:
    WireFrameAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        verticalLayout->insertWidget(0, _tfe = new TFEditor);
        _tfe->SetShowOpacityMap(false);
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _tfe->Update(dataMgr, paramsMgr, rParams); }
};

class WireFrameGeometrySubtab : public QWidget, public Ui_WireFrameGeometryGUI {
    Q_OBJECT

public:
    WireFrameGeometrySubtab(QWidget *parent)
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

class WireFrameAnnotationSubtab : public QWidget, public Ui_WireFrameAnnotationGUI {
    Q_OBJECT

public:
    WireFrameAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};
#endif    // WIREFRAMESUBTABS_H
