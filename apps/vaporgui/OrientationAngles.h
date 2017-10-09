#ifndef ORIENTATIONANGLES_H
#define ORIENTATIONANGLES_H

#include <QObject>
#include "vapor/MyBase.h"
#include "ui_OrientationAnglesGUI.h"

QT_USE_NAMESPACE

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class RenderEventRouter;

class OrientationAngles : public QWidget, public Ui_OrientationAnglesGUI {

    Q_OBJECT

  public:
    OrientationAngles(QWidget *parent) { setupUi(this); };

    void Reinit(){};

    virtual ~OrientationAngles() {}

    virtual void Update(
        const VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams){};

  protected slots:

  private:
    const VAPoR::DataMgr *_dataMgr;
    VAPoR::ParamsMgr *_paramsMgr;
    VAPoR::RenderParams *_rParams;
};

#endif //ORIENTATIONANGLES_H
