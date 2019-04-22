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
            _velocityField     ( 9 ),
            _colorField        ( 3 ),
            _colorMapTexOffset ( 0 )
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
    _cache_steadyNumOfSteps = 0;
    _cache_velocityMltp     = 1.0;
    _cache_seedGenMode      = 0;

    _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
    _colorStatus            = FlowStatus::SIMPLE_OUTOFDATE;
    _steadyTotalSteps       = 0;

    _advectionComplete      = false;
    _coloringComplete       = false;
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
    _timestamps = _dataMgr->GetTimeCoordinates();

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
    int rv;     // return value

    _updateFlowCacheAndStates( params );
    _velocityField.UpdateParams( params );
    _colorField.UpdateParams( params );

    if( _velocityStatus == FlowStatus::SIMPLE_OUTOFDATE )
    {
        if( _cache_seedGenMode == 0 )
        {
            std::vector<flow::Particle> seeds;
            _genSeedsXY( seeds, _timestamps.at(0) );
            _advection.UseSeedParticles( seeds );
        }
        else if( _cache_seedGenMode == 1 )
        {
            rv = _advection.InputStreamsGnuplot( params->GetSeedInputFilename() );
            if( rv != 0 )
            {
                MyBase::SetErrMsg("Input seed list wrong!");
                return flow::FILE_ERROR;
            }
        }

        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
        _steadyTotalSteps = 0;
    }
    else if( _velocityStatus == FlowStatus::TIME_STEP_OOD )
    {
        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
    }

    if( !params->UseSingleColor() )
    {
        if( _colorStatus == FlowStatus::SIMPLE_OUTOFDATE )
        {
            _advection.ResetParticleValues();
            _coloringComplete = false;
            _colorStatus      = FlowStatus::UPTODATE;
        }
        else if( _colorStatus == FlowStatus::TIME_STEP_OOD )
        {
            _coloringComplete = false;
            _colorStatus      = FlowStatus::UPTODATE;
        }
    }

    if( !_advectionComplete )
    {
        float deltaT = 0.05;          // For only 1 timestep case
        if( _timestamps.size() > 1 )  // For multiple timestep case
            deltaT *= _timestamps[1] - _timestamps[0];

        rv = flow::ADVECT_HAPPENED;

        /* Advection scheme 1: advect a maximum number of steps.
         * This scheme is used for steady flow */
        if( params->GetIsSteady() )
        {
            size_t actualSteps = 0;
            int numOfSteps = params->GetSteadyNumOfSteps();
            for( size_t i = _steadyTotalSteps; i < numOfSteps && rv == flow::ADVECT_HAPPENED; i++ )
            {
                rv = _advection.AdvectOneStep( &_velocityField, deltaT );
                _steadyTotalSteps++;
                actualSteps++;
            }

        }

        /* Advection scheme 2: advect to a certain timestamp.
         * This scheme is used for unsteady flow */
        else
        {
            for( int i = 1; i <= _cache_currentTS; i++ )
            {
                rv = _advection.AdvectTillTime( &_velocityField, deltaT, _timestamps.at(i) );
            }
        }

        _advectionComplete = true;
    }


    if( !_coloringComplete )
    {
        rv = _advection.CalculateParticleValues( &_colorField, true );
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

    // In case of steady flow, we render up to _cache_steadyNumOfSteps + 1.
    if( _cache_isSteady )
    {
        size_t numOfStep = params->GetSteadyNumOfSteps();
        numOfPart     = stream.size();
        numOfPart     = numOfPart < numOfStep+1 ? numOfPart : numOfStep+1;
        float* buffer = new float[ 4 * numOfPart ];
        size_t i = 0;
        for( const auto& p : stream )
        {
            if( i < numOfPart )
            {
                std::memcpy( buffer + i * 4, glm::value_ptr(p.location), sizeof(glm::vec3) );
                buffer[ i * 4 + 3 ] = p.value;
                i++;
            }
            else
                break;
        }
        bufPtr = buffer;
    }
    // In case of unsteady, we use particles up to currentTS
    else 
    {
        for( const auto& p : stream )
        {
            if( p.time <= _timestamps.at( _cache_currentTS ) )
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
    /* Strategy:
     * First, compare parameters that if changed, they would put both steady and unsteady
     *   streams out of date.
     * Second, branch into steady and unsteady cases, and deal with them separately.
     */

    // Check seed generation mode
    if( _cache_seedGenMode != params->GetSeedGenMode() )
    {
        _cache_seedGenMode  = params->GetSeedGenMode();
        _velocityStatus     = FlowStatus::SIMPLE_OUTOFDATE;
        _colorStatus        = FlowStatus::SIMPLE_OUTOFDATE;
    }

    // Check seed input filename
    if( _cache_seedInputFilename != params->GetSeedInputFilename() )
    {
        _cache_seedInputFilename  = params->GetSeedInputFilename();
        // we only update status if the current seed generation mode IS seed list.
        if( _cache_seedGenMode   == 1 )
        {
            _velocityStatus     = FlowStatus::SIMPLE_OUTOFDATE;
            _colorStatus        = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }

    // Check variable names
    // If names not the same, entire stream is out of date
    // Note: variable names are kept in VaporFields.
    std::vector<std::string> varnames = params->GetFieldVariableNames();
    if( ( varnames.at(0) != _velocityField.VelocityNames[0] ) ||
        ( varnames.at(1) != _velocityField.VelocityNames[1] ) ||
        ( varnames.at(2) != _velocityField.VelocityNames[2] ) )
        _velocityStatus = FlowStatus::SIMPLE_OUTOFDATE;
    std::string colorVarName = params->GetColorMapVariableName();
    if( colorVarName != _colorField.ScalarName )
        _colorStatus = FlowStatus::SIMPLE_OUTOFDATE;

    // Check compression parameters
    // If these parameters not the same, entire stream is out of date
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

    // Check velocity multiplier
    // If the multiplier is changed, then the entire stream is out of date
    const float mult = params->GetVelocityMultiplier();
    if( _cache_velocityMltp != mult )
    {
        _cache_velocityMltp       = mult;
        _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
    }

    /* Now we branch into steady and unsteady cases, and treat them separately */
    if( params->GetIsSteady() )
    {
        if( _cache_isSteady )   // steady state isn't changed
        {
            if( params->GetSteadyNumOfSteps() > _cache_steadyNumOfSteps )
            {
                if( _colorStatus     == FlowStatus::UPTODATE )
                    _colorStatus      = FlowStatus::TIME_STEP_OOD;
                if( _velocityStatus  == FlowStatus::UPTODATE )
                    _velocityStatus   = FlowStatus::TIME_STEP_OOD;
            }
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();

            if( _cache_currentTS     != params->GetCurrentTimestep() )
            {
                _cache_currentTS      = params->GetCurrentTimestep();
                _colorStatus          = FlowStatus::SIMPLE_OUTOFDATE;
                _velocityStatus       = FlowStatus::SIMPLE_OUTOFDATE;
            }
        }
        else    // switched from unsteady to steady. Everything is out of date in this case.
        {
            _cache_isSteady         = true;
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
            _cache_currentTS        = params->GetCurrentTimestep();
            _colorStatus            = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }
    /* in case of unsteady flow */
    else
    {
        if( !_cache_isSteady )  // unsteady state isn't changed
        {
            if( _cache_currentTS     < params->GetCurrentTimestep() )
            {
                if( _colorStatus    == FlowStatus::UPTODATE )
                    _colorStatus     = FlowStatus::TIME_STEP_OOD;
                if( _velocityStatus == FlowStatus::UPTODATE )
                    _velocityStatus  = FlowStatus::TIME_STEP_OOD;
            }
            _cache_currentTS        = params->GetCurrentTimestep();
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
        }
        else    // switched from steady to unsteady
        {
            _cache_isSteady         = false;
            _cache_steadyNumOfSteps = params->GetSteadyNumOfSteps();
            _cache_currentTS        = params->GetCurrentTimestep();
            _colorStatus            = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
        }
    }
}

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



int
FlowRenderer::_genSeedsXY( std::vector<flow::Particle>& seeds, float timeVal ) const
{
    int numX = 8, numY = 8;
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


