#include "VGeometry.h"

VGeometry::VGeometry(QWidget *parent, int dim, const std::vector<float> &range) : QWidget(parent)
{
    VAssert(dim == 2 || dim == 3);
    VAssert(range.size() == dim * 2);
    for (int i = 0; i < dim; i++) VAssert(range[i * 2] < range[i * 2 + 1]);

    _dim = dim;
    _xrange = new VRange(this, range[0], range[1], "XMin", "XMax");
    _yrange = new VRange(this, range[2], range[3], "YMin", "YMax");
    if (_dim == 3)
        _zrange = new VRange(this, range[4], range[5], "ZMin", "ZMax");
    else    // Create anyway. Will be hidden though.
    {
        _zrange = new VRange(this, 0.0f, 100.0f, "ZMin", "ZMax");
        _zrange->hide();
    }

    connect(_xrange, SIGNAL(_rangeChanged()), this, SLOT(_respondChanges()));
    connect(_yrange, SIGNAL(_rangeChanged()), this, SLOT(_respondChanges()));
    connect(_zrange, SIGNAL(_rangeChanged()), this, SLOT(_respondChanges()));

    _layout = new QVBoxLayout(this);
    _layout->addWidget(_xrange);
    _layout->addWidget(_yrange);
    _layout->addWidget(_zrange);
}

void VGeometry::SetDimAndRange(int dim, const std::vector<float> &range)
{
    VAssert(dim == 2 || dim == 3);
    VAssert(range.size() == dim * 2);
    for (int i = 0; i < dim; i++) VAssert(range[i * 2] < range[i * 2 + 1]);

    /* Adjust the appearance if necessary */
    if (_dim == 2 && dim == 3)
        _zrange->show();
    else if (_dim == 3 && dim == 2)
        _zrange->hide();
    _dim = dim;

    _xrange->SetRange(range[0], range[1]);
    _yrange->SetRange(range[2], range[3]);
    if (_dim == 3) _zrange->SetRange(range[4], range[5]);
}

void VGeometry::SetCurrentValues(const std::vector<float> &vals)
{
    VAssert(vals.size() == _dim * 2);
    for (int i = 0; i < _dim; i++) VAssert(vals[i * 2] < vals[i * 2 + 1]);

    /* VRange widgets will only respond to values within their ranges */
    _xrange->SetCurrentValLow(vals[0]);
    _xrange->SetCurrentValHigh(vals[1]);
    _yrange->SetCurrentValLow(vals[2]);
    _yrange->SetCurrentValHigh(vals[3]);
    if (_dim == 3) {
        _zrange->SetCurrentValLow(vals[4]);
        _zrange->SetCurrentValHigh(vals[5]);
    }
}

void VGeometry::GetCurrentValues(std::vector<float> &vals) const
{
    vals.resize(_dim * 2, 0.0f);
    _xrange->GetCurrentValRange(vals[0], vals[1]);
    _yrange->GetCurrentValRange(vals[2], vals[3]);
    if (_dim == 3) _zrange->GetCurrentValRange(vals[4], vals[5]);
}

void VGeometry::_respondChanges() { emit _geometryChanged(); }
