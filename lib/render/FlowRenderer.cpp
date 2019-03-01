#include "vapor/glutil.h"
#include "vapor/FlowRenderer.h"
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
}

// Destructor
FlowRenderer::~FlowRenderer()
{ }


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
    return 0;
}


#ifndef WIN32
double FlowRenderer::_getElapsedSeconds( const struct timeval* begin, 
                                         const struct timeval* end ) const
{
    return (end->tv_sec - begin->tv_sec) + ((end->tv_usec - begin->tv_usec)/1000000.0);
}
#endif

