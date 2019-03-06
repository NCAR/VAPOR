#ifndef FLOWRENDERER_H
#define FLOWRENDERER_H

#include "vapor/glutil.h"
#ifndef WIN32
#include <sys/time.h>
#endif

#include "vapor/Renderer.h"
#include "vapor/FlowParams.h"
#include "vapor/GLManager.h"
#include "vapor/Advection.h"
#include "vapor/VelocityField.h"

#include <glm/glm.hpp>

namespace VAPoR 
{

class RENDER_API FlowRenderer : public Renderer
{
public:

    FlowRenderer(  const ParamsMgr*  pm, 
                std::string&      winName, 
                std::string&      dataSetName,
                std::string&      instName, 
                DataMgr*          dataMgr);

    virtual ~FlowRenderer();

    static std::string GetClassType()
    {
        return ("Flow");
    }

protected:
    // C++ stuff: pure virtual functions from Renderer
    int  _initializeGL();
    int  _paintGL( bool fast );
    void _clearCache() {};

    flow::Advection         _advec;
    flow::VelocityField*    _velField;

    // OpenGL stuff: shaders
    ShaderProgram*      _shader;
    GLuint              _vertexArrayId; 
    GLuint              _vertexBufferId; 


    void _useOceanField();
    void _useSteadyVAPORField();
    int  _drawAStream( const std::vector<flow::Particle>& s ) const;

    int  _getAGrid( const FlowParams* params,           // Input
                    int               timestep,         // Input
                    std::string&      varName,          // Input
                    Grid**            gridpp  ) const;  // Output

#ifndef WIN32
    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;
#endif

};  // End of class FlowRenderer

};  // End of namespace VAPoR

#endif 
