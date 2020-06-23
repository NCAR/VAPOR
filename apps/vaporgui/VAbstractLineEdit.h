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

    bool _sciNotation;
    int  _decimalDigits;

    void _showMenu( const QPoint& pos );

signals:

    // Required to propotage changes up up to Params, via PWidgets
    void ValueChanged( int value );
    void ValueChanged( double value );
    void ValueChanged( const std::string& value );

    // Required to propogate changes from the menus 
    // up to Params, via PWidgets
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};
