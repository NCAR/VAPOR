#ifndef QRANGE_H
#define QRANGE_H

#include <QWidget>

namespace Ui {
class QRange;
}

// Note: 
//   One might think it's a good idea for this QRange class to use a RangeCombo.
//   But it violates the principles of object oriented programming.
//   That's because a RangeCombo requires two Combos to keep them in sync.
//   However, as a private member, Combo is hidden by QSliderEdit, and 
//   QRange simply cannot access Combos.

class QRange : public QWidget
{
    Q_OBJECT

public:
    explicit QRange(QWidget *parent = 0);
    ~QRange();

    void SetExtents( double min,        double max );
    void GetValue(   double& smallVal,  double& bigVal );
    void SetValue(   double  smallVal,  double  bigVal );
    void SetMainLabel( const QString& );
    void SetDecimals(  int dec );            // how many digits after the decimal point
    void SetIntType(   bool );               // how many digits after the decimal point

signals:
    void rangeChanged();

private slots:
    void _minChanged( double );
    void _maxChanged( double );
    void _minChanged( int );
    void _maxChanged( int );

private:
    Ui::QRange*   _ui;
};

#endif // QRANGE_H
