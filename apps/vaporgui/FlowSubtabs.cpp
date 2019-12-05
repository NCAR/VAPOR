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

#define verbose     1

#define UNSTEADY_STRING    "Pathlines"
#define STEADY_STRING      "Streamlines"
#define GRIDDED_STRING     "Gridded"
#define LIST_STRING        "List of seeds"
#define RANDOM_STRING      "Random"

#define MIN_AXIS_SEEDS      1
#define MAX_AXIS_SEEDS      1000
#define MIN_RANDOM_SEEDS    1
#define MAX_RANDOM_SEEDS    1000000

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
    _TFEditor = new TFEditor;

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

    _createSeedingSection();
    _createIntegrationSection();
}

void FlowSeedingSubtab::_createSeedingSection() {
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
    connect( _xSeedSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );
    _ySeedSliderEdit = new VSliderEdit();
    _ySeedSliderEdit->SetIntType(true);
    _ySeedSliderEdit->SetRange( MIN_AXIS_SEEDS, MAX_AXIS_SEEDS );
    _griddedSeedsFrame->addWidget( new VLineItem("Y axis seeds", _ySeedSliderEdit ) );
    connect( _ySeedSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );
    _zSeedSliderEdit = new VSliderEdit();
    _zSeedSliderEdit->SetIntType(true);
    _zSeedSliderEdit->SetRange( MIN_AXIS_SEEDS, MAX_AXIS_SEEDS );
    _griddedSeedsFrame->addWidget( new VLineItem("Z axis seeds", _zSeedSliderEdit ) );
    connect( _zSeedSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );

    // Rake selector
    _rakeWidget = new VGeometry2();
    _seedDistributionSection->layout()->addWidget( _rakeWidget );
    connect( _rakeWidget, SIGNAL( _ValueChanged( const std::vector<float>& ) ),
        this, SLOT( _rakeGeometryChanged( const std::vector<float>& ) ) );

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
    connect( _randomSeedsSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _rakeNumOfSeedsChanged() ) );

    _biasWeightSliderEdit = new VSliderEdit(-1, 1, 0);
    _randomSeedsFrame->addWidget( new VLineItem( "Bias weight", _biasWeightSliderEdit ) );
    connect( _biasWeightSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _biasWeightChanged( double ) ) );

    _biasVariableComboBox = new VComboBox( std::vector<std::string>() );
    _randomSeedsFrame->addWidget( new VLineItem( "Bias variable", _biasVariableComboBox ) );
    connect( _biasVariableComboBox, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _biasVariableChanged( const std::string& ) ) );

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
    connect( _pathlineDirectionCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _pathlineDirectionChanged( std::string ) ) );
    _pathlineFrame->addWidget( new VLineItem("Flow direction", _pathlineDirectionCombo) );

    _pathlineLengthSliderEdit = new VSliderEdit();
    _pathlineLengthSliderEdit->SetIntType(true);
    connect( _pathlineLengthSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _pathlineLengthChanged(int) ) );
    _pathlineFrame->addWidget( new VLineItem("Pathline length", _pathlineLengthSliderEdit));

    // Unsteady flow options
    //
    _streamlineFrame = new VFrame();
    _integrationSection->layout()->addWidget( _streamlineFrame );

    _streamlineLengthSliderEdit = new VSliderEdit();
    _streamlineLengthSliderEdit->SetIntType(true);
    connect( _streamlineLengthSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _streamlineLengthChanged( int ) ) );
    _streamlineFrame->addWidget( 
        new VLineItem("Streamline length", _streamlineLengthSliderEdit));

    _streamlineStartSliderEdit= new VSliderEdit();
    _streamlineStartSliderEdit->SetIntType(true);
    connect( _streamlineStartSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _streamlineStartTimeChanged( int ) ) );
    _streamlineFrame->addWidget(
        new VLineItem("Injection start time - NOOP", _streamlineStartSliderEdit));

    _streamlineEndSliderEdit = new VSliderEdit();
    _streamlineEndSliderEdit->SetIntType(true);
    connect( _streamlineEndSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _streamlineEndTimeChanged( int ) ) );
    _streamlineFrame->addWidget(
        new VLineItem("Injection end time - NOOP", _streamlineEndSliderEdit));
    
    _streamlineInjIntervalSliderEdit = new VSliderEdit();
    _streamlineInjIntervalSliderEdit->SetIntType(true);
    connect( _streamlineInjIntervalSliderEdit,  SIGNAL( ValueChanged( double ) ), 
        this, SLOT( _seedInjIntervalChanged(int) ));
    _streamlineFrame->addWidget(
        new VLineItem("Injection interval - NOOP", _streamlineInjIntervalSliderEdit));

    _streamlineLifetimeSliderEdit = new VSliderEdit();
    _streamlineLifetimeSliderEdit->SetIntType(true);
    connect( _streamlineLifetimeSliderEdit,  SIGNAL( ValueChanged( double ) ), 
        this, SLOT( _streamlineLifetimeChanged(int) ) );
    _streamlineFrame->addWidget(
        new VLineItem("Seed lifetime - NOOP", _streamlineLifetimeSliderEdit) );

    // Universal options: Velocity multiplier and periodicity checkboxes
    //    
    _velocityMultiplierLineEdit = new VLineEdit();
    _velocityMultiplierLineEdit->SetIsDouble( true );
    connect( _velocityMultiplierLineEdit, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _velocityMultiplierChanged( const std::string& ) ) );
    _integrationSection->layout()->addWidget( 
        new VLineItem("Velocity multiplier", _velocityMultiplierLineEdit));

    // Periodicity Checkboxes
    //
    _periodicXCheckBox = new VCheckBox();
    connect( _periodicXCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( 
        new VLineItem("X axis periodicity", _periodicXCheckBox));
    _periodicYCheckBox = new VCheckBox();
    connect( _periodicYCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( 
        new VLineItem("Y axis periodicity", _periodicYCheckBox));
    _periodicZCheckBox = new VCheckBox();
    connect( _periodicZCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->layout()->addWidget( 
        new VLineItem("Z axis periodicity", _periodicZCheckBox));

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
    _rakeWidget->SetValue( _params->GetRake() );
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
    _pathlineLengthSliderEdit->SetValue( steadyNumOfSteps );
    int numTS = dataMgr->GetNumTimeSteps();
    _pathlineLengthSliderEdit->SetRange( 0, numTS-1 );

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


/*
    int steadyNumOfSteps    = _params->GetSteadyNumOfSteps();
    _steadyNumOfSteps->SetEditText( QString::number( steadyNumOfSteps ) );
    // Update the past num of steps widget
    int totalNumOfTimeSteps = dataMgr->GetNumTimeSteps();
    _pastNumOfTimeSteps->SetRange( 0, totalNumOfTimeSteps - 1 );
    int valParams = _params->GetPastNumOfTimeSteps();
    if( valParams < 0 )     // initial value, we need to set it to all time steps!
    {
        _pastNumOfTimeSteps->SetCurrentValue( totalNumOfTimeSteps - 1 );
        _params->SetPastNumOfTimeSteps( totalNumOfTimeSteps - 1 );
    }
    else
    {
        _pastNumOfTimeSteps->SetCurrentValue( valParams );
    }

    // Update the seed injection interval widget
    _seedInjInterval->SetRange(0, totalNumOfTimeSteps - 1 );
    int injIntv = _params->GetSeedInjInterval();
    if( injIntv < 0 )       // initial value, we set it to 0
    {
        _seedInjInterval->SetCurrentValue( 0 );
        _params->SetSeedInjInterval( 0 );
    }
    else
    {
        _seedInjInterval->SetCurrentValue( injIntv );
    }

    // Update flow direction combo
    auto dir  = _params->GetFlowDirection();
    if(  dir >= 0 && dir < _pathlineDirection->GetNumOfItems() )
        _pathlineDirection->SetIndex( dir );
    else
    {
        _pathlineDirection->SetIndex(  0 );
        _params->SetFlowDirection( 0 ); // use 0 as the default option
    }

    // Update seed generation mode combo
    auto genMod = _params->GetSeedGenMode();
    if( genMod >= 0 && genMod < _seedGenMode->GetNumOfItems() )
        _seedGenMode->SetIndex( genMod );
    else
    {
        _seedGenMode->SetIndex(  0 );
        _params->SetSeedGenMode( 0 ); // use 0 as the default option
    }

    _hideShowWidgets();

    // Update rake range
    std::vector<double> minExt, maxExt;
    std::vector<int>    axes;
    VAPoR::DataMgrUtils::GetExtents( dataMgr, 
                                     _params->GetCurrentTimestep(), 
                                     _params->GetFieldVariableNames(),         
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
    _rake->SetDimAndRange( 3, range );

    // Update rake values 
    auto rakeVals = _params->GetRake();
    // In case the user hasn't set the rake, set the current value to be the rake extents,
       plus update the params.  Otherwise, apply the actual rake values.
    if( std::isnan( rakeVals[0] ) )
    {
        _rake->SetCurrentValues( range );
        _params->SetRake( range );
    }
    else
    {
        _rake->SetCurrentValues( rakeVals );
    }

    // Update rake num. of seeds
    auto rakeNumOfSeeds = _params->GetRakeNumOfSeeds();
    _rakeXNum->SetEditText( QString::number( rakeNumOfSeeds[0] ) );
    _rakeYNum->SetEditText( QString::number( rakeNumOfSeeds[1] ) );
    _rakeZNum->SetEditText( QString::number( rakeNumOfSeeds[2] ) );
    _rakeTotalNum->SetEditText( QString::number( rakeNumOfSeeds[3] ) );

    // Update rake random bias variable and strength
    if( _rakeBiasVariable->GetNumOfItems() < 1 )    // Not filled with variables yet
    {
        auto varNames3d = dataMgr->GetDataVarNames( 3 );
        for( int i = 0; i < varNames3d.size(); i++ )
            _rakeBiasVariable->AddOption( varNames3d[i], i );
        _rakeBiasVariable->SetIndex( 0 );           // Set the 1st variable name
    }
    auto varParams = _params->GetRakeBiasVariable();
    if(  varParams.empty() )    // The variable isn't set by the user yet. Let's set it!
    {
        auto varDefault = _rakeBiasVariable->GetCurrentText();
        _params->SetRakeBiasVariable( varDefault );
    }
    else                        // Find the variable and set it in the GUI! 
    {
        for( int i = 0; i < _rakeBiasVariable->GetNumOfItems(); i++ )
        {
            auto varName = _rakeBiasVariable->GetItemText( i );
            if(  varName.compare( varParams ) == 0 )
            {
                _rakeBiasVariable->SetIndex( i );
                break;
            }
        }
    }

    _rakeBiasStrength->SetCurrentValue( _params->GetRakeBiasStrength() );

    // Update input and output file names
    if( !_params->GetSeedInputFilename().empty() ) 
        _fileReader->SetPath( _params->GetSeedInputFilename() );
    if( !_params->GetFlowlineOutputFilename().empty() ) 
        _fileWriter->SetPath( _params->GetFlowlineOutputFilename() );
}
*/

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
    std::cout << "No params function for streamline length" << std::endl;
}

void 
FlowSeedingSubtab::_pathlineLengthChanged( int newval )
{
    int oldval;
    oldval = (int)_params->GetSteadyNumOfSteps();

    if( newval >= 0 )    // in the valid range
    {
        // Only write back to _params if newval is different from ldval 
        if( newval != oldval )
            _params->SetSteadyNumOfSteps( newval );
    }
    else
        _pathlineLengthSliderEdit->SetValue( oldval );
}


void FlowSeedingSubtab::_configureFlowType ( const std::string& value ) {
    bool isSteady = true;
    if ( value == UNSTEADY_STRING ) {
        isSteady = false;
        /*_streamlineLengthSliderEdit->Show();
        _streamlineInjIntervalSliderEdit->Show();
        _streamlineStartSliderEdit->Show();
        _streamlineEndSliderEdit->Show();
        _streamlineLifetimeSliderEdit->Show();
        _pathlineLengthSliderEdit->Hide();
        _pathlineDirectionCombo->Hide();*/
        _streamlineFrame->show();
        _pathlineFrame->hide();
    }
    else {
        /*_streamlineLengthSliderEdit->Hide();
        _streamlineInjIntervalSliderEdit->Hide();
        _streamlineStartSliderEdit->Hide();
        _streamlineEndSliderEdit->Hide();
        _streamlineLifetimeSliderEdit->Hide();
        _pathlineLengthSliderEdit->Show();
        _pathlineDirectionCombo->Show();*/
        _streamlineFrame->hide();
        _pathlineFrame->show();
    }
    
    if (verbose) std::cout << "Flow combo changed to " << value << endl;

    if ( _params != nullptr ) {
        _params->SetIsSteady( (long)isSteady );
        if (verbose) std::cout << "Flow params changed to " << _params->GetIsSteady() << endl;
    }

    if (verbose) std::cout << std::endl;
}

void FlowSeedingSubtab::_configureSeedType( const std::string& value) {
    if ( value == GRIDDED_STRING ) {
        _griddedSeedsFrame->show();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->hide();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::UNIFORM );
    }
    else if ( value == LIST_STRING ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->show();
        _randomSeedsFrame->hide();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::LIST );
    }
    else if ( value == RANDOM_STRING ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->show();
        if ( _params != nullptr ) 
            _params->SetSeedGenMode( (int)VAPoR::FlowSeedMode::RANDOM);
    }
}

/*
void
FlowSeedingSubtab::_steadyGotClicked()
{
    bool userInput = _steady->GetCheckState();
    _params->SetIsSteady( userInput );
}
*/
 
/*   
void 
FlowSeedingSubtab::_hideShowWidgets()
{
    bool isSteady = _params->GetIsSteady();
    if( isSteady )
    {
        _steadyNumOfSteps->show();
        _pathlineDirection->show();
        _seedInjInterval->hide();
        _pastNumOfTimeSteps->hide();
    }
    else
    {
        _steadyNumOfSteps->hide();
        _pathlineDirection->hide();
        _seedInjInterval->show();
        _pastNumOfTimeSteps->show();
    }

    int genMod = _params->GetSeedGenMode();    // genMod must be valid at this point
    if( genMod == static_cast<int>(FlowSeedMode::UNIFORM) )
    {
        _rake->show();
        _rakeXNum->show();
        _rakeYNum->show();
        _rakeZNum->show();
        _rakeTotalNum->hide();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    }
    else if( genMod == static_cast<int>(FlowSeedMode::RANDOM_STRING) )
    {
        _rake->show();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->show();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    }
    else if( genMod == static_cast<int>(FlowSeedMode::RANDOM_STRING_BIAS) )
    {
        _rake->show();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->show();
        _rakeBiasVariable->show();
        _rakeBiasStrength->show();
    }
    else if( genMod == static_cast<int>(FlowSeedMode::LIST) )
    {
        _rake->hide();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->hide();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    }
}
*/

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
    std::cout << "_rakeNumOfSeedsChanged from " << seedsVector[0] << " " << seedsVector[1] << " " << seedsVector[2] << " " << seedsVector[3] << std::endl;
    _params->SetRakeNumOfSeeds( seedsVector );
    std::cout << "_rakeNumOfSeedsChanged to   " << _params->GetRakeNumOfSeeds()[0] << " " << _params->GetRakeNumOfSeeds()[1] << " " << _params->GetRakeNumOfSeeds()[2] << " " << _params->GetRakeNumOfSeeds()[3] << std::endl;
}

void 
FlowSeedingSubtab::_seedListFileChanged( const std::string& value ) {
    _params->SetSeedInputFilename( value );
    std::cout << "_params->GetSeedInputFilename is " << _params->GetSeedInputFilename() << endl;

}


void
FlowSeedingSubtab::_rakeGeometryChanged( const std::vector<float>& range )
{
    cout << " FlowSeedingSubtab::_rakeGeometryChanged() " << endl;
    cout << "       " << range[0] << " " << range[1] << endl;
    cout << "       " << range[2] << " " << range[3] << endl;
    cout << "       " << range[4] << " " << range[5] << endl;
    VAssert( range.size() == 6 );
    _params->SetRake( range );
}



void
FlowSeedingSubtab::_seedGenModeChanged( int newIdx )
{
    _params->SetSeedGenMode( newIdx );
}

void
FlowSeedingSubtab::_pathlineDirectionChanged()
{
    int index = _pathlineDirectionCombo->GetCurrentIndex();
    _params->SetFlowDirection( index );
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
