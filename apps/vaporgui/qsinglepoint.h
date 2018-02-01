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

signals:

private slots:
    void _pointChanged(double);

private:
    Ui::QSinglePoint *_ui;
};

#endif    // QSINGLEPOINT_H
