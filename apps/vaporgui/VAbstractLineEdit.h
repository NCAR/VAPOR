#pragma once

#include <QLineEdit>

#include "VContainer.h"

class VNumericFormatMenu;
class VCheckBoxAction;
class VSpinBoxAction;

//! \class VAbstractSliderEdit
//! \ingroup Public_GUI
//! \brief An abstract class defining functions used by different
//! V*LineEdits (VDoubleLineEdit, VIntLineEdit, and VStringLineEdit).
//! Supports menus that allow different functionalities, depending
//! on the data type being used by the line edit.

class VAbstractLineEdit : public VContainer {
    Q_OBJECT

protected:
    explicit VAbstractLineEdit( bool useNumericMenu = false);

public slots:
    //! Set the integer value in the VAbstractLineEdit
    virtual void SetValue( int value );

    //! Set the double value in the VAbstractLineEdit
    virtual void SetValue( double value );

    //! Set the string value in the VAbstractLineEdit
    virtual void SetValue( const std::string& value );

    //! If the line edit is numeric, get the number of digits of the number being displayed
    int GetNumDigits() const;
    
    //! If the line edit is numeric, set the number of digits of the number being displayed
    void SetNumDigits( int digits );

    //! If the line edit is numeric, get whether the display is in scientific notation
    bool GetSciNotation() const;
    
    //! If the line edit is numeric, set whether the display is in scientific notation
    void SetSciNotation( bool sciNotation );

public slots:
    
    //! Called whenever the line edit's value is changed.  Must be reimplemented by derived classes
    //! to handle correct formatting
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
