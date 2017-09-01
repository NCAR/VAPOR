#ifndef CONTOURSUBTABS_H
#define CONTOURSUBTABS_H

#include "ContourAppearanceGUI.h"
#include "ContourVariablesGUI.h"
#include "ContourGeometryGUI.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {
    Q_OBJECT

public:
    ContourVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::SCALAR | VariablesWidget::HGT), (VariablesWidget::DimFlags)(VariablesWidget::THREED | VariablesWidget::TWOD));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class ContourAppearanceSubtab : public QWidget, public Ui_ContourAppearanceGUI {
    Q_OBJECT

public:
    ContourAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->Reinit((TFWidget::Flags)(0));
        _TFWidget->mappingFrame->setIsolineSliders(true);
        _TFWidget->mappingFrame->setOpacityMapping(false);

        //_TFWidget->setEventRouter(dynamic_cast<RenderEventRouter*>(parent));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(dataMgr, paramsMgr, rParams);
        _ColorbarWidget->Update(dataMgr, paramsMgr, rParams);
    }
};

class ContourGeometrySubtab : public QWidget, public Ui_ContourGeometryGUI {
    Q_OBJECT

public:
    ContourGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit(GeometryWidget::TWOD);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _geometryWidget->Update(paramsMgr, dataMgr, rParams); }

private:
};

#endif    // CONTOURSUBTABS_H
