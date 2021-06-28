#include "V3DIntInput.h"
#include <QHBoxLayout>
#include <QLabel>
#include "VIntLineEdit.h"
#include <vapor/VAssert.h>

V3DIntInput::V3DIntInput()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    layout->addWidget(new QLabel("X"));
    layout->addWidget(_x = new VIntLineEdit);
    layout->addWidget(new QLabel("Y"));
    layout->addWidget(_y = new VIntLineEdit);
    layout->addWidget(new QLabel("Z"));
    layout->addWidget(_z = new VIntLineEdit);

    for (auto input : {_x, _y, _z}) QObject::connect(input, &VIntLineEdit::ValueChanged, this, &V3DIntInput::axisValueChanged);
}

void V3DIntInput::SetValue(int x, int y, int z)
{
    _x->SetValueInt(x);
    _y->SetValueInt(y);
    _z->SetValueInt(z);
}

void V3DIntInput::SetValue(const std::vector<int> &xyz)
{
    VAssert(xyz.size() == 3);
    SetValue(xyz[0], xyz[1], xyz[2]);
}

void V3DIntInput::axisValueChanged(int)
{
    int x = _x->GetValueInt();
    int y = _y->GetValueInt();
    int z = _z->GetValueInt();

    emit ValueChanged(x, y, z);
    emit ValueChangedVec(std::vector<int>{x, y, z});
}
