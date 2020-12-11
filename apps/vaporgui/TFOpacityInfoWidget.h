#pragma once

#include "TFInfoWidget.h"
#include <VDoubleLineEdit.h>

namespace VAPoR {
    class RenderParams;
}

class TFOpacityInfoWidget : public TFInfoWidget {
    Q_OBJECT
    
public:
    TFOpacityInfoWidget(const std::string &variableNameTag);
    
public:
    void SetOpacity(float opacity);
    
protected:
    void updateOpacity();
    float getOpacityFromEdit() const;
    
    void controlPointChanged();
    
private:
    VDoubleLineEdit *_opacityEdit;
    
    float _opacity;
    
signals:
    void ControlPointChanged(float value, float opacity);
    
public slots:
    void SetControlPoint(float value, float opacity);
    void DeselectControlPoint();
    
private slots:
    void opacityEditChanged();
};
