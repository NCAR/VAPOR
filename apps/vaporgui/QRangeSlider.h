#pragma once

#include <QSlider>

class QStylePainter;

//! \class QRangeSlider
//! It is the same as a QSlider except it has two independent slider controls
class QRangeSlider : public QSlider {
    Q_OBJECT

public:
    QRangeSlider();
    QRangeSlider(Qt::Orientation orientation);
    QSize minimumSizeHint() const;
    void  SetValue(float min, float max);
    void  setStyle(QStyle *style) = delete;

signals:
    //! User began to change the value.
    void ValueChangedBegin();
    //! User changed the value but they have not finalized it.
    void ValueChangedIntermediate(float min, float max);
    //! User finalized changing the value.
    void ValueChanged(float min, float max);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    //    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    int _position[2];
    int _value[2];
    int _grabbedControl = -1;
    int _lastSelectedControl = 0;

    bool _grabbedBar = false;
    int  _grabbedBarPosition;
    int  _grabbedBarStartPosition;
    int  _grabbedBarControlStartPositions[2];

    void paintHandle(QStylePainter &p, int i);
    void paintTrack(QStylePainter &p);
    bool doesHandleContainPixel(int handle, const QPoint &pixel) const;
    bool doesGrooveContainPixel(const QPoint &pixel) const;
    bool isSliderDown(int i) const;
    void swapSliders();
};
