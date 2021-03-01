#pragma once

#include <string>
#include <QComboBox>
#include "VHBoxWidget.h"

//! class VComboBox
//!
//! Wraps a QComboBox and provides vaporgui's standard setter/getter functions
//! and signals.

class VComboBox : public VHBoxWidget {
    Q_OBJECT

public:
    VComboBox(const std::vector<std::string> &values = {});

    void SetOptions(const std::vector<std::string> &values);
    void SetIndex(int index);
    void SetValue(const std::string &value);
    void SetItemEnabled(int index, bool enabled);

    std::string GetValue() const;
    int         GetCurrentIndex() const;
    int         GetCount() const;

private:
    QComboBox *_combo;

public slots:
    void emitComboChanged(const QString &value);

signals:
    void ValueChanged(std::string value);
    void IndexChanged(int index);
};
