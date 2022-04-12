#include <QWidget>

#include "VActions.h"
#include "VDoubleSliderEditMenu.h"

// clang-format off

VDoubleSliderEditMenu::VDoubleSliderEditMenu(QWidget *parent) : 
VDoubleRangeMenu( parent, false, 2, 0., 10., false)
{
    _dynamicUpdateAction = new VCheckBoxAction("Dynamic update enabled", false);
    connect(_dynamicUpdateAction, &VCheckBoxAction::clicked, this, &VDoubleSliderEditMenu::_dynamicUpdateChanged);
    addAction(_dynamicUpdateAction);
    _dynamicUpdateAction->setEnabled(false);
}

void VDoubleSliderEditMenu::AllowDynamicUpdate() const { _dynamicUpdateAction->setEnabled(true); }

void VDoubleSliderEditMenu::SetDynamicUpdate(bool dynamicUpdateEnabled) { _dynamicUpdateAction->SetValue(dynamicUpdateEnabled); }

void VDoubleSliderEditMenu::_dynamicUpdateChanged(bool dynamicUpdateEnabled) { emit DynamicUpdateChanged(dynamicUpdateEnabled); }

// clang-format on
