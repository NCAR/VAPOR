#ifndef BARBSUBTABS_H
#define BARBSUBTABS_H

#include "BarbAppearanceGUI.h"
#include "BarbVariablesGUI.h"
#include "BarbGeometryGUI.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class BarbVariablesSubtab : public QWidget, public Ui_BarbVariablesGUI {

    Q_OBJECT

  public:
    BarbVariablesSubtab(QWidget *parent) {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::VECTOR | VariablesWidget::HGT |
                                                                 VariablesWidget::COLOR),
                                 VariablesWidget::THREED);
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _variablesWidget->Update(dataMgr, paramsMgr, rParams);
        //_variablesWidget->Update(dataMgr, rParams);
    }
};

class BarbAppearanceSubtab : public QWidget, public Ui_BarbAppearanceGUI {

    Q_OBJECT

  public:
    BarbAppearanceSubtab(QWidget *parent) {
        setupUi(this);
        _TFWidget->setEventRouter(dynamic_cast<RenderEventRouter *>(parent));
        _TFWidget->Reinit((TFWidget::Flags)(TFWidget::COLORMAPPED));
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _TFWidget->Update(paramsMgr, dataMgr, rParams);
        _ColorBarFrame->Update(paramsMgr, dataMgr, rParams);
    }
};

class BarbGeometrySubtab : public QWidget, public Ui_BarbGeometryGUI {

    Q_OBJECT

  public:
    BarbGeometrySubtab(QWidget *parent) {
        setupUi(this);
        _geometryWidget->Reinit(
            GeometryWidget::TWOD);
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    }

  private:
};

#endif //BARBSUBTABS_H
