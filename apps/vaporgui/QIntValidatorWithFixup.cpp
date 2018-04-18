#include "QIntValidatorWithFixup.h"

void QIntValidatorWithFixup::fixup(QString &input) const
{
    QIntValidator::fixup(input);    // make sure the locale is correct

    int val = input.toInt();
    if (val > top()) {
        val = top();
        input = QString::number(val);
    } else if (val < bottom()) {
        val = bottom();
        input = QString::number(val);
    }
}
