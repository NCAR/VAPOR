#pragma once

#include "VAbstractLineEdit.h"

template <class T>
class VLineEditTemplate : public VAbstractLineEdit {

    public:
        T GetValue() const;

        virtual void SetValue( T value ); 

    protected:
        VLineEditTemplate( T value, bool useMenu=true );

        virtual void _valueChanged();
        std::string _formatValue( T value );
        
        T _value;
};

template <>
class VLineEditTemplate <std::string> : public VAbstractLineEdit {
    protected:
        VLineEditTemplate( const std::string& value );
        
        virtual void SetValue( const std::string& value );
        std::string GetValue() const;

        virtual void _valueChanged();

        std::string _value;
};

/*class VIntLineEdit3 : public VLineEditTemplate<int> {
    public:
        VIntLineEdit3( int value, bool useMenu=true ) 
        : VLineEditTemplate<int>(value, useMenu) {}
};

class VDoubleLineEdit3 : public VLineEditTemplate<double> {
    public:
        VDoubleLineEdit3( double value, bool useMenu=true ) 
        : VLineEditTemplate<double>(value, useMenu) {}
};

class VStringLineEdit3 : public VLineEditTemplate<std::string> {
    public:
        VStringLineEdit3( const std::string& value ) 
        : VLineEditTemplate<std::string>(value) {}
};*/
