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

#define verbose     1

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

/*    _velocityMltp = new VLineEdit( this, "Field Scale Factor", "1.0" );
    _layout->addWidget( _velocityMltp );

    _periodicX = new VCheckBox( this, "Particles periodic in X" );
    _layout->addWidget( _periodicX );
    _periodicY = new VCheckBox( this, "Particles periodic in Y" );
    _layout->addWidget( _periodicY );
    _periodicZ = new VCheckBox( this, "Particles periodic in Z" );
    _layout->addWidget( _periodicZ );

    connect( _velocityMltp,     SIGNAL( _editingFinished() ), this, SLOT( _velocityMultiplierChanged() ) );
    connect( _periodicX,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicY,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicZ,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );*/
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
    /*auto mltp = _params->GetVelocityMultiplier();
    _velocityMltp->SetEditText( QString::number( mltp, 'f', 3 ) );

    auto bools = _params->GetPeriodic();
    _periodicX->SetCheckState( bools[0] );
    _periodicY->SetCheckState( bools[1] );
    _periodicZ->SetCheckState( bools[2] );*/
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

    _createIntegrationSection();
    _createSeedingSection();
}

void FlowSeedingSubtab::_createSeedingSection() {
    _seedDistributionSection = new VSection("Seed Distribution Settings");
    layout()->addWidget( _seedDistributionSection );

    std::vector<std::string> values = {GRIDDED, RANDOM, LISTOFSEEDS};
    _seedTypeCombo = new VComboBox(values);
    _seedDistributionSection->AddWidget( new VLineItem("Seed distribution type", _seedTypeCombo ));
    connect( _seedTypeCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _configureSeedType( std::string )));

    // Gridded seed selection
    _griddedSeedsFrame = new VFrame();
    _seedDistributionSection->AddWidget( _griddedSeedsFrame );

    _xSeedSliderEdit = new VSliderEdit();
    _xSeedSliderEdit->SetIntType(true);
    _griddedSeedsFrame->addWidget( new VLineItem( "X axis seeds", _xSeedSliderEdit ) );
    //connect
    _ySeedSliderEdit = new VSliderEdit();
    _ySeedSliderEdit->SetIntType(true);
    _griddedSeedsFrame->addWidget( new VLineItem("Y axis seeds", _ySeedSliderEdit ) );
    //connect
    _zSeedSliderEdit = new VSliderEdit();
    _zSeedSliderEdit->SetIntType(true);
    _griddedSeedsFrame->addWidget( new VLineItem("Z axis seeds", _zSeedSliderEdit ) );
    //connect

    // List of seeds selection
    _listOfSeedsFrame = new VFrame();
    _seedDistributionSection->AddWidget( _listOfSeedsFrame );
    
    _listOfSeedsFileReader = new VFileReader();
    _listOfSeedsFrame->addWidget( new VLineItem("List of seeds file", _listOfSeedsFileReader ) );
    //connect

    // Random distribution selection
    _randomSeedsFrame = new VFrame();
    _seedDistributionSection->AddWidget( _randomSeedsFrame );
    
    _randomSeedSpinBox = new VSpinBox( 0, 100000 );
    _randomSeedsFrame->addWidget( new VLineItem("Number of random seeds", _randomSeedSpinBox ) );
    // connect

    _biasVariableComboBox = new VComboBox( std::vector<std::string>() );
    _randomSeedsFrame->addWidget( new VLineItem( "Bias variable", _biasVariableComboBox ) );
    // connect

    _biasWeightSliderEdit = new VSliderEdit();
    _randomSeedsFrame->addWidget( new VLineItem( "Bias weight", _biasWeightSliderEdit ) );

    _configureSeedType( GRIDDED );
}

void FlowSeedingSubtab::_createIntegrationSection() {
    _integrationSection = new VSection("Flow Integration Settings");
    layout()->addWidget( _integrationSection );

    std::vector<std::string> values = {STEADY, UNSTEADY};
    _flowTypeCombo = new VComboBox(values);
    _integrationSection->AddWidget( new VLineItem("Flow type", _flowTypeCombo ));
    connect( _flowTypeCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _configureFlowType( std::string )));

    // Steady flow options
    //
    _pathlineWidgetFrame = new VFrame();

    values = { "Forward", "Backward", "Bi-Directional" };
    _pathlineDirectionCombo = new VComboBox(values);
    _integrationSection->AddWidget( new VLineItem("Flow direction", _pathlineDirectionCombo) );
    connect( _pathlineDirectionCombo, SIGNAL( ValueChanged( std::string )),
        this, SLOT( _flowDirectionChanged( std::string ) ) );

    _pathlineLengthSliderEdit = new VSliderEdit();
    _pathlineLengthSliderEdit->SetIntType(true);
    connect( _pathlineLengthSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _pathlineLengthChanged(int) ) );
    _integrationSection->AddWidget( 
        new VLineItem("Pathline length", _pathlineLengthSliderEdit));

    // Unsteady flow options
    //
    _streamlineLengthSliderEdit = new VSliderEdit();
    _streamlineLengthSliderEdit->SetIntType(true);
    _integrationSection->AddWidget( 
        new VLineItem("Streamline length", _streamlineLengthSliderEdit));
    connect( _streamlineLengthSliderEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( _streamlineLengthChanged( int ) ) );

    _streamlineStartSliderEdit= new VSliderEdit();
    _streamlineStartSliderEdit->SetIntType(true);
    _integrationSection->AddWidget(
        new VLineItem("Seeding start time interval - NOOP", _streamlineStartSliderEdit));

    _streamlineEndSliderEdit = new VSliderEdit();
    _streamlineEndSliderEdit->SetIntType(true);
    _integrationSection->AddWidget(
        new VLineItem("Seeding end time - NOOP", _streamlineEndSliderEdit));
    
    _streamlineInjIntervalSliderEdit = new VSliderEdit();
    _streamlineInjIntervalSliderEdit->SetIntType(true);
    _integrationSection->AddWidget(
        new VLineItem("Seeding interval", _streamlineInjIntervalSliderEdit));
    connect( _streamlineInjIntervalSliderEdit,  SIGNAL( ValueChanged( double )  ), 
        this, SLOT( _seedInjIntervalChanged(int) ));

    _streamlineLifetimeSliderEdit = new VSliderEdit();
    _streamlineLifetimeSliderEdit->SetIntType(true);
    _integrationSection->AddWidget(
        new VLineItem("Seed lifetime - NOOP", _streamlineLifetimeSliderEdit));

    // Periodicity Checkboxes
    //
    _periodicXCheckBox = new VCheckBox();
    connect( _periodicXCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->AddWidget( 
        new VLineItem("X axis periodicity", _periodicXCheckBox));
    _periodicYCheckBox = new VCheckBox();
    connect( _periodicYCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->AddWidget( 
        new VLineItem("Y axis periodicity", _periodicYCheckBox));
    _periodicZCheckBox = new VCheckBox();
    connect( _periodicZCheckBox, SIGNAL( ValueChanged( bool )),
        this, SLOT( _periodicClicked() ) );
    _integrationSection->AddWidget( 
        new VLineItem("Z axis periodicity", _periodicZCheckBox));

    _velocityMultiplierLineEdit = new VLineEdit();
    _velocityMultiplierLineEdit->SetIsDouble( true );
    connect( _velocityMultiplierLineEdit, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _velocityMultiplierChanged( const std::string& ) ) );
    _integrationSection->AddWidget( 
        new VLineItem("Velocity multiplier", _velocityMultiplierLineEdit));
    VLineEdit* test = new VLineEdit("test");
    test->SetIsDouble( true );
    connect( test, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( _velocityMultiplierChanged( const std::string& ) ) );
    _integrationSection->AddWidget( 
        new VLineItem("test", test));

    _configureFlowType();
}
/*    _steady = new VCheckBox( this, "Use Steady Flow" );
    _layout->addWidget( _steady );
    connect( _steady, SIGNAL( _checkboxClicked() ), this, SLOT( _steadyGotClicked() ) );
    _steadyNumOfSteps = new VLineEdit( this, "Steady Integration Steps", "100" );
    _layout->addWidget( _steadyNumOfSteps);
    connect( _steadyNumOfSteps, SIGNAL( _editingFinished() ), this, SLOT( _steadyNumOfStepsChanged() ) );
    _pastNumOfTimeSteps = new VIntSlider( this, "Display Past Num. of Time Steps", 1, 2 );
    _layout->addWidget( _pastNumOfTimeSteps );
    connect( _pastNumOfTimeSteps,SIGNAL(_valueChanged(int) ), this, SLOT( _pastNumOfTimeStepsChanged(int) ));
    _seedInjInterval = new VIntSlider( this, "Seed Injection Interval", 0, 1 );
    _layout->addWidget( _seedInjInterval );

    connect( _seedInjInterval,  SIGNAL(_valueChanged(int)  ), this, SLOT( _seedInjIntervalChanged(int) ));

    // Index numbers are in agreement with what's in FlowRenderer.h
    _flowDirection = new VComboBox( this, "Steady Flow Direction" );
    _flowDirection->AddOption( "Forward",        static_cast<int>(FlowDir::FORWARD) );
    _flowDirection->AddOption( "Backward",       static_cast<int>(FlowDir::BACKWARD) );
    _flowDirection->AddOption( "Bi-Directional", static_cast<int>(FlowDir::BI_DIR) );
    _layout->addWidget( _flowDirection );
    connect( _flowDirection, SIGNAL(_indexChanged(int)), this, SLOT( _flowDirectionChanged(int) ) );
    _seedGenMode = new VComboBox( this, "Seed Generation Mode" );

    _hline1 = new QFrame(this);
    _hline1->setFrameShape( QFrame::HLine );
    _layout->addWidget( _hline1 );

    // The following two widgets deal with flow line output and seed point input
    _fileWriter = new VFileWriter( this, "Output Flow Lines" );
    _fileWriter->SetFileFilter( QString::fromAscii("*.txt") );
    _layout->addWidget( _fileWriter );
    connect( _fileWriter, SIGNAL( _pathChanged() ), this, SLOT( _fileWriterChanged() ) );
   
    _fileReader = new VFileReader( this, "Input Seed File" );
    _fileReader->SetFileFilter( QString::fromAscii("*.txt") );
    _layout->addWidget( _fileReader );
    connect( _fileReader, SIGNAL( _pathChanged() ), this, SLOT( _fileReaderChanged() ) );

    _hline2 = new QFrame(this);
    _hline2->setFrameShape( QFrame::HLine );
    _layout->addWidget( _hline2 );

    // Index numbers are in agreement with what's in FlowRenderer.h
    _seedGenMode->AddOption( "From a Rake, Uniformly", static_cast<int>(FlowSeedMode::UNIFORM) );
    _seedGenMode->AddOption( "From a Rake, Randomly",  static_cast<int>(FlowSeedMode::RANDOM) );
    _seedGenMode->AddOption( "From a Rake, Randomly with Bias", 
                             static_cast<int>(FlowSeedMode::RANDOM_BIAS) );
    _seedGenMode->AddOption( "From a List",            static_cast<int>(FlowSeedMode::LIST) );
    _layout->addWidget( _seedGenMode );
    connect( _seedGenMode, SIGNAL( _indexChanged(int) ), this, SLOT( _seedGenModeChanged(int) ) );

    // Set up the Rake selector
    std::vector<float> geoRange = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0}; // temporary range
    _rake = new VGeometry( this, 3, geoRange );
    _layout->addWidget( _rake );
    connect( _rake, SIGNAL( _geometryChanged() ), this, SLOT( _rakeGeometryChanged() ) );

    // Set up rake seed number controls
    _rakeXNum     = new VLineEdit( this, "Num. of Seeds in X",  "1" );
    _rakeYNum     = new VLineEdit( this, "Num. of Seeds in Y",  "1" );
    _rakeZNum     = new VLineEdit( this, "Num. of Seeds in Z",  "1" );
    _rakeTotalNum = new VLineEdit( this, "Total Num. of Seeds", "1" );
    _layout->addWidget( _rakeXNum );
    _layout->addWidget( _rakeYNum );
    _layout->addWidget( _rakeZNum );
    _layout->addWidget( _rakeTotalNum );
    connect( _rakeXNum, SIGNAL( _editingFinished() ), this, SLOT( _rakeNumOfSeedsChanged() ) );
    connect( _rakeYNum, SIGNAL( _editingFinished() ), this, SLOT( _rakeNumOfSeedsChanged() ) );
    connect( _rakeZNum, SIGNAL( _editingFinished() ), this, SLOT( _rakeNumOfSeedsChanged() ) );
    connect( _rakeTotalNum, SIGNAL( _editingFinished() ), this, SLOT( _rakeNumOfSeedsChanged() ) );

    _rakeBiasVariable = new VComboBox( this, "Random Bias Variable" );
    _rakeBiasStrength = new VSlider(   this, "Random Bias Strength", -5.0f, 5.0f );
    _layout->addWidget( _rakeBiasVariable );
    _layout->addWidget( _rakeBiasStrength );
    connect( _rakeBiasVariable, SIGNAL( _indexChanged(int) ), this, SLOT( _rakeBiasVariableChanged(int) ) );
    connect( _rakeBiasStrength, SIGNAL( _valueChanged() ),    this, SLOT( _rakeBiasStrengthChanged()    ) );
    
    VAssert(parent);
    connect(parent, SIGNAL(currentChanged(int)), this, SLOT(_selectedTabChanged(int)));
}
*/

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
        _flowTypeCombo->SetValue( STEADY );
    else
        _flowTypeCombo->SetValue( UNSTEADY );

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
    cout << "steadyNumOfSteps " << steadyNumOfSteps << endl;
    _pathlineLengthSliderEdit->SetValue( steadyNumOfSteps );
    int numTS = dataMgr->GetNumTimeSteps();
    _pathlineLengthSliderEdit->SetRange( 0, numTS-1 );
   
    // Periodicity checkboxes 
    auto bools = _params->GetPeriodic();
    _periodicXCheckBox->SetValue( bools[0] );
    _periodicYCheckBox->SetValue( bools[1] );
    _periodicZCheckBox->SetValue( bools[2] );

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

    auto mltp = _params->GetVelocityMultiplier();
    _velocityMultiplierLineEdit->SetValue( std::to_string( mltp ) );
    
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
    if(  dir >= 0 && dir < _flowDirection->GetNumOfItems() )
        _flowDirection->SetIndex( dir );
    else
    {
        _flowDirection->SetIndex(  0 );
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
FlowSeedingSubtab::_streamlineLengthChanged( int newVal )
{
    if( newVal != _params->GetPastNumOfTimeSteps() )
    {
        _params->SetPastNumOfTimeSteps( newVal );
    }
}


/*
void 
FlowSeedingSubtab::_seedInjIntervalChanged( int newVal )
{
    if( newVal != _params->GetSeedInjInterval() )
    {
        _params->SetSeedInjInterval( newVal );
    }
}
*/

/*
#include <QScrollArea>
void FlowSeedingSubtab::_selectedTabChanged(int index)
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
*/


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
    if ( value == UNSTEADY ) {
        isSteady = false;
        _streamlineLengthSliderEdit->Show();
        _streamlineInjIntervalSliderEdit->Show();
        _streamlineStartSliderEdit->Show();
        _streamlineEndSliderEdit->Show();
        _streamlineLifetimeSliderEdit->Show();

        _pathlineLengthSliderEdit->Hide();
        _pathlineDirectionCombo->Hide();
    }
    else {
        _streamlineLengthSliderEdit->Hide();
        _streamlineInjIntervalSliderEdit->Hide();
        _streamlineStartSliderEdit->Hide();
        _streamlineEndSliderEdit->Hide();
        _streamlineLifetimeSliderEdit->Hide();

        _pathlineLengthSliderEdit->Show();
        _pathlineDirectionCombo->Show();
    }
    
    if (verbose) std::cout << "Flow combo changed to " << value << endl;

    if ( _params != nullptr ) {
        _params->SetIsSteady( (long)isSteady );
        if (verbose) std::cout << "Flow params changed to " << _params->GetIsSteady() << endl;
    }

    if (verbose) std::cout << std::endl;
}

void FlowSeedingSubtab::_configureSeedType( const std::string& value ) {
    if ( value == GRIDDED ) {
        _griddedSeedsFrame->show();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->hide();
    }
    else if ( value == LISTOFSEEDS ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->show();
        _randomSeedsFrame->hide();
    }
    else if ( value == RANDOM ) {
        _griddedSeedsFrame->hide();
        _listOfSeedsFrame->hide();
        _randomSeedsFrame->show();
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
        _flowDirection->show();
        _seedInjInterval->hide();
        _pastNumOfTimeSteps->hide();
    }
    else
    {
        _steadyNumOfSteps->hide();
        _flowDirection->hide();
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
    else if( genMod == static_cast<int>(FlowSeedMode::RANDOM) )
    {
        _rake->show();
        _rakeXNum->hide();
        _rakeYNum->hide();
        _rakeZNum->hide();
        _rakeTotalNum->show();
        _rakeBiasVariable->hide();
        _rakeBiasStrength->hide();
    }
    else if( genMod == static_cast<int>(FlowSeedMode::RANDOM_BIAS) )
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

/*    
void 
FlowSeedingSubtab::_rakeBiasVariableChanged( int idx )
{
    // idx is always a valid value, since it's returned by the GUI
    auto varGUI     = _rakeBiasVariable->GetCurrentText();
    auto varParams  = _params->GetRakeBiasVariable();
    if(  varGUI.compare( varParams ) != 0 )
    {
        _params->SetRakeBiasVariable( varGUI );
    }
}
*/
 
/*   
void 
FlowSeedingSubtab::_rakeBiasStrengthChanged()
{
    // The value returned from the GUI is always valid
    auto strenGUI  = _rakeBiasStrength->GetCurrentValue();
    if(  strenGUI != _params->GetRakeBiasStrength() )
    {
        _rakeBiasStrength->SetCurrentValue( strenGUI );
        _params->SetRakeBiasStrength( strenGUI );
    }
}
*/


void
FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    // These fields should ALWAYS contain legal values, even when not in use.
    //   That's why we validate every one of them!                          
    
    const std::vector<long> oldVal = _params->GetRakeNumOfSeeds();
    std::vector<long> newVal( 4, -1 );
    
    std::vector<VLineEdit*> pointers = {_rakeXNum, _rakeYNum, _rakeZNum, _rakeTotalNum};
    for( int i = 0; i < 4; i++ )
    {
        long tmp;
        try
        {
            tmp = std::stol( pointers[i]->GetEditText() );
        }
        catch( const std::invalid_argument& e ) // If not a long number
        {
            std::cerr << "bad input: " << pointers[i]->GetEditText() << std::endl;
            newVal[i] = oldVal[i];
            pointers[i]->SetEditText( QString::number( oldVal[i] ) );
            continue;
        }
        
        if( tmp > 0 )   // In the valid range, which is positive here
        {
            newVal[i] = tmp;
            // std::stol() would convert "383aaa" without throwing an exception.
            // We set the correct text based on the number identified.      
            pointers[i]->SetEditText( QString::number( tmp ) );
        }
        else
        {
            newVal[i] = oldVal[i];
            pointers[i]->SetEditText( QString::number( oldVal[i] ) );
        }
    }

    // Only write back to _params when newVal is different from oldVal
    bool diff = false;
    for( int i = 0; i < 4; i++ )
    {
        if( newVal[i] != oldVal[i] )
        {
            diff = true;
            break;
        }
    }
    if( diff )
    {
        _params->SetRakeNumOfSeeds( newVal );
    }
}


/*
void
FlowSeedingSubtab::_rakeGeometryChanged()
{
    std::vector<float> range;
    _rake->GetCurrentValues( range );
    VAssert( range.size() == 6 );
    _params->SetRake( range );
}
*/


void
FlowSeedingSubtab::_seedGenModeChanged( int newIdx )
{
    _params->SetSeedGenMode( newIdx );
}


/*
void
FlowSeedingSubtab::_fileReaderChanged()
{
    std::string filename = _fileReader->GetPath();
    _params->SetSeedInputFilename( filename );
}
*/

/*
void
FlowSeedingSubtab::_fileWriterChanged()
{
    std::string filename = _fileWriter->GetPath();
    _params->SetFlowlineOutputFilename( filename );
    _params->SetNeedFlowlineOutput( true );
}
*/


void
FlowSeedingSubtab::_flowDirectionChanged()
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
