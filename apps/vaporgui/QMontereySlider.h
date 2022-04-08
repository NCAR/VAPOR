#pragma once

#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QStyleOptionComplex>
#include <QSlider>
#include <QColor>
#include "math.h"

class QMontereySlider : public QSlider {
public:
    explicit QMontereySlider(Qt::Orientation orientation, QWidget *parent = nullptr) : QSlider(orientation, parent){};
    explicit QMontereySlider(QWidget *parent = nullptr) : QSlider(parent)
    {
        this->setStyleSheet("\
            QSlider::groove:horizontal {\
            height: 8px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */ \
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);\
            margin: 2px 0;\
        }\
        \
        QSlider::handle:horizontal {\
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);\
            border: 1px solid #5c5c5c;\
            width: 18px;\
            margin: -2px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ \
            border-radius: 3px;\
        }");
    };
};
