#ifndef QDOUBLEVALIDATOR2_H
#define QDOUBLEVALIDATOR2_H

//
// This is a derived QDoubleValidator that also implements the fixup function.
//

#include <QDoubleValidator>
#include <iostream>

class QDoubleValidator2 : public QDoubleValidator {
public:
    QDoubleValidator2(QObject *parent = 0) : QDoubleValidator(parent = 0) {}
    QDoubleValidator2(double bottom, double top, int decimals, QObject *parent = 0) : QDoubleValidator(bottom, top, decimals, parent = 0) {}

    //
    // overload fixup() from QDoubleValidator
    //
    virtual void fixup(QString &input) const override
    {
        double val = input.toDouble();
        if (val > top())
            val = top();
        else if (val < bottom())
            val = bottom();

        input = QString::number(val);
    }
};

#endif
