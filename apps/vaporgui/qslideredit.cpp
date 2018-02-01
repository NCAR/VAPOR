#include "qslideredit.h"
#include "ui_qslideredit.h"

#include <cmath>
#include <iostream>

QSliderEdit::QSliderEdit(QWidget *parent) : QWidget(parent), _ui(new Ui::QSliderEdit)
{
    _ui->setupUi(this);

    _validator = new QDoubleValidator2(_ui->_myLineEdit);
    _validator->setDecimals(4);
    _ui->_myLineEdit->setValidator(_validator);

    // Connect slots
    connect(_ui->_mySlider, SIGNAL(valueChanged(int)),    // for update LineEdit
            this, SLOT(_mySlider_valueChanged(int)));
    connect(_ui->_mySlider, SIGNAL(sliderReleased()),    // for emit a signal
            this, SLOT(_mySlider_released()));
    connect(_ui->_myLineEdit, SIGNAL(returnPressed()),    // for emit a signal
            this, SLOT(_myLineEdit_valueChanged()));
}

QSliderEdit::~QSliderEdit() { delete _ui; }

void QSliderEdit::SetText(std::string &text) { _ui->_myLabel->setText(QString::fromStdString(text)); }

void QSliderEdit::SetExtents(double min, double max)
{
    _validator->setBottom(min);
    _validator->setTop(max);
    _ui->_mySlider->setRange(std::floor(min), std::ceil(max));
}

void QSliderEdit::_mySlider_valueChanged(int value)
{
    QString text = QString::number(value);
    _validator->fixup(text);
    _ui->_myLineEdit->blockSignals(true);
    _ui->_myLineEdit->setText(text);
    _ui->_myLineEdit->blockSignals(false);
}

void QSliderEdit::_mySlider_released() { emit valueChanged(_ui->_myLineEdit->text().toDouble()); }

void QSliderEdit::_myLineEdit_valueChanged()
{
    double val = _ui->_myLineEdit->text().toDouble();

    _ui->_mySlider->blockSignals(true);
    _ui->_mySlider->setSliderPosition(std::round(val));
    _ui->_mySlider->blockSignals(false);

    emit valueChanged(val);
}

void QSliderEdit::SetDecimals(int dec)
{
    if (dec > 0)
        _validator->setDecimals(dec);
    else if (dec == 0) {
        // if the extents ARE essentially integers
        if (std::floor(_validator->top()) == _validator->top() && std::floor(_validator->bottom()) == _validator->bottom())
            _validator->setDecimals(dec);
        else
            std::cerr << "QSliderEdit extents aren't integers while ZERO decimal is set" << std::endl;

    } else
        // raise error
        ;
}

double QSliderEdit::GetCurrentVal() { return (_ui->_myLineEdit->text().toDouble()); }
