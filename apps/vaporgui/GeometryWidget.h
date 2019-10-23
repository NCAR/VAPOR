#ifndef GEOMETRYWIDGET_H
#define GEOMETRYWIDGET_H

#include "ui_GeometryWidgetGUI.h"
#include "RangeCombos.h"
#include "Flags.h"

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class GeometryWidget : public QWidget, public Ui_GeometryWidgetGUI {
    Q_OBJECT

public:
    GeometryWidget(QWidget *parent = 0);

    void Reinit(DimFlags dimFlags, VariableFlags varFlags, GeometryFlags geometryFlags = (GeometryFlags)(0));

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
    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams, VAPoR::Box *box = nullptr);

signals:
    void valueChanged();

private slots:
    void setPoint(double point);
    void setRange(double min, double max, int dim = -1);
    void adjustLayoutToPlanar(int plane, bool reinit = true);

private:
    void adjustLayoutTo2D();
    void adjustLayoutToPlanarXY(bool reinit);
    void adjustLayoutToPlanarXZ(bool reinit);
    void adjustLayoutToPlanarYZ(bool reinit);
    void reinitBoxToPlanarAxis(int planarAxis, QSliderEdit *slider);
    void showOrientationOptions();
    void hideOrientationOptions();
    void connectWidgets();

    void getFullExtents(std::vector<double> &minFullExtents, std::vector<double> &maxFullExtents);
    void updateRangeLabels(std::vector<double> minExt, std::vector<double> maxExt);
    void updateBoxCombos(std::vector<double> &minFullExt, std::vector<double> &maxFullExt);
    bool getAuxiliaryExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);
    bool getVectorExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);
    bool getVariableExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);

    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;
    VAPoR::Box *         _box;

    Combo *     _minXCombo;
    Combo *     _maxXCombo;
    RangeCombo *_xRangeCombo;

    Combo *     _minYCombo;
    Combo *     _maxYCombo;
    RangeCombo *_yRangeCombo;

    Combo *     _minZCombo;
    Combo *     _maxZCombo;
    RangeCombo *_zRangeCombo;

    std::vector<std::string>           _dataSetNames;
    std::map<std::string, std::string> _visNames;
    std::map<std::string, std::string> _renTypeNames;

    DimFlags      _dimFlags;
    VariableFlags _varFlags;
    GeometryFlags _geometryFlags;

    bool _useAuxVariables;    // for Statistics utility
};

#endif    // GEOMETRYWIDGET_H
