#ifndef QSLIDEREDIT_H
#define QSLIDEREDIT_H

#include <QWidget>
#include <QDoubleValidator>

#include "Combo.h"

namespace Ui {
class QSliderEdit;
}

class QSliderEdit : public QWidget 
{
    Q_OBJECT

public:
    explicit QSliderEdit(QWidget *parent = 0);
    ~QSliderEdit();

    void    SetLabel( const QString& text );
    void    SetDecimals( int dec );                // how many digits after the decimal point
    void    SetExtents( double min, double max ); 
    double  GetCurrentValue();
    void    SetValue( double );
    void    SetIntType( bool );                    // default is false, which means double type

signals:
    // This is the signal a QSliderEdit emits.
    void valueChanged(double);
    void valueChanged(int);

private slots:
    void _comboValueChanged( double );
    void _comboValueChanged( int );

private:
    Ui::QSliderEdit*    _ui;
    Combo*              _combo;     // keeps the slider and lineEdit in sync
};

#endif // QSLIDEREDIT_H
