#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

class VIntLineEdit : public VAbstractLineEdit {
    public:
        VIntLineEdit( int value, bool useMenu=true );

        virtual void SetValue( int value );
    
        virtual int GetValue() const;

    protected:
        virtual void _valueChanged();

        std::string  _formatValue( int value );
        
        int _value;
};
