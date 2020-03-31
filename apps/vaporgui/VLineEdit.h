#pragma once

#include <string>

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidgetAction>

#include "VContainer.h"

//! \class VLineEdit
//!
//! Wraps a QLineEdit with vaporgui's standard setter/getter functions.
//! Handles string and double types, as well as precision and display of
//! double values.

class VLineEdit : public VContainer {
    Q_OBJECT

public:
    VLineEdit(const std::string &value = "");

    void        SetValue(double value);
    void        SetValue(const std::string &value);
    std::string GetValue() const;

    // Note: VLineEdit does NOT support an integer type at this point.
    // When isDouble == false, it represents a string.
    void SetIsDouble(bool isDouble);
    //! Sets the line edit to read-only based on the value of b
    void SetReadOnly(bool b);

    void UseDoubleMenu();

private:
    QLineEdit *_lineEdit;

    std::string _value;
    bool        _isDouble;
    bool        _scientific;
    bool        _menuEnabled;
    int         _decDigits;

public slots:
    void emitLineEditChanged();

    void ShowContextMenu(const QPoint &);

private slots:
    void _decimalDigitsChanged(int value);
    void _scientificClicked(bool value);

signals:
    void ValueChanged(const std::string &value);
};

class SpinBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    SpinBoxAction(const QString &title, int value) : QWidgetAction(NULL)
    {
        QWidget *    widget = new QWidget(NULL);
        QHBoxLayout *layout = new QHBoxLayout();
        QLabel *     label = new QLabel(title);    // bug fixed here, pointer was missing
        _spinBox = new QSpinBox(NULL);

        _spinBox->setValue(value);
        layout->setContentsMargins(-1, 0, -1, 0);
        layout->addWidget(label);
        layout->addWidget(_spinBox);
        widget->setLayout(layout);

        // We need to use SIGNAL/SLOT macros here because the arguments
        // of the signal and slot do not match
        connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged()));

        setDefaultWidget(widget);
    }

private:
    QSpinBox *_spinBox;

private slots:
    void spinBoxChanged()
    {
        int  value = _spinBox->value();
        emit editingFinished(value);
    }

signals:
    void editingFinished(int);
};

class CheckBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    CheckBoxAction(const QString &title, bool value) : QWidgetAction(NULL)
    {
        QWidget *    widget = new QWidget(NULL);
        QHBoxLayout *layout = new QHBoxLayout();
        QLabel *     label = new QLabel(title);
        _checkBox = new QCheckBox(NULL);
        _checkBox->setChecked(value);

        layout->setContentsMargins(-1, 0, 20, 0);
        layout->addWidget(label);
        layout->addStretch();
        layout->addWidget(_checkBox);
        widget->setLayout(layout);

        connect(_checkBox, &QCheckBox::clicked, this, &CheckBoxAction::checkBoxChanged);

        setDefaultWidget(widget);
    }

private:
    QCheckBox *_checkBox;

private slots:
    void checkBoxChanged(bool value) { emit clicked(value); }

signals:
    void clicked(bool);
};
