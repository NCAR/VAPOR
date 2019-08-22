#include "FlowSubtabs.h"
#include "vapor/DataMgrUtils.h"

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

    _velocityMltp = new VLineEdit( this, "Velocity Multiplier", "1.0" );
    _layout->addWidget( _velocityMltp );

    _steady = new VCheckBox( this, "Use Steady Flow" );
    _layout->addWidget( _steady );

    _steadyNumOfSteps = new VLineEdit( this, "Steady Integration Steps", "100" );
    _layout->addWidget( _steadyNumOfSteps);

    _periodicX = new VCheckBox( this, "Particles periodic in X" );
    _layout->addWidget( _periodicX );
    _periodicY = new VCheckBox( this, "Particles periodic in Y" );
    _layout->addWidget( _periodicY );
    _periodicZ = new VCheckBox( this, "Particles periodic in Z" );
    _layout->addWidget( _periodicZ );

    connect( _steady,           SIGNAL( _checkboxClicked() ), this, SLOT( _steadyGotClicked() ) );
    connect( _velocityMltp,     SIGNAL( _editingFinished() ), this, SLOT( _velocityMultiplierChanged() ) );
    connect( _steadyNumOfSteps, SIGNAL( _editingFinished() ), this, SLOT( _steadyNumOfStepsChanged() ) );

    connect( _periodicX,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicY,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
    connect( _periodicZ,        SIGNAL( _checkboxClicked() ), this, SLOT( _periodicClicked() ) );
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

    auto mltp = _params->GetVelocityMultiplier();
    _velocityMltp->SetEditText( QString::number( mltp, 'f', 3 ) );

    int numOfSteps = _params->GetSteadyNumOfSteps();
    _steadyNumOfSteps->SetEditText( QString::number( numOfSteps ) );

    auto bools = _params->GetPeriodic();
    _periodicX->SetCheckState( bools[0] );
    _periodicY->SetCheckState( bools[1] );
    _periodicZ->SetCheckState( bools[2] );
}
    
void 
FlowVariablesSubtab::_periodicClicked()
{
    std::vector<bool> bools( 3, false );
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
    double newval, oldval;
    oldval = _params->GetVelocityMultiplier();
    try
    {
        newval = std::stod( _velocityMltp->GetEditText() );
    }
    catch ( const std::invalid_argument& e )
    {
        std::cerr << "bad input: " << _velocityMltp->GetEditText() << std::endl;
        _velocityMltp->SetEditText( QString::number( oldval, 'f', 3 ) );
        return;
    }

    if( newval >= 0.001 && newval <= 1.0 )    // in the valid range
    {
        /* std::stod() would convert "3.83aaa" without throwing an exception.
           We set the correct text based on the number identified.        */
        _velocityMltp->SetEditText( QString::number( newval, 'f', 3 ) );
        /* Only write back to _params if newval is different from oldval */
        if( newval != oldval )
            _params->SetVelocityMultiplier( newval );
    }
    else
        _velocityMltp->SetEditText( QString::number( oldval, 'f', 3 ) );
}

void 
FlowVariablesSubtab::_steadyNumOfStepsChanged()
{
    int newval, oldval;
    oldval = (int)_params->GetSteadyNumOfSteps();
    try
    {
        newval = std::stoi( _steadyNumOfSteps->GetEditText() );
    }
    catch ( const std::invalid_argument& e )
    {
        std::cerr << "bad input: " << _steadyNumOfSteps->GetEditText() << std::endl;
        _steadyNumOfSteps->SetEditText( QString::number( oldval ) );
        return;
    }

    if( newval >= 0 )    // in the valid range
    {
        /* std::stoi() would convert "383aaa" without throwing an exception.
           We set the correct text based on the number identified.        */
        _steadyNumOfSteps->SetEditText( QString::number( newval ) );
        /* Only write back to _params if newval is different from oldval */
        if( newval != oldval )
            _params->SetSteadyNumOfSteps( newval );
    }
    else
        _steadyNumOfSteps->SetEditText( QString::number( oldval ) );
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
    _seedGenMode = new VComboBox( this, "Seed Generation Mode" );
    /* Index numbers are in agreement with what's in FlowRenderer.h */
    _seedGenMode->AddOption( "Programatically", 0 );
    _seedGenMode->AddOption( "From a List",     1 );
    _seedGenMode->AddOption( "From a Rake, Uniformly",  2 );
    _seedGenMode->AddOption( "From a Rake, Randomly",   3 );
    _seedGenMode->AddOption( "From a Rake, Randomly with Bias",   4 );
    _layout->addWidget( _seedGenMode );
    connect( _seedGenMode, SIGNAL( _indexChanged(int) ), this, SLOT( _seedGenModeChanged(int) ) );
   
    _fileReader = new VFileReader( this, "Input Seed File" );
    _fileReader->SetFileFilter( QString::fromAscii("*.txt") );
    _layout->addWidget( _fileReader );
    connect( _fileReader, SIGNAL( _pathChanged() ), this, SLOT( _fileReaderChanged() ) );

    _flowDirection = new VComboBox( this, "Steady Flow Direction" );
    /* Index numbers are in agreement with what's in FlowRenderer.h */
    _flowDirection->AddOption( "Forward", 0 );
    _flowDirection->AddOption( "Backward", 1 );
    _flowDirection->AddOption( "Bi-Directional", 2 );
    _layout->addWidget( _flowDirection );
    connect( _flowDirection, SIGNAL(_indexChanged(int)), this, SLOT( _flowDirectionChanged(int) ) );

    _fileWriter = new VFileWriter( this, "Output Flow Lines" );
    _fileWriter->SetFileFilter( QString::fromAscii("*.txt") );
    _layout->addWidget( _fileWriter );
    connect( _fileWriter, SIGNAL( _pathChanged() ), this, SLOT( _fileWriterChanged() ) );

    _outputButton = new QPushButton( "Output Flow Lines", this );
    _layout->addWidget( _outputButton );
    connect( _outputButton, SIGNAL( clicked() ), this, SLOT( _outputButtonClicked() ) );

    /* Set up the Rake selector */
    std::vector<float> geoRange = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0}; // temporary range
    _rake = new VGeometry( this, 3, geoRange );
    _layout->addWidget( _rake );
    connect( _rake, SIGNAL( _geometryChanged() ), this, SLOT( _rakeGeometryChanged() ) );

    /* Set up rake seed number controls */
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
    _rakeBiasStrength = new VSlider(   this, "Random Bias Strength", 1.0f, 10.0f );
    _layout->addWidget( _rakeBiasVariable );
    _layout->addWidget( _rakeBiasStrength );
    connect( _rakeBiasVariable, SIGNAL( _indexChanged(int) ), this, SLOT( _rakeBiasVariableChanged(int) ) );
    connect( _rakeBiasStrength, SIGNAL( _valueChanged() ),    this, SLOT( _rakeBiasStrengthChanged()    ) );
}

void FlowSeedingSubtab::Update( VAPoR::DataMgr      *dataMgr,
                                VAPoR::ParamsMgr    *paramsMgr,
                                VAPoR::RenderParams *params )
{
    _params = dynamic_cast<VAPoR::FlowParams*>(params);
    VAssert( _params );

    /* Update seed generation mode combo */
    long idx = _params->GetSeedGenMode();
    if( idx >= 0 && idx < _seedGenMode->GetNumOfItems() )
        _seedGenMode->SetIndex( idx );
    else
        _seedGenMode->SetIndex( 0 );

    /* Update flow direction combo */
    idx = _params->GetFlowDirection();
    if( idx >= 0 && idx < _flowDirection->GetNumOfItems() )
        _flowDirection->SetIndex( idx );
    else
        _flowDirection->SetIndex( 0 );

    /* Update rake range */
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

    /* Update rake values */
    auto rakeVals = _params->GetRake();
    /* In case the user hasn't set the rake, set the current value to be the rake extents,
       plus update the params.  Otherwise, apply the actual rake values. */
    if( std::isnan( rakeVals[0] ) )
    {
        _rake->SetCurrentValues( range );
        _params->SetRake( range );
    }
    else
    {
        _rake->SetCurrentValues( rakeVals );
    }

    /* Update rake random bias variable and strength */
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

    auto strenParams = _params->GetRakeBiasStrength();
    if(  strenParams < 0 )  // Strength isn't set by the user yet. Let's set it to 1.0!
    {
        _params->SetRakeBiasStrength( 1.0f );
        _rakeBiasStrength->SetCurrentValue( 1.0f );
    }
    else
        _rakeBiasStrength->SetCurrentValue( strenParams );

    /* Update input and output file names */
    if( !_params->GetSeedInputFilename().empty() ) 
        _fileReader->SetPath( _params->GetSeedInputFilename() );
    if( !_params->GetFlowlineOutputFilename().empty() ) 
        _fileWriter->SetPath( _params->GetFlowlineOutputFilename() );
}
    
void 
FlowSeedingSubtab::_rakeBiasVariableChanged( int idx )
{
    std::cout << "new variable is: " << _rakeBiasVariable->GetItemText( idx ) << std::endl;
}
    
void 
FlowSeedingSubtab::_rakeBiasStrengthChanged()
{
    std::cout << "new strength is: " << _rakeBiasStrength->GetCurrentValue( ) << std::endl;
}

void
FlowSeedingSubtab::_rakeNumOfSeedsChanged()
{
    /* These fields should ALWAYS contain legal values, even when not in use.
       That's why we validate every one of them!                           */
    
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
            /* std::stol() would convert "383aaa" without throwing an exception.
               We set the correct text based on the number identified.        */
            pointers[i]->SetEditText( QString::number( tmp ) );
        }
        else
        {
            newVal[i] = oldVal[i];
            pointers[i]->SetEditText( QString::number( oldVal[i] ) );
        }
    }

    /* Only write back to _params when newVal is different from oldVal */
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
        for( int i = 0; i < 4; i++ )
            std::cout << newVal[i] << "  ";
        std::cout << std::endl;
        _params->SetRakeNumOfSeeds( newVal );
    }
}

void
FlowSeedingSubtab::_rakeGeometryChanged()
{
    std::vector<float> range;
    _rake->GetCurrentValues( range );
    for( int i = 0; i < range.size(); i++ )
        std::cout << range[i] << ",  ";
    std::cout << std::endl;
}

void
FlowSeedingSubtab::_outputButtonClicked( )
{
    _params->SetNeedFlowlineOutput( true );
}

void
FlowSeedingSubtab::_seedGenModeChanged( int newIdx )
{
    _params->SetSeedGenMode( newIdx );
}

void
FlowSeedingSubtab::_fileReaderChanged()
{
    std::string filename = _fileReader->GetPath();
    _params->SetSeedInputFilename( filename );
}

void
FlowSeedingSubtab::_fileWriterChanged()
{
    std::string filename = _fileWriter->GetPath();
    _params->SetFlowlineOutputFilename( filename );
}

void
FlowSeedingSubtab::_flowDirectionChanged( int newIdx )
{
    _params->SetFlowDirection( newIdx );
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
