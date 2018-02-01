#include "qsinglepoint.h"
#include "ui_qsinglepoint.h"

// for debug
#include <iostream>

QSinglePoint::QSinglePoint(QWidget *parent) : QWidget(parent), _ui(new Ui::QSinglePoint)
{
    _ui->setupUi(this);

    _ui->_xSliderEdit->SetExtents(50.0, 100.0);
    _ui->_ySliderEdit->SetExtents(50.5, 100.5);

    // Connect signals and slots
    connect(_ui->_xSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_pointChanged(double)));
}

QSinglePoint::~QSinglePoint() { delete _ui; }

void QSinglePoint::_pointChanged(double val)    // val isn't used though
{
    std::cout << val << std::endl;
}
