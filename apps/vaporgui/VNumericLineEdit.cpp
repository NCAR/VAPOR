#include <QLineEdit>

#include "VNumericFormatMenu.h"
#include "VNumericLineEdit.h"

VNumericLineEdit::VNumericLineEdit() :
    //VContainer(),
    VStringLineEdit(),
    _sciNotation( false ),
    _decimalDigits( 5 )
{
    _lineEdit = new QLineEdit;
    layout()->addWidget( _lineEdit );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

    _menu = new VNumericFormatMenu( this, _sciNotation, _decimalDigits );
    _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( _lineEdit, &QLineEdit::customContextMenuRequested,
        this, &VNumericLineEdit::_showMenu );
    
    connect( _menu, &VNumericFormatMenu::SciNotationChanged,
        this, &VNumericLineEdit::SetSciNotation );
    connect( _menu, &VNumericFormatMenu::DecimalDigitsChanged,
        this, &VNumericLineEdit::SetNumDigits );
}

void VNumericLineEdit::RemoveContextMenu() {
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
    setContextMenuPolicy( Qt::NoContextMenu );
}

int VNumericLineEdit::GetNumDigits() const { 
    return _decimalDigits; 
}

void VNumericLineEdit::SetNumDigits( int digits ) { 
    _decimalDigits = digits; 
    _valueChanged();
    emit DecimalDigitsChanged( _decimalDigits );
}

bool VNumericLineEdit::GetSciNotation() const { 
    return _sciNotation;
}

void VNumericLineEdit::SetSciNotation( bool sciNotation ) { 
    _sciNotation = sciNotation; 
    _valueChanged();
    emit SciNotationChanged( _sciNotation );
}

void VNumericLineEdit::_showMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
};
