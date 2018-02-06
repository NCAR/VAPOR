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
    void GetRange(std::vector<double> &range);
    void SetMainLabel(const QString &);
    void SetDecimals(int dec);    // how many digits after the decimal point

signals:
    void rangeChanged();

private slots:
    void _minChanged(double);
    void _maxChanged(double);

private:
    Ui::QRange *_ui;
};

#endif    // QRANGE_H
