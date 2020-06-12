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

class VIntLineEdit : public VContainer {
    Q_OBJECT

    public:
        VIntLineEdit( int value );
        void SetValue( int value );
        int  GetValue() const;

    private:
        QLineEdit* _lineEdit;
        int        _value;

    private slots:
        void emitChange();

    signals:
        void ValueChanged( const int value );
};

class VDoubleLineEdit : public VContainer {
    Q_OBJECT

    public:
        VDoubleLineEdit( double value );

        void SetValue( double value );
        double GetValue() const;

    private:
        QLineEdit* _lineEdit;
        double     _value;

    private slots:
        void emitChange();

    signals:
        void ValueChanged( const double value );
};

class VStringLineEdit : public VContainer {
    Q_OBJECT

    public:
        VStringLineEdit( const std::string& value );

        void SetValue( const std::string& value );
        std::string GetValue() const;

    private:
        QLineEdit*  _lineEdit;
        std::string _value;

    private slots:
        void emitChange();

    signals:
        void ValueChanged( const std::string& value );
};
