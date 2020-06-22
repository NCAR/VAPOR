#pragma once

#include <QLineEdit>

#include "VContainer.h"

class VNumericFormatMenu;
class VCheckBoxAction;
class VSpinBoxAction;

class VAbstractLineEdit : public VContainer {
    Q_OBJECT

protected:
    explicit VAbstractLineEdit( bool useNumericMenu = false);

public slots:
    virtual void SetValue( int value );
    virtual void SetValue( double value );
    virtual void SetValue( const std::string& value );

    int GetNumDigits() const;
    void SetNumDigits( int digits );

    bool GetSciNotation() const;
    void SetSciNotation( bool sciNotation );

public slots:
    virtual void _valueChanged() = 0;

protected:
    QLineEdit*          _lineEdit;
    VNumericFormatMenu* _menu;
    VCheckBoxAction*    _sciNotationAction;
    VSpinBoxAction*     _decimalAction;

    bool _sciNotation;
    int  _decimalDigits;

    void _showMenu( const QPoint& pos );

signals:
    void ValueChanged( int value );
    void ValueChanged( double value );
    void ValueChanged( const std::string& value );
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};
