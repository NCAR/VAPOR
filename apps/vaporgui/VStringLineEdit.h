#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>
#include <QLineEdit>

#include "VHBoxWidget.h"

class QStringLineEdit;

//! \class VStringLineEdit
//! \ingroup Public_GUI
//! \brief A wrapper for a QLineEdit that handles user input of type string, 
//! and provides Vapor's standard setters, getters, and signals

class VStringLineEdit : public VHBoxWidget {
    Q_OBJECT

    public:
        VStringLineEdit( std::string value = "" );

        //! Set the current string value in the line edit
        void SetValueString( std::string value );
   
        //! Get the current value in the line edit 
        std::string GetValueString() const;

        //! Remove the current context menu
        void RemoveContextMenu();

        //! Create a custom context menu for the QLineEdit
        void SetCustomContextMenu();
    
    void SetReadOnly(bool b) { _lineEdit->setReadOnly(b); }
    void Clear() { SetValueString(""); }

    private:
        QLineEdit*  _lineEdit;
        std::string _strValue;

    protected:
        std::string _getText() const;

    protected slots:
        virtual void _valueChanged();

    signals:
        void ValueChanged( const std::string& value );
};
