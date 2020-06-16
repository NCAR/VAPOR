#pragma once

#include <string>

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QWidgetAction>

#include "VContainer.h"

class VSpinBoxAction;
class VCheckBoxAction;

//! \class VLineEdit
//!
//! Wraps a QLineEdit with vaporgui's standard setter/getter functions.
//! Handles string and double types, as well as precision and display of
//! double values.

class VLineEdit2 : public VContainer {
    Q_OBJECT

    public:
        VLineEdit2( const std::string& value );

        void SetValue( const std::string& value );
        std::string GetValue() const;

    protected:
        QLineEdit*  _lineEdit;

    private slots:
        void _emitChange();

    signals:
        void ValueChanged( const std::string& value );
};

//class VNumericLineEdit : public VContainer {
class VNumericLineEdit : public VLineEdit2 {
    Q_OBJECT

    public:
        VNumericLineEdit( bool useMenu );
        void SetSciNotation( bool sciNotation );
        void SetNumDigits( int precision );
        virtual void Reformat() = 0;

    protected:
        //QLineEdit*       _lineEdit;

        QMenu*           _menu;
        VCheckBoxAction* _sciNotationAction;
        bool             _sciNotation;
        VSpinBoxAction*  _decimalAction;
        int              _decimalDigits;

        virtual void _emitChange() = 0;
        void _showMenu( const QPoint& pos );
        void _decimalDigitsChanged( int digits );

    signals:
        void FormatChanged();
};

class VIntLineEdit : public VNumericLineEdit {
    Q_OBJECT

    public:
        VIntLineEdit( int value, bool useMenu=true );
        void SetValue( int value );
        int  GetValue() const;
        void Reformat() override;

    protected:
        int _value;

        void _emitChange() override;
        void _enableDecimalPrecision( bool enalbed );

    signals:
        void ValueChanged( const int value );
};

class VDoubleLineEdit : public VNumericLineEdit {
    Q_OBJECT

    public:
        VDoubleLineEdit( double value, bool useMenu=true );
        void SetValue( double value );
        double GetValue() const;
        void Reformat() override;

    protected:
        double     _value;

        void _emitChange() override;

    signals:
        void ValueChanged( const double value );
};
