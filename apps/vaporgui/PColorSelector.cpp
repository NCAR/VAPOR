#include "PColorSelector.h"
#include "QColorWidget.h"
#include <vector>
#include <vapor/ParamsBase.h>

PColorSelector::PColorSelector(const std::string &tag, const std::string &label) : PLineItem(tag, _colorWidget = new QColorWidget)
{
    connect(_colorWidget, SIGNAL(colorChanged(QColor)), this, SLOT(colorChanged(QColor)));
}

QColor PColorSelector::VectorToQColor(const std::vector<double> &v)
{
    if (v.size() != 3) return QColor::fromRgb(0, 0, 0);
    return QColor::fromRgb(v[0] * 255, v[1] * 255, v[2] * 255);
}

std::vector<double> PColorSelector::QColorToVector(const QColor &c)
{
    std::vector<double> v(3, 0);
    v[0] = c.redF();
    v[1] = c.greenF();
    v[2] = c.blueF();
    return v;
}

void PColorSelector::updateGUI() const
{
    QColor color = VectorToQColor(getParams()->GetValueDoubleVec(GetTag()));
    _colorWidget->setColor(color);
}

void PColorSelector::colorChanged(QColor color) { getParams()->SetValueDoubleVec(GetTag(), GetTag(), QColorToVector(color)); }
