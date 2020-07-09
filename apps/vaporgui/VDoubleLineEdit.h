#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

//! \class VDoubleLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type double,
//! and provides Vapor's standard setters, getters, and signals

class VDoubleLineEdit : public VAbstractLineEdit {
    public:
        VDoubleLineEdit( double value, bool useMenu=true );

        //! Set the current double value in the line edit
        virtual void SetValue( double value );
    
        //! Get the current double value in the line edit
        virtual double GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( double value );
        
        double _value;
};
