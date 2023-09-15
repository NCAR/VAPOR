#pragma once

#include <QDoubleValidator>

//! \class VDoubleValidator
//! Use as a direct replacement of QDoubleValidator.
//! They behave the same except QDoubleValidator does not accept values outside
//! the range while VDoubleValidator does not allow the user to input
//! values outside of the range. This solves some quirks with QLineEdit
//! With QDoubleValidator, the user can type a value outside the range in the text
//! box but the text box will say the value is invalid and it will stop working
//! VDoubleValidator, will not allow the user to type in a value outside the range
//! omitting the above problem.

class VDoubleValidator : public QDoubleValidator {
public:
    using QDoubleValidator::QDoubleValidator;
    virtual void              fixup(QString &input) const override;
    virtual QValidator::State validate(QString &input, int &pos) const override;
};
