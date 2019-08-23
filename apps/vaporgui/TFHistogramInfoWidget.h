#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>

namespace VAPoR {
    class RenderParams;
}

class TFHistogramInfoWidget : public TFInfoWidget {
    Q_OBJECT
    
public:
    TFHistogramInfoWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public slots:
    void SetControlPoint(float value);
};
