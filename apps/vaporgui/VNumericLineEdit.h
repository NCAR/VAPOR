#pragma once

#include <QLineEdit>

#include "vapor/VAssert.h"

#include "VHBoxWidget.h"
#include "VStringLineEdit.h"

class VNumericFormatMenu;
class VCheckBoxAction;
class VSpinBoxAction;

//! \class VNumericSliderEdit
//! \ingroup Public_GUI
//! \brief An abstract class defining functions used by different
//! V*LineEdits (VDoubleLineEdit, VIntLineEdit, and VStringLineEdit).
//! Supports menus that allow different functionalities, depending
//! on the data type being used by the line edit.

class VNumericLineEdit : public VStringLineEdit {
    Q_OBJECT

protected:
    explicit VNumericLineEdit( int decimals = 5 );

public:
    //! If the line edit is numeric, get the number of digits of the number being displayed
    int GetNumDigits() const;
    
    //! If the line edit is numeric, set the number of digits of the number being displayed
    void SetNumDigits( int digits );

    //! If the line edit is numeric, get whether the display is in scientific notation
    bool GetSciNotation() const;
    
    //! If the line edit is numeric, set whether the display is in scientific notation
    void SetSciNotation( bool sciNotation );

protected slots:
    
    //! Called whenever the line edit's value is changed.  Must be reimplemented by derived classes
    //! to handle correct formatting
    virtual void _valueChanged() = 0;

    //! Show a custom context menu
    virtual void _showMenu( const QPoint& pos );

protected:
    VNumericFormatMenu* _menu;

    bool _sciNotation;
    int  _decimalDigits;

signals:

    // Required to propogate changes from the menus 
    // up to Params, via PWidgets
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};
