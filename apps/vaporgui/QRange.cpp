#include "QRange.h"
#include "ui_QRange.h"

QRange::QRange(QWidget *parent) : QWidget(parent), _ui(new Ui::QRange)
{
    _ui->setupUi(this);

    _ui->minSliderEdit->SetLabel(QString("Min:"));
    _ui->maxSliderEdit->SetLabel(QString("Max:"));

    connect(_ui->minSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_minChanged(double)));
    connect(_ui->maxSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_maxChanged(double)));
    connect(_ui->minSliderEdit, SIGNAL(valueChanged(int)), this, SLOT(_minChanged(int)));
    connect(_ui->maxSliderEdit, SIGNAL(valueChanged(int)), this, SLOT(_maxChanged(int)));
}

QRange::~QRange() { delete _ui; }

void QRange::SetExtents(double min, double max)
{
    _ui->minSliderEdit->SetExtents(min, max);
    _ui->maxSliderEdit->SetExtents(min, max);
    _ui->minSliderEdit->SetValue(min);
    _ui->maxSliderEdit->SetValue(max);
}

void QRange::GetValue(double &min, double &max)
{
    min = _ui->minSliderEdit->GetCurrentValue();
    max = _ui->maxSliderEdit->GetCurrentValue();
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

void QRange::_minChanged(int value)
{
    if ((double)value > _ui->maxSliderEdit->GetCurrentValue()) _ui->maxSliderEdit->SetValue((double)value);
    emit rangeChanged();
}

void QRange::_maxChanged(int value)
{
    if ((double)value < _ui->minSliderEdit->GetCurrentValue()) _ui->minSliderEdit->SetValue((double)value);
    emit rangeChanged();
}

void QRange::SetMainLabel(const QString &label) { _ui->mainLabel->setText(label); }

void QRange::SetDecimals(int dec)
{
    _ui->minSliderEdit->SetDecimals(dec);
    _ui->maxSliderEdit->SetDecimals(dec);
}

void QRange::SetIntType(bool val)
{
    _ui->minSliderEdit->SetIntType(val);
    _ui->maxSliderEdit->SetIntType(val);
}

void QRange::SetValue(double smallVal, double bigVal)
{
    _ui->minSliderEdit->SetValue(smallVal);
    _ui->maxSliderEdit->SetValue(bigVal);
}
