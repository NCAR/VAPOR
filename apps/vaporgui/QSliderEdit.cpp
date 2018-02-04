#include "QSliderEdit.h"
#include "ui_QSliderEdit.h"

#include <cmath>
#include <iostream>

QSliderEdit::QSliderEdit(QWidget *parent) : QWidget(parent), _ui(new Ui::QSliderEdit)
{
    _ui->setupUi(this);

    _decimals = 2;
    _validator = new QDoubleValidator2(_ui->myLineEdit);
    _validator->setDecimals(_decimals);
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

void QSliderEdit::SetText(const QString &text) { _ui->myLabel->setText(text); }

void QSliderEdit::SetExtents(double min, double max)
{
    _validator->setBottom(min);
    _validator->setTop(max);
    _ui->mySlider->setRange(std::floor(min), std::ceil(max));
}

void QSliderEdit::_mySlider_valueChanged(int value)
{
    QString text = QString::number(value, 'g', _decimals);
    _validator->fixup(text);
    _ui->myLineEdit->blockSignals(true);
    _ui->myLineEdit->setText(text);
    _ui->myLineEdit->blockSignals(false);
}

void QSliderEdit::_mySlider_released()
{
    // The value displayed in the LineEdit should be returned
    emit valueChanged(_ui->myLineEdit->text().toDouble());
}

void QSliderEdit::_myLineEdit_valueChanged()
{
    double val = _ui->myLineEdit->text().toDouble();

    _ui->mySlider->blockSignals(true);
    _ui->mySlider->setSliderPosition(std::round(val));
    _ui->mySlider->blockSignals(false);

    emit valueChanged(val);
}

void QSliderEdit::SetDecimals(int dec)
{
    if (dec > 0) {
        _decimals = dec;
        _validator->setDecimals(dec);
    } else if (dec == 0) {
        // if the extents ARE essentially integers
        if (std::floor(_validator->top()) == _validator->top() && std::floor(_validator->bottom()) == _validator->bottom()) {
            _decimals = dec;
            _validator->setDecimals(dec);
        } else
            std::cerr << "QSliderEdit extents aren't integers while ZERO decimal is set" << std::endl;

    } else
        // raise error
        ;
}

double QSliderEdit::GetCurrentValue() { return (_ui->myLineEdit->text().toDouble()); }

void QSliderEdit::SetValue(double value)
{
    QString text = QString::number(value);
    _validator->fixup(text);
    _ui->myLineEdit->blockSignals(true);
    _ui->myLineEdit->setText(text);
    _ui->myLineEdit->blockSignals(false);

    double val = _ui->myLineEdit->text().toDouble();
    _ui->mySlider->blockSignals(true);
    _ui->mySlider->setSliderPosition(std::round(val));
    _ui->mySlider->blockSignals(false);
}
