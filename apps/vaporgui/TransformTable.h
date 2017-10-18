#ifndef TRANSFORMTABLE_H
#define TRANSFORMTABLE_H

#include <QObject>
#include "vapor/MyBase.h"
#include "vapor/ControlExecutive.h"
#include "ui_TransformTableGUI.h"

QT_USE_NAMESPACE

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class RenderEventRouter;

//!
//! \class TransformTable
//! \ingroup Public_GUI
//! \brief A reusable promoted widget for transforming different data sets
//! \author Scott Pearse
//! \version 3.0
//! \date  September 2017

class TransformTable : public QWidget, public Ui_TransformTableGUI {
    Q_OBJECT

public:
    enum Flags { VIEWPOINT = (1u << 0), RENDERER = (1u << 1) };

    TransformTable(QWidget *parent);

    void Reinit(Flags flags) { _flags = flags; }

    virtual ~TransformTable(){};

    // Update function for ViewpointParams transforms,
    // which requires a ControlExec
    //
    virtual void Update(VAPoR::ControlExec *controlExec);

    // Update function for RenderParams
    //
    virtual void Update(VAPoR::RenderParams *rParams);

protected slots:
    void scaleChanged(int row, int col);
    void rotationChanged(int row, int col);
    void translationChanged(int row, int col);

private:
    const VAPoR::ControlExec *_controlExec;
    const VAPoR::DataMgr *    _dataMgr;
    VAPoR::ParamsMgr *        _paramsMgr;
    VAPoR::RenderParams *     _rParams;
    Flags                     _flags;

    void updateTransformTable(QTableWidget *table, string target, vector<double> values, int row);
    void updateViewpointScales();
    void updateViewpointTranslations();
    void updateViewpointRotations();
    void updateRendererScales();
    void updateRendererTranslations();
    void updateRendererRotations();

    void setViewpointScales(string dataset, vector<double> s);
    void setViewpointTranslations(string dataset, vector<double> t);
    void setViewpointRotations(string dataset, vector<double> r);
    void setRendererScales(vector<double> s);
    void setRendererTranslations(vector<double> t);
    void setRendererRotations(vector<double> r);
};

#endif    // TRANSFORMTABLE_H
