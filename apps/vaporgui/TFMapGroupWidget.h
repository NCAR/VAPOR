#pragma once

#include <QTabWidget>
#include <QStackedWidget>
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
class TFMapGroupWidget;
class TFMapWidget;
class TFMap;
class TFMapInfoGroupWidget;
class TFIsoValueWidget;
class TFMappingRangeSelector;

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}


class TFMapGroupWidget : public QWidget {
    Q_OBJECT
    
    std::vector<TFMapWidget*> _maps;
    
public:
    TFMapGroupWidget();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    TFMapInfoGroupWidget *CreateInfoGroup();
    
    TFHistogramMap *histo;
    
private:
    void add(TFMapWidget *map);
    
private slots:
    void mapActivated(TFMapWidget *map);
};



class TFMapInfoGroupWidget : public QStackedWidget {
    Q_OBJECT
    
    std::vector<TFInfoWidget*> _infos;
    
public:
    TFMapInfoGroupWidget();
    void Update(VAPoR::RenderParams *rParams);
    
    friend class TFMapGroupWidget;
    
private:
    void add(TFMapWidget *map);
    
private slots:
    void mapActivated(TFMap *map);
};
