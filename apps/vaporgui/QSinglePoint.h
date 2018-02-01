#ifndef QSINGLEPOINT_H
#define QSINGLEPOINT_H

#include <QWidget>

namespace Ui {
class QSinglePoint;
}

class QSinglePoint : public QWidget {
    Q_OBJECT

public:
    explicit QSinglePoint(QWidget *parent = 0);
    ~QSinglePoint();

    void SetDimensionality(int);
    int  GetDimensionality();
    void GetCurrentPoint(std::vector<double> &);

signals:
    void PointUpdated();

private slots:
    void _coordinateChanged(double);

private:
    Ui::QSinglePoint *_ui;
    int               _dimensionality;
};

#endif    // QSINGLEPOINT_H
