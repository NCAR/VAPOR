#include "FlowSubtabs.h"
#include "vapor/DataMgrUtils.h"

#include "VFrame.h"
#include "VSpinBox.h"
#include "VComboBox.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include "VSliderEdit.h"
#include "VLineEdit.h"
#include "VFileSelector.h"
#include "VGeometry2.h"

#include <QScrollArea>

#define verbose     1

#define UNSTEADY_STRING    "Streamlines"
#define STEADY_STRING      "Pathlines"
#define GRIDDED_STRING     "Gridded"
#define LIST_STRING        "List of seeds"
#define RANDOM_STRING      "Random"

#define MIN_AXIS_SEEDS      1
#define MAX_AXIS_SEEDS      1000
#define MIN_RANDOM_SEEDS    1
#define MAX_RANDOM_SEEDS    1000000

#define MAX_PATHLINE_LENGTH 10000

#define X                   0
#define Y                   1
#define Z                   2
#define RANDOM_INDEX        3

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
}

void 
FlowVariablesSubtab::Update( VAPoR::DataMgr      *dataMgr,
                             VAPoR::ParamsMgr    *paramsMgr,
                             VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
}
    

//
//================================
//
FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _TFEditor = new TFEditor(true);

    _layout->addWidget( _TFEditor, 0, 0 );

    _params = NULL;
}

void FlowAppearanceSubtab::Update(  VAPoR::DataMgr *dataMgr,
                                    VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);
    _TFEditor->Update(dataMgr, paramsMgr, rParams);
}


//
//================================
//
FlowSeedingSubtab::FlowSeedingSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _params = nullptr;

    _createIntegrationSection();
    _createSeedingSection( parent );

    // Lastly add file selector for writing seed geometry
    //
    _geometryWriterSection= new VSection("Write Flowlines to File");
    layout()->addWidget( _geometryWriterSection );
    _geometryWriter = new VFileWriter();
    _geometryWriter->HideLineEdit( true );
    connect( _geometryWriter, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _geometryWriterFileChanged( const std::string& ) ) );
    _geometryWriterFrame = new VFrame();
    _geometryWriterSection->layout()->addWidget( new VLineItem("Target file", _geometryWriter) );
    layout()->addWidget( _geometryWriterSection );
}

void FlowSeedingSubtab::_createSeedingSection( QWidget* parent ) {
    _seedDistributionSection = new VSection("Seed Distribution Settings");
    layout()->addWidget( _seedDistributionSection );

    std::vector<std::string> values = {GRIDDED_STRING, RANDOM_STRING, LIST_STRING};
    _seedTypeCombo = new VComboBox(values);
    _seedDistributionSection->layout()->addWidget( new VLineItem("Seed distribution type", _seedTypeCombo ));
    connect( _seedTypeCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _configureSeedType( std::string )));

    // Gridded seed selection
    _griddedSeedsFrame = new VFrame();
    _seedDistributionSection->layout()->addWidget( _griddedSeedsFrame );

    _xSeedSliderEdit = new VSliderEdit();
    _xSeedSliderEdit->SetIntType(true);
    _xSeedSliderEdit->SetRange( MIN_AXIS_SEEDS, MAX_AXIS_SEEDS );
    _griddedSeedsFrame->addWidget( new VLineItem( "X axis seeds", _xSeedSliderEdit ) );
    connect( _xSeedSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );
    _ySeedSliderEdit = new VSliderEdit();
    _ySeedSliderEdit->SetIntType(true);
    _ySeedSliderEdit->SetRange( MIN_AXIS_SEEDS, MAX_AXIS_SEEDS );
    _griddedSeedsFrame->addWidget( new VLineItem("Y axis seeds", _ySeedSliderEdit ) );
    connect( _ySeedSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );
    _zSeedSliderEdit = new VSliderEdit();
    _zSeedSliderEdit->SetIntType(true);
    _zSeedSliderEdit->SetRange( MIN_AXIS_SEEDS, MAX_AXIS_SEEDS );
    _griddedSeedsFrame->addWidget( new VLineItem("Z axis seeds", _zSeedSliderEdit ) );
    connect( _zSeedSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );

    // List of seeds selection
    _listOfSeedsFileReader = new VFileReader();
    connect( _listOfSeedsFileReader, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _seedListFileChanged( const std::string& ) ) );
    _listOfSeedsFrame = new VFrame();
    _listOfSeedsFrame->addWidget( new VLineItem("List of seeds file", _listOfSeedsFileReader ) );
    _seedDistributionSection->layout()->addWidget( _listOfSeedsFrame );

    // Random distribution selection
    _randomSeedsFrame = new VFrame();
    _seedDistributionSection->layout()->addWidget( _randomSeedsFrame );
    
    _randomSeedsSliderEdit = new VSliderEdit( MIN_RANDOM_SEEDS, MAX_RANDOM_SEEDS );
    _randomSeedsSliderEdit->SetIntType( true );
    _randomSeedsFrame->addWidget( new VLineItem("Seed count", _randomSeedsSliderEdit ) );
    connect( _randomSeedsSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );

    _biasWeightSliderEdit = new VSliderEdit(-1, 1, 0);
    _randomSeedsFrame->addWidget( new VLineItem( "Bias weight", _biasWeightSliderEdit ) );
    connect( _biasWeightSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _biasStrengthChanged( double ) ) );

    _biasVariableComboBox = new VComboBox( std::vector<std::string>() );
    _randomSeedsFrame->addWidget( new VLineItem( "Bias variable", _biasVariableComboBox ) );
    connect( _biasVariableComboBox, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _biasVariableChanged( const std::string& ) ) );
    
    // Rake selector
    _rakeRegionSection = new VSection("Rake Region");
    layout()->addWidget( _rakeRegionSection );
    _rakeWidget = new VGeometry2();
    _rakeRegionSection->layout()->addWidget( _rakeWidget );
    connect( _rakeWidget, SIGNAL( ValueChanged( const std::vector<float>& ) ),
        this, SLOT( _rakeGeometryChanged( const std::vector<float>& ) ) );

    VAssert(parent);
    connect(parent, SIGNAL(currentChanged(int)), this, SLOT(_selectedTabChanged(int)));

    _configureSeedType( GRIDDED_STRING );
}

void FlowSeedingSubtab::_createIntegrationSection() {
    _integrationSection = new VSection("Flow Integration Settings");
    layout()->addWidget( _integrationSection );

    // Steady flow options
    //
    std::vector<std::string> values = {STEADY_STRING, UNSTEADY_STRING};
    _flowTypeCombo = new VComboBox(values);
    connect( _flowTypeCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _configureFlowType( std::string )));
    _integrationSection->layout()->addWidget( new VLineItem("Flow type", _flowTypeCombo ));

    _pathlineFrame = new VFrame();
    _integrationSection->layout()->addWidget( _pathlineFrame );

    values = { "Forward", "Backward", "Bi-Directional" };
    _pathlineDirectionCombo = new VComboBox(values);
    connect( _pathlineDirectionCombo, SIGNAL( ValueChanged( int )),
        this, SLOT( _pathlineDirectionChanged( int ) ) );
    _pathlineFrame->addWidget( new VLineItem("Flow direction", _pathlineDirectionCombo) );

    _pathlineSamplesSliderEdit = new VSliderEdit();
    _pathlineSamplesSliderEdit->SetIntType(true);
    connect( _pathlineSamplesSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _pathlineSamplesChanged( int ) ) );
    _pathlineFrame->addWidget( new VLineItem("Number of samples", _pathlineSamplesSliderEdit));

    // Unsteady flow options
    //
    _streamlineFrame = new VFrame();
    _integrationSection->layout()->addWidget( _streamlineFrame );

    _streamlineLengthSliderEdit = new VSliderEdit();
    _streamlineLengthSliderEdit->SetIntType(true);
    connect( _streamlineLengthSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _streamlineLengthChanged( int ) ) );
    _streamlineFrame->addWidget( new VLineItem("Streamline length", _streamlineLengthSliderEdit));

    _streamlineStartSliderEdit= new VSliderEdit();
    _streamlineStartSliderEdit->SetIntType(true);
    connect( _streamlineStartSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _streamlineStartTimeChanged( int ) ) );
    _streamlineFrame->addWidget( new VLineItem("Injection start time", _streamlineStartSliderEdit));
    _streamlineStartSliderEdit->setEnabled(false);

    _streamlineEndSliderEdit = new VSliderEdit();
    _streamlineEndSliderEdit->SetIntType(true);
    connect( _streamlineEndSliderEdit, SIGNAL( ValueChanged( int ) ),
        this, SLOT( _streamlineEndTimeChanged( int ) ) );
    _streamlineFrame->addWidget( new VLineItem("Injection end time", _streamlineEndSliderEdit));
    _streamlineEndSliderEdit->setEnabled(false);
    
    _streamlineInjIntervalSliderEdit = new VSliderEdit();
    _streamlineInjIntervalSliderEdit->SetIntType(true);
    connect( _streamlineInjIntervalSliderEdit,  SIGNAL( ValueChanged( int ) ), 
        this, SLOT( _seedInjIntervalChanged(int) ));
    _streamlineFrame->addWidget( new VLineItem("Injection interval", _streamlineInjIntervalSliderEdit));

    _streamlineLifetimeSliderEdit = new VSliderEdit();
    _streamlineLifetimeSliderEdit->SetIntType(true);
    connect( _streamlineLifetimeSliderEdit,  SIGNAL( ValueChanged( int ) ), 
        this, SLOT( _streamlineLifetimeChanged(int) ) );
    _streamlineFrame->addWidget( new VLineItem("Seed lifetime", _streamlineLifetimeSliderEdit) );
    _streamlineLifetimeSliderEdit->setEnabled(false);

    // Universal options: Velocity multiplier and periodicity checkboxes
    //    
    _velocityMultiplierLineEdit = new VLineEdit();
    _velocityMultiplierLineEdit->SetIsDouble( true );
    connect( _velocityMultiplierLineEdit, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _velocityMultiplierChanged( const std::string& ) ) );
    _integrationSection->layout()->addWidget( new VLineItem("Velocity multiplier", _velocityMultiplierLineEdit));

    // Periodicity Checkboxes
    //
    _periodicXCheckBox = new VCheckBox();
    connect( _periodicXCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( new VLineItem("X axis periodicity", _periodicXCheckBox));
    _periodicYCheckBox = new VCheckBox();
    connect( _periodicYCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( new VLineItem("Y axis periodicity", _periodicYCheckBox));
    _periodicZCheckBox = new VCheckBox();
    connect( _periodicZCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( new VLineItem("Z axis periodicity", _periodicZCheckBox));

    _configureFlowType(STEADY_STRING);
}

void FlowSeedingSubtab::Update( VAPoR::DataMgr      *dataMgr,
                                VAPoR::ParamsMgr    *paramsMgr,
                                VAPoR::RenderParams *params )
{
    _params = dynamic_cast<VAPoR::FlowParams*>(params);
    _paramsMgr = paramsMgr;
    VAssert( _params );

    // Update integration tab
    //
    bool isSteady = _params->GetIsSteady();
    if ( isSteady )
        _flowTypeCombo->SetValue( STEADY_STRING );
    else
        _flowTypeCombo->SetValue( UNSTEADY_STRING );

    _updateSteadyFlowWidgets(dataMgr);
    _updateUnsteadyFlowWidgets(dataMgr);

    // Periodicity checkboxes 
    auto bools = _params->GetPeriodic();
    _periodicXCheckBox->SetValue( bools[X] );
    _periodicYCheckBox->SetValue( bools[Y] );
    _periodicZCheckBox->SetValue( bools[Z] );

    // Velocity multiplier
    auto mltp = _params->GetVelocityMultiplier();
    _velocityMultiplierLineEdit->SetValue( std::to_string( mltp ) );

    // Update seeding tab
    //
    int mode = _params->GetSeedGenMode();
    if ( mode == (int)VAPoR::FlowSeedMode::UNIFORM )
        _seedTypeCombo->SetValue( GRIDDED_STRING );
    else if ( mode == (int)VAPoR::FlowSeedMode::RANDOM  )
        _seedTypeCombo->SetValue( RANDOM_STRING );
    else if ( mode == (int)VAPoR::FlowSeedMode::LIST    )
        _seedTypeCombo->SetValue( LIST_STRING );

    // Random rake values
    std::vector< std::string > vars = dataMgr->GetDataVarNames(3);  // Do we support 2D flow?
    _biasVariableComboBox->SetOptions( vars );
    std::string var = _params->GetRakeBiasVariable();
    _biasVariableComboBox->SetValue( var );
    
    double bias = _params->GetRakeBiasStrength();
    _biasWeightSliderEdit->SetValue( bias );

    // Random and Gridded # seeds
    std::vector<long> seedVec = _params->GetRakeNumOfSeeds();
    _xSeedSliderEdit->SetValue( seedVec[X] );
    _ySeedSliderEdit->SetValue( seedVec[Y] );
    _zSeedSliderEdit->SetValue( seedVec[Z] );
    _randomSeedsSliderEdit->SetValue( seedVec[RANDOM_INDEX] );

    // Update rake
    std::vector<double> minExt, maxExt;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents( dataMgr, 
                                     _params->GetCurrentTimestep(), 
                                     _params->GetFieldVariableNames(),         
                                     _params->GetRefinementLevel(),         
                                     _params->GetCompressionLevel(),         
                                     minExt, 
                                     maxExt, 
                                     axes  );
    VAssert( minExt.size() == 3 && maxExt.size() == 3 );
    std::vector<float> range;
    for( int i = 0; i < 3; i++ )
    {
        range.push_back( float(minExt[i]) );
        range.push_back( float(maxExt[i]) );
    }
    _rakeWidget->SetRange( range );
#warning FlowSubtabs GUI is currently initializing FlowParams rake values
    auto rakeVals = _params->GetRake();
    /* In case the user hasn't set the rake, set the current value to be the rake extents,
       plus update the params.  Otherwise, apply the actual rake values.*/
    if( std::isnan( rakeVals[0] ) )
    {
        _rakeWidget->SetValue( range );
        _params->SetRake( range );
    }
    else
    {
        _rakeWidget->SetValue( rakeVals );
    }
}

void FlowSeedingSubtab::_updateSteadyFlowWidgets( VAPoR::DataMgr* dataMgr ) {
    // Steady flow direction combo
    int dir = _params->GetFlowDirection();
    if(  dir >= 0 && dir < _pathlineDirectionCombo->GetCount() )
        _pathlineDirectionCombo->SetIndex( dir );
    else
    {
        _pathlineDirectionCombo->SetIndex( 0 );
        _params->SetFlowDirection( 0 ); // use 0 as the default option
    }

    // Steady flow integration length (flowNumOfSteps)
    int steadyNumOfSteps = _params->GetSteadyNumOfSteps();
    _pathlineSamplesSliderEdit->SetValue( steadyNumOfSteps );
    _pathlineSamplesSliderEdit->SetRange( 0, MAX_PATHLINE_LENGTH );

}

void FlowSeedingSubtab::_updateUnsteadyFlowWidgets( VAPoR::DataMgr* dataMgr) {
    int numTS = dataMgr->GetNumTimeSteps();

    // Unsteady flow integration length
    _streamlineLengthSliderEdit->SetRange( 0, numTS - 1 );
    int valParams = _params->GetPastNumOfTimeSteps();
    if( valParams < 0 )     // initial value, we need to set it to all time steps!
    {
        _streamlineLengthSliderEdit->SetValue( 1 );
        _params->SetPastNumOfTimeSteps( 1 );
    }
    else
    {
        _streamlineLengthSliderEdit->SetValue( valParams );
    }

    // Seed injection interval
    _streamlineInjIntervalSliderEdit->SetRange(0, numTS - 1 );
    int injIntv = _params->GetSeedInjInterval();
    if( injIntv < 0 )       // initial value, we set it to 0
    {
        _streamlineInjIntervalSliderEdit->SetValue( 0 );
        _params->SetSeedInjInterval( 0 );
    }
    else
    {
        _streamlineInjIntervalSliderEdit->SetValue( injIntv );
    }
}

void 
FlowSeedingSubtab::_periodicClicked()
{
    std::vector<bool> bools( 3, false );
    bools[0] = _periodicXCheckBox->GetValue();
    bools[1] = _periodicYCheckBox->GetValue();
    bools[2] = _periodicZCheckBox->GetValue();
    _params->SetPeriodic( bools );
}

void
FlowSeedingSubtab::_velocityMultiplierChanged( const std::string& value )
{
    double oldval = _params->GetVelocityMultiplier();
    double newval;
    try
    {
        newval = std::stod( value );
    }
    catch ( const std::invalid_argument& e )
    {
        std::cerr << "bad input: " << _velocityMultiplierLineEdit->GetValue() << std::endl;
        //_velocityMultiplierLineEdit->SetEditText( QString::number( oldval, 'f', 3 ) );
        return;
    }

    if( newval >= 0.001 && newval <= 1000.0 )   // in the valid range
    {
        // std::stod() would convert "3.83aaa" without throwing an exception.
        // We set the correct text based on the number identified.
        _velocityMultiplierLineEdit->SetValue( std::to_string(newval) ); //QString::number( newval, 'f', 3 ) );
        // Only write back to _params if newval is different from oldval 
        if( newval != oldval )
            _params->SetVelocityMultiplier( newval );
    }
    else
        //_velocityMultiplierLineEdit->SetEditText( QString::number( oldval, 'f', 3 ) );
        _velocityMultiplierLineEdit->SetValue( std::to_string( oldval ) );
}


void 
FlowSeedingSubtab::_streamlineStartTimeChanged( int newVal )
{
    std::cout << "No params function for streamline injection start time" << std::endl;
}

void 
FlowSeedingSubtab::_streamlineEndTimeChanged( int newVal )
{
    std::cout << "No params function for streamline injection end time" << std::endl;
}

void 
FlowSeedingSubtab::_streamlineLifetimeChanged( int newVal )
{
    std::cout << "No params function for streamline injection lifetime" << std::endl;
}

void 
FlowSeedingSubtab::_streamlineLengthChanged( int newVal )
{
    _params->SetPastNumOfTimeSteps( newVal );
}

void 
FlowSeedingSubtab::_pathlineSamplesChanged( int newval )
{
    _params->SetSteadyNumOfSteps( newval );
}

void
FlowSeedingSubtab::_seedInjIntervalChanged( int interval ) {
    _params->SetSeedInjInterval( interval );
}

void FlowSeedingSubtab::_configureFlowType ( const std::string& value ) {
    bool isSteady = true;
    if ( value == UNSTEADY_STRING ) {
        isSteady = false;
        _streamlineFrame->show();
        _pathlineFrame->hide();
    }
    else {
        _streamlineFrame->hide();
        _pathlineFrame->show();
    }
    
    if ( _params != nullptr ) {
        _params->SetIsSteady( (long)isSteady );
    }
}

void FlowSeedingSubtab::_configureSeedType( const std::string& value) {
    if ( value == GRIDDED_STRING ) {
        _griddedSeedsFrame->show();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->hide();
        _rakeRegionSection->show();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::UNIFORM );
    }
    else if ( value == LIST_STRING ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->show();
        _randomSeedsFrame->hide();
        _rakeRegionSection->hide();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::LIST );
    }
    else if ( value == RANDOM_STRING ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->show();
        _rakeRegionSection->show();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::RANDOM_BIAS);
    }
}

void 
FlowSeedingSubtab::_biasVariableChanged( const std::string& variable )
{
    _params->SetRakeBiasVariable( variable );
}
 
void 
FlowSeedingSubtab::_biasStrengthChanged( double strength )
{
    _params->SetRakeBiasStrength( strength );
}


void
FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    std::vector<long> seedsVector(4, (long)1.0);
    seedsVector[X] = _xSeedSliderEdit->GetValue();
    seedsVector[Y] = _ySeedSliderEdit->GetValue();
    seedsVector[Z] = _zSeedSliderEdit->GetValue();
    seedsVector[RANDOM_INDEX] = _randomSeedsSliderEdit->GetValue();
    _params->SetRakeNumOfSeeds( seedsVector );
}

void 
FlowSeedingSubtab::_seedListFileChanged( const std::string& value ) {
    _params->SetSeedInputFilename( value );
}


void
FlowSeedingSubtab::_rakeGeometryChanged( const std::vector<float>& range )
{
    VAssert( range.size() == 6 );
    _params->SetRake( range );
}

void
FlowSeedingSubtab::_selectedTabChanged(int index)
{
    if (!_paramsMgr)
        return;

    const QTabWidget *parent = dynamic_cast<QTabWidget*>(sender());
    VAssert(parent);
    const QScrollArea *area = dynamic_cast<QScrollArea*>(parent->widget(index));
    VAssert(area);
    const QWidget *widget = area->widget();

    GUIStateParams *gp = (GUIStateParams *)_paramsMgr->GetParams(GUIStateParams::GetClassType());

    gp->SetFlowSeedTabActive(widget == this);
}

void
FlowSeedingSubtab::_seedGenModeChanged( int newIdx )
{
#warning FlowSeedingSubtab::_seedGenModeChanged() has a hack to get around numeric values representing seed generation modes
    cout << newIdx << endl;
    if (newIdx > 0)
        newIdx++;
    cout << newIdx << endl;
    _params->SetSeedGenMode( newIdx );
}

void
FlowSeedingSubtab::_pathlineDirectionChanged( int index )
{
    _params->SetFlowDirection( index );
}

void FlowSeedingSubtab::_geometryWriterFileChanged( const std::string& file ) {
    _params->SetFlowlineOutputFilename( file );
    _params->SetNeedFlowlineOutput( true );
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
