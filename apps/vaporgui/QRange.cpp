#include "QRange.h"
#include "ui_QRange.h"

QRange::QRange(QWidget *parent) : QWidget(parent), _ui(new Ui::QRange)
{
    _ui->setupUi(this);

    _ui->minSliderEdit->SetText(QString::fromAscii("min:"));
    _ui->maxSliderEdit->SetText(QString::fromAscii("max:"));

    connect(_ui->minSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_minChanged(double)));
    connect(_ui->maxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_maxChanged(double)));
}

QRange::~QRange() { delete _ui; }

void QRange::SetExtents(double min, double max)
{
    _ui->minSliderEdit->SetExtents(min, max);
    _ui->maxSliderEdit->SetExtents(min, max);
}

void QRange::GetRange(double range[2])
{
    range[0] = _ui->minSliderEdit->GetCurrentValue();
    range[1] = _ui->maxSliderEdit->GetCurrentValue();
}

void QRange::_minChanged(double value)
{
    if (value > _ui->maxSliderEdit->GetCurrentValue()) _ui->maxSliderEdit->SetValue(value);

    emit RangeChanged();
}

void QRange::_maxChanged(double value)
{
    if (value < _ui->minSliderEdit->GetCurrentValue()) _ui->minSliderEdit->SetValue(value);

    emit RangeChanged();
}
