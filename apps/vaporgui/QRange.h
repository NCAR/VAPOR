#ifndef QRANGE_H
#define QRANGE_H

#include <QWidget>

namespace Ui {
class QRange;
}

class QRange : public QWidget {
    Q_OBJECT

public:
    explicit QRange(QWidget *parent = 0);
    ~QRange();

    void SetExtents(double min, double max);
    void GetRange(double[2]);

signals:
    void RangeChanged();

private slots:
    void _minChanged(double);
    void _maxChanged(double);

private:
    Ui::QRange *_ui;
};

#endif    // QRANGE_H
