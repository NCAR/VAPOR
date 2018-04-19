#ifndef QINTVALIDATORWITHFIXUP_H
#define QINTVALIDATORWITHFIXUP_H

//
// This is a derived QIntValidator that implements the fixup function.
//

#include <QIntValidator>

class QIntValidatorWithFixup : public QIntValidator {
public:
    explicit QIntValidatorWithFixup(QObject *parent = 0) : QIntValidator(parent = 0) {}
    explicit QIntValidatorWithFixup(int bottom, int top, QObject *parent = 0) : QIntValidator(bottom, top, parent = 0) {}

    //
    // overload fixup() from QIntValidator
    //
    virtual void fixup(QString &input) const override;
};

#endif
