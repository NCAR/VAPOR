#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VActions.h"
#include "VLineEdit2.h"
#include "ErrorReporter.h"

VLESignals::VLESignals( VLineEdit2* vle2 ) : public QObject {
    connect( _lineEdit, &QLineEdit::editingFinished,
        this, &VLESignals::emitChange );
}

//void VLESignals::emitChange() {
//    emit ValueChanged
//}

template <class T>
VLineEdit2<T>::VLineEdit2( const T& value )
: VLESignals(),
  VContainer(), 
  _value( value )
{
    _lineEdit = new QLineEdit;
    layout()->addWidget(_lineEdit);
}

template <class T>
T VLineEdit2<T>::GetValue() const {
    return _value;
}

template <class T>
void VLineEdit2<T>::SetValue( T value ) {
    std::cout << "    void VLineEdit2<T>::SetValue( T value ) {" << std::endl;
    
    _value = value;
}

VIntLineEdit::VIntLineEdit( int value ) : VLineEdit2<int>( value ) {
    SetValue( value );
}

void VIntLineEdit::SetValue( int value ) {
    _lineEdit->setText( QString::number( value ) );
    std::cout << "void VIntLineEdit::SetValue( int value ) " << std::endl;
    VLineEdit2::SetValue( value );
}

VDoubleLineEdit::VDoubleLineEdit( double value ) : VLineEdit2<double>( value ) {
    SetValue( value );
    emit ValueChanged( _value );
}

void VDoubleLineEdit::SetValue( double value ) {
    _lineEdit->setText( QString::number( value ) );
    std::cout << "void VDoubleLineEdit::SetValue( double value ) " << std::endl;
    VLineEdit2::SetValue( value );
}

VStringLineEdit::VStringLineEdit( std::string value ) : VLineEdit2<std::string>( value ) {
    SetValue( value );
    emit ValueChanged( _value );
}

void VStringLineEdit::SetValue( std::string value ) {
    _lineEdit->setText( QString::fromStdString( value ) );
    std::cout << "void VStringLineEdit::SetValue( std::string value ) " << std::endl;
    VLineEdit2::SetValue( value );
}
