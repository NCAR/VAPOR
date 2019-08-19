#pragma once

#include <QTabWidget>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "ParamsWidgets.h"

class TFOpacityWidget;
class TFColorWidget;
class TFHistogramWidget;
class QRangeSlider;

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
    QWidget *_tab() const;
};
