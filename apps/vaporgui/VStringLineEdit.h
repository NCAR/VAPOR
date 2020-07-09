#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

//! \class VStringLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type string, 
//! and provides Vapor's standard setters, getters, and signals

class VStringLineEdit : public VAbstractLineEdit {
    public:
        VStringLineEdit( std::string value );

        //! Set the current string value in the line edit
        virtual void SetValue( std::string value );
   
        //! Get the current value in the line edit 
        virtual std::string GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( std::string value );
        
        std::string _value;
};
