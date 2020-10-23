#include "VDoubleRangeMenu.h"
#include "VActions.h"

VDoubleRangeMenu::VDoubleRangeMenu(QWidget *parent, bool sciNotation, double decimalDigits, double min, double max, bool rangeChangable)
: VNumericFormatMenu(parent, sciNotation, decimalDigits), _minRangeAction(new VDoubleLineEditAction("Minimum value", min)), _maxRangeAction(new VDoubleLineEditAction("Maximum value", max))
{
    connect(_minRangeAction, &VDoubleLineEditAction::ValueChanged, this, &VDoubleRangeMenu::_minChanged);
    addAction(_minRangeAction);

    connect(_maxRangeAction, &VDoubleLineEditAction::ValueChanged, this, &VDoubleRangeMenu::_maxChanged);
    addAction(_maxRangeAction);

    if (!rangeChangable) {
        _minRangeAction->setEnabled(false);
        _maxRangeAction->setEnabled(false);
    }
}

void VDoubleRangeMenu::AllowUserRange(bool allowed)
{
    _minRangeAction->setEnabled(allowed);
    _maxRangeAction->setEnabled(allowed);
}

void VDoubleRangeMenu::SetMinimum(double min) { _minRangeAction->SetValue(min); }

void VDoubleRangeMenu::SetMaximum(double max) { _maxRangeAction->SetValue(max); }

void VDoubleRangeMenu::_minChanged(double min) { emit MinChanged(min); }

void VDoubleRangeMenu::_maxChanged(double max) { emit MaxChanged(max); }
