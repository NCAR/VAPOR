#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

class VDoubleLineEdit : public VAbstractLineEdit {
    public:
        VDoubleLineEdit( double value, bool useMenu=true );

        virtual void SetValue( double value );
    
        virtual double GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( double value );
        
        double _value;
};
