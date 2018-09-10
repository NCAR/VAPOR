#ifndef SLICESUBTABS_H
#define SLICESUBTABS_H

#include "ui_SliceAppearanceGUI.h"
#include "ui_SliceVariablesGUI.h"
#include "ui_SliceGeometryGUI.h"
#include "ui_SliceAnnotationGUI.h"
#include "Flags.h"

#include <vapor/SliceParams.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class SliceVariablesSubtab : public QWidget, public Ui_SliceVariablesGUI {

    Q_OBJECT

  public:
    SliceVariablesSubtab(QWidget *parent);

    void Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams);
};

class SliceAppearanceSubtab : public QWidget, public Ui_SliceAppearanceGUI {

    Q_OBJECT

  public:
    SliceAppearanceSubtab(QWidget *parent);

    void Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams);

  private slots:
    void _xSampleRateChanged(int rate);
    void _ySampleRateChanged(int rate);
    void _zSampleRateChanged(int rate);

  private:
    VAPoR::SliceParams *_params;
};

class SliceGeometrySubtab : public QWidget, public Ui_SliceGeometryGUI {

    Q_OBJECT

  public:
    SliceGeometrySubtab(QWidget *parent);

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams);
};

class SliceAnnotationSubtab : public QWidget, public Ui_SliceAnnotationGUI {

    Q_OBJECT

  public:
    SliceAnnotationSubtab(QWidget *parent);

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams);
};
#endif //SLICESUBTABS_H
