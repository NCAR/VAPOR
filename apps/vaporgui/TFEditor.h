#pragma once

#include <QTabWidget>
#include <QStackedWidget>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "ParamsWidgets.h"
#include <vector>
#include "VSection.h"

#include <QToolButton>

class TFOpacityWidget;
class TFColorWidget;
class TFHistogramMap;
class TFHistogramWidget;
class QRangeSlider;
class QRangeSliderTextCombo;
class TFInfoWidget;
class TFMapsGroup;
class TFMapWidget;
class TFMap;
class TFMapsInfoGroup;
class TFIsoValueWidget;

class TFEditor : public VSection {
    Q_OBJECT
    
public:
    TFEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    VAPoR::RenderParams *_rParams = nullptr;
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    QRangeSliderTextCombo *range;
    ParamsWidgetDropdown *colorMapTypeDropdown;
    TFMapsGroup *_maps;
    TFMapsInfoGroup *_mapsInfo;
    
    void _updateMappingRangeControl(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    void _getDataRange(VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams, float *min, float *max) const;
    
private slots:
    void _rangeChangedBegin();
    void _rangeChangedIntermediate(float left, float right);
    void _rangeChanged(float left, float right);
    void _test();
    void _loadColormap(std::string path);
    void _loadColormap();
    void _loadTransferFunction();
    void _saveTransferFunction();
    void _histogramDynamicScalingToggled(bool on);
};







#include <QWidgetAction>
class ColorMapMenuItem : public QWidgetAction {
    Q_OBJECT
    
    static std::map<std::string, QIcon> icons;
    static QIcon getCachedIcon(const std::string &path);
    static QSize getIconSize();
    static QSize getIconPadding();
    
    const std::string _path;
    
public:
    ColorMapMenuItem(const std::string &path);
    static void CloseMenu(QAction *action);
    
signals:
    void triggered(std::string colormapPath);
    
private slots:
    void _clicked();
};


class TFMapsGroup : public QWidget {
    Q_OBJECT
    
    std::vector<TFMapWidget*> _maps;
    
public:
    TFMapsGroup();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    TFMapsInfoGroup *CreateInfoGroup();
    
    TFHistogramMap *histo;
    
private:
    void add(TFMapWidget *map);
    
private slots:
    void mapActivated(TFMapWidget *map);
};



class TFMapsInfoGroup : public QStackedWidget {
    Q_OBJECT
    
    std::vector<TFInfoWidget*> _infos;
    
public:
    TFMapsInfoGroup();
    void Update(VAPoR::RenderParams *rParams);
    
    friend class TFMapsGroup;
    
private:
    void add(TFMapWidget *map);
    
private slots:
    void mapActivated(TFMap *map);
};
