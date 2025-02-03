#pragma once

#include <string>

#include <QWidget>
#include <QRadioButton>

//#include "VHBoxWidget.h"
#include "VGroup.h"

//! class VRadioButtons
//!
//! Wraps a QRadioButtons and provides vaporgui's standard setter/getter functions
//! and signals.

class VRadioButtons : public PWidget{
    Q_OBJECT
    PGroup* _group;
    std::vector<QRadioButton*> _buttons;

public:
    VRadioButtons();

    void SetValues(std::vector<std::string>& types);
    void SetValue(std::string& checked);

    std::string GetValue() const;

public slots:
    //void radioButtonChecked(QRadioButton* rb);
    void radioButtonChecked();

signals:
    //void ValueChanged(std::string value);
    void ValueChanged();
};
