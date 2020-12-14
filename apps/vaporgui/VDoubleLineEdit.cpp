#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VDoubleLineEdit.h"

VDoubleLineEdit::VDoubleLineEdit(double value) : VNumericLineEdit(), _value(value)
{
    std::string formattedNumber = _formatValue(_value);
    SetValueString(formattedNumber);
}

void VDoubleLineEdit::SetValueDouble(double value)
{
    std::string formattedNumber = _formatValue(value);

    try {
        _value = std::stod(formattedNumber);
    } catch (const std::invalid_argument &) {
        return;
    } catch (const std::out_of_range &) {
        return;
    }

    SetValueString(formattedNumber);
}

double VDoubleLineEdit::GetValueDouble() const { return _value; }

void VDoubleLineEdit::SetRange(double min, double max)
{
    _min = min;
    _max = max;
}

void VDoubleLineEdit::_valueChanged()
{
    std::string str = _getText();

    double value;
    try {
        value = std::stod(str);

        value = std::min(_max, std::max(_min, value));

        // If value changed, update and emit, otherwiese revert to old value
        if (value != _value) {
            SetValueDouble(value);
            emit ValueChanged(_value);
        } else {
            SetValueDouble(_value);
        }
    } catch (const std::invalid_argument &) {
        SetValueDouble(_value);
    } catch (const std::out_of_range &) {
        SetValueDouble(_value);
    }
}

std::string VDoubleLineEdit::_formatValue(double value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(_decimalDigits);
    if (_sciNotation) {
        stream << std::scientific;
        stream << value;
    } else {
        stream << value;
    }
    return stream.str();
}
