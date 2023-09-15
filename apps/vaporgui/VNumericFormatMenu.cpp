#include <string>
#include <sstream>
#include <iomanip>

#include <QWidget>

#include "VActions.h"
#include "VNumericFormatMenu.h"

VNumericFormatMenu::VNumericFormatMenu(QWidget *parent, bool sciNotation, int decimalDigits)
: QMenu(parent), _sciNotationAction(new VCheckBoxAction("Scientific notation", sciNotation)), _decimalAction(new VSpinBoxAction("Decimal digits", decimalDigits))
{
    connect(_sciNotationAction, &VCheckBoxAction::clicked, this, &VNumericFormatMenu::_sciNotationChanged);
    addAction(_sciNotationAction);

    connect(_decimalAction, &VSpinBoxAction::editingFinished, this, &VNumericFormatMenu::_decimalDigitsChanged);
    addAction(_decimalAction);
}

void VNumericFormatMenu::SetDecimalDigits(int digits) { _decimalAction->SetValue(digits); }

void VNumericFormatMenu::SetSciNotation(bool sciNotation) { _sciNotationAction->SetValue(sciNotation); }

void VNumericFormatMenu::_decimalDigitsChanged(int digits) { emit DecimalDigitsChanged(digits); }

void VNumericFormatMenu::_sciNotationChanged(bool sciNotation) { emit SciNotationChanged(sciNotation); }
