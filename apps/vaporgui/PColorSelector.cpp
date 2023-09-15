#include "PColorSelector.h"
#include "QColorWidget.h"
#include <vector>
#include <vapor/ParamsBase.h>

PColorSelector::PColorSelector(const std::string &tag, const std::string &label) : PLineItem(tag, label, _colorWidget = new QColorWidget)
{
    connect(_colorWidget, SIGNAL(colorChanged(QColor)), this, SLOT(colorChanged(QColor)));
}

PColorSelector *PColorSelector::ShowAlpha()
{
    _colorWidget->ShowAlpha = true;
    return this;
}

QColor PColorSelector::VectorToQColor(const std::vector<double> &v)
{
    if (v.size() == 3) return QColor::fromRgbF(v[0], v[1], v[2]);
    if (v.size() == 4) return QColor::fromRgbF(v[0], v[1], v[2], v[3]);
    return QColor::fromRgbF(0, 0, 0);
}

std::vector<double> PColorSelector::QColorToVector(const QColor &c)
{
    std::vector<double> v(4, 0);
    v[0] = c.redF();
    v[1] = c.greenF();
    v[2] = c.blueF();
    v[3] = c.alphaF();
    return v;
}

void PColorSelector::updateGUI() const
{
    QColor color = VectorToQColor(getParams()->GetValueDoubleVec(getTag()));
    _colorWidget->setColor(color);
}

void PColorSelector::colorChanged(QColor color) { getParams()->SetValueDoubleVec(getTag(), "", QColorToVector(color)); }
