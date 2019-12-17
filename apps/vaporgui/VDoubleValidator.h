#pragma once

#include <QDoubleValidator>

//! \class VDoubleValidator
//! Use as a direct replacement of QDoubleValidator.
//! They behave the same except QDoubleValidator does not accept values outside
//! the range while VDoubleValidator does not allow the user to input
//! values outside of the range. This solves some quirks with QLineEdit

class VDoubleValidator : public QDoubleValidator {
public:
    using QDoubleValidator::QDoubleValidator;
    virtual void              fixup(QString &input) const override;
    virtual QValidator::State validate(QString &input, int &pos) const override;
};
