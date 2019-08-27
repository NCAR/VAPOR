#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/OceanField.h"
#include "vapor/SteadyVAPORVelocity.h"
#include "vapor/UnsteadyVAPORVelocity.h"
#include "vapor/SteadyVAPORScalar.h"
#include "vapor/Particle.h"
#include <iostream>
#include <cstring>
#include <random>

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
    _cache_flowDirection    = 0;
    for( int i = 0; i < 3; i++ )
        _cache_periodic[i] = false;

    _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
    _colorStatus            = FlowStatus::SIMPLE_OUTOFDATE;

    _advectionComplete      = false;
    _coloringComplete       = false;

    _2ndAdvection           = nullptr;
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

    if( _2ndAdvection )
    {
        delete _2ndAdvection;
        _2ndAdvection = nullptr;
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

    if( params->GetNeedFlowlineOutput() )
    {
        rv = _advection.OutputStreamsGnuplot( params->GetFlowlineOutputFilename() );
        if( rv != 0 )
        {
                MyBase::SetErrMsg("Output flow lines wrong!");
                return flow::FILE_ERROR;
        }
        if( _2ndAdvection )     // bi-directional advection
        {
            rv = _2ndAdvection->OutputStreamsGnuplot( params->GetFlowlineOutputFilename(), true );
        }

        params->SetNeedFlowlineOutput( false );
    }

    _updateFlowCacheAndStates( params );
    _velocityField.UpdateParams( params );
    _colorField.UpdateParams( params );

    if( _velocityStatus == FlowStatus::SIMPLE_OUTOFDATE )
    {
        /* Read seeds from a file is a special case, so we put it up front */
        if( _cache_seedGenMode == 1 )
        {
            rv = _advection.InputStreamsGnuplot( params->GetSeedInputFilename() );
            if( rv != 0 )
            {
                MyBase::SetErrMsg("Input seed list wrong!");
                return flow::FILE_ERROR;
            }
            if( _2ndAdvection )     // bi-directional advection
            {
                _2ndAdvection->InputStreamsGnuplot( params->GetSeedInputFilename() );
                _updatePeriodicity( _2ndAdvection ); 
            }
        }
        else 
        {
            std::vector<flow::Particle> seeds;
            if( _cache_seedGenMode == 0 )       // Generate seeds from a built-in function
                _genSeedsXY( seeds, _timestamps.at(0) );
            else if( _cache_seedGenMode == 2 )  // Seeds from a rake, uniformly
                _genSeedsRakeUniform( seeds, _timestamps.at(0) );

            /* diagnose seeds */
            /*std::cout << "Total number of seeds: " << seeds.size() << std::endl;
            auto s = seeds.front();
            printf("front seed: (%f, %f, %f)\n", s.location.x, s.location.y, s.location.z );
            s = seeds.back();
            printf("back seed: (%f, %f, %f)\n", s.location.x, s.location.y, s.location.z );
            */

            // Note on UseSeedParticles(): this is the only function that resets
            //   all the streams inside of an Advection class.
            //   It should immediately be followed by a function to set its periodicity
            _advection.UseSeedParticles( seeds );
            _updatePeriodicity( &_advection );
            if( _2ndAdvection )     // bi-directional advection
            {
                _2ndAdvection->UseSeedParticles( seeds );
                _updatePeriodicity( _2ndAdvection ); 
            }
        }

        _advectionComplete = false;
        _velocityStatus = FlowStatus::UPTODATE;
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
            if( _2ndAdvection )     // bi-directional advection
                _2ndAdvection->ResetParticleValues();
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
            /* If the advection is single-directional */
            if( params->GetFlowDirection() == 1 )   // backward integration
                deltaT *= -1.0f;
            int numOfSteps = params->GetSteadyNumOfSteps();
            for( size_t i = _advection.GetMaxNumOfSteps(); i < numOfSteps && rv == flow::ADVECT_HAPPENED; i++ )
            {
                rv = _advection.AdvectOneStep( &_velocityField, deltaT );
            }

            /* If the advection is bi-directional */
            if( _2ndAdvection )
            {
                assert( deltaT > 0.0f );
                float   deltaT2 = deltaT * -1.0f;
                rv = flow::ADVECT_HAPPENED;
                for( size_t i = _2ndAdvection->GetMaxNumOfSteps(); 
                            i < numOfSteps && rv == flow::ADVECT_HAPPENED; i++ )
                {
                    rv = _2ndAdvection->AdvectOneStep( &_velocityField, deltaT2 );
                }
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
        if( _2ndAdvection )     // bi-directional advection
            rv = _2ndAdvection->CalculateParticleValues( &_colorField, true );
        _coloringComplete = true;
    }

    _prepareColormap( params );
    _renderFromAnAdvection( &_advection, params, fast );
    /* If the advection is bi-directional */
    if( _2ndAdvection )
        _renderFromAnAdvection( _2ndAdvection, params, fast );
    _restoreGLState();

    return 0;
}

int
FlowRenderer::_renderFromAnAdvection( const flow::Advection* adv,
                                      FlowParams*            params,
                                      bool                   fast )
{
    size_t numOfStreams = adv->GetNumberOfStreams();
    auto   numOfPart    = params->GetSteadyNumOfSteps() + 1;
    bool   singleColor  = params->UseSingleColor();


    if( _cache_isSteady )
    {
        std::vector<float> vec;
        for( size_t s = 0; s < numOfStreams; s++ )
        {
            const auto& stream = adv->GetStreamAt( s );
            for( size_t i = 0; i < stream.size() && i < numOfPart; i++ )
            {
                const auto& p = stream[i];
                _particleHelper1( vec, p, singleColor );
            }   // Finish processing a stream
            if( !vec.empty() )
            {
                _drawALineSeg( vec.data(), vec.size() / 4, singleColor );
                vec.clear();
            }
        }   // Finish processing all streams
    }
    else    // Unsteady flow (only occurs with forward direction)
    {
        std::vector<float> vec;
        for( size_t s = 0; s < numOfStreams; s++ )
        {
            const auto& stream = adv->GetStreamAt( s );
            for( const auto& p : stream )
            {
                // Finish this stream once we go beyond the current TS
                if( p.time > _timestamps.at( _cache_currentTS ) )
                    break;

                _particleHelper1( vec, p, singleColor );
            }   // Finish processing a stream
            if( !vec.empty() )
            {
                _drawALineSeg( vec.data(), vec.size() / 4, singleColor );
                vec.clear();
            }
        }   // Finish processing all streams
    }

    return 0;
}

void
FlowRenderer::_particleHelper1( std::vector<float>&     vec, 
                                const flow::Particle&   p, 
                                bool                    singleColor ) const
{
    if( !p.IsSpecial() )    // p isn't a separator
    {
        vec.push_back( p.location.x );
        vec.push_back( p.location.y );
        vec.push_back( p.location.z );
        vec.push_back( p.value );
    }
    else                    // p is a separator
    {
        _drawALineSeg( vec.data(), vec.size() / 4, singleColor );
        vec.clear();
    }
}


int
FlowRenderer::_drawALineSeg( const float* buf, size_t numOfParts, bool singleColor ) const
{
    // Make some OpenGL function calls
    glm::mat4 modelview  = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();
    _shader->Bind();
    _shader->SetUniform("MV", modelview);
    _shader->SetUniform("Projection", projection);
    _shader->SetUniform("colorMapRange", glm::make_vec3(_colorMapRange));
    _shader->SetUniform( "singleColor", int(singleColor) );

    glActiveTexture( GL_TEXTURE0 + _colorMapTexOffset );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    _shader->SetUniform("colorMapTexture", _colorMapTexOffset);

    glBindVertexArray( _vertexArrayId );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, _vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 4 * numOfParts, buf, GL_STREAM_DRAW );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_LINE_STRIP, 0, numOfParts );

    // Some OpenGL cleanup
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glBindVertexArray( 0 );

    return 0;
}

void
FlowRenderer::_updateFlowCacheAndStates( const FlowParams* params )
{
    /* 
     * Strategy:
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

    // Check periodicity
    // If periodicity changes along any dimension, then the entire stream is out of date
    const auto& peri = params->GetPeriodic();
    if( ( _cache_periodic[0] != peri.at(0) ) ||
        ( _cache_periodic[1] != peri.at(1) ) ||
        ( _cache_periodic[2] != peri.at(2) )  )
    {
        for( int i = 0; i < 3; i++ )
            _cache_periodic[i] = peri[i];
        _colorStatus              = FlowStatus::SIMPLE_OUTOFDATE;
        _velocityStatus           = FlowStatus::SIMPLE_OUTOFDATE;
    }
    

    /* 
     * Now we branch into steady and unsteady cases, and treat them separately 
     */
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

        if( _cache_flowDirection   != params->GetFlowDirection() )
        {
            _colorStatus            = FlowStatus::SIMPLE_OUTOFDATE;
            _velocityStatus         = FlowStatus::SIMPLE_OUTOFDATE;
            _cache_flowDirection    = params->GetFlowDirection();
            if( _cache_flowDirection == 2 && !_2ndAdvection )
                _2ndAdvection = new flow::Advection();
            if( _cache_flowDirection != 2 && _2ndAdvection )
            {
                delete _2ndAdvection;
                _2ndAdvection = nullptr;
            }
        }
    }
    else  // in case of unsteady flow
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


int
FlowRenderer::_genSeedsXY( std::vector<flow::Particle>& seeds, float timeVal ) const
{
    int numX = 4, numY = 4;
    std::vector<double>  extMin, extMax;
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );
    params->GetBox()->GetExtents( extMin, extMax );
    float stepX = (extMax[0] - extMin[0]) / (numX + 1.0f);
    float stepY = (extMax[1] - extMin[1]) / (numY + 1.0f);
    float stepZ = extMin[2] + (extMax[2] - extMin[2]) / 4.0f;

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
FlowRenderer::_genSeedsRakeUniform( std::vector<flow::Particle>& seeds, 
                                    float timeVal ) const
{
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );
    VAssert( params );

    /* retrieve rake from params */
    auto rake = params->GetRake();
    VAssert( rake.size() == 6 );
    for( int i = 0; i < 3; i++ )
        VAssert( rake[i*2+1] >= rake[i*2] );

    /* retrieve seed numbers from params */
    auto rakeSeeds = params->GetRakeNumOfSeeds();
    VAssert( rakeSeeds.size() == 4 ); 
    for( int i = 0; i < 3; i++ )    // we only need first 3 values for unifrm seeds
        VAssert( rakeSeeds[i] > 0 );

    /* Create arrays that contain X, Y, and Z coordinates */
    float start[3], step[3];
    for( int i = 0; i < 3; i++ )    // for each of the X, Y, Z dimensions
    {
        if( rakeSeeds[i] == 1 )     // one seed in this dimension
        {
            start[i] = rake[i*2] + 0.5f * (rake[i*2+1] - rake[i*2]);
            step[i]  = 0.0f;
        }
        else                        // more than one seed in this dimension
        {
            start[i] = rake[i*2];
            step[i]  = (rake[i*2+1] - rake[i*2]) / float(rakeSeeds[i] - 1);
        }
    }

    /* Populate the list of seeds */
    seeds.resize( rakeSeeds[0] * rakeSeeds[1] * rakeSeeds[2] );
    int idx = 0;
    for( int k = 0; k < rakeSeeds[2]; k++ )
        for( int j = 0; j < rakeSeeds[1]; j++ )
            for( int i = 0; i < rakeSeeds[0]; i++ )
            {
                seeds[idx].location.x = start[0] + float(i) * step[0];
                seeds[idx].location.y = start[1] + float(j) * step[1];
                seeds[idx].location.z = start[2] + float(k) * step[2];
                seeds[idx].time       = timeVal;
                idx++;
            }

    return 0;
}


int
FlowRenderer::_genSeedsRakeRandom( std::vector<flow::Particle>& seeds, 
                                   float timeVal ) const
{
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );

    /* retrieve rake from params */
    auto rake = params->GetRake();
    VAssert( rake.size() == 6 );
    for( int i = 0; i < 3; i++ )
        VAssert( rake[i*2+1] >= rake[i*2] );

    /* retrieve random seed numbers from params */
    auto rakeSeeds = params->GetRakeNumOfSeeds();
    VAssert( rakeSeeds.size() == 4 ); 
    auto totalNumOfSeeds = rakeSeeds[3];    // We only need the 4th value for random seeds
    
    /* Create three uniform distributions in 3 dimensions */
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> distX( rake[0], rake[1] );
    std::uniform_real_distribution<float> distY( rake[2], rake[3] );
    std::uniform_real_distribution<float> distZ( rake[4], rake[5] );

    seeds.resize( totalNumOfSeeds );
    for( long i = 0; i < totalNumOfSeeds; i++ )
    {
        seeds[i].location.x = distX(gen);
        seeds[i].location.y = distY(gen);
        seeds[i].location.z = distZ(gen);
        seeds[i].time       = timeVal;
    }

    return 0;
}


int
FlowRenderer::_genSeedsRakeRandomBiased( std::vector<flow::Particle>& seeds, 
                                         float timeVal ) const
{
    FlowParams* params = dynamic_cast<FlowParams*>( GetActiveParams() );

    /* retrieve rake from params */
    auto rake = params->GetRake();
    VAssert( rake.size() == 6 );
    for( int i = 0; i < 3; i++ )
        VAssert( rake[i*2+1] >= rake[i*2] );
    std::vector<double> rakeExtMin, rakeExtMax;
    for( int i = 0; i < 3; i++ )
    {
        rakeExtMin.push_back( rake[ i*2   ] );
        rakeExtMax.push_back( rake[ i*2+1 ] );
    }

    /* retrieve bias variable and strength from params */
    auto biasVar   = params->GetRakeBiasVariable();
    auto biasStren = params->GetRakeBiasStrength();

    /* retrieve random seed numbers from params */
    auto rakeSeeds = params->GetRakeNumOfSeeds();
    VAssert( rakeSeeds.size() == 4 ); 
    auto totalNumOfSeeds = rakeSeeds[3];    // We only need the 4th value for random seeds
    
    /* request a grid representing the rake area */
    Grid* grid = _dataMgr->GetVariable( params->GetCurrentTimestep(),
                                        biasVar,
                                        params->GetRefinementLevel(),
                                        params->GetCompressionLevel(),
                                        rakeExtMin,
                                        rakeExtMax );
    if( grid == nullptr )
    {
        MyBase::SetErrMsg("Not able to get a grid!");
        return flow::GRID_ERROR;
    }

    /* Find the bias variable range of this rake area */
    float rakeMin = 0.0f, rakeMax = 0.0f; 
    auto itr    = grid->cbegin();
    auto endItr = grid->cend();
    if( grid->HasMissingData() )
    {
        float mv = grid->GetMissingValue();
        for( ; itr != endItr; ++itr )   // initialize rakeMin and rakeMax
        {
            if( *itr != mv )
            {
                rakeMin = std::abs(*itr);
                rakeMax = std::abs(*itr);
                break;
            }
        }
        for( ; itr != endItr; ++itr )   // find the min and max
        {
            if( *itr != mv )
            {
                float v = std::abs(*itr);
                rakeMin = rakeMin < v ? rakeMin : v;
                rakeMax = rakeMax > v ? rakeMax : v;
            }
        }
    }
    else
    {
        rakeMin = std::abs(*itr);
        rakeMax = std::abs(*itr);
        for( ; itr != endItr; ++itr )   // find the min and max
        {
            float v = std::abs(*itr);
            rakeMin = rakeMin < v ? rakeMin : v;
            rakeMax = rakeMax > v ? rakeMax : v;
        }
    }
    if( rakeMin == 0.0f && rakeMax == 0.0f )    // All are missing values in the rake
    {
        delete grid;
        return _genSeedsRakeRandom( seeds, timeVal );  // Use random seeds
    }

    /* Scale rakeMin and rakeMax so every possible value has some probability */
    float probMin = rakeMin - (rakeMax - rakeMin) * 0.05f;
    float probMax = rakeMax + (rakeMax - rakeMin) * 0.05f;
    float probLen = probMax - probMin;

    /* 
     * The bias strategy is:
     * we generate uniform random seeds in the rake, but for each seed, we decide 
     * if to discard it based on a probability function. This process terminates
     * when sufficient amount of seeds are generated.
     */
    
    /* Create three uniform distributions in 3 dimensions */
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> distX( rake[0], rake[1] );
    std::uniform_real_distribution<float> distY( rake[2], rake[3] );
    std::uniform_real_distribution<float> distZ( rake[4], rake[5] );
    /* Create a uniform distribution between 0.0 and 1.0 */
    std::uniform_real_distribution<float> zeroOne( 0.0f, 1.0f );

    int seedIdx = 0;
    seeds.resize( totalNumOfSeeds );
    while( seedIdx < totalNumOfSeeds )
    {
        const std::vector<double> cand{ distX(gen), distY(gen), distZ(gen) };   // generate a random seed
        float seedVal = grid->GetValue( cand );
        float prob    = std::abs(seedVal) / probLen;    // prob is a value between 0.0 and 1.0;
        prob          = std::pow( prob, biasStren );    // prob is scaled by a strength.
        if( zeroOne(gen) < prob )   // keep this seed
        {
            seeds[seedIdx].location.x = cand[0];
            seeds[seedIdx].location.y = cand[1];
            seeds[seedIdx].location.z = cand[2];
            seeds[seedIdx].time       = timeVal;
            seedIdx++;
        }
    }

    delete grid;
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

void
FlowRenderer::_updatePeriodicity( flow::Advection* advc )
{
    glm::vec3 minxyz, maxxyz;
    _velocityField.GetFirstStepVelocityIntersection( minxyz, maxxyz );

    if( _cache_periodic[0] )
        advc->SetXPeriodicity( true, minxyz.x, maxxyz.x );
    else
        advc->SetXPeriodicity( false );

    if( _cache_periodic[1] )
        advc->SetYPeriodicity( true, minxyz.y, maxxyz.y );
    else
        advc->SetYPeriodicity( false );

    if( _cache_periodic[2] )
        advc->SetZPeriodicity( true, minxyz.z, maxxyz.z );
    else
        advc->SetZPeriodicity( false );
}

#ifndef WIN32
double FlowRenderer::_getElapsedSeconds( const struct timeval* begin, 
                                         const struct timeval* end ) const
{
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec)/1000000.0);
}
#endif


