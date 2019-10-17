#ifndef TWODSUBTABS_H
#define TWODSUBTABS_H

#include "ui_TwoDAppearanceGUI.h"
#include "ui_TwoDVariablesGUI.h"
#include "ui_TwoDGeometryGUI.h"
#include "ui_TwoDAnnotationGUI.h"
#include "Flags.h"
#include <TFEditor.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class TwoDVariablesSubtab : public QWidget, public Ui_TwoDVariablesGUI {
    Q_OBJECT

public:
    TwoDVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR | HEIGHT), (DimFlags)(TWOD));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
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
