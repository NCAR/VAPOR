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

    // Member variables
    flow::Advection     _advec;
    std::vector<float>  _colorMap;
    float               _colorMapRange[3];   // min, max, and their diff

    // A few variables to keep the current advection states
    size_t              _cache_currentTS;
    int                 _cache_refinementLevel;
    int                 _cache_compressionLevel;
    bool                _cache_isSteady;
    bool                _state_velocitiesUpToDate;
    bool                _state_scalarUpToDate;

    // Member variables for OpenGL
    const  GLint        _colorMapTexOffset;
    ShaderProgram*      _shader;
    GLuint              _vertexArrayId; 
    GLuint              _vertexBufferId; 
    GLuint              _colorMapTexId;

    // Member functions
    //void _useOceanField();
    int  _useSteadyVAPORField( const FlowParams* );

    int  _drawAStream( const std::vector<flow::Particle>&,
                       const FlowParams*      ) const;

    int  _getAGrid( const FlowParams* params,           // Input
                    int               timestep,         // Input
                    std::string&      varName,          // Input
                    Grid**            gridpp  ) const;  // Output

    int  _genSeedsXY( std::vector<flow::Particle>& seeds ) const;

    void _updateColormap( FlowParams* );

    // Update values of _cache_* and _state_* member variables.
    void _updateFlowStates( const FlowParams* );


#ifndef WIN32
    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;
#endif

};  // End of class FlowRenderer

};  // End of namespace VAPoR

#endif 
