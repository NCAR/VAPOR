#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include "VContainer.h"

class QStringLineEdit;

//! \class VStringLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type string, 
//! and provides Vapor's standard setters, getters, and signals

class VStringLineEdit : public VContainer {
    Q_OBJECT

    public:
        VStringLineEdit( std::string value = "" );

        //! Set the current string value in the line edit
        virtual void SetValueString( std::string value );
   
        //! Get the current value in the line edit 
        virtual std::string GetValueString() const;

    protected:
        QLineEdit*  _lineEdit;
        std::string _strValue;

        std::string _formatValue( std::string value );
        std::string _getText() const;
        

    protected slots:
        virtual void _valueChanged();

    signals:
        void ValueChanged( const std::string& value );
};
