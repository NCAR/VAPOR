#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VNumericLineEdit.h"

//! \class VIntLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type int,
//! and provides Vapor's standard setters, getters, and signals

class VIntLineEdit : public VNumericLineEdit {
    Q_OBJECT

public:
    VIntLineEdit(int value = 0);

    //! Set the current int value in the line edit
    void SetValueInt(int value);

    //! Get the current int value in the line iedit
    int GetValueInt() const;

protected:
    virtual void _valueChanged();

    std::string _formatValue(int value);
    bool        _checkOverflow(double value);

    int _value;

signals:
    void ValueChanged(int value);
};
