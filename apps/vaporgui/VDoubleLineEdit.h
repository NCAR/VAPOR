#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VNumericLineEdit.h"

//! \class VDoubleLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type double,
//! and provides Vapor's standard setters, getters, and signals

class VDoubleLineEdit : public VNumericLineEdit {
    Q_OBJECT

    public:
        VDoubleLineEdit( double value = 0.0 );

        //! Set the current double value in the line edit
        void SetValueDouble( double value );
    
        //! Get the current double value in the line edit
        double GetValueDouble() const;

    protected:
        std::string  _formatValue( double value );
        
        double _value;

    protected slots:
        void _valueChanged() override;

    signals:
        void ValueChanged( double value );
};
