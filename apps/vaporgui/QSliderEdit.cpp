#include "QSliderEdit.h"
#include "ui_QSliderEdit.h"

#include <cmath>
#include <iostream>
#include <cassert>

QSliderEdit::QSliderEdit(QWidget *parent) : QWidget(parent), _ui(new Ui::QSliderEdit)
{
    _ui->setupUi(this);

    _decimals = 4;
    _validator = new QDoubleValidator(_ui->myLineEdit);
    _ui->myLineEdit->setValidator(_validator);

    connect(_ui->mySlider, SIGNAL(valueChanged(int)),    // for update LineEdit
            this, SLOT(_mySlider_valueChanged(int)));
    connect(_ui->mySlider, SIGNAL(sliderReleased()),    // for emit a signal
            this, SLOT(_mySlider_released()));
    connect(_ui->myLineEdit, SIGNAL(editingFinished()),    // for emit a signal
            this, SLOT(_myLineEdit_valueChanged()));
}

QSliderEdit::~QSliderEdit()
{
    delete _ui;
    if (_validator) {
        delete _validator;
        _validator = NULL;
    }
}

void QSliderEdit::SetLabel(const QString &text) { _ui->myLabel->setText(text); }

void QSliderEdit::SetExtents(double min, double max)
{
    _min = min;
    _max = max;
    _ui->mySlider->setRange(std::floor(min), std::ceil(max));
}

void QSliderEdit::_lineEditSetValue(double dval)
{
    _ui->myLineEdit->blockSignals(true);
    if (_decimals > 0) {
        if (dval > _max)
            dval = _max;
        else if (dval < _min)
            dval = _min;
        _ui->myLineEdit->setText(QString::number(dval, 'g', _decimals));
    } else {
        assert(std::floor(dval) == dval);
        assert(dval >= _min);
        assert(dval <= _max);
        _ui->myLineEdit->setText(QString::number((long int)dval, 10));
    }
    _ui->myLineEdit->blockSignals(false);
}
void QSliderEdit::_mySlider_valueChanged(int value) { this->_lineEditSetValue((double)value); }

void QSliderEdit::_mySlider_released()
{
    // The value displayed in the LineEdit should be returned
    emit valueChanged(_ui->myLineEdit->text().toDouble());
}

void QSliderEdit::_myLineEdit_valueChanged()
{
    double val = _ui->myLineEdit->text().toDouble();
    bool   updateLineEdit = false;
    if (val > _max) {
        val = _max;
        updateLineEdit = true;
    } else if (val < _min) {
        val = _min;
        updateLineEdit = true;
    }
    if (updateLineEdit) this->_lineEditSetValue(val);

    _ui->mySlider->blockSignals(true);
    _ui->mySlider->setSliderPosition(std::round(val));
    _ui->mySlider->blockSignals(false);

    emit valueChanged(val);
}

void QSliderEdit::SetDecimals(int dec)
{
    if (dec > 0) {
        _decimals = dec;
        //_validator->setDecimals( dec );
    } else if (dec == 0) {
        // if the extents ARE essentially integers
        if (std::floor(_min) == _min && std::floor(_max) == _max) {
            _decimals = dec;
            //_validator->setDecimals( dec );
        } else {
            std::cerr << "QSliderEdit extents aren't integers while ZERO decimal is set" << std::endl;
            std::cerr << "_min, _max == " << _min << ", " << _max << std::endl;
        }

    } else
        // raise error
        ;
}

double QSliderEdit::GetCurrentValue() { return (_ui->myLineEdit->text().toDouble()); }

void QSliderEdit::SetValue(double value)
{
    this->_lineEditSetValue(value);

    if (value > _max)
        value = _max;
    else if (value < _min)
        value = _min;
    _ui->mySlider->blockSignals(true);
    _ui->mySlider->setSliderPosition(std::round(value));
    _ui->mySlider->blockSignals(false);
}
