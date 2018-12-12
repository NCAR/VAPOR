#include "QSliderEdit.h"
#include "ui_QSliderEdit.h"

#include <cmath>
#include <iostream>
#include <cassert>

QSliderEdit::QSliderEdit(QWidget *parent) : QWidget(parent), _ui(new Ui::QSliderEdit)
{
    _ui->setupUi(this);

    _combo = new Combo(_ui->myLineEdit, _ui->mySlider);
    _combo->SetPrecision(5);

    connect(_combo, SIGNAL(valueChanged(double)), this, SLOT(_comboValueChanged(double)));
    connect(_combo, SIGNAL(valueChanged(int)), this, SLOT(_comboValueChanged(int)));
}

QSliderEdit::~QSliderEdit()
{
    delete _combo;
    delete _ui;
}

void QSliderEdit::SetLabel(const QString &text) { _ui->myLabel->setText(text); }

void QSliderEdit::SetExtents(double min, double max)
{
    double value = GetCurrentValue();
    _combo->Update(min, max, value);
}

void QSliderEdit::SetDecimals(int dec) { _combo->SetPrecision(dec); }

double QSliderEdit::GetCurrentValue() { return _combo->GetValue(); }

void QSliderEdit::SetValue(double value) { _combo->SetSliderLineEdit(value); }

void QSliderEdit::_comboValueChanged(double val) { emit valueChanged(val); }

void QSliderEdit::_comboValueChanged(int val) { emit valueChanged(val); }

void QSliderEdit::SetIntType(bool val) { _combo->SetIntType(val); }
