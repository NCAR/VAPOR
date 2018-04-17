#include "QIntValidatorWithFixup.h"

void QIntValidatorWithFixup::fixup(QString &input) const {
    int val = input.toInt();
    if (val > top())
        val = top();
    else if (val < bottom())
        val = bottom();

    input = QString::number(val);
}
