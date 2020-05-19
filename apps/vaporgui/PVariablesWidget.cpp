#include "PVariablesWidget.h"

#include "PDisplay.h"

#include "VComboBox.h"
#include "FidelityWidget.h"
#include "FidelityWidget2.h"
#include "VLineComboBox.h"
#include "VContainer.h"
#include "PFidelityWidget3.h"

#include "vapor/RenderParams.h"

#include <QLayout>
#include <QLabel>
#include <QSpacerItem>

const std::string PVariablesWidget::_sectionTitle = "P Variable Selection";

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;
}

PVariablesWidget::PVariablesWidget() 
    //: PWidget( "", _vSection = new VSection( _sectionTitle ) ),
    : PWidget( "", _container = new VContainer( new QVBoxLayout ) ),
    _activeDim( 3 ),
    _initialized( false )
{
    //_vSection = new VSection( _sectionTitle );
    _vSection = new VSection( "V Widgets" );
    _container->layout()->addWidget( _vSection );

    _dimCombo = new VLineComboBox( "Variable Dimension" );
    _dimCombo->SetOptions( {"3D", "2D"} );
    _vSection->layout()->addWidget( _dimCombo );
    connect( _dimCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_dimChanged );

    _scalarCombo = new VLineComboBox( "Variable name" );
    connect( _scalarCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_scalarVarChanged );
    _vSection->layout()->addWidget( _scalarCombo );

    _xFieldCombo = new VLineComboBox( "  X" );
    connect( _xFieldCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_xFieldVarChanged );
    _vSection->layout()->addWidget( _xFieldCombo );

    _yFieldCombo = new VLineComboBox( "  Y" );
    connect( _yFieldCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_yFieldVarChanged );
    _vSection->layout()->addWidget( _yFieldCombo );

    _zFieldCombo = new VLineComboBox( "  Z" );
    connect( _zFieldCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_zFieldVarChanged );
    _vSection->layout()->addWidget( _zFieldCombo );

    _colorCombo = new VLineComboBox( "Color mapped variable" );
    connect( _colorCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_colorVarChanged );
    _vSection->layout()->addWidget( _colorCombo );
    
    _heightCombo = new VLineComboBox( "Height variable" );
    connect( _heightCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_heightVarChanged );
    _vSection->layout()->addWidget( _heightCombo );

    _fidelityWidget = new PFidelityWidget3();
    _container->layout()->addWidget( _fidelityWidget );

    _pvshli = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "PVarSelector3DHLI",
        &VAPoR::RenderParams::SetVariableName,
        &VAPoR::RenderParams::GetVariableName
    );
    _container->layout()->addWidget( _pvshli );
    
    _container->layout()->addItem( 
        new QSpacerItem( 1, 2000, QSizePolicy::Minimum, QSizePolicy::Maximum ) 
    );

}

void PVariablesWidget::Reinit(
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

    //VAPoR::RenderParams* rParams = dynamic_cast<VAPoR::RenderParams*>(_params);
    //rParams->SetDefaultVariables( _activeDim, false );

    VariableFlags fdf = (VariableFlags)0;
    if (_variableFlags & SCALAR)
        fdf = (VariableFlags)(fdf | SCALAR);

    if (_variableFlags & VECTOR)
        fdf = (VariableFlags)(fdf | VECTOR);

    if (_variableFlags & HEIGHT)
        fdf = (VariableFlags)(fdf | HEIGHT);

    _fidelityWidget->Reinit(fdf);

    //variableSelectionWidget->adjustSize();
    //adjustSize();
}

void PVariablesWidget::updateGUI() const {
std::cout << "void PVariablesWidget::updateGUI() const {" << std::endl;
    
    // We can't assign _rParams in a const function, so we need to 
    // dynamic_cast in all functions that use RenderParams
    //_rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );

    std::vector<std::string> twoDVars = _dataMgr->GetDataVarNames( 2 );

    std::vector<std::string> activeVars;
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


    _fidelityWidget->Update( _params, _paramsMgr, _dataMgr );
};

void PVariablesWidget::_dimChanged() {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );

    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    rParams->SetDefaultVariables( _activeDim, false );

    //Update( _dataMgr, _paramsMgr, _params );

}

void PVariablesWidget::_scalarVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    rParams->SetVariableName( var );
}

void PVariablesWidget::_xFieldVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    std::vector< std::string > fieldVars = rParams->GetFieldVariableNames();
    fieldVars[X] = var;
    rParams->SetFieldVariableNames( fieldVars );
}

void PVariablesWidget::_yFieldVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    std::vector< std::string > fieldVars = rParams->GetFieldVariableNames();
    fieldVars[Y] = var;
    rParams->SetFieldVariableNames( fieldVars );
}

void PVariablesWidget::_zFieldVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    std::vector< std::string > fieldVars = rParams->GetFieldVariableNames();
    fieldVars[Z] = var;
    rParams->SetFieldVariableNames( fieldVars );
}

void PVariablesWidget::_colorVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    rParams->SetColorMapVariableName( var );
}

void PVariablesWidget::_heightVarChanged( std::string var ) {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    rParams->SetHeightVariableName( var );
}

int PVariablesWidget::GetActiveDimension() const {

}

DimFlags PVariablesWidget::GetDimFlags() const {

}

void PVariablesWidget::Configure2DFieldVars() {

}

void PVariablesWidget::Configure3DFieldVars() {

}
