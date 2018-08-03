#ifndef DVRSUBTABS_H
#define DVRSUBTABS_H

#include "ui_DVRAppearanceGUI.h"
#include "ui_DVRVariablesGUI.h"
#include "ui_DVRGeometryGUI.h"

#include <vapor/DVRParams.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class DVRVariablesSubtab : public QWidget, public Ui_DVRVariablesGUI {
    Q_OBJECT

public:
    DVRVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::SCALAR), (VariablesWidget::DimFlags)(VariablesWidget::THREED), (VariablesWidget::ColorFlags)(0));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class DVRAppearanceSubtab : public QWidget, public Ui_DVRAppearanceGUI {
    Q_OBJECT

public:
    DVRAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->Reinit((TFWidget::Flags)(0));

        _dvrParams = nullptr;
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(dataMgr, paramsMgr, rParams);
        _ColorbarWidget->Update(dataMgr, paramsMgr, rParams);

        _dvrParams = dynamic_cast<VAPoR::DVRParams *>(rParams);
        assert(_dvrParams);
        _lightingCheckBox->setChecked(_dvrParams->GetLighting());
    }

private slots:
    void on__lightingCheckBox_toggled(bool checked) { _dvrParams->SetLighting(checked); }

private:
    VAPoR::DVRParams *_dvrParams;
};

class DVRGeometrySubtab : public QWidget, public Ui_DVRGeometryGUI {
    Q_OBJECT

public:
    DVRGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit(GeometryWidget::THREED, GeometryWidget::MINMAX, GeometryWidget::SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

#endif    // DVRSUBTABS_H
