#ifndef QSLIDEREDIT_H
#define QSLIDEREDIT_H

#include <QWidget>
#include "QDoubleValidator2.h"

namespace Ui {
class QSliderEdit;
}

class QSliderEdit : public QWidget {
    Q_OBJECT

public:
    explicit QSliderEdit(QWidget *parent = 0);
    ~QSliderEdit();

    void   SetText(const QString &text);    // for the label
    void   SetDecimals(int dec);            // how many digits after the decimal point
    void   SetExtents(double min, double max);
    double GetCurrentVal();

signals:
    // This is the only signal a QSliderEdit emits.
    void valueChanged(double);

private slots:
    void _mySlider_valueChanged(int value);
    void _mySlider_released();
    void _myLineEdit_valueChanged();

private:
    Ui::QSliderEdit *  _ui;
    QDoubleValidator2 *_validator;
};

#endif    // QSLIDEREDIT_H
