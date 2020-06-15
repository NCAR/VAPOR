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
    VLineEdit( const std::string& value = "");

    void SetValue( int value );
    void SetValue( double value );
    void SetValue( const std::string& value );
    std::string GetValue() const;

    //! Sets the line edit to read-only based on the value of b
    void SetReadOnly(bool b);

    void UseDoubleMenu();

private:
    QLineEdit*  _lineEdit;

    std::string _value;
    bool        _scientific;
    bool        _menuEnabled;
    int         _decDigits;

public slots:
    void emitLineEditChanged();

    void ShowContextMenu(const QPoint&);

private slots:
    void _decimalDigitsChanged( int value );
    void _scientificClicked( bool value );

signals:
    void ValueChanged( const std::string& value );
    void ValueChanged( double value );
    void ValueChanged( int value );
};
