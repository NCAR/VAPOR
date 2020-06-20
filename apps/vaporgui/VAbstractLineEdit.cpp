#include <QLineEdit>

#include "VContainer.h"
#include "VActions.h"
#include "VNumericFormatMenu.h"
#include "VAbstractLineEdit.h"

VAbstractLineEdit::VAbstractLineEdit( bool useNumericMenu ) :
    VContainer(),
    _sciNotation( false ),
    _decimalDigits( 5 )
{
    _lineEdit = new QLineEdit;
    layout()->addWidget( _lineEdit );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

    if (useNumericMenu) {
        
        _menu = new VNumericFormatMenu( this, _sciNotation, _decimalDigits );
        _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( _lineEdit, &QLineEdit::customContextMenuRequested,
            this, &VAbstractLineEdit::_showMenu );
        
        connect( _menu, &VNumericFormatMenu::SciNotationChanged,
            this, &VAbstractLineEdit::SetSciNotation );
        connect( _menu, &VNumericFormatMenu::DecimalDigitsChanged,
            this, &VAbstractLineEdit::SetNumDigits );
    }
}

void VAbstractLineEdit::SetValue( int value ) { 
    emit ValueChanged( value ); 
}

void VAbstractLineEdit::SetValue( double value ) { 
    emit ValueChanged( value ); 
}

void VAbstractLineEdit::SetValue( const std::string& value ) { 
    emit ValueChanged( value ); 
}

int VAbstractLineEdit::GetNumDigits() const { 
    return _decimalDigits; 
}

void VAbstractLineEdit::SetNumDigits( int digits ) { 
    _decimalDigits = digits; 
    _valueChanged();
    emit DecimalDigitsChanged( _decimalDigits );
}

bool VAbstractLineEdit::GetSciNotation() const { 
    return _sciNotation;
}

void VAbstractLineEdit::SetSciNotation( bool sciNotation ) { 
    _sciNotation = sciNotation; 
    _valueChanged();
    emit SciNotationChanged( _sciNotation );
}

/*void VAbstractLineEdit::_setDecimalDigits( int digits ) { 
    _decimalDigits = digits;
    _valueChanged();
    emit DecimalDigitsChanged( _decimalDigits );
}

void VAbstractLineEdit::_setSciNotation( bool sciNotation ) {
    _sciNotation = sciNotation;
    _valueChanged();
    emit SciNotationChanged( _sciNotation );
}*/

void VAbstractLineEdit::_showMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
};
