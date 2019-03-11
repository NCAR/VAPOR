#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/OceanField.h"
#include "vapor/SteadyVAPORField.h"
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
            _colorMapTexOffset ( 0 )
{ 
    // Initialize OpenGL states
    _shader         = nullptr;
    _vertexArrayId  = 0;
    _vertexBufferId = 0;
    _colorMapTexId  = 0;

    // Initialize advection states
    _cache_currentTS        =  0;
    _cache_refinementLevel  = -2;
    _cache_compressionLevel = -2;
    _cache_isSteady         = false;
    _state_scalarUpToDate   = false;
    _state_velocitiesUpToDate = false;
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

    int ready  = _advec.IsReady();
    if( ready != 0 )
    {
        _useSteadyVAPORField( params );
    }

    // Update color map texture
    _updateColormap( params );
    glActiveTexture( GL_TEXTURE0 + _colorMapTexOffset );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA32F,     _colorMap.size()/4,
                   0, GL_RGBA,       GL_FLOAT,       _colorMap.data() );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );

    size_t numOfStreams = _advec.GetNumberOfStreams();
    for( size_t i = 0; i < numOfStreams; i++ )
    {
        const auto& s = _advec.GetStreamAt( i );
        _drawAStream( s, params );
    }

    return 0;
}


int
FlowRenderer::_drawAStream( const std::vector<flow::Particle>& stream,
                            const FlowParams* params ) const
{
    size_t numOfPart = stream.size();
    float* posBuf    = new float[ 4 * numOfPart ];
    size_t offset    = 0;
    for( const auto& p : stream )
    {
        std::memcpy( posBuf + offset, glm::value_ptr(p.location), sizeof(glm::vec3) );
        offset += 3;
        posBuf[ offset++ ] = p.value;
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
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 4 * numOfPart, posBuf, GL_STREAM_DRAW );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_LINE_STRIP, 0, numOfPart );

    // Some OpenGL cleanup
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindTexture( GL_TEXTURE_1D,  _colorMapTexId );
    glBindVertexArray( 0 );

    delete[] posBuf;

    return 0;
}

void
FlowRenderer::_updateFlowStates( const FlowParams* params )
{
    if( _cache_currentTS != params->GetCurrentTimestep() )
    {
        _cache_currentTS = params->GetCurrentTimestep();
        if( _cache_isSteady )   // current time step only matters with steady flow
        {
            _state_velocitiesUpToDate = false;
            _state_scalarUpToDate = false;
        }
    }
    if( _cache_refinementLevel != params->GetRefinementLevel() )
    {
        _cache_refinementLevel = params->GetRefinementLevel();
        _state_velocitiesUpToDate = false;
        _state_scalarUpToDate = false;
    }
    if( _cache_compressionLevel != params->GetCompressionLevel() )
    {
        _cache_compressionLevel = params->GetCompressionLevel();
        _state_velocitiesUpToDate = false;
        _state_scalarUpToDate = false;
    }
    if( _cache_isSteady != params->GetIsSteady() )
    {
        _state_velocitiesUpToDate = false;
        _state_scalarUpToDate = false;
    }
}

#if 0
void
FlowRenderer::_useOceanField()
{
    flow::OceanField* field = new flow::OceanField();
    _advec.UseField( field );
    _advec.SetBaseStepSize( 0.1f );

    int numOfSeeds = 5, numOfSteps = 100;
    std::vector<flow::Particle> seeds( numOfSeeds );
    seeds[0].location = glm::vec3( 0.65f, 0.65f, 0.1f );
    seeds[1].location = glm::vec3( 0.3f, 0.3f, 0.1f );
    for( int i = 2; i < numOfSeeds; i++ )
        seeds[i].location = glm::vec3( float(i + 1) / float(numOfSeeds + 1), 0.0f, 0.0f );
    _advec.UseSeedParticles( seeds );
    for( int i = 0; i < numOfSteps; i++ )
        _advec.Advect( flow::Advection::RK4 );
}
#endif

int
FlowRenderer::_useSteadyVAPORField( const FlowParams* params )
{
    // Step 1: retrieve variable names from the params class
    std::vector<std::string> velVars = params->GetFieldVariableNames();
    assert( velVars.size() == 3 );  // need to have three components
    for( auto& s : velVars )
    {
        if( s.empty() )
        {
            MyBase::SetErrMsg("Missing velocity field");
            return flow::GRID_ERROR;
        }
    }

    // Step 2: use these variable names to get data grids
    Grid *gridU, *gridV, *gridW;
    int currentTS = params->GetCurrentTimestep();
    int rv  = _getAGrid( params, currentTS, velVars[0], &gridU );
    if( rv != 0 )   return rv;
    rv      = _getAGrid( params, currentTS, velVars[1], &gridV );
    if( rv != 0 )   return rv;
    rv      = _getAGrid( params, currentTS, velVars[2], &gridW );
    if( rv != 0 )   return rv;

    // Step 3: repeat step 2 for the color mapping variable
    Grid* scalarP    = nullptr;
    bool singleColor = params->UseSingleColor();
    if( !singleColor )
    {
        /* std::string scalarVar = params->GetColorMapVariableName();
        if( !scalarVar.empty() )
        {
            rv      = _getAGrid( params, currentTS, scalarVar, &scalarP );
            if( rv != 0 )   return rv;
        } */
    }
    
    // Step 4: create a SteadyVAPORField using these grids, and ask Advection to use it!
    flow::SteadyVAPORField* field = new flow::SteadyVAPORField();
    field->UseVelocities( gridU, gridV, gridW );
    //if( !singleColor )
    //    field->UseScalar( scalarP );
    
    // Plant seeds
    std::vector<flow::Particle> seeds;
    _genSeedsXY( seeds );
    _advec.UseSeedParticles( seeds );
    _advec.UseField( field );

    // Do some advection
    int numOfSteps = 200;
    for( int i = 0; i < numOfSteps; i++ )
        _advec.Advect( flow::Advection::RK4 );
    
    return 0;
}

int
FlowRenderer::_genSeedsXY( std::vector<flow::Particle>& seeds ) const
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
FlowRenderer::_updateColormap( FlowParams* params )
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
}

#ifndef WIN32
double FlowRenderer::_getElapsedSeconds( const struct timeval* begin, 
                                         const struct timeval* end ) const
{
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec)/1000000.0);
}
#endif

