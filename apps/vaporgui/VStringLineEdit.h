#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

class VStringLineEdit : public VAbstractLineEdit {
    public:
        VStringLineEdit( std::string value );

        virtual void SetValue( std::string value );
    
        virtual std::string GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( std::string value );
        
        std::string _value;
};
