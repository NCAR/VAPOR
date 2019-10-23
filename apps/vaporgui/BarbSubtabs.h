#ifndef BARBSUBTABS_H
#define BARBSUBTABS_H

#include "ui_BarbAppearanceGUI.h"
#include "ui_BarbVariablesGUI.h"
#include "ui_BarbGeometryGUI.h"
#include "ui_BarbAnnotationGUI.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
class BarbParams;
}    // namespace VAPoR

class TFEditor;

class BarbVariablesSubtab : public QWidget, public Ui_BarbVariablesGUI {
    Q_OBJECT

public:
    BarbVariablesSubtab(QWidget *parent);

    void Initialize(VAPoR::BarbParams *bParams, VAPoR::DataMgr *dataMgr);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
};

class BarbAppearanceSubtab : public QWidget, public Ui_BarbAppearanceGUI {
    Q_OBJECT

public:
    BarbAppearanceSubtab(QWidget *parent);

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

private slots:
    void xDimChanged(int i);
    void yDimChanged(int i);
    void zDimChanged(int i);
    void lengthChanged(double d);
    void thicknessChanged(double d);
    void recalculateScales();

private:
    void _hideZDimWidgets();
    void _showZDimWidgets();
    bool _isVariable2D() const;

    VAPoR::BarbParams *_bParams;
    VAPoR::DataMgr *   _dataMgr;
    VAPoR::ParamsMgr * _paramsMgr;
    Combo *            _xDimCombo;
    Combo *            _yDimCombo;
    Combo *            _lengthCombo;
    Combo *            _thicknessCombo;
    TFEditor *         _tfe;
};

class BarbGeometrySubtab : public QWidget, public Ui_BarbGeometryGUI {
    Q_OBJECT

public:
    BarbGeometrySubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);

private:
    VAPoR::BarbParams *_bParams;
};

class BarbAnnotationSubtab : public QWidget, public Ui_BarbAnnotationGUI {
    Q_OBJECT

public:
    BarbAnnotationSubtab(QWidget *parent);

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);
};

#endif    // BARBSUBTABS_H
