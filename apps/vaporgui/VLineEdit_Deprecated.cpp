#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VLineEdit_Deprecated.h"
#include "ErrorReporter.h"

VLineEdit_Deprecated::VLineEdit_Deprecated(const std::string &value) : VHBoxWidget(), _value(value), _isDouble(false), _scientific(false), _menuEnabled(false), _decDigits(10)
{
    _lineEdit = new QLineEdit;
    SetValue(_value);
    layout()->addWidget(_lineEdit);

    connect(_lineEdit, &QLineEdit::editingFinished, this, &VLineEdit_Deprecated::emitLineEditChanged);
}

void VLineEdit_Deprecated::UseDoubleMenu()
{
    _menuEnabled = true;

    _lineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_lineEdit, &QLineEdit::customContextMenuRequested, this, &VLineEdit_Deprecated::ShowContextMenu);
}

void VLineEdit_Deprecated::SetValue(double value)
{
    VAssert(_isDouble);

    std::stringstream stream;
    if (_menuEnabled) {
        stream << std::fixed << std::setprecision(_decDigits);
        if (_scientific) stream << std::scientific;
    }
    stream << value << std::endl;
    _value = stream.str();

    _lineEdit->blockSignals(true);
    _lineEdit->setText(QString::fromStdString(_value));
    _lineEdit->blockSignals(false);
}

void VLineEdit_Deprecated::SetValue(const std::string &value)
{
    _value = value;

    _lineEdit->blockSignals(true);
    _lineEdit->setText(QString::fromStdString(_value));
    _lineEdit->blockSignals(false);
}

std::string VLineEdit_Deprecated::GetValue() const { return _value; }

void VLineEdit_Deprecated::SetIsDouble(bool isDouble) { _isDouble = isDouble; }

void VLineEdit_Deprecated::SetReadOnly(bool b) { _lineEdit->setReadOnly(b); }

void VLineEdit_Deprecated::emitLineEditChanged()
{
    std::string value = _lineEdit->text().toStdString();
    SetValue(value);
    emit ValueChanged(_value);
}

void VLineEdit_Deprecated::ShowContextMenu(const QPoint &pos)
{
    if (!_menuEnabled) return;

    QMenu menu;

    SpinBoxAction *decimalAction = new SpinBoxAction(tr("Decimal digits"), _decDigits);
    connect(decimalAction, &SpinBoxAction::editingFinished, this, &VLineEdit_Deprecated::_decimalDigitsChanged);
    menu.addAction(decimalAction);

    CheckBoxAction *checkBoxAction = new CheckBoxAction(tr("Scientific"), _scientific);
    connect(checkBoxAction, &CheckBoxAction::clicked, this, &VLineEdit_Deprecated::_scientificClicked);
    menu.addAction(checkBoxAction);

    QPoint globalPos = _lineEdit->mapToGlobal(pos);
    menu.exec(globalPos);
}

void VLineEdit_Deprecated::_decimalDigitsChanged(int value)
{
    _decDigits = value;
    SetValue(_value);
}

void VLineEdit_Deprecated::_scientificClicked(bool value)
{
    _scientific = value;
    SetValue(_value);
}
