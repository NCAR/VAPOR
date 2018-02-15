#ifndef QSINGLEPOINT_H
#define QSINGLEPOINT_H

#include <QWidget>

namespace Ui {
class QSinglePoint;
}

class QSinglePoint : public QWidget 
{
    Q_OBJECT

public:
    explicit QSinglePoint(QWidget *parent = 0);
    ~QSinglePoint();

    void SetExtents( const std::vector<double>& min, const std::vector<double>& max );

    void SetDimensionality( int );
    int  GetDimensionality( );
    void GetCurrentPoint( std::vector<double>& );
    void SetMainLabel( const QString& );
    void SetDecimals( int dec );                // how many digits after the decimal point
    void SetValue( const std::vector<double>& );

signals:
    void pointUpdated();

private slots:
    void _coordinateChanged(double); 

private:
    Ui::QSinglePoint*   _ui;
    int                 _dimensionality;
};

#endif // QSINGLEPOINT_H
