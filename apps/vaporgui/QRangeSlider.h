#pragma once

#include <QSlider>

class QStylePainter;

class QRangeSlider : public QSlider {
    Q_OBJECT
    
public:
    QRangeSlider();
    QRangeSlider(Qt::Orientation orientation);
    QSize minimumSizeHint() const;
    void SetValue(float min, float max);
    
signals:
    void ValueChanged(float min, float max);
    
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    
    int _position[2];
    int _value[2];
    int _grabbedControl = -1;
    int _lastSelectedControl = 0;
    
    void paintHandle(QStylePainter &p, int i);
    void paintTrack(QStylePainter &p);
    bool doesHandleContainPixel(int handle, const QPoint &pixel) const;
    bool isSliderDown(int i) const;
    void swapSliders();
};
