#ifndef BARBSUBTABS_H
#define BARBSUBTABS_H

#include "ui_BarbAppearanceGUI.h"
#include "ui_BarbVariablesGUI.h"
#include "ui_BarbGeometryGUI.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
class BarbParams;
}    // namespace VAPoR

class BarbVariablesSubtab : public QWidget, public Ui_BarbVariablesGUI {
    Q_OBJECT

public:
    BarbVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::VECTOR | VariablesWidget::HGT | VariablesWidget::COLOR), (VariablesWidget::DimFlags)(VariablesWidget::TWOD),
                                 (VariablesWidget::ColorFlags)(VariablesWidget::CONST | VariablesWidget::COLORVAR));
        //(VariablesWidget::DimFlags)(VariablesWidget::THREED));
    }

    void Initialize(VAPoR::BarbParams *bParams, VAPoR::DataMgr *dataMgr);
    void pushVarStartingWithLetter(vector<string> searchVars, vector<string> &returnVars, char letter);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class BarbAppearanceSubtab : public QWidget, public Ui_BarbAppearanceGUI {
    Q_OBJECT

public:
    BarbAppearanceSubtab(QWidget *parent);    //{
                                              //		setupUi(this);
                                              //		_TFWidget->Reinit((TFWidget::Flags)
                                              //			(TFWidget::COLORVAR | TFWidget::PRIORITYCOLORVAR));
                                              //	}

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr,
                VAPoR::RenderParams *rParams);    // {
                                                  //		_TFWidget->Update(dataMgr, paramsMgr, rParams);
                                                  //		_ColorbarWidget->Update(dataMgr, paramsMgr, rParams);
                                                  //	}

    void Initialize(VAPoR::BarbParams *rParams);

private slots:
    void   xDimChanged(int i);
    void   yDimChanged(int i);
    void   zDimChanged(int i);
    void   lengthChanged(double d);
    void   thicknessChanged(double d);
    double CalculateDomainLength(int ts);

private:
    VAPoR::BarbParams *_bParams;
    VAPoR::DataMgr *   _dataMgr;
    Combo *            _xDimCombo;
    Combo *            _yDimCombo;
    Combo *            _zDimCombo;
    Combo *            _lengthCombo;
    Combo *            _thicknessCombo;
};

class BarbGeometrySubtab : public QWidget, public Ui_BarbGeometryGUI {
    Q_OBJECT

public:
    BarbGeometrySubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _bParams = (VAPoR::BarbParams *)rParams;
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }

    /*private slots:
    void xDimChanged(int i);
    void yDimChanged(int i);
    void zDimChanged(int i);
    void lengthChanged(double d);
    void thicknessChanged(double d);
*/
private:
    VAPoR::BarbParams *_bParams;
    // Combo* _xDimCombo;
    // Combo* _yDimCombo;
    // Combo* _zDimCombo;
    // Combo* _lengthCombo;
    // Combo* _thicknessCombo;
};

#endif    // BARBSUBTABS_H
