#include "QRangeSlider.h"
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QMouseEvent>
#include <QBitmap>

#define QT_STOPS 1000000000

QRangeSlider::QRangeSlider()
: QRangeSlider(Qt::Orientation::Horizontal)
{}

QRangeSlider::QRangeSlider(Qt::Orientation orientation)
: QSlider(orientation)
{
    _position[0] = 0;
    _value[0] = 0;
    _position[1] = QT_STOPS-1;
    _value[1] = QT_STOPS-1;
    this->setRange(0, QT_STOPS);
    this->setTracking(true);
}

QSize QRangeSlider::minimumSizeHint() const
{
    return QSlider::minimumSizeHint();
}

void QRangeSlider::SetValue(float min, float max)
{
    _position[0] = min * (QT_STOPS-1);
    _value[0] = min * (QT_STOPS-1);
    _position[1] = max * (QT_STOPS-1);
    _value[1] = max * (QT_STOPS-1);
    update();
}

void QRangeSlider::paintEvent(QPaintEvent* event)
{
    QStylePainter p(this);
    
    paintTrack(p);
    
    int drawId = (_lastSelectedControl + 1) % 2;
    paintHandle(p, drawId);
    drawId = (drawId + 1) % 2;
    paintHandle(p, drawId);
}

void QRangeSlider::paintHandle(QStylePainter &p, int i)
{
    QStyleOptionSlider option;
    this->initStyleOption( &option );
    
    option.subControls = QStyle::SC_SliderHandle;
    option.sliderValue = _value[i];
    option.sliderPosition = _position[i];
    if (isSliderDown(i)) {
        option.activeSubControls = QStyle::SC_SliderHandle;
        option.state |= QStyle::State_Sunken;
    }
    p.drawComplexControl(QStyle::CC_Slider, option);
}

void QRangeSlider::paintTrack(QStylePainter &p)
{
    QStyleOptionSlider option;
    this->initStyleOption( &option );
    
    option.subControls = QStyle::SC_SliderGroove;
    p.drawComplexControl(QStyle::CC_Slider, option);
    
    QRect groove = this->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
    groove.adjust(0, 0, -1, 0);
    
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::transparent);
    QStylePainter pixStylePainter(&pixmap, this);
    pixStylePainter.drawComplexControl(QStyle::CC_Slider, option);
    
    option.sliderPosition = _position[0];
    const QRect left = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);
    option.sliderPosition = _position[1];
    const QRect right = style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);
    
    QRect highlight(0, 0, width(), height());
    if (option.orientation == Qt::Horizontal) {
        highlight.setLeft(left.center().x());
        highlight.setRight(right.center().x());
    } else {
        highlight.setBottom(left.center().y());
        highlight.setTop(right.center().y());
    }
    
    p.setClipRegion(QRegion(pixmap.mask()));
    p.fillRect(highlight, QPalette::Highlight);
    p.setClipRegion(QRegion(), Qt::NoClip);
}

void QRangeSlider::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0, id = _lastSelectedControl; i < 2; i++, id = (id+1)%2) {
        if (doesHandleContainPixel(id, event->pos())) {
            _grabbedControl = id;
            _lastSelectedControl = id;
            setValue(_value[id]);
            QSlider::mousePressEvent(event);
            break;
        }
    }
}

void QRangeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);
    if (_grabbedControl >= 0) {
        _value[_grabbedControl] = value();
        _position[_grabbedControl] = sliderPosition();
        _grabbedControl = -1;
        emit ValueChanged(_value[0]/(float)QT_STOPS, _value[1]/(float)QT_STOPS);
    }
}

void QRangeSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (_grabbedControl >= 0) {
        setValue(_value[_grabbedControl]);
        QSlider::mouseMoveEvent(event);
        
        if (_grabbedControl == 0)
            if (sliderPosition() > _position[1])
                swapSliders();
        if (_grabbedControl == 1)
            if (sliderPosition() < _position[0])
                swapSliders();
        
        _value[_grabbedControl] = value();
        _position[_grabbedControl] = sliderPosition();
        
        emit ValueChangedIntermediate(_value[0]/(float)QT_STOPS, _value[1]/(float)QT_STOPS);
    }
}

bool QRangeSlider::doesHandleContainPixel(int handle, const QPoint &pixel) const
{
    QStyleOptionSlider option;
    initStyleOption(&option);
    option.sliderValue    = _value[handle];
    option.sliderPosition = _position[handle];
    
    QStyle::SubControl selected = style()->hitTestComplexControl(QStyle::CC_Slider, &option, pixel, this);
    
    return selected == QStyle::SC_SliderHandle;
}

bool QRangeSlider::doesGrooveContainPixel(const QPoint &pixel) const
{
    QStyleOptionSlider option;
    initStyleOption(&option);
    option.sliderValue    = _value[0];
    option.sliderPosition = _position[0];
    
    QStyle::SubControl selected = style()->hitTestComplexControl(QStyle::CC_Slider, &option, pixel, this);
    
    return selected == QStyle::SC_SliderGroove;
}

bool QRangeSlider::isSliderDown(int i) const
{
    return _grabbedControl == i;
}

void QRangeSlider::swapSliders()
{
    std::swap(_value[0], _value[1]);
    std::swap(_position[0], _position[1]);
    _lastSelectedControl = (_lastSelectedControl+1) % 2;
    switch (_grabbedControl) {
        case  0: _grabbedControl = 1; break;
        case  1: _grabbedControl = 0; break;
        case -1: break;
    }
}
