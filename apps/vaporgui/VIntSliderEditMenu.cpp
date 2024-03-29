#include <QWidget>

#include "VActions.h"
#include "VIntSliderEditMenu.h"

// clang-format off

VIntSliderEditMenu::VIntSliderEditMenu( QWidget *parent ) : 
VIntRangeMenu( parent, false, 2, 0., 10., false )
{
    _dynamicUpdateAction = new VCheckBoxAction("Dynamic update enabled", false);
    connect(_dynamicUpdateAction, &VCheckBoxAction::clicked, this, &VIntSliderEditMenu::_dynamicUpdateChanged);
    addAction(_dynamicUpdateAction);
    _dynamicUpdateAction->setEnabled(false);
}

void VIntSliderEditMenu::AllowDynamicUpdate() const { _dynamicUpdateAction->setEnabled(true); }

void VIntSliderEditMenu::SetDynamicUpdate(bool dynamicUpdateEnabled) { _dynamicUpdateAction->SetValue(dynamicUpdateEnabled); }

void VIntSliderEditMenu::_dynamicUpdateChanged(bool dynamicUpdateEnabled) { emit DynamicUpdateChanged(dynamicUpdateEnabled); }

// clang-format on
