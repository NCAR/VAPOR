#pragma once

#include <QFrame>

class TFInfoWidget;

class TFMapWidget : public QFrame {
    Q_OBJECT
    
public:
    virtual TFInfoWidget *CreateInfoWidget() = 0;
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
};
