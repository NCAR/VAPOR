#include "V3DInput.h"
#include <QHBoxLayout>
#include <QLabel>
#include "VDoubleLineEdit.h"
#include <vapor/VAssert.h>

V3DInput::V3DInput()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    layout->addWidget(new QLabel("X"));
    layout->addWidget(_x = new VDoubleLineEdit);
    layout->addWidget(new QLabel("Y"));
    layout->addWidget(_y = new VDoubleLineEdit);
    layout->addWidget(new QLabel("Z"));
    layout->addWidget(_z = new VDoubleLineEdit);

    for (auto input : {_x, _y, _z}) QObject::connect(input, &VDoubleLineEdit::ValueChanged, this, &V3DInput::axisValueChanged);
}

void V3DInput::SetValue(double x, double y, double z)
{
    _x->SetValueDouble(x);
    _y->SetValueDouble(y);
    _z->SetValueDouble(z);
}

void V3DInput::SetValue(const std::vector<double> &xyz)
{
    VAssert(xyz.size() == 3);
    SetValue(xyz[0], xyz[1], xyz[2]);
}

void V3DInput::axisValueChanged(double)
{
    double x = _x->GetValueDouble();
    double y = _y->GetValueDouble();
    double z = _z->GetValueDouble();

    emit ValueChanged(x, y, z);
    emit ValueChangedVec(std::vector<double>{x, y, z});
}
