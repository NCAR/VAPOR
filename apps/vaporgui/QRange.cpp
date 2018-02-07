#include "QRange.h"
#include "ui_QRange.h"

QRange::QRange(QWidget *parent) : QWidget(parent), _ui(new Ui::QRange)
{
    _ui->setupUi(this);

    _ui->minSliderEdit->SetLabel(QString::fromAscii("Min:"));
    _ui->maxSliderEdit->SetLabel(QString::fromAscii("Max:"));

    connect(_ui->minSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_minChanged(double)));
    connect(_ui->maxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_maxChanged(double)));
}

QRange::~QRange() { delete _ui; }

void QRange::SetExtents(double min, double max)
{
    _ui->minSliderEdit->SetExtents(min, max);
    _ui->maxSliderEdit->SetExtents(min, max);
}

void QRange::GetRange(std::vector<double> &range)
{
    range.clear();
    range.push_back(_ui->minSliderEdit->GetCurrentValue());
    range.push_back(_ui->maxSliderEdit->GetCurrentValue());
}

void QRange::_minChanged(double value)
{
    if (value > _ui->maxSliderEdit->GetCurrentValue()) _ui->maxSliderEdit->SetValue(value);

    emit rangeChanged();
}

void QRange::_maxChanged(double value)
{
    if (value < _ui->minSliderEdit->GetCurrentValue()) _ui->minSliderEdit->SetValue(value);

    emit rangeChanged();
}

void QRange::SetMainLabel(const QString &label) { _ui->mainLabel->setText(label); }

void QRange::SetDecimals(int dec)
{
    _ui->minSliderEdit->SetDecimals(dec);
    _ui->maxSliderEdit->SetDecimals(dec);
}
