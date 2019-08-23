#pragma once

#include <QTabWidget>
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

class TFEditor : public QTabWidget {
    Q_OBJECT
    
public:
    TFEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    
    TFOpacityWidget *tff;
    TFHistogramWidget *tfh;
    TFColorWidget *colorWidget;
    QRangeSlider *range;
    ParamsWidgetDropdown *colorMapTypeDropdown;
    TFInfoWidget *controlPointWidget;
    TFInfoWidget *colorInfo;
    TFMapsGroup *_maps;
    QWidget *_tab() const;
};



class TFMapsGroup : public QWidget {
    Q_OBJECT
    
    std::vector<TFMapWidget*> _widgets;
    
public:
    TFMapsGroup();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
};
