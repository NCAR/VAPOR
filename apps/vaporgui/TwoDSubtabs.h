#ifndef TWODSUBTABS_H
#define TWODSUBTABS_H

#include "TwoDAppearanceGUI.h"
#include "TwoDVariablesGUI.h"
#include "TwoDGeometryGUI.h"

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
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::SCALAR | VariablesWidget::HGT), VariablesWidget::TWOD);
    }

    void Update(const VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class TwoDAppearanceSubtab : public QWidget, public Ui_TwoDAppearanceGUI {
    Q_OBJECT

public:
    TwoDAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->setEventRouter(dynamic_cast<RenderEventRouter *>(parent));
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(paramsMgr, dataMgr, rParams);
        _ColorBarFrame->Update(paramsMgr, dataMgr, rParams);
    }
};

class TwoDGeometrySubtab : public QWidget, public Ui_TwoDGeometryGUI {
    Q_OBJECT

public:
    TwoDGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit(GeometryWidget::TWOD);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _geometryWidget->Update(paramsMgr, dataMgr, rParams); }

private:
};

#endif    // TWODSUBTABS_H
