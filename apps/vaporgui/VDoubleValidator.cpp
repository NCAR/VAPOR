#include "VDoubleValidator.h"

void VDoubleValidator::fixup(QString &input) const
{
    bool   ok;
    double v = locale().toDouble(input, &ok);
    if (ok) {
        if (v > top()) input.setNum(top());
        if (v < bottom()) input.setNum(bottom());
    }
}

QValidator::State VDoubleValidator::validate(QString &input, int &pos) const
{
    State s = QDoubleValidator::validate(input, pos);

    if (s == State::Intermediate) {
        bool   ok;
        double v = locale().toDouble(input, &ok);
        if (ok)
            if (v > top() || v < bottom()) s = State::Invalid;
    }

    return s;
}
