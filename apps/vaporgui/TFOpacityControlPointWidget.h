#pragma once

#include "TFControlPointWidget.h"
#include <QLineEdit>

namespace VAPoR {
    class RenderParams;
}

class TFOpacityControlPointWidget : public TFControlPointWidget {
    Q_OBJECT
    
public:
    TFOpacityControlPointWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public:
    void DeselectControlPoint();
    void SetOpacity(float opacity);
    void SetControlPoint(float value, float opacity);
    
protected:
    void updateOpacity();
    float getOpacityFromEdit() const;
    
    void controlPointChanged();
    
private:
    QLineEdit *_opacityEdit;
    
    float _opacity;
    
signals:
    void ControlPointChanged(float value, float opacity);
    
private slots:
    void opacityEditChanged();
};
