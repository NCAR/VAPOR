#pragma once

#include <string>

//#include <QWidget>
#include <QRadioButton>

#include "VHBoxWidget.h"

class QRadioButton;

//! class VRadioButton
//!
//! Wraps a QRadioButton and provides vaporgui's standard setter/getter functions
//! and signals.

class VRadioButton : public VHBoxWidget {
    Q_OBJECT

public:
    VRadioButton(bool value = false);

    void SetValue(bool value);

    bool GetValue() const;

    std::string GetText() const;

private:
    QRadioButton *_radioButton;

public slots:
    void emitRadioButtonChanged(bool checked);

signals:
    void ValueChanged(bool checked);
};
