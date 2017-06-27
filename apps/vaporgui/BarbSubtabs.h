#ifndef BARBSUBTABS_H
#define BARBSUBTABS_H

#include "BarbAppearanceGUI.h"
#include "BarbVariablesGUI.h"
#include "BarbGeometryGUI.h"
#include "vapor/BarbParams.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class BarbVariablesSubtab : public QWidget, public Ui_BarbVariablesGUI {
    Q_OBJECT

public:
    BarbVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::VECTOR | VariablesWidget::HGT | VariablesWidget::COLOR),
                                 (VariablesWidget::DimFlags)(VariablesWidget::TWOD | VariablesWidget::THREED));
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _variablesWidget->Update(dataMgr, paramsMgr, rParams);
        //_variablesWidget->Update(dataMgr, rParams);
    }
};

class BarbAppearanceSubtab : public QWidget, public Ui_BarbAppearanceGUI {
    Q_OBJECT

public:
    BarbAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->setEventRouter(dynamic_cast<RenderEventRouter *>(parent));
        _TFWidget->Reinit((TFWidget::Flags)(TFWidget::COLORMAPPED));
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _TFWidget->Update(paramsMgr, dataMgr, rParams);
        _ColorBarFrame->Update(paramsMgr, dataMgr, rParams);
    }
};

class BarbGeometrySubtab : public QWidget, public Ui_BarbGeometryGUI {
    Q_OBJECT

public:
    BarbGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((GeometryWidget::Flags)((GeometryWidget::VECTOR) | (GeometryWidget::THREED)));

        connect(_xDimSpinBox, SIGNAL(valueChanged(int)), this, SLOT(xDimChanged(int)));
        connect(_yDimSpinBox, SIGNAL(valueChanged(int)), this, SLOT(yDimChanged(int)));
        connect(_zDimSpinBox, SIGNAL(valueChanged(int)), this, SLOT(zDimChanged(int)));
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _rParams = rParams;
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    }

private slots:
    void xDimChanged(int i)
    {
        VAPoR::BarbParams *bParams = (VAPoR::BarbParams *)_rParams;
        vector<long>       longDims = bParams->GetGrid();
        int                dims[3];

        dims[0] = i;
        dims[1] = (int)longDims[1];
        dims[2] = (int)longDims[2];
        bParams->SetGrid(dims);
    }

    void yDimChanged(int i)
    {
        VAPoR::BarbParams *bParams = (VAPoR::BarbParams *)_rParams;
        vector<long>       longDims = bParams->GetGrid();
        int                dims[3];

        dims[0] = (int)longDims[0];
        dims[1] = i;
        dims[2] = (int)longDims[2];
        bParams->SetGrid(dims);
    }

    void zDimChanged(int i)
    {
        VAPoR::BarbParams *bParams = (VAPoR::BarbParams *)_rParams;
        vector<long>       longDims = bParams->GetGrid();
        int                dims[3];

        dims[0] = (int)longDims[0];
        dims[1] = (int)longDims[1];
        dims[2] = i;
        bParams->SetGrid(dims);
    }

private:
    VAPoR::RenderParams *_rParams;
};

#endif    // BARBSUBTABS_H
