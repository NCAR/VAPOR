#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>

namespace VAPoR {
    class RenderParams;
}

class TFOpacityInfoWidget : public TFInfoWidget {
    Q_OBJECT
    
public:
    TFOpacityInfoWidget();
    
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
