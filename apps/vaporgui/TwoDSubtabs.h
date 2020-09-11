#ifndef TWODSUBTABS_H
#define TWODSUBTABS_H

#include "ui_TwoDAppearanceGUI.h"
#include "ui_TwoDGeometryGUI.h"
#include "ui_TwoDAnnotationGUI.h"
#include "Flags.h"
#include <TFEditor.h>
#include "PGroup.h"
#include "PSection.h"
#include "PVariableWidgets.h"
#include "PFidelitySection.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class TwoDVariablesSubtab : public QWidget {
    Q_OBJECT
    PGroup *_pg;

public:
    TwoDVariablesSubtab(QWidget *parent)
    {
        setLayout(new QVBoxLayout);
        ((QVBoxLayout *)layout())->insertWidget(1, _pg = new PGroup);
        PSection *vars = new PSection("Variable Selection");
        vars->Add(new PScalarVariableSelector2DHLI);
        vars->Add(new PHeightVariableSelectorHLI);
        _pg->Add(vars);
        _pg->Add(new PFidelitySection);
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _pg->Update(rParams, paramsMgr, dataMgr); }
};

class TwoDAppearanceSubtab : public QWidget, public Ui_TwoDAppearanceGUI {
    Q_OBJECT

    TFEditor *_tfe;

public:
    TwoDAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        verticalLayout->insertWidget(0, _tfe = new TFEditor);
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _tfe->Update(dataMgr, paramsMgr, rParams); }
};

class TwoDGeometrySubtab : public QWidget, public Ui_TwoDGeometryGUI {
    Q_OBJECT

public:
    TwoDGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((DimFlags)TWOD, (VariableFlags)SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

class TwoDAnnotationSubtab : public QWidget, public Ui_TwoDAnnotationGUI {
    Q_OBJECT

public:
    TwoDAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};
#endif    // TWODSUBTABS_H
