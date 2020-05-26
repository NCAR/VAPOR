#include "VVariablesSection.h"

#include "VFidelitySection.h"
#include "VComboBox.h"
#include "VLineComboBox.h"
#include "VContainer.h"

#include "vapor/RenderParams.h"

#include <QLayout>
#include <QLabel>
#include <QSpacerItem>

const std::string VVariablesSection::_sectionTitle = "VVariablesSection";

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;
}

VVariablesSection::VVariablesSection() 
    : VSection( _sectionTitle ),
    _activeDim( 3 ),
    _initialized( false )
{
    //setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    //setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    //setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );

    _dimCombo = new VLineComboBox( "Variable Dimension" );
    _dimCombo->SetOptions( {"3D", "2D"} );
    layout()->addWidget( _dimCombo );
    connect( _dimCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_dimChanged );

    _scalarCombo = new VLineComboBox( "Variable name" );
    connect( _scalarCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_scalarVarChanged );
    layout()->addWidget( _scalarCombo );

    _xFieldCombo = new VLineComboBox( "  X" );
    connect( _xFieldCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_xFieldVarChanged );
    layout()->addWidget( _xFieldCombo );

    _yFieldCombo = new VLineComboBox( "  Y" );
    connect( _yFieldCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_yFieldVarChanged );
    layout()->addWidget( _yFieldCombo );

    _zFieldCombo = new VLineComboBox( "  Z" );
    connect( _zFieldCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_zFieldVarChanged );
    layout()->addWidget( _zFieldCombo );

    _colorCombo = new VLineComboBox( "Color mapped variable" );
    connect( _colorCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_colorVarChanged );
    layout()->addWidget( _colorCombo );
    
    _heightCombo = new VLineComboBox( "Height variable" );
    connect( _heightCombo, &VLineComboBox::ValueChanged,
        this, &VVariablesSection::_heightVarChanged );
    layout()->addWidget( _heightCombo );

    //_fidelitySection = new VFidelitySection();
    //layout()->addWidget( _fidelitySection );

    //layout()->addItem( 
    //    new QSpacerItem( 1, 2000, QSizePolicy::Minimum, QSizePolicy::Maximum ) 
    //);

}

void VVariablesSection::Reinit(
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

    VariableFlags fdf = (VariableFlags)0;
    if (_variableFlags & SCALAR)
        fdf = (VariableFlags)(fdf | SCALAR);

    if (_variableFlags & VECTOR)
        fdf = (VariableFlags)(fdf | VECTOR);

    if (_variableFlags & HEIGHT)
        fdf = (VariableFlags)(fdf | HEIGHT);

    //_fidelitySection->Reinit(fdf);

    //variableSelectionWidget->adjustSize();
    //adjustSize();
}

void VVariablesSection::Update(
    VAPoR::RenderParams* rParams,
    VAPoR::ParamsMgr*    paramsMgr,
    VAPoR::DataMgr*      dataMgr
) {
   
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;
 
    std::vector<std::string> activeVars;
    std::vector<std::string> twoDVars = _dataMgr->GetDataVarNames( 2 );
    if ( _activeDim == 3 ) {
        activeVars = _dataMgr->GetDataVarNames( 3 );
    }
    else {
        activeVars = twoDVars;
    }

    _scalarCombo->SetOptions( activeVars );
    _scalarCombo->SetValue( rParams->GetVariableName() );

    _xFieldCombo->SetOptions( activeVars );
    _yFieldCombo->SetOptions( activeVars );
    _zFieldCombo->SetOptions( activeVars );
    std::vector< std::string > fieldVars = rParams->GetFieldVariableNames();
    _xFieldCombo->SetValue( fieldVars[X] );
    _yFieldCombo->SetValue( fieldVars[Y] );
    _zFieldCombo->SetValue( fieldVars[Z] );
   
    _colorCombo->SetOptions( activeVars );
    _colorCombo->SetValue( rParams->GetColorMapVariableName() );
 
    _heightCombo->SetOptions( activeVars );
    _heightCombo->SetValue( rParams->GetHeightVariableName() );

    //_fidelitySection->Update( _rParams, _paramsMgr, _dataMgr );
};

void VVariablesSection::_dimChanged() {
    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    _rParams->SetDefaultVariables( _activeDim, false );

    //Update( _dataMgr, _paramsMgr, _params );

}

void VVariablesSection::_scalarVarChanged( std::string var ) {
    _rParams->SetVariableName( var );
}

void VVariablesSection::_xFieldVarChanged( std::string var ) {
    _rParams->SetXFieldVariableName( var );
}

void VVariablesSection::_yFieldVarChanged( std::string var ) {
    _rParams->SetYFieldVariableName( var );
}

void VVariablesSection::_zFieldVarChanged( std::string var ) {
    _rParams->SetZFieldVariableName( var );
}

void VVariablesSection::_colorVarChanged( std::string var ) {
    _rParams->SetColorMapVariableName( var );
}

void VVariablesSection::_heightVarChanged( std::string var ) {
    _rParams->SetHeightVariableName( var );
}

int VVariablesSection::GetActiveDimension() const {

}

DimFlags VVariablesSection::GetDimFlags() const {

}

void VVariablesSection::Configure2DFieldVars() {

}

void VVariablesSection::Configure3DFieldVars() {

}
