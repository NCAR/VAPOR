#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/OceanField.h"
#include "vapor/Particle.h"
#include <iostream>
#include <cstring>

#define GLERROR     -10

using namespace VAPoR;

static RendererRegistrar<FlowRenderer> registrar(
    FlowRenderer::GetClassType(), FlowParams::GetClassType() );

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
                      dataMgr )
{ 
    _shader = nullptr;
    _velField   = nullptr;

    _vertexArrayId = 0;
    _vertexBufferId = 0;
}

// Destructor
FlowRenderer::~FlowRenderer()
{ 
    if( _velField )
    {
        delete _velField;
        _velField = nullptr;
    }

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
}


int
FlowRenderer::_initializeGL()
{
    ShaderProgram *shader   = nullptr;
    if( (shader = _glManager->shaderManager->GetShader("FlowLine")) )
        _shader      = shader;
    else
        return GLERROR;

    /* Create Vertex Array Object (VAO) */
    glGenVertexArrays( 1, &_vertexArrayId );
    glGenBuffers(      1, &_vertexBufferId );



    return 0;
}

int
FlowRenderer::_paintGL( bool fast )
{
    if( !_advec.IsReady() )
        _useOceanField();

/*
    size_t numOfStreams = _advec.GetNumberOfStreams();
    for( size_t i = 0; i < numOfStreams; i++ )
    {
        const auto& s = _advec.GetStreamAt( i );
        _drawAStream( s );
    }
*/

    glm::mat4 modelview  = _glManager->matrixManager->GetModelViewMatrix();
    glm::mat4 projection = _glManager->matrixManager->GetProjectionMatrix();
    _shader->Bind();
    _shader->SetUniform("MV", modelview);
    _shader->SetUniform("Projection", projection);
    float vert[] = {0.0, 0.0, 0.0, 10.0, 10.0, 10.0};
    glBindVertexArray( _vertexArrayId );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, _vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STREAM_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_LINES, 0, 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );

    return 0;
}


int
FlowRenderer::_drawAStream( const std::vector<flow::Particle>& stream ) const
{
    size_t numOfPart = stream.size();
    float* posBuf    = new float[ 3 * numOfPart ];
    size_t offset    = 0;
    for( const auto& p : stream )
    {
        std::memcpy( posBuf + offset, glm::value_ptr(p.location), sizeof(glm::vec3) );
        offset += 3;
    }

    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, _vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * numOfPart, posBuf, GL_STREAM_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_LINE_STRIP, 0, numOfPart );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );

    delete[] posBuf;

    return 0;
}


void
FlowRenderer::_useOceanField()
{
    if( _velField )
    {
        delete _velField;
        _velField = nullptr;
    }
    _velField = new flow::OceanField();
    _advec.UseVelocityField( _velField );
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


#ifndef WIN32
double FlowRenderer::_getElapsedSeconds( const struct timeval* begin, 
                                         const struct timeval* end ) const
{
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec)/1000000.0);
}
#endif

