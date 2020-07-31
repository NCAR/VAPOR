#include <QLineEdit>

#include "VNumericFormatMenu.h"
#include "VNumericLineEdit.h"

VNumericLineEdit::VNumericLineEdit(int decimals) : VStringLineEdit(), _sciNotation(false), _decimalDigits(decimals)
{
    _menu = new VNumericFormatMenu(this, _sciNotation, _decimalDigits);

    connect(_menu, &VNumericFormatMenu::SciNotationChanged, this, &VNumericLineEdit::SetSciNotation);
    connect(_menu, &VNumericFormatMenu::DecimalDigitsChanged, this, &VNumericLineEdit::SetNumDigits);
    SetCustomContextMenu();
}

int VNumericLineEdit::GetNumDigits() const { return _decimalDigits; }

void VNumericLineEdit::SetNumDigits(int digits)
{
    _decimalDigits = digits;
    _valueChanged();
    emit DecimalDigitsChanged(_decimalDigits);
}

bool VNumericLineEdit::GetSciNotation() const { return _sciNotation; }

void VNumericLineEdit::SetSciNotation(bool sciNotation)
{
    _sciNotation = sciNotation;
    _valueChanged();
    emit SciNotationChanged(_sciNotation);
}

void VNumericLineEdit::_showMenu(const QPoint &pos)
{
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
};
