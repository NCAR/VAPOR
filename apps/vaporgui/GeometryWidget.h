#ifndef REGIONSLIDERWIDGET_H
#define REGIONSLIDERWIDGET_H

#include "GeometryWidgetGUI.h"
#include "subeventrouter.h"
#include "RangeCombos.h"

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class GeometryWidget : public QWidget, public Ui_GeometryWidgetGUI {
    Q_OBJECT

public:
    //! Bit mask to indicate whether 2D, 3D, or 2D and 3D variables are to
    //! be supported
    //
    enum Flags { TWOD = (1u << 0), THREED = (1u << 1), VECTOR = (1u << 2) };

    GeometryWidget(QWidget *parent = 0);

    void Reinit(Flags flags);

    ~GeometryWidget();

    QString name() const { return "GeometryWidget"; }
    QString includeFile() const { return "GeometryWidget.h"; }
    QString group() const { return tr("Region Control Widgets"); }
    QString toolTip() const { return tr("A Region Control Widget"); }
    QString whatsThis() const
    {
        return tr("This widget contains all widgets "
                  "necessary for making changes to a "
                  "user-defined region.");
    }
    bool isContainer() const { return true; }
    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);

public slots:
    void setRange(double min, double max);
    void copyRegion();
    void updateRenTypeCombo();
    void updateRenNameCombo();

private:
    void   connectWidgets();
    size_t getCurrentTimestep();
    void   updateRangeLabels(vector<double> minExt, vector<double> maxExt);
    void   GetVectorExtents(size_t ts, int level, vector<double> minFullExt, vector<double> maxFullExt);
    void   updateVisCombo();

    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;

    Combo *     _minXCombo;
    Combo *     _maxXCombo;
    RangeCombo *_xRangeCombo;

    Combo *     _minYCombo;
    Combo *     _maxYCombo;
    RangeCombo *_yRangeCombo;

    Combo *     _minZCombo;
    Combo *     _maxZCombo;
    RangeCombo *_zRangeCombo;

    Flags _flags;
};

#endif    // REGIONSLIDERWIDGET_H
