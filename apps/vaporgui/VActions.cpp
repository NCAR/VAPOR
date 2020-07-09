#include <string>

#include "VActions.h"
#include "VIntSpinBox.h"
#include "VCheckBox.h"
#include "VStringLineEdit.h"
#include "VIntLineEdit.h"
#include "VDoubleLineEdit.h"

VSpinBoxAction::VSpinBoxAction (const std::string& title, int value) : 
  QWidgetAction (NULL) {

    _spinBox = new VIntSpinBox(1, 10);
    _spinBox->SetValue( value );

    VLineItem* vli = new VLineItem( title, _spinBox );
    vli->setContentsMargins( 5, 0, 5, 0 );

    connect( _spinBox, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _spinBoxChanged( int ) ) );

    setDefaultWidget(vli);
}

void VSpinBoxAction::SetValue( int value ) {
    _spinBox->SetValue( value );
}

void VSpinBoxAction::_spinBoxChanged( int value ) {
    emit editingFinished( value );
}

VCheckBoxAction::VCheckBoxAction (const std::string& title, bool value) : 
  QWidgetAction (NULL) 
{
    _checkBox = new VCheckBox( value );
    VLineItem* vli = new VLineItem( title, _checkBox );
    vli->setContentsMargins( 5, 0, 5, 0 );

    connect( _checkBox, &VCheckBox::ValueChanged,
        this, &VCheckBoxAction::_checkBoxChanged );

    setDefaultWidget(vli);
}

void VCheckBoxAction::SetValue( bool value ) {
    _checkBox->SetValue( value );
}

void VCheckBoxAction::_checkBoxChanged( bool value ) {
    emit clicked( value );
}


VStringLineEditAction::VStringLineEditAction( const std::string& title, std::string value ) :
  QWidgetAction(nullptr) {
    _lineEdit = new VStringLineEdit( value ); //std::to_string( value ) );
    VLineItem* vli = new VLineItem( title, _lineEdit );
    vli->setContentsMargins( 5, 0, 5, 0 );

    connect( _lineEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _lineEditChanged( int ) ) );

    setDefaultWidget( vli );
}

void VStringLineEditAction::SetValue( const std::string& value ) {
    _lineEdit->SetValue( value );
}
    
void VStringLineEditAction::_lineEditChanged( int value ) {
    emit ValueChanged( value );
}

VIntLineEditAction::VIntLineEditAction( const std::string& title, int value ) :
  QWidgetAction(nullptr) {
    _lineEdit = new VIntLineEdit( value );
    VLineItem* vli = new VLineItem( title, _lineEdit );
    vli->setContentsMargins( 5, 0, 5, 0 );

    connect( _lineEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _lineEditChanged( int ) ) );

    setDefaultWidget( vli );
}

void VIntLineEditAction::SetValue( int value ) {
    _lineEdit->SetValue( value );
}

void VIntLineEditAction::_lineEditChanged( int value ) {
    emit ValueChanged( value );
}

VDoubleLineEditAction::VDoubleLineEditAction( const std::string& title, double value ) :
  QWidgetAction(nullptr) {
    _lineEdit = new VDoubleLineEdit( value );
    VLineItem* vli = new VLineItem( title, _lineEdit );
    vli->setContentsMargins( 5, 0, 5, 0 );

    connect( _lineEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _lineEditChanged( double ) ) );

    setDefaultWidget( vli );
}

void VDoubleLineEditAction::SetValue( double value ) {
    _lineEdit->SetValue( value );
}

void VDoubleLineEditAction::_lineEditChanged( double value ) {
    emit ValueChanged( value );
}
