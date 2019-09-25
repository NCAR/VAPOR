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
class TFMapsGroup;
class TFMapWidget;
class TFMap;
class TFMapsInfoGroup;
class TFIsoValueWidget;
class TFMappingRangeSelector;

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}


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
