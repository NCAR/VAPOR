#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
#include "vapor/OceanField.h"
#include "vapor/Particle.h"
#include <iostream>

#define GLERROR     -10

using namespace VAPoR;

static RendererRegistrar<FlowRenderer> registrar(
    FlowRenderer::GetClassType(), FlowParams::GetClassType()
);

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
    _lineShader = nullptr;
    _velField   = nullptr;
}

// Destructor
FlowRenderer::~FlowRenderer()
{ 
    if( _velField )
    {
        delete _velField;
        _velField = nullptr;
    }
}


int
FlowRenderer::_initializeGL()
{
    ShaderProgram *shader   = nullptr;
    if( (shader = _glManager->shaderManager->GetShader("FlowLine")) )
        _lineShader      = shader;
    else
        return GLERROR;

    return 0;
}

int
FlowRenderer::_paintGL( bool fast )
{
    if( !_advec.IsReady() )
        _useOceanField();

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

