#pragma once

#include <string>

#include <QWidget>
#include <QCheckBox>

#include "VHBoxWidget.h"

//! class VCheckBox
//!
//! Wraps a QCheckBox and provides vaporgui's standard setter/getter functions
//! and signals.

class VCheckBox : public VHBoxWidget {
    Q_OBJECT

public:
    VCheckBox(bool value = false);

    void SetValue(bool value);

    bool GetValue() const;

private:
    QCheckBox *_checkBox;

public slots:
    void emitCheckBoxChanged(bool checked);

signals:
    void ValueChanged(bool checked);
};
