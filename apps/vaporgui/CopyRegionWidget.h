#ifndef COPYREGIONWIDGET_H
#define COPYREGIONWIDGET_H

#include "ui_CopyRegionWidgetGUI.h"

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class CopyRegionWidget : public QWidget, public Ui_CopyRegionWidgetGUI {
    Q_OBJECT

public:
    CopyRegionWidget(QWidget *parent = 0);

    ~CopyRegionWidget();

    QString name() const { return "CopyRegionWidget"; }
    QString includeFile() const { return "CopyRegionWidget.h"; }
    QString group() const { return tr("A widget for copying one renderer's region to another"); }
    QString toolTip() const
    {
        return tr("A widget for copying one renderer's "
                  "region to another");
    }
    QString whatsThis() const
    {
        return tr("This widget contains all widgets "
                  "necessary for copying renderer regions");
    }
    bool isContainer() const { return true; }
    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams);

    void hideSinglePointTabHeader();

signals:
    void valueChanged();

private slots:
    void setPoint(double point);
    void setRange(double min, double max, int dim = -1);
    void copyRegion();

private:
    void adjustLayoutToMinMax();
    void adjustLayoutToSinglePoint();
    void adjustLayoutTo2D();
    void connectWidgets();
    void updateRangeLabels(std::vector<double> minExt, std::vector<double> maxExt);
    void updateCopyCombo();
    // void updateDimFlags();
    void updateBoxCombos(std::vector<double> &minFullExt, std::vector<double> &maxFullExt);

    bool getAuxiliaryExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);

    bool getVectorExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);

    bool getVariableExtents(std::vector<double> &minFullExts, std::vector<double> &maxFullExts);

    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;

    Combo *_spXCombo;
    Combo *_spYCombo;
    Combo *_spZCombo;

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
    DisplayFlags  _displayFlags;

    bool _useAuxVariables;    // for Statistics utility

    static const std::string _nDimsTag;
};

#endif    // GEOMETRYWIDGET_H
