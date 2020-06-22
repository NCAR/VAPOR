#include "VDoubleRangeMenu.h"
#include "VActions.h"

VDoubleRangeMenu::VDoubleRangeMenu( 
    QWidget* parent, 
    bool sciNotation, double decimalDigits,
    double min, double max 
    ) 
    : VNumericFormatMenu( parent, sciNotation, decimalDigits ),
      _minRangeAction( new VDoubleLineEditAction( "Minimum value", min ) ),
      _maxRangeAction( new VDoubleLineEditAction( "Maximum value", max ) )
{
    connect( _minRangeAction, &VDoubleLineEditAction::ValueChanged,
        this, &VDoubleRangeMenu::_minChanged);
    addAction( _minRangeAction );

    connect( _maxRangeAction, &VDoubleLineEditAction::ValueChanged,
        this, &VDoubleRangeMenu::_maxChanged);
    addAction( _maxRangeAction );
}

void VDoubleRangeMenu::SetMinRange( double min ) { 
    _minRangeAction->SetValue( min ); 
}

void VDoubleRangeMenu::SetMaxRange( double max ) { 
    _maxRangeAction->SetValue( max ); 
}

void VDoubleRangeMenu::_minChanged( double min ) { 
    emit MinChanged( min ); 
}

void VDoubleRangeMenu::_maxChanged( double max ) { 
    emit MaxChanged( max ); 
}
