#include "vapor/RenderParams.h"

#include "PVariablesWidget.h"
//#include "VPFidelitySection.h"
#include "VComboBox.h"
#include "VLineComboBox.h"
#include "VContainer.h"

#include <QLayout>
#include <QLabel>
#include <QSpacerItem>

const std::string PVariablesWidget::_sectionTitle = "Variable Selection";

namespace {
    size_t X = 0;
    size_t Y = 1;
    size_t Z = 2;
}

PVariablesWidget::PVariablesWidget() 
    : PWidget( "", _container = new VContainer( new QVBoxLayout ) ),
    _activeDim( 3 ),
    _initialized( false )
{
    //setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    //setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    //setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );

    _vSection = new VSection( "Variable Selection (VSection)" );
    _container->layout()->addWidget( _vSection );

    _dimCombo = new VLineComboBox( "Variable Dimension (VLineComboBox)" );
    _dimCombo->SetOptions( {"3D", "2D"} );
    _vSection->layout()->addWidget( _dimCombo );
    connect( _dimCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_dimChanged );
    
    _pscalarHLIContainer3D = new VContainer();
    _pscalarHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "Variable name (HLI)",
        //&VAPoR::RenderParams::SetVariableName,
        //&VAPoR::RenderParams::GetVariableName
        &VAPoR::RenderParams::GetVariableName,
        &VAPoR::RenderParams::SetVariableName
    );
    _pscalarHLIContainer3D->layout()->addWidget( _pscalarHLI3D );
    _vSection->layout()->addWidget( _pscalarHLIContainer3D );
  
    _pXFieldHLIContainer3D = new VContainer();
    _pXFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  X (HLI)",
        &VAPoR::RenderParams::GetXFieldVariableName,
        &VAPoR::RenderParams::SetXFieldVariableName
    );
    _pXFieldHLIContainer3D->layout()->addWidget( _pXFieldHLI3D );
    _vSection->layout()->addWidget( _pXFieldHLIContainer3D );

    _pYFieldHLIContainer3D = new VContainer();
    _pYFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  Y (HLI)",
        &VAPoR::RenderParams::GetYFieldVariableName,
        &VAPoR::RenderParams::SetYFieldVariableName
    );
    _pYFieldHLIContainer3D->layout()->addWidget( _pYFieldHLI3D );
    _vSection->layout()->addWidget( _pYFieldHLIContainer3D );

    _pZFieldHLIContainer3D = new VContainer();
    _pZFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  Z (HLI)",
        &VAPoR::RenderParams::GetZFieldVariableName,
        &VAPoR::RenderParams::SetZFieldVariableName
    );
    _pZFieldHLIContainer3D->layout()->addWidget( _pZFieldHLI3D );
    _vSection->layout()->addWidget( _pZFieldHLIContainer3D );

    _pheightHLIContainer2D = new VContainer();
    _pheightHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Color variable (HLI)",
        &VAPoR::RenderParams::GetHeightVariableName,
        &VAPoR::RenderParams::SetHeightVariableName
    );
    _pheightHLIContainer2D->layout()->addWidget( _pheightHLI2D );
    _vSection->layout()->addWidget( _pheightHLIContainer2D );

    _pcolorHLIContainer3D = new VContainer();
    _pcolorHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "Color variable (HLI)",
        &VAPoR::RenderParams::GetColorMapVariableName,
        &VAPoR::RenderParams::SetColorMapVariableName
    );
    _pcolorHLIContainer3D->layout()->addWidget( _pcolorHLI3D );
    _vSection->layout()->addWidget( _pcolorHLIContainer3D );

    _pscalarHLIContainer2D = new VContainer();
    _pscalarHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Variable name (HLI)",
        //&VAPoR::RenderParams::SetVariableName,
        //&VAPoR::RenderParams::GetVariableName
        &VAPoR::RenderParams::GetVariableName,
        &VAPoR::RenderParams::SetVariableName
    );
    _pscalarHLIContainer2D->layout()->addWidget( _pscalarHLI2D );
    _vSection->layout()->addWidget( _pscalarHLIContainer2D );
  
    _pXFieldHLIContainer2D = new VContainer();
    _pXFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  X (HLI)",
        &VAPoR::RenderParams::GetXFieldVariableName,
        &VAPoR::RenderParams::SetXFieldVariableName
    );
    _pXFieldHLIContainer2D->layout()->addWidget( _pXFieldHLI2D );
    _vSection->layout()->addWidget( _pXFieldHLIContainer2D );

    _pYFieldHLIContainer2D = new VContainer();
    _pYFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  Y (HLI)",
        &VAPoR::RenderParams::GetYFieldVariableName,
        &VAPoR::RenderParams::SetYFieldVariableName
    );
    _pYFieldHLIContainer2D->layout()->addWidget( _pYFieldHLI2D );
    _vSection->layout()->addWidget( _pYFieldHLIContainer2D );

    _pZFieldHLIContainer2D = new VContainer();
    _pZFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  Z (HLI)",
        &VAPoR::RenderParams::GetZFieldVariableName,
        &VAPoR::RenderParams::SetZFieldVariableName
    );
    _pZFieldHLIContainer2D->layout()->addWidget( _pZFieldHLI2D );
    _vSection->layout()->addWidget( _pZFieldHLIContainer2D );

    _pcolorHLIContainer2D = new VContainer();
    _pcolorHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Color variable (HLI)",
        &VAPoR::RenderParams::GetColorMapVariableName,
        &VAPoR::RenderParams::SetColorMapVariableName
    );
    _pcolorHLIContainer2D->layout()->addWidget( _pcolorHLI2D );
    _vSection->layout()->addWidget( _pcolorHLIContainer2D );

    _fidelityWidget = new VPFidelitySection();
    _container->layout()->addWidget( _fidelityWidget );

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

    //
    // PWidgetHLI
    // 
    if ( _activeDim == 2 ) {
        _pscalarHLIContainer3D->hide();
        _pXFieldHLI3D->hide();
        _pYFieldHLI3D->hide();
        _pZFieldHLI3D->hide();
        _pcolorHLI3D->hide();
        if (_variableFlags & SCALAR) {
            _pscalarHLIContainer2D->show();
        }
        else {
            _pscalarHLIContainer2D->hide();
        }

        if ( _variableFlags & VECTOR ) {
            _pXFieldHLIContainer2D->show();
            _pYFieldHLIContainer2D->show();
            _pZFieldHLIContainer2D->show();
        }
        else {
            _pXFieldHLIContainer2D->hide();
            _pYFieldHLIContainer2D->hide();
            _pZFieldHLIContainer2D->hide();
        }

        if ( _variableFlags & HEIGHT ) {
            _pheightHLIContainer2D->show();
        }
        else {
            _pheightHLIContainer2D->hide();
        }

        if ( _variableFlags & COLOR ) {
            _pcolorHLIContainer2D->show();
        }
        else {
            _pcolorHLIContainer2D->hide();
        }
    }
    else {
        _pscalarHLIContainer2D->hide();
        _pXFieldHLIContainer2D->hide();
        _pYFieldHLIContainer2D->hide();
        _pZFieldHLIContainer2D->hide();
        _pcolorHLIContainer2D->hide();
        if (_variableFlags & SCALAR) {
            _pscalarHLIContainer3D->show();
        }
        else {
            _pscalarHLIContainer3D->hide();
        }

        if ( _variableFlags & VECTOR ) {
            _pXFieldHLIContainer3D->show();
            _pYFieldHLIContainer3D->show();
            _pZFieldHLIContainer3D->show();
        }
        else {
            _pXFieldHLIContainer3D->hide();
            _pYFieldHLIContainer3D->hide();
            _pZFieldHLIContainer3D->hide();
        }

        if ( _variableFlags & HEIGHT ) {
            _pheightHLIContainer2D->show();
        }
        else {
            _pheightHLIContainer2D->hide();
        }

        if ( _variableFlags & COLOR ) {
            _pcolorHLIContainer3D->show();
        }
        else {
            _pcolorHLIContainer3D->hide();
        }
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
    _fidelityWidget->Reinit( (VariableFlags)(SCALAR) );
}

void PVariablesWidget::updateGUI() const {
    
    std::vector<std::string> twoDVars = _dataMgr->GetDataVarNames( 2 );

    std::vector<std::string> activeVars;
    if ( _activeDim == 3 ) {
        activeVars = _dataMgr->GetDataVarNames( 3 );
    }
    else {
        activeVars = twoDVars;
    }

    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );
    _fidelityWidget->Update( rParams, _paramsMgr, _dataMgr );

    //
    // Our widgets must be updated manually because they cannot
    // be added to a PGroup, which would otherwise update them.
    //
    // They cannot be added to a PGroup becuase they need to be
    // wrapped in a VContainer in order to be shown/hidden.
    //
    _pscalarHLI2D->Update( rParams, _paramsMgr, _dataMgr );
    _pXFieldHLI2D->Update( rParams, _paramsMgr, _dataMgr );
    _pYFieldHLI2D->Update( rParams, _paramsMgr, _dataMgr );
    _pZFieldHLI2D->Update( rParams, _paramsMgr, _dataMgr );
    _pheightHLI2D->Update( rParams, _paramsMgr, _dataMgr );
    _pcolorHLI2D->Update( rParams, _paramsMgr, _dataMgr );

    _pscalarHLI3D->Update( rParams, _paramsMgr, _dataMgr );
    _pXFieldHLI3D->Update( rParams, _paramsMgr, _dataMgr );
    _pYFieldHLI3D->Update( rParams, _paramsMgr, _dataMgr );
    _pZFieldHLI3D->Update( rParams, _paramsMgr, _dataMgr );
    _pcolorHLI3D->Update( rParams, _paramsMgr, _dataMgr );
};

void PVariablesWidget::_dimChanged() {
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( _params );

    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    rParams->SetDefaultVariables( _activeDim, false );
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
