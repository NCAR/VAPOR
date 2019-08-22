#pragma once

#include "TFInfoWidget.h"
#include <QLineEdit>
#include "QColorWidget.h"

namespace VAPoR {
    class RenderParams;
}

class TFColorInfoWidget : public TFInfoWidget {
    Q_OBJECT
    
public:
    TFColorInfoWidget();
    
    void Update(VAPoR::RenderParams *rParams);
    
public:
    void DeselectControlPoint();
    void SetColor(const QColor &color);
    void SetControlPoint(float value, const QColor &color);
    
protected:
    void controlPointChanged();
    
private:
    QColorWidget *_colorEdit;
    
signals:
    void ControlPointChanged(float value, QColor color);
    
private slots:
    void opacityEditChanged();
};
