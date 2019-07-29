#include "OSPRayEnableCheckbox.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>

using namespace VAPoR;

OSPRayEnableCheckbox::OSPRayEnableCheckbox(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    this->setLayout(layout);
    
    QLabel *label = new QLabel("OSPRay");
    QSpacerItem *spacer = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    _checkBox = new QCheckBox();
    connect(_checkBox, SIGNAL(clicked(bool)), this, SLOT(checkbox_clicked(bool)));
    
    layout->addWidget(label);
    layout->addItem(spacer);
    layout->addWidget(_checkBox);
}

void OSPRayEnableCheckbox::Update(VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _checkBox->setChecked(rp->GetValueLong(RenderParams::OSPRayEnabledTag, false));
}

void OSPRayEnableCheckbox::checkbox_clicked(bool checked)
{
    if (_renderParams)
        _renderParams->SetValueLong(RenderParams::OSPRayEnabledTag, RenderParams::OSPRayEnabledTag, checked);
}
