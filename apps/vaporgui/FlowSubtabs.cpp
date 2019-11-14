#include "FlowSubtabs.h"
#include "VaporWidgets.h"

#define disableNoOps 1
#define verbose 1

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

    /*_velocityMltp = new QLineEdit( this );
    _layout->addWidget( _velocityMltp );

    _steady = new VCheckBox( this, "Use Steady Flow" );
    _layout->addWidget( _steady );

    _steadyNumOfSteps = new QLineEdit( this );
    _layout->addWidget( _steadyNumOfSteps);

    _periodicX = new VCheckBox( this, "Particles periodic in X" );
    _layout->addWidget( _periodicX );
    _periodicY = new VCheckBox( this, "Particles periodic in Y" );
    _layout->addWidget( _periodicY );
    _periodicZ = new VCheckBox( this, "Particles periodic in Z" );
    _layout->addWidget( _periodicZ );

    connect( _steady,           SIGNAL( _checkboxClicked() ), this, SLOT( _steadyGotClicked() ) );
    connect( _velocityMltp,     SIGNAL( editingFinished() ),  this, SLOT( _velocityMultiplierChanged() ) );
    connect( _steadyNumOfSteps, SIGNAL( editingFinished() ),  this, SLOT( _steadyNumOfStepsChanged() ) );

    connect( _periodicX,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicY,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicZ,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
*/
}

void 
FlowVariablesSubtab::Update( VAPoR::DataMgr      *dataMgr,
                             VAPoR::ParamsMgr    *paramsMgr,
                             VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);

/*
    // Update custom widgets
    bool isSteady = _params->GetIsSteady();
    _steady->SetCheckState( isSteady );

    auto mltp = _params->GetVelocityMultiplier();
    _velocityMltp->setText( QString::number( mltp, 'f', 3 ) );

    int numOfSteps = _params->GetSteadyNumOfSteps();
    _steadyNumOfSteps->setText( QString::number( numOfSteps ) );

    auto bools = _params->GetPeriodic();
    _periodicX->SetCheckState( bools[0] );
    _periodicY->SetCheckState( bools[1] );
    _periodicZ->SetCheckState( bools[2] );
*/
}
   
/* 
void 
FlowVariablesSubtab::_periodicClicked()
{
    std::vector<bool> bools( 3, false );
    cout << "periodicity changed " << bools[0] << " " << bools[1] << " " << bools[2] << endl;
    bools[0] = _periodicX->GetCheckState();
    bools[1] = _periodicY->GetCheckState();
    bools[2] = _periodicZ->GetCheckState();
    _params->SetPeriodic( bools );
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
    bool ok;
    double d = _velocityMltp->text().toDouble( &ok );
    if( ok )    // Scott: this verification is no longer needed once the line edit has its own validator
        _params->SetVelocityMultiplier( d );
}

void 
FlowVariablesSubtab::_steadyNumOfStepsChanged()
{
    bool ok;
    int i = _steadyNumOfSteps->text().toInt( &ok );
    if( ok )    // Scott: this verification is no longer needed once the line edit has its own validator
        _params->SetSteadyNumOfSteps( i );
}
*/

//
//================================
//
FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget* parent) : 
    QVaporSubtab(parent),
    _params(nullptr)
{
    _streamlineAppearanceTab = new VTabWidget( this, "Streamline Appearance Settings" );

    _shapeCombo = new VComboBox( this, "Integration type" );
    _shapeCombo->AddOption( "Tube", 0 );
    _shapeCombo->AddOption( "Points", 1 );
    _shapeCombo->AddOption( "Arrows", 2 );
    _streamlineAppearanceTab->AddWidget( _shapeCombo );
    _streamlineAppearanceTab->AddWidget( _shapeCombo );
    if (disableNoOps) _shapeCombo->setEnabled( false );

    _colorCombo = new VComboBox( this, "Color" );
    _colorCombo->AddOption( "Constant" );
    _colorCombo->AddOption( "Color mapped variable" );
    _colorCombo->AddOption( "Distance from start" );
    _colorCombo->AddOption( "Seed index" );
    _streamlineAppearanceTab->AddWidget( _colorCombo );
    if (disableNoOps) _colorCombo->setEnabled( false );

    _lengthSpinBox = new VSpinBox( this, "Length" );
    _streamlineAppearanceTab->AddWidget( _lengthSpinBox );
    if (disableNoOps) _lengthSpinBox->setEnabled( false );

    _sizeSpinBox = new VSpinBox( this, "Size" );
    _streamlineAppearanceTab->AddWidget( _sizeSpinBox );
    if (disableNoOps) _sizeSpinBox->setEnabled( false );

    _smoothnessSliderEdit = new QSliderEdit( this );
    _smoothnessSliderEdit->SetLabel( "Smoothness" );
    if (disableNoOps) _smoothnessSliderEdit->setEnabled( false );
    _streamlineAppearanceTab->AddWidget( _smoothnessSliderEdit );

    _layout->addWidget( _streamlineAppearanceTab );

    _TFWidget = new TFWidget(this);
    _TFWidget->Reinit((TFFlags)(SAMPLING | CONSTANT_COLOR));

    //_layout->addWidget( _TFWidget, 0, 0 );
    _layout->addWidget( _TFWidget );

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
FlowIntegrationSubtab::FlowIntegrationSubtab(
    QWidget* parent) : 
    QVaporSubtab(parent),
    _dataMgr(nullptr),
    _paramsMgr(nullptr),
    _params(nullptr),
    _initialized(false)
{
    _integrationSettingsTab = new VTabWidget( this, "Flow Integration Settings" );

    _integrationTypeCombo = new VComboBox( this, "Integration type" );
    _integrationTypeCombo->AddOption( "Steady", 0 );
    _integrationTypeCombo->AddOption( "Unsteady", 1 );
    _integrationSettingsTab->AddWidget( _integrationTypeCombo );
    connect( _integrationTypeCombo, SIGNAL( _indexChanged(int) ),
        this, SLOT( _configureIntegrationType() ));

    _integrationLengthEdit = new VLineEdit( this, "Integration length" );
    _integrationSettingsTab->AddWidget( _integrationLengthEdit );
    connect( _integrationLengthEdit, SIGNAL( _editingFinished() ),
        this, SLOT( _integrationLengthChanged() ) );
    if (disableNoOps)
        _integrationLengthEdit->setEnabled( false );

    _directionCombo = new VComboBox( this, "Integration direction" );
    _directionCombo->AddOption( "Forward", 0 );
    _directionCombo->AddOption( "Backward", 1 );
    _directionCombo->AddOption( "Bi-directional", 2 );
    _integrationSettingsTab->AddWidget( _directionCombo );
    connect( _directionCombo, SIGNAL( _indexChanged( int ) ),
        this, SLOT( _integrationDirectionChanged() ) );

    _startSpinBox = new VSpinBox( this, "Injection start time step", 0 );
    _integrationSettingsTab->AddWidget( _startSpinBox );
    if (disableNoOps) 
        _startSpinBox->setEnabled(false);

    _endSpinBox = new VSpinBox( this, "Injection end time step" );
    _integrationSettingsTab->AddWidget( _endSpinBox );
    if (disableNoOps) 
        _endSpinBox->setEnabled(false);

    _lifespanSpinBox = new VSpinBox( this, "Seed lifespan after injection" );
    _integrationSettingsTab->AddWidget( _lifespanSpinBox );
    if (disableNoOps) 
        _lifespanSpinBox->setEnabled(false);

    _intervalSpinBox = new VSpinBox( this, "Seed injection interval", 1);
    _integrationSettingsTab->AddWidget( _intervalSpinBox );
    if (disableNoOps) 
        _intervalSpinBox->setEnabled(false);
    
    _periodicBoundaryComboX = new VCheckBox( this, "X axis periodicity" );
    _integrationSettingsTab->AddWidget( _periodicBoundaryComboX );
    connect( _periodicBoundaryComboX, SIGNAL( _checkboxClicked() ),
        this, SLOT( _periodicityChanged() ) );

    _periodicBoundaryComboY = new VCheckBox( this, "Y axis periodicity" );
    _integrationSettingsTab->AddWidget( _periodicBoundaryComboY );
    connect( _periodicBoundaryComboY, SIGNAL( _checkboxClicked() ),
        this, SLOT( _periodicityChanged() ) );

    _periodicBoundaryComboZ = new VCheckBox( this, "Z axis periodicity" );
    _integrationSettingsTab->AddWidget( _periodicBoundaryComboZ );
    connect( _periodicBoundaryComboZ, SIGNAL( _checkboxClicked() ),
        this, SLOT( _periodicityChanged() ) );

    _multiplierLineEdit = new VLineEdit( this, "Vector field multiplier" );
    _integrationSettingsTab->AddWidget( _multiplierLineEdit );
    connect( _multiplierLineEdit, SIGNAL( _editingFinished() ),
        this, SLOT( _multiplierChanged() ) );

    _configureIntegrationType();
    _layout->addWidget( _integrationSettingsTab );

}

void FlowIntegrationSubtab::_periodicityChanged() {
    std::vector<bool> periodicity = {
        _periodicBoundaryComboX->GetCheckState(),
        _periodicBoundaryComboY->GetCheckState(),
        _periodicBoundaryComboZ->GetCheckState()
    };
    if (verbose) std::cout << "GUI changed periodicity to " << periodicity[0] << " " << periodicity[1] << " " << periodicity[2] << endl;
    _params->SetPeriodic( periodicity );
    if (verbose) {
        std::vector<bool> bools = _params->GetPeriodic();
        cout << "Params periodicity changed to " << bools[0] << " " << bools[1] << " " << bools[2] << endl << endl;
    }
}

void FlowIntegrationSubtab::_integrationLengthChanged() {
    long value = stod(_integrationLengthEdit->GetEditText());
    if (verbose) std::cout << "integration length edit changed to " << value << std::endl;
    _params->SetSteadyNumOfSteps( value );
    if (verbose) std::cout << "integration length params changed to " << _params->GetSteadyNumOfSteps() << std::endl;
    if (verbose) std::cout << std::endl;
}

void FlowIntegrationSubtab::_integrationDirectionChanged() {
    int paramsValue;
    string direction = _directionCombo->GetCurrentText();
    if ( direction == "Forward" )
        paramsValue = 0;
    if ( direction == "Backward" )
        paramsValue = 1;
    if (direction == "Bi-directional" )
        paramsValue = 2;

    if (verbose) std::cout << "Direction combo changed to " << direction << endl;

    _params->SetFlowDirection( paramsValue );

    if (verbose) std::cout << "Params direction changed to " << _params->GetFlowDirection() << endl;
    if (verbose) std::cout << endl;
}

void FlowIntegrationSubtab::_multiplierChanged() {
    double value = stod(_multiplierLineEdit->GetEditText());
    if (verbose) std::cout << "Vector multiplier line edit changed to " << value << std::endl;
    _params->SetVelocityMultiplier( value );
    if (verbose) std::cout << "Vector multiplier params changed to " << _params->GetVelocityMultiplier() << std::endl;
    if (verbose) std::cout << std::endl;
}

void FlowIntegrationSubtab::_configureIntegrationType() {
    bool isSteady = true;
    string seedType = _integrationTypeCombo->GetCurrentText();
    if ( seedType == "Steady" ) {
        _startSpinBox->hide();
        _endSpinBox->hide();
        _lifespanSpinBox->hide();
        _intervalSpinBox->hide();
        _directionCombo->show();
        _integrationLengthEdit->show();
    }
    else {
        isSteady = false;
        _params->SetIsSteady( false );
        _startSpinBox->show();
        _endSpinBox->show();
        _lifespanSpinBox->show();
        _intervalSpinBox->show();
        _directionCombo->hide();
        _integrationLengthEdit->hide();
    }
    
    if (verbose) std::cout << "Integration combo changed to " << seedType << endl;

    if ( _params != nullptr ) {
        _params->SetIsSteady( isSteady );
        if (verbose) std::cout << "Integration params changed to " << _params->GetIsSteady() << endl;
    }

    if (verbose) std::cout << std::endl;
}

void FlowIntegrationSubtab::_initialize() {
    int numTimeSteps = _dataMgr->GetNumTimeSteps();
    _endSpinBox->SetValue( numTimeSteps );
    _lifespanSpinBox->SetValue( numTimeSteps );
}

void FlowIntegrationSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);

    if (!_initialized) {
        _initialize();
        _initialized = true;
    }

    bool isSteady = _params->GetIsSteady();
    if ( isSteady )
        _integrationTypeCombo->SetIndex( 0 );
    else
        _integrationTypeCombo->SetIndex( 1 );

    // Do we really want to use magic numbers for these?
    // I think that storing strings would make the code more readable.
    // -Scott
    int paramsValue = _params->GetFlowDirection();
    if ( paramsValue == 0 )
        _directionCombo->SetIndex( 0 );
    if ( paramsValue == 1 )
        _directionCombo->SetIndex( 1 );
    if (paramsValue == 2 )
        _directionCombo->SetIndex( 2 );

    double multiplier = _params->GetVelocityMultiplier();
    _multiplierLineEdit->SetEditText( std::to_string(multiplier));
}

//
//================================
//
FlowSeedingSubtab::FlowSeedingSubtab(QWidget* parent) : 
    QVaporSubtab(parent),
    _dataMgr(nullptr),
    _paramsMgr(nullptr),
    _params(nullptr)
{
    _seedSettingsTab = new VTabWidget( this, "Seed Distribution Settings" );
    _distributionCombo = new VComboBox( this, "Seed distribution type" );
    _distributionCombo->AddOption( "Gridded", 0 );
    _distributionCombo->AddOption( "Random", 1 );
    _distributionCombo->AddOption( "List of points", 2 );
    _seedSettingsTab->AddWidget( _distributionCombo );
    connect( _distributionCombo, SIGNAL( _indexChanged(int) ),
        this, SLOT( _configureRakeType() ));

    //
    // Random rake options
    //
    _randomCountSpinBox = new VSpinBox( this, "Number of random seeds", 64 );
    _seedSettingsTab->AddWidget( _randomCountSpinBox );
    if (disableNoOps) 
        _randomCountSpinBox->setEnabled( false );

    _biasVariableCombo = new VComboBox( this, "Random distribution bias variable" );
    _seedSettingsTab->AddWidget( _biasVariableCombo );
    if (disableNoOps) 
        _biasVariableCombo->setEnabled( false );

    _biasSliderEdit = new QSliderEdit( this );
    _biasSliderEdit->SetLabel( "Bias weight" );
    _seedSettingsTab->AddWidget( _biasSliderEdit );
    if (disableNoOps) 
        _biasSliderEdit->setEnabled( false );

    //
    // Gridded rake options
    //
    _xDistributionSpinBox = new VSpinBox( this, "X axis seeds", 8 );
    _seedSettingsTab->AddWidget( _xDistributionSpinBox );
    if (disableNoOps) 
        _xDistributionSpinBox->setEnabled( false );
    
    _yDistributionSpinBox = new VSpinBox( this, "Y axis seeds", 8 );
    _seedSettingsTab->AddWidget( _yDistributionSpinBox );
    if (disableNoOps) 
        _yDistributionSpinBox->setEnabled( false );
    
    _zDistributionSpinBox = new VSpinBox( this, "Z axis seeds", 8 );
    _seedSettingsTab->AddWidget( _zDistributionSpinBox );
    if (disableNoOps) 
        _zDistributionSpinBox->setEnabled( false );

    // List of seed file picker
    _seedpointFileReader = new VFileReader( this, "Seed File" );
    _seedSettingsTab->AddWidget( _seedpointFileReader );
    _layout->addWidget( _seedSettingsTab );
    connect( _seedpointFileReader, SIGNAL( _pathChanged() ),
        this, SLOT( _seedInputFileChanged() ) );

    // Rake region selector 
    _geometryWidget   = new GeometryWidget(this);
    _geometryWidget->Reinit( 
        (DimFlags)THREED,
        (VariableFlags)VECTOR
    );
    _layout->addWidget( _geometryWidget );

    _configureRakeType();

    _exportGeometryWriter = new VFileWriter( 
        this, 
        "Export geometry file",
        "Select",
        QDir::homePath().toStdString()
    );
    _exportGeometryWriter->SetFileFilter( (std::string)"*.txt" );
    _layout->addWidget( _exportGeometryWriter );
    connect( _exportGeometryWriter, SIGNAL( _pathChanged() ),
        this, SLOT( _exportGeometryPathChanged() ) );


    _exportGeometryButton = new VPushButton(
        this,
        "",
        "Export"
    );
    _layout->addWidget( _exportGeometryButton );
    connect( _exportGeometryButton, SIGNAL( _pressed() ),
        this, SLOT( _exportButtonClicked() ) );
    
//
// ToDo: Turn off hack to set default to LIST OF SEEDS!!!    
//
    _distributionCombo->SetIndex(2);
}

void FlowSeedingSubtab::Update( VAPoR::DataMgr      *dataMgr,
                                VAPoR::ParamsMgr    *paramsMgr,
                                VAPoR::RenderParams *params )
{
    _params = dynamic_cast<VAPoR::FlowParams*>(params);

    //VAPoR::Box* rakeBox = params->GetRakeBox();
    //_geometryWidget->Update(paramsMgr, dataMgr, params, rakeBox);
    _geometryWidget->Update(paramsMgr, dataMgr, params );

    long genMode = _params->GetSeedGenMode();
    if ( genMode == 3 || genMode == 4 )
        _distributionCombo->SetIndex( 1 );
    else if ( genMode == 2 )
        _distributionCombo->SetIndex( 0 );
    else if ( genMode == 1 )
        _distributionCombo->SetIndex( 2 );

    string file = _params->GetSeedInputFilename();
    _seedpointFileReader->SetPath( file );

    file = _params->GetFlowlineOutputFilename();
    _exportGeometryWriter->SetPath( file );
}

void
FlowSeedingSubtab::_exportButtonClicked( )
{

//  I'm a little unclear on how this is currently configured.
//  I believe that triggering a file export would be an event issued by the
//  user, not something that would b e enabled or disabled.
    if ( _params->GetNeedFlowlineOutput() )
        _params->SetNeedFlowlineOutput( false );
    else
        _params->SetNeedFlowlineOutput( true);
    if (verbose) std::cout << "exportButton set NeedFlowlineOutput to " << _params->GetNeedFlowlineOutput() << endl;
}

void FlowSeedingSubtab::_exportGeometryPathChanged() {
    string path = _exportGeometryWriter->GetPath();
    if (verbose) std::cout << "Geometry path dialog set " << path << std::endl;
    _params->SetFlowlineOutputFilename( path );
    if (verbose) std::cout << "Geometry path params set " << _params->GetFlowlineOutputFilename() << std::endl;
    if (verbose) std::cout << std::endl;
}

void FlowSeedingSubtab::_seedInputFileChanged() {
    string file = _seedpointFileReader->GetPath();
    if (verbose) std::cout << "Input seed file changed to " << file << std::endl;
    _params->SetSeedInputFilename( file );
    if (verbose) std::cout << "Input seed params changed to " << _params->GetSeedInputFilename() << endl;
    if (verbose) std::cout << std::endl;
}

void FlowSeedingSubtab::_seedpointReaderPathChanged()
{
    std::string filename = _seedpointFileReader->GetPath();
    _params->SetFlowlineOutputFilename( filename );
}

void FlowSeedingSubtab::_configureRakeType() {
    // FlowParams specifies its rake type with magic numbers.  We need to set 
    // the appropriate value from the following list:
    //  0 - programmatical
    //  1 - list of seeds
    //  2 - uniform
    //  3 - random
    //  4 - random+bias
    //
    long paramsValue;

    string seedType = _distributionCombo->GetCurrentText();
    if ( seedType == "Random" ) {
        paramsValue = 3;

        _randomCountSpinBox->show();
        _biasVariableCombo->show();
        _biasSliderEdit->show();

        _xDistributionSpinBox->hide();
        _yDistributionSpinBox->hide();
        _zDistributionSpinBox->hide();

        _seedpointFileReader->hide();
        
        _geometryWidget->setEnabled(true);
    }
    else if ( seedType == "Gridded" ) {
        paramsValue = 2;
        _randomCountSpinBox->hide();
        _biasVariableCombo->hide();
        _biasSliderEdit->hide();

        _xDistributionSpinBox->show();
        _yDistributionSpinBox->show();
        _zDistributionSpinBox->show();

        _seedpointFileReader->hide();

        _geometryWidget->setEnabled(true);
    }
    else { // ( seedType == "List of points" )
        paramsValue = 1;
        _randomCountSpinBox->hide();
        _biasVariableCombo->hide();
        _biasSliderEdit->hide();

        _xDistributionSpinBox->hide();
        _yDistributionSpinBox->hide();
        _zDistributionSpinBox->hide();

        _seedpointFileReader->show();

        _geometryWidget->setEnabled(false);
    }

    if (verbose) std::cout << "Distribution type combo set to " << seedType << std::endl;

    if ( _params != nullptr) {
        _params->SetSeedGenMode( paramsValue );
        if (verbose) std::cout << "Distribution param set to " << _params->GetSeedGenMode() << std::endl;
    }
    if (verbose) std::cout << std::endl;
}

//
//================================
//
FlowGeometrySubtab::FlowGeometrySubtab(QWidget* parent) : 
    QVaporSubtab(parent),
    _params(nullptr)
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
