#pragma once

#include <QFrame>

class TFMapWidget : public QFrame {
    Q_OBJECT
    
public:
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
};
