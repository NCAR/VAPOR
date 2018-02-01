#include "QSinglePoint.h"
#include "ui_QSinglePoint.h"

// for debug
#include <iostream>

QSinglePoint::QSinglePoint(QWidget *parent) : QWidget(parent), _ui(new Ui::QSinglePoint)
{
    _ui->setupUi(this);

    _ui->_xSliderEdit->SetText(QString::fromAscii("X"));
    _ui->_ySliderEdit->SetText(QString::fromAscii("Y"));
    _ui->_zSliderEdit->SetText(QString::fromAscii("Z"));
    _ui->_tSliderEdit->SetText(QString::fromAscii("T"));

    // Connect signals and slots
    connect(_ui->_xSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_coordinateChanged(double)));
    connect(_ui->_ySliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_coordinateChanged(double)));
    connect(_ui->_zSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_coordinateChanged(double)));
    connect(_ui->_tSliderEdit, SIGNAL(valueChanged(double)), this, SLOT(_coordinateChanged(double)));

    // debug
    _ui->_xSliderEdit->SetExtents(50.0, 100.0);
    _ui->_ySliderEdit->SetExtents(50.5, 100.5);
}

QSinglePoint::~QSinglePoint() { delete _ui; }

void QSinglePoint::_coordinateChanged(double value)    // value isn't used though
{
    std::cout << value << std::endl;
    emit PointUpdated();
}

void QSinglePoint::SetDimensionality(int dim) { _dimensionality = dim; }

int QSinglePoint::GetDimensionality() { return _dimensionality; }

void QSinglePoint::GetCurrentPoint(std::vector<double> &point)
{
    point.clear();
    point.push_back(_ui->_xSliderEdit->GetCurrentValue());
    point.push_back(_ui->_ySliderEdit->GetCurrentValue());
    if (_dimensionality >= 3) point.push_back(_ui->_zSliderEdit->GetCurrentValue());
    if (_dimensionality >= 4) point.push_back(_ui->_tSliderEdit->GetCurrentValue());
}
