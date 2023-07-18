#pragma once

#include <QLineEdit>
#include <QColor>

class QStylePainter;

//! \class QColorWidget
//!
//! Displays a color and allows the user to modify said color

class QColorWidget : public QLineEdit {
    Q_OBJECT

    QColor _color;

public:
    bool ShowAlpha = false;

    QColorWidget();
    QSize minimumSizeHint() const;

    void   setColor(const QColor &color);
    QColor getColor() const;

signals:
    void colorChanged(QColor color);

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};
