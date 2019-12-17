#pragma once

#include <QDoubleValidator>

//! \class VDoubleValidator
//! Use as a direct replacement of QDoubleValidator.

class VDoubleValidator : public QDoubleValidator {
public:
    using QDoubleValidator::QDoubleValidator;
    virtual void              fixup(QString &input) const override;
    virtual QValidator::State validate(QString &input, int &pos) const override;
};
