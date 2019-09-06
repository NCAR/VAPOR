#pragma once

#include <QColor>
class QPicture;
class QPainter;
class QRect;

namespace QPaintUtils {
    void DropShadow (QPainter &p, QPicture &picture, float radius, QColor color = Qt::black);
    void InnerShadow(QPainter &p, QPicture &picture, float radius, QColor color = Qt::black);
    
    void BoxDropShadow (QPainter &p, QRect box, float radius, QColor color = Qt::black);
    void BoxInnerShadow(QPainter &p, QRect box, float radius, QColor color = Qt::black);
}
