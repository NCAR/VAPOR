#include "QColorWidget.h"
#include <QApplication>
#include <QColorDialog>

QColorWidget::QColorWidget()
{
    setColor(Qt::blue);
    setReadOnly(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(minimumSizeHint());
}

QSize QColorWidget::minimumSizeHint() const
{
    //    How Qt buttons determine size
    //    QSize fontSize = fontMetrics().size(Qt::TextShowMnemonic, QString("XXXX"));

    float height = QLineEdit::minimumSizeHint().height();

    return QSize(height * 1.618, height);
}

void QColorWidget::setColor(const QColor &color)
{
    _color = color;
    QPalette pal = palette();
    //    pal.setColor(QPalette::Background, color);
    pal.setColor(QPalette::Base, color);
    pal.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
}

QColor QColorWidget::getColor() const { return _color; }

void QColorWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QColor newColor = QColorDialog::getColor(_color);
    QApplication::restoreOverrideCursor();

    if (newColor.isValid() && newColor != _color) {
        setColor(newColor);
        emit colorChanged(newColor);
    }
}

void QColorWidget::enterEvent(QEvent *event)
{
    if (isEnabled()) {
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        setToolTip(_color.name());
    } else {
        setToolTip("");
    }
}

void QColorWidget::leaveEvent(QEvent *event) { QApplication::restoreOverrideCursor(); }
