#include "FlowSubtabs.h"
#include "VariablesWidget.h"
#include "TFWidget.h"
#include "GeometryWidget.h"
#include "CopyRegionWidget.h"
#include "TransformTable.h"
#include "ColorbarWidget.h"
#include "VaporWidgets.h"

QVaporSubtab::QVaporSubtab(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0,0,0,0);
    _layout->insertSpacing(-1, 20);
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum);
}


//
//================================
//
FlowVariablesSubtab::FlowVariablesSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _variablesWidget = new VariablesWidget(this);
    _variablesWidget->Reinit(   (VariableFlags)(VECTOR | COLOR),
                                (DimFlags)(THREED) );
    _layout->addWidget( _variablesWidget, 0, 0 );

    _steady = new VCheckBox( this, "Use Steady Flow" );
    _layout->addWidget( _steady );

    _xMultiplier = new QLineEdit( this );
    _layout->addWidget( _xMultiplier );

    _yMultiplier = new QLineEdit( this );
    _layout->addWidget( _yMultiplier );

    _zMultiplier = new QLineEdit( this );
    _layout->addWidget( _zMultiplier );

    connect( _steady,   SIGNAL( _checkboxClicked() ),   this, SLOT( _steadyGotClicked() ) );
    connect( _xMultiplier, SIGNAL( returnPressed() ),   this, SLOT( _velocityMultiplierChanged() ) );
    connect( _yMultiplier, SIGNAL( returnPressed() ),   this, SLOT( _velocityMultiplierChanged() ) );
    connect( _zMultiplier, SIGNAL( returnPressed() ),   this, SLOT( _velocityMultiplierChanged() ) );
    connect( _xMultiplier, SIGNAL( editingFinished() ), this, SLOT( _velocityMultiplierChanged() ) );
    connect( _yMultiplier, SIGNAL( editingFinished() ), this, SLOT( _velocityMultiplierChanged() ) );
    connect( _zMultiplier, SIGNAL( editingFinished() ), this, SLOT( _velocityMultiplierChanged() ) );
}

void 
FlowVariablesSubtab::Update( VAPoR::DataMgr      *dataMgr,
                             VAPoR::ParamsMgr    *paramsMgr,
                             VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);

    // Update custom widgets
    bool isSteady = _params->GetIsSteady();
    _steady->SetCheckState( isSteady );

    const auto& vec = _params->GetVelocityMultiplier();
    _xMultiplier->setText( QString::number( vec.at(0), 'f', 3 ) );
    _yMultiplier->setText( QString::number( vec.at(1), 'f', 3 ) );
    _zMultiplier->setText( QString::number( vec.at(2), 'f', 3 ) );
}

void
FlowVariablesSubtab::_steadyGotClicked()
{
    bool userInput = _steady->GetCheckState();
    _params->SetIsSteady( userInput );
}

void
FlowVariablesSubtab::_velocityMultiplierChanged()
{
    std::vector<double> input( 3, 1.0 );
    bool ok;
    double d;
    d = _xMultiplier->text().toDouble( &ok );
    if( ok )    // We don't need this verification once the line edit has its own validator
        input[0] = d;
    d = _yMultiplier->text().toDouble( &ok );
    if( ok )    
        input[1] = d;
    d = _zMultiplier->text().toDouble( &ok );
    if( ok )    
        input[2] = d;
    
    _params->SetVelocityMultiplier( input );
}


//
//================================
//
FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _TFWidget = new TFWidget(this);
    _TFWidget->Reinit((TFFlags)(SAMPLING | CONSTANT_COLOR));

    _layout->addWidget( _TFWidget, 0, 0 );

    _params = NULL;
}

void FlowAppearanceSubtab::Update(  VAPoR::DataMgr *dataMgr,
                                    VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);

    _TFWidget->Update(dataMgr, paramsMgr, rParams);
}


//
//================================
//
FlowSeedingSubtab::FlowSeedingSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _geometryWidget   = new GeometryWidget(this);
    _geometryWidget->Reinit( 
        (DimFlags)THREED,
        (VariableFlags)VECTOR
    );
    _layout->addWidget( _geometryWidget );
   
    _fileReader = new VFileReader( this, "Seed File" );
    _layout->addWidget( _fileReader );
}

void FlowSeedingSubtab::Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams
    )
{
    //VAPoR::Box* rakeBox = rParams->GetRakeBox();
    //_geometryWidget->Update(paramsMgr, dataMgr, rParams, rakeBox);
    _geometryWidget->Update(paramsMgr, dataMgr, rParams );
}

void FlowSeedingSubtab::_pushTestPressed() 
{
    cout << "Push button pressed" << endl;
}

void FlowSeedingSubtab::_comboBoxSelected( int index ) 
{
    string option = "*** Need to turn on _comboTest at FlowSubtabs.cpp:107";
    cout << "Combo selected at index " << index << " for option " << option << endl;
}

void FlowSeedingSubtab::_checkBoxSelected() 
{
    bool checked = 0;//_checkboxTest->GetCheckState();
    cout << "Checkbox is checked? " << checked << endl;
}


//
//================================
//
FlowGeometrySubtab::FlowGeometrySubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _geometryWidget   = new GeometryWidget(this);
    _copyRegionWidget = new CopyRegionWidget(this);
    _transformTable   = new TransformTable(this);
    _geometryWidget->Reinit( 
        (DimFlags)THREED,
        (VariableFlags)VECTOR
    );

    _layout->addWidget( _geometryWidget, 0 ,0 );
    _layout->addWidget( _copyRegionWidget, 0 ,0 );
    _layout->addWidget( _transformTable, 0 ,0 );

    _params = NULL;
}

void FlowGeometrySubtab::Update( VAPoR::ParamsMgr *paramsMgr,
                                 VAPoR::DataMgr *dataMgr,
                                 VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}


//
//================================
//
FlowAnnotationSubtab::FlowAnnotationSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _colorbarWidget = new ColorbarWidget(this);
    _layout->addWidget(_colorbarWidget, 0, 0);
}

void FlowAnnotationSubtab::Update(  VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::DataMgr *dataMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _colorbarWidget->Update(dataMgr, paramsMgr, rParams);
}
