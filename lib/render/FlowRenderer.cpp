#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/OceanField.h"
#include "vapor/SteadyVAPORVelocity.h"
#include "vapor/UnsteadyVAPORVelocity.h"
#include "vapor/SteadyVAPORScalar.h"
#include "vapor/Particle.h"
#include <iostream>
#include <cstring>

#define GL_ERROR     -20

using namespace VAPoR;

static RendererRegistrar<FlowRenderer> registrar( FlowRenderer::GetClassType(), 
                                                  FlowParams::GetClassType() );

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


// Constructor
FlowRenderer::FlowRenderer( const ParamsMgr*    pm,
                      std::string&        winName,
                      std::string&        dataSetName,
                      std::string&        instName,
                      DataMgr*            dataMgr )
          : Renderer( pm,
                      winName,
                      dataSetName,
                      FlowParams::GetClassType(),
                      FlowRenderer::GetClassType(),
                      instName,
                      dataMgr ),
            _colorMapTexOffset ( 0 ),
            _velocityField     ( 9 ),
            _colorField        ( 3 )
{ 
    // Initialize OpenGL states
    _shader         = nullptr;
    _vertexArrayId  = 0;
    _vertexBufferId = 0;
    _colorMapTexId  = 0;

    // Initialize advection states
    _cache_currentTS        = -1;
    _cache_refinementLevel  = -2;
    _cache_compressionLevel = -2;
    _cache_isSteady         = false;
    _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
    _colorStatus           = FlowStatus::SIMPLE_OUTOFDATE;

    _advectionComplete      = false;
    _coloringComplete       = false;

    //_colorField = nullptr;
}

// Destructor
FlowRenderer::~FlowRenderer()
{ 
    // Delete vertex arrays
    if( _vertexArrayId )
    {
        glDeleteVertexArrays(1, &_vertexArrayId );
        _vertexArrayId = 0;
    }
    if( _vertexBufferId )
    {
        glDeleteBuffers( 1, &_vertexBufferId );
        _vertexBufferId = 0;
    }

    if( _colorMapTexId )
    {
        glDeleteTextures( 1, &_colorMapTexId );
        _colorMapTexId = 0;
    }
}


int
FlowRenderer::_initializeGL()
{
    // First prepare the VelocityField
    _velocityField.AssignDataManager( _dataMgr );
    _colorField.AssignDataManager(    _dataMgr );

    // Followed by real OpenGL initializations
    ShaderProgram *shader   = nullptr;
    if( (shader = _glManager->shaderManager->GetShader("FlowLine")) )
        _shader      = shader;
    else
        return GL_ERROR;

    /* Create Vertex Array Object (VAO) */
    glGenVertexArrays( 1, &_vertexArrayId );
    glGenBuffers(      1, &_vertexBufferId );

    /* Generate and configure 1D texture: _colorMapTexId */
    glGenTextures( 1, &_colorMapTexId );
    glActiveTexture( GL_TEXTURE0 + _colorMapTexOffset );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glBindTexture( GL_TEXTURE_1D, 0 );

    return 0;
}

int
FlowRenderer::_paintGL( bool fast )
{
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );

    _updateFlowCacheAndStates( params );
    _velocityField.UpdateParams( params );
    _colorField.UpdateParams( params );

    if( _velocityStatus == FlowStatus::SIMPLE_OUTOFDATE )
    {
        std::vector<flow::Particle> seeds;
        _genSeedsXY( seeds, _cache_timestamps.at(0) );
        _advection.UseSeedParticles( seeds );
        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
    }
    else if( _velocityStatus == FlowStatus::TIME_STEP_OFD )
    {
        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
    }

    if( !params->UseSingleColor() && _colorStatus == FlowStatus::SIMPLE_OUTOFDATE )
    {
        _coloringComplete = false;
        _colorStatus      = FlowStatus::UPTODATE;
    }

    if( !_advectionComplete )
    {
        float deltaT = 0.05;    // For steady flow, and unsteady with only 1 time step.
        if( _cache_timestamps.size() > 1 )
            deltaT *= _cache_timestamps[1] - _cache_timestamps[0];
        
        int rv = _advection.Advect( &_velocityField, deltaT );
        size_t totalSteps = 1, maxSteps = 70;
        while( rv == flow::ADVECT_HAPPENED && totalSteps < maxSteps )
        {
            rv = _advection.Advect( &_velocityField, deltaT );
            totalSteps++;
        }

        _advectionComplete = true;
    }

    if( !_coloringComplete )
    {
        int rv = _advection.CalculateParticleProperty( &_colorField, true );
        _coloringComplete = true;
    }

    _purePaint( params, fast );
    _restoreGLState();

    return 0;
}

int
FlowRenderer::_purePaint( FlowParams* params, bool fast )
{
    _prepareColormap( params );

    size_t numOfStreams = _advection.GetNumberOfStreams();
    for( size_t i = 0; i < numOfStreams; i++ )
    {
        const auto& s = _advection.GetStreamAt( i );
        if( fast )
            _drawAStreamAsLines( s, params );
        else
        {
            _drawAStreamAsLines( s, params );
        }
    }
    
    return 0;
}

int
FlowRenderer::_drawAStreamAsLines( const std::vector<flow::Particle>& stream,
                                   const FlowParams* params ) const
{
    size_t numOfPart = 0;
    std::vector<float> vec;
    const float* bufPtr = nullptr;

    if( _cache_isSteady ) // In case of steady flow, we render all available particles
    {
        numOfPart     = stream.size();
        float* buffer = new float[ 4 * numOfPart ];
        size_t offset = 0;
        for( const auto& p : stream )
        {
            std::memcpy( buffer + offset, glm::value_ptr(p.location), sizeof(glm::vec3) );
            offset += 3;
            buffer[ offset++ ] = p.value;
        }
        bufPtr = buffer;
    }
    else // In case of unsteady, we use particles up to currentTS
    {
        for( const auto& p : stream )
        {
            if( p.time <= _cache_timestamps.at( _cache_currentTS ) )
            {
                vec.push_back( p.location.x );
                vec.push_back( p.location.y );
                vec.push_back( p.location.z );
                vec.push_back( p.value );
            }
            else
                break;
        }
        numOfPart = vec.size() / 4;
        bufPtr    = vec.data();
    }

    // Make some OpenGL function calls
    glm::mat4 modelview  = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();
    _shader->Bind();
    _shader->SetUniform("MV", modelview);
    _shader->SetUniform("Projection", projection);
    _shader->SetUniform("colorMapRange", glm::make_vec3(_colorMapRange));
    bool singleColor = params->UseSingleColor();
    _shader->SetUniform( "singleColor", int(singleColor) );

    glActiveTexture( GL_TEXTURE0 + _colorMapTexOffset );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    _shader->SetUniform("colorMapTexture", _colorMapTexOffset);

    glBindVertexArray( _vertexArrayId );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, _vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 4 * numOfPart, bufPtr, GL_STREAM_DRAW );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_LINE_STRIP, 0, numOfPart );

    // Some OpenGL cleanup
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glBindVertexArray( 0 );

    if( _cache_isSteady )
        delete[] bufPtr;

    return 0;
}

void
FlowRenderer::_updateFlowCacheAndStates( const FlowParams* params )
{
    // Check variable names
    std::vector<std::string> varnames = params->GetFieldVariableNames();
    if( varnames.size() == 3 )
    {
        if( ( varnames[0] != _velocityField.VelocityNames[0] ) ||
            ( varnames[1] != _velocityField.VelocityNames[1] ) ||
            ( varnames[2] != _velocityField.VelocityNames[2] ) )
            _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    }
    else
    {
        MyBase::SetErrMsg("Missing velocity variable");
        std::cout << "Missing velocity variable" << std::endl;
    }
    std::string colorVarName = params->GetColorMapVariableName();
    if( colorVarName != _colorField.ScalarName )
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;

    // Check compression parameters
    if( _cache_refinementLevel != params->GetRefinementLevel() )
    {
        _cache_refinementLevel    = params->GetRefinementLevel();
        _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
    }
    if( _cache_compressionLevel  != params->GetCompressionLevel() )
    {
        _cache_compressionLevel   = params->GetCompressionLevel();
        _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check steady/unsteady status
    if( _cache_isSteady != params->GetIsSteady() )
    {
        _cache_isSteady           = params->GetIsSteady();
        _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Time step is a little tricky...
    if( _cache_currentTS != params->GetCurrentTimestep() )
    {
        _cache_currentTS  = params->GetCurrentTimestep();
        _cache_timestamps = _dataMgr->GetTimeCoordinates();
        if( _cache_isSteady )
        {
            _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
        }
        else
        {   // !! Only apply status "TIME_STEP_OFD" if the old status is "UPTODATE" !!
            if( _colorStatus         == FlowStatus::UPTODATE )
                _colorStatus          = FlowStatus::TIME_STEP_OFD;
            if( _velocityStatus      == FlowStatus::UPTODATE )
                _velocityStatus       = FlowStatus::TIME_STEP_OFD;
        }
    }

    /* I'm not sure if this piece of code is necessary
     *
    int rv  = _advection.CheckReady();
    if( rv != 0 )
    {
        _state_velocitiesUpToDate = false;
        _state_scalarUpToDate     = false;
        return;
    } */
}

#if 0
int
FlowRenderer::_useSteadyColorField( const FlowParams* params )
{
    // The caller of this function is responsible for checking if 
    //   this function is in need to be called.
    //
    std::string colorVarName = params->GetColorMapVariableName();
    if( colorVarName.empty() )
    {
        MyBase::SetErrMsg("Missing color mapping variable");
        return flow::GRID_ERROR;
    }
    
    Grid *grid;
    int rv  = _getAGrid( params, _cache_currentTS, colorVarName, &grid );
    if( rv != 0 )   
        return rv;

    flow::SteadyVAPORScalar* ptr = new flow::SteadyVAPORScalar();
    ptr->UseGrid( grid );
    ptr->ScalarName = colorVarName;

    if( _colorField )
        delete _colorField;
    _colorField = ptr;

    return 0;
}
#endif

/*
int
FlowRenderer::_colorLastParticle()
{
    // The caller of this function is responsible for checking if 
    //   this function is in need to be called.
    //
    if( _colorField == nullptr )
        return flow::NO_FIELD_YET;

    size_t numOfStreams = _advection.GetNumberOfStreams();
    for( size_t i = 0; i < numOfStreams; i++ )
    {
        const auto& stream   = _advection.GetStreamAt( i );
        const auto& particle = stream.back();
        float oldValue       = particle.value;
        if( oldValue == 0.0f )  // We only calculate its value if it's not been calculated yet.
        {
            float newVal;
            int rv  = _colorField->GetScalar( particle.time, particle.location, newVal );
            if( rv == 0 )   // We have the new value!
                _advection.AssignLastParticleValueOfAStream( newVal, i );
            else            // Copy the value from previous particle
                _advection.RepeatLastTwoParticleValuesOfAStream( i );
        }
    }
    return 0;
}*/

/*
int
FlowRenderer::_populateParticleProperties( const std::string& varname,
                                           const FlowParams*  params,
                                           bool  useAsColor )
{
    flow::ScalarField* scalar = nullptr;
    if( useAsColor )
        scalar = _colorField;
    else
    {
        std::string name = varname; // Subsequent functions ain't const
        if( params->GetIsSteady() )
        {
            Grid *grid;
            int rv  = _getAGrid( params, _cache_currentTS, name, &grid );
            if( rv != 0 )   
                return rv;

            flow::SteadyVAPORScalar* ptr = new flow::SteadyVAPORScalar();
            ptr->UseGrid( grid );
            ptr->ScalarName = name;
            scalar = ptr;
        }
        else
        { 
            // create an UnsteadyVAPORScalar
        }
    }

    if( scalar == nullptr )
        return flow::NO_FIELD_YET;

    std::vector<float> properties;
    size_t numOfStreams = _advection.GetNumberOfStreams();
    for( size_t i = 0; i < numOfStreams; i++ )
    {
        const auto& stream   = _advection.GetStreamAt( i );
        properties.clear();
        float newVal = 0.0f;
        for( const auto& p : stream )
        {
            scalar->GetScalar( p.time, p.location, newVal );
            properties.push_back( newVal );
        }
        if( useAsColor )
            _advection.AssignParticleValuesOfAStream( properties, i );
        else
            _advection.AttachParticlePropertiesOfAStream( properties, i );
    }

    if( scalar != _colorField ) // Clean up scalar if it's what we just created
        delete scalar;
    
    return 0;
}
*/

#if 0
int
FlowRenderer::_useSteadyVAPORField( const FlowParams* params )
{
    // The caller of this function is responsible for checking if 
    //   this function is in need to be called.
    //
    // Step 1: retrieve variable names from the params class
    std::vector<std::string> varnames = params->GetFieldVariableNames();
    assert( varnames.size() == 3 );  // need to have three components
    for( auto& s : varnames )
    {
        if( s.empty() )
        {
            MyBase::SetErrMsg("Missing velocity field");
            return flow::GRID_ERROR;
        }
    }

    // Step 2: use these variable names to get data grids
    Grid *gridU, *gridV, *gridW;
    int rv  = _getAGrid( params, _cache_currentTS, varnames[0], &gridU );
    if( rv != 0 )   return rv;
    rv      = _getAGrid( params, _cache_currentTS, varnames[1], &gridV );
    if( rv != 0 )   return rv;
    rv      = _getAGrid( params, _cache_currentTS, varnames[2], &gridW );
    if( rv != 0 )   return rv;

    // Step 3: create a SteadyVAPORVelocity using these grids, and ask Advection to use it!
    flow::SteadyVAPORVelocity* velocity = new flow::SteadyVAPORVelocity();
    velocity->UseGrids( gridU, gridV, gridW );
    velocity->VelocityNameU = varnames[0];
    velocity->VelocityNameV = varnames[1];
    velocity->VelocityNameW = varnames[2];
    
    // Get ready Advection class
    std::vector<flow::Particle> seeds;
    _genSeedsXY( seeds, 0.0f );
    _advection.UseSeedParticles( seeds );
    _advection.UseVelocity( velocity );

    _velocityStatus = FlowStatus::UPTODATE;
    
    return 0;
}

int
FlowRenderer::_useUnsteadyVAPORField( const FlowParams* params )
{
    // Step 1: collect all variable names
    std::vector<std::string> varnames = params->GetFieldVariableNames();
    assert( varnames.size() == 3 );  // need to have three components
    for( auto& s : varnames )
    {
        if( s.empty() )
        {
            MyBase::SetErrMsg("Missing velocity field");
            return flow::GRID_ERROR;
        }
    }

    // Step 2: Create an UnsteadyVAPORVelocity field
    flow::UnsteadyVAPORVelocity* velocity = new flow::UnsteadyVAPORVelocity();
    velocity->VelocityNameU = varnames[0];
    velocity->VelocityNameV = varnames[1];
    velocity->VelocityNameW = varnames[2];
    const auto& timeCoords  = _dataMgr->GetTimeCoordinates();
    int numOfTimesteps      = _dataMgr->GetNumTimeSteps();
    assert( numOfTimesteps == timeCoords.size() );

    Grid *gridU, *gridV, *gridW;
    int rv;
    for( size_t ts = 0; ts < numOfTimesteps; ts++ )
    {
        rv      = _getAGrid( params, ts, varnames[0], &gridU );
        if( rv != 0 )   return rv;
        rv      = _getAGrid( params, ts, varnames[1], &gridV );
        if( rv != 0 )   return rv;
        rv      = _getAGrid( params, ts, varnames[2], &gridW );
        if( rv != 0 )   return rv;
        velocity->AddTimeStep( gridU, gridV, gridW, timeCoords[ts] );
    }

    _advection.UseVelocity( velocity );
    std::vector<flow::Particle> seeds;
    _genSeedsXY( seeds, timeCoords[0] );
    _advection.UseSeedParticles( seeds );

    _velocityStatus = FlowStatus::UPTODATE;

    return 0;
}

int
FlowRenderer::_useUnsteadyColorField( const FlowParams* params )
{
    return 0;
}
#endif

/*
int
FlowRenderer::_AddTimestepUnsteadyVAPORField( const FlowParams* )
{
    // Step 1: collect all variable names
    const auto& nameU = _advection.GetVelocityNameU();
    const auto& nameV = _advection.GetVelocityNameV();
    const auto& nameW = _advection.GetVelocityNameW();

    // Step 2: prepare for adding new timesteps
    const auto& timeCoords = _dataMgr->GetTimeCoordinates();
    Grid  *gridU, *gridV, *gridW;
    int   rv;
    flow::UnsteadyVAPORVelocity* ptr = dynamic_cast<flow::UnsteadyVAPORVelocity*>();
    size_t alreadyHave = _advection->GetNumberOfTimesteps();

    return 0;
} */

int
FlowRenderer::_genSeedsXY( std::vector<flow::Particle>& seeds, float timeVal ) const
{
    int numX = 5, numY = 5;
    std::vector<double>           extMin, extMax;
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );
    params->GetBox()->GetExtents( extMin, extMax );
    float stepX = (extMax[0] - extMin[0]) / (numX + 1.0);
    float stepY = (extMax[1] - extMin[1]) / (numY + 1.0);
    float stepZ = extMin[2] + (extMax[2] - extMin[2]) / 100.0;

    seeds.resize( numX * numY );
    for( int y = 0; y < numY; y++ )
        for( int x = 0; x < numX; x++ )
        {
            int idx = y * numX + x;
            seeds[idx].location.x = extMin[0] + (x+1.0f) * stepX;
            seeds[idx].location.y = extMin[1] + (y+1.0f) * stepY;
            seeds[idx].location.z = stepZ ;
            seeds[idx].time       = timeVal;
        }

    return 0;
}

int
FlowRenderer::_getAGrid( const FlowParams* params, 
                         int               timestep,
                         std::string&      varName,
                         Grid**            gridpp  ) const
{
    std::vector<double>           extMin, extMax;
    params->GetBox()->GetExtents( extMin, extMax );
    Grid* grid = _dataMgr->GetVariable( timestep,
                                        varName,
                                        params->GetRefinementLevel(),
                                        params->GetCompressionLevel(),
                                        extMin,
                                        extMax );
    if( grid == nullptr )
    {
        MyBase::SetErrMsg("Not able to get a grid!");
        return flow::GRID_ERROR;
    }
    else
    {
        *gridpp = grid;
        return 0;
    }
}
    
void
FlowRenderer::_prepareColormap( FlowParams* params )
{
    if( params->UseSingleColor() )
    {
        float singleColor[4];
        params->GetConstantColor( singleColor );
        singleColor[3]            = 1.0f;      // 1.0 in alpha channel
        _colorMap.resize( 8 );                 // _colorMap will have 2 RGBA values
        for( int i = 0; i < 8; i++ )
            _colorMap [i]         = singleColor[ i % 4 ];
        _colorMapRange[0]         = 0.0f;   // min value of the color map
        _colorMapRange[1]         = 0.0f;   // max value of the color map
        _colorMapRange[2]         = 1e-5f;  // diff of color map. Has to be non-zero though.
    }
    else
    {
        // This is the line that's not const
        VAPoR::MapperFunction* mapperFunc = params->GetMapperFunc
                                            ( params->GetColorMapVariableName() );
        mapperFunc->makeLut( _colorMap );
        assert( _colorMap.size()  % 4 == 0 );
        std::vector<double> range = mapperFunc->getMinMaxMapValue();
        _colorMapRange[0]         = float(range[0]);
        _colorMapRange[1]         = float(range[1]);
        _colorMapRange[2]         = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ?
                                    (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f ;
    }
    glActiveTexture( GL_TEXTURE0 + _colorMapTexOffset );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA32F,     _colorMap.size()/4,
                   0, GL_RGBA,       GL_FLOAT,       _colorMap.data() );
}

void
FlowRenderer::_restoreGLState() const
{
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_1D, 0 );
}

#ifndef WIN32
double FlowRenderer::_getElapsedSeconds( const struct timeval* begin, 
                                         const struct timeval* end ) const
{
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec)/1000000.0);
}
#endif


