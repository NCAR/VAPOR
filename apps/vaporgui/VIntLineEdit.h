#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

//! \class VIntLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type int,
//! and provides Vapor's standard setters, getters, and signals

class VIntLineEdit : public VAbstractLineEdit {
    public:
        VIntLineEdit( int value, bool useMenu=true );

        //! Set the current int value in the line edit
        virtual void SetValue( int value );
   
        //! Get the current int value in the line iedit 
        virtual int GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( int value );
        
        int _value;
};
