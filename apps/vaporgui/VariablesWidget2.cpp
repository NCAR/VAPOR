#include "VariablesWidget2.h"

#include "PDisplay.h"

#include "VComboBox.h"
#include "FidelityWidget.h"
#include "FidelityWidget2.h"
#include "VLineComboBox.h"

#include "vapor/RenderParams.h"

#include <QLayout>
#include <QLabel>

const std::string VariablesWidget2::_sectionTitle = "Variable Selection";

VariablesWidget2::VariablesWidget2() 
    : VSection( _sectionTitle ),
    _activeDim( 3 ),
    _initialized( false )
{
    _dimCombo = new VComboBox( {"3D", "2D"} );
    _dimLineItem = new VLineItem( "Variable Dimension", _dimCombo );
    layout()->addWidget( _dimLineItem );
    connect( _dimCombo, &VComboBox::ValueChanged,
        this, &VariablesWidget2::_dimChanged );

    _dimLineItem->hide();

    _scalarCombo = new VLineComboBox( "Variable name" );
    connect( _scalarCombo, &VLineComboBox::ValueChanged,
        this, &VariablesWidget2::_scalarVarChanged );
    layout()->addWidget( _scalarCombo );

    _fidelityWidget = new FidelityWidget2();
    layout()->addWidget( _fidelityWidget );
    layout()->addWidget( new FidelityWidget(nullptr) );

    layout()->addWidget( new VSection( "Bar of foo" ) ); 
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
        cout << "showing vecs" << endl;
        /*_xFieldCombo->show();
        _yFieldCombo->show();
        _zFieldCombo->show();
        _xFieldCombo->setVisible( true );
        _yFieldCombo->setVisible( true );
        _zFieldCombo->setVisible( true );*/
    }
    else {
        cout << "hiding vecs" << endl;
        /*_xFieldCombo->hide();
        _yFieldCombo->hide();
        _zFieldCombo->hide();
        _xFieldCombo->setVisible( false );
        _yFieldCombo->setVisible( false );
        _zFieldCombo->setVisible( false );*/
    }

    if ( _variableFlags & HEIGHT ) {
        //_heightCombo->show();
    }
    else {
        //_heightCombo->hide();
    }

    //_rParams->SetDefaultVariables( _activeDim, false );

    // If the renderer is only 3D, hide the 2D orientation selector
    /*orientationFrame->hide();

    VariableFlags fdf = (VariableFlags)0;
    if (_variableFlags & SCALAR)
        fdf = (VariableFlags)(fdf | SCALAR);

    if (_variableFlags & VECTOR)
        fdf = (VariableFlags)(fdf | VECTOR);

    if (_variableFlags & HEIGHT)
        fdf = (VariableFlags)(fdf | HEIGHT);

    _fidelityWidget->Reinit(fdf); */

    //variableSelectionWidget->adjustSize();
    //adjustSize();
}

void VariablesWidget2::Update(
    //const VAPoR::DataMgr *dataMgr,
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    //_pg->Update( _rParams, _paramsMgr, _dataMgr );
    _fidelityWidget->Update( _dataMgr, _paramsMgr, _rParams );

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
};

void VariablesWidget2::_dimChanged() {
    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    _rParams->SetDefaultVariables( _activeDim, false );

    Update( _dataMgr, _paramsMgr, _rParams );
}

void VariablesWidget2::_scalarVarChanged( std::string var ) {
    std::cout << "scalarVarChanged " << var << endl;
    _rParams->SetVariableName( var );
}

int VariablesWidget2::GetActiveDimension() const {

}

DimFlags VariablesWidget2::GetDimFlags() const {

}

void VariablesWidget2::Configure2DFieldVars() {

}

void VariablesWidget2::Configure3DFieldVars() {

}
