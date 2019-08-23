#pragma once

#include <QTabWidget>
#include <QStackedWidget>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "ParamsWidgets.h"
#include <vector>

class TFOpacityWidget;
class TFColorWidget;
class TFHistogramWidget;
class QRangeSlider;
class TFInfoWidget;
class TFMapsGroup;
class TFMapWidget;
class TFMapsInfoGroup;

class TFEditor : public QTabWidget {
    Q_OBJECT
    
public:
    TFEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    
    QRangeSlider *range;
    ParamsWidgetDropdown *colorMapTypeDropdown;
    TFMapsGroup *_maps;
    TFMapsInfoGroup *_mapsInfo;
    QWidget *_tab() const;
};



class TFMapsGroup : public QWidget {
    Q_OBJECT
    
    std::vector<TFMapWidget*> _maps;
    
public:
    TFMapsGroup();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    TFMapsInfoGroup *CreateInfoGroup();
};



class TFMapsInfoGroup : public QStackedWidget {
    Q_OBJECT
    
public:
    TFMapsInfoGroup();
    void Update(VAPoR::RenderParams *rParams);
};
