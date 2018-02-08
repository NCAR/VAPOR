#ifndef QSLIDEREDIT_H
#define QSLIDEREDIT_H

#include <QWidget>
#include <QDoubleValidator>

namespace Ui {
class QSliderEdit;
}

class QSliderEdit : public QWidget {
    Q_OBJECT

public:
    explicit QSliderEdit(QWidget *parent = 0);
    ~QSliderEdit();

    void   SetLabel(const QString &text);
    void   SetDecimals(int dec);    // how many digits after the decimal point
    void   SetExtents(double min, double max);
    double GetCurrentValue();
    void   SetValue(double);

signals:
    // This is the only signal a QSliderEdit emits.
    void valueChanged(double);

private slots:
    void _mySlider_valueChanged(int value);
    void _mySlider_released();
    void _myLineEdit_valueChanged();

private:
    Ui::QSliderEdit * _ui;
    QDoubleValidator *_validator;    // it does NOT handle decimals, nor min and max extents.
    int               _decimals;
    double            _min, _max;
    void              _lineEditSetValue(double);
};

#endif    // QSLIDEREDIT_H
