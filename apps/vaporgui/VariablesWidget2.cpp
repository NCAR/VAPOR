#include "VariablesWidget2.h"

#include "VComboBox.h"
#include "FidelityWidget.h"
#include "FidelityWidget2.h"
#include "VLineComboBox.h"

#include "vapor/RenderParams.h"

#include <QLayout>
#include <QLabel>

const std::string VariablesWidget2::_sectionTitle = "Variable Selection";

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;
}

VariablesWidget2::VariablesWidget2() 
    : VSection( _sectionTitle ),
    _activeDim( 3 ),
    _initialized( false )
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );

    _dimCombo = new VLineComboBox( "Variable Dimension" );
    _dimCombo->SetOptions( {"3D", "2D"} );
    layout()->addWidget( _dimCombo );
    connect( _dimCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_dimChanged );

    _scalarCombo = new VLineComboBox( "Variable name" );
    connect( _scalarCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_scalarVarChanged );
    layout()->addWidget( _scalarCombo );

    _xFieldCombo = new VLineComboBox( "  X" );
    connect( _xFieldCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_xFieldVarChanged );
    layout()->addWidget( _xFieldCombo );

    _yFieldCombo = new VLineComboBox( "  Y" );
    connect( _yFieldCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_yFieldVarChanged );
    layout()->addWidget( _yFieldCombo );

    _zFieldCombo = new VLineComboBox( "  Z" );
    connect( _zFieldCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_zFieldVarChanged );
    layout()->addWidget( _zFieldCombo );

    _colorCombo = new VLineComboBox( "Color mapped variable" );
    connect( _colorCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_colorVarChanged );
    layout()->addWidget( _colorCombo );
    
    _heightCombo = new VLineComboBox( "Height variable" );
    connect( _heightCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_heightVarChanged );
    layout()->addWidget( _heightCombo );

    /*_fidelityWidget = new FidelityWidget2();
    layout()->addWidget( _fidelityWidget );*/
}

void VariablesWidget2::Reinit(
    VariableFlags variableFlags,
    DimFlags dimFlags
) {
    _variableFlags = variableFlags;
    _dimFlags = dimFlags;

    // If the renderer is not both 2D and 3D, hide
    // the dimension selector and set the _activeDim
    if (! ( ( _dimFlags & 2 ) && ( _dimFlags & 3 ) )
    ) { 
        _dimCombo->hide();
        if (dimFlags & THREED) {
            _activeDim = 3;
        }
        else {
            _activeDim = 2;
        }
    }

    if (_variableFlags & SCALAR) {
        _scalarCombo->show();
    }
    else {
        _scalarCombo->hide();
    }

    if ( _variableFlags & VECTOR ) {
        _xFieldCombo->show();
        _yFieldCombo->show();
        _zFieldCombo->show();
    }
    else {
        _xFieldCombo->hide();
        _yFieldCombo->hide();
        _zFieldCombo->hide();
    }

    if ( _variableFlags & HEIGHT ) {
        _heightCombo->show();
    }
    else {
        _heightCombo->hide();
    }

    if ( _variableFlags & COLOR ) {
        _colorCombo->show();
    }
    else {
        _colorCombo->hide();
    }

    //_rParams->SetDefaultVariables( _activeDim, false );

    /*VariableFlags fdf = (VariableFlags)0;
    if (_variableFlags & SCALAR)
        fdf = (VariableFlags)(fdf | SCALAR);

    if (_variableFlags & VECTOR)
        fdf = (VariableFlags)(fdf | VECTOR);

    if (_variableFlags & HEIGHT)
        fdf = (VariableFlags)(fdf | HEIGHT);

    _fidelityWidget->Reinit(fdf);*/

    //variableSelectionWidget->adjustSize();
    //adjustSize();
}

void VariablesWidget2::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    std::vector<std::string> twoDVars = _dataMgr->GetDataVarNames( 2 );

    std::vector<std::string> activeVars;
    if ( _activeDim == 3 ) {
        activeVars = _dataMgr->GetDataVarNames( 3 );
    }
    else {
        activeVars = twoDVars;
    }

    _scalarCombo->SetOptions( activeVars );
    _scalarCombo->SetValue( _rParams->GetVariableName() );

    _xFieldCombo->SetOptions( activeVars );
    _yFieldCombo->SetOptions( activeVars );
    _zFieldCombo->SetOptions( activeVars );
    std::vector< std::string > fieldVars = _rParams->GetFieldVariableNames();
    _xFieldCombo->SetValue( fieldVars[X] );
    _yFieldCombo->SetValue( fieldVars[Y] );
    _zFieldCombo->SetValue( fieldVars[Z] );
   
    _colorCombo->SetOptions( activeVars );
    _colorCombo->SetValue( _rParams->GetColorMapVariableName() );
 
    _heightCombo->SetOptions( activeVars );
    _heightCombo->SetValue( _rParams->GetHeightVariableName() );

    //_fidelityWidget->Update( _dataMgr, _paramsMgr, _rParams );
};

void VariablesWidget2::_dimChanged() {
    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    _rParams->SetDefaultVariables( _activeDim, false );

    //Update( _dataMgr, _paramsMgr, _rParams );
}

void VariablesWidget2::_scalarVarChanged( std::string var ) {
    _rParams->SetVariableName( var );
}

void VariablesWidget2::_xFieldVarChanged( std::string var ) {
    std::vector< std::string > fieldVars = _rParams->GetFieldVariableNames();
    fieldVars[X] = var;
    _rParams->SetFieldVariableNames( fieldVars );
}

void VariablesWidget2::_yFieldVarChanged( std::string var ) {
    std::vector< std::string > fieldVars = _rParams->GetFieldVariableNames();
    fieldVars[Y] = var;
    _rParams->SetFieldVariableNames( fieldVars );
}

void VariablesWidget2::_zFieldVarChanged( std::string var ) {
    std::vector< std::string > fieldVars = _rParams->GetFieldVariableNames();
    fieldVars[Z] = var;
    _rParams->SetFieldVariableNames( fieldVars );
}

void VariablesWidget2::_colorVarChanged( std::string var ) {
    _rParams->SetColorMapVariableName( var );
}

void VariablesWidget2::_heightVarChanged( std::string var ) {
    _rParams->SetHeightVariableName( var );
}

int VariablesWidget2::GetActiveDimension() const {

}

DimFlags VariablesWidget2::GetDimFlags() const {

}

void VariablesWidget2::Configure2DFieldVars() {

}

void VariablesWidget2::Configure3DFieldVars() {

}
