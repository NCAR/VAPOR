#include "VIntRangeMenu.h"
#include "VActions.h"

VIntRangeMenu::VIntRangeMenu(QWidget *parent, bool sciNotation, int decimalDigits, int min, int max, bool rangeChangable)
: VNumericFormatMenu(parent, sciNotation, decimalDigits), _minRangeAction(new VIntLineEditAction("Minimum value", min)), _maxRangeAction(new VIntLineEditAction("Maximum value", max))
{
    connect(_minRangeAction, &VIntLineEditAction::ValueChanged, this, &VIntRangeMenu::_minChanged);
    addAction(_minRangeAction);

    connect(_maxRangeAction, &VIntLineEditAction::ValueChanged, this, &VIntRangeMenu::_maxChanged);
    addAction(_maxRangeAction);

    if (!rangeChangable) {
        _minRangeAction->setEnabled(false);
        _maxRangeAction->setEnabled(false);
    }
}

void VIntRangeMenu::AllowUserRange(bool allowed)
{
    _minRangeAction->setEnabled(allowed);
    _maxRangeAction->setEnabled(allowed);
}

void VIntRangeMenu::SetMinimum(int min) { _minRangeAction->SetValue(min); }

void VIntRangeMenu::SetMaximum(int max) { _maxRangeAction->SetValue(max); }

void VIntRangeMenu::_minChanged(int min) { emit MinChanged(min); }

void VIntRangeMenu::_maxChanged(int max) { emit MaxChanged(max); }
