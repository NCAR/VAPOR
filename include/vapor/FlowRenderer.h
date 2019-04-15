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
#include "vapor/VaporField.h"

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

    enum class FlowStatus
    {
        SIMPLE_OUTOFDATE,   // When variable name or compression is out of date,
        TIME_STEP_OOD,      // Existing particles are good, but need to advect more
        UPTODATE            // Everything is up-to-date
    };

protected:
    // C++ stuff: pure virtual functions from Renderer
    int  _initializeGL();
    int  _paintGL( bool fast );
    void _clearCache() {};

    // Member variables
    flow::Advection     _advection;
    flow::VaporField    _velocityField;
    flow::VaporField    _colorField;
    std::vector<float>  _colorMap;
    std::vector<double> _timestamps;
    float               _colorMapRange[3];   // min, max, and their diff
    bool                _advectionComplete;
    bool                _coloringComplete;
    unsigned int        _steadyTotalSteps;

    // A few variables to keep the current advection states
    int                 _cache_refinementLevel;
    int                 _cache_compressionLevel;
    float               _cache_velocityMltp;
    bool                _cache_isSteady;
    int                 _cache_steadyNumOfSteps;
    size_t              _cache_currentTS;

    FlowStatus          _velocityStatus;
    FlowStatus          _colorStatus;

    // Member variables for OpenGL
    const  GLint        _colorMapTexOffset;
    ShaderProgram*      _shader;
    GLuint              _vertexArrayId; 
    GLuint              _vertexBufferId; 
    GLuint              _colorMapTexId;

    //
    // Member functions
    //
    int  _genSeedsXY( std::vector<flow::Particle>& seeds, float timeVal ) const;

    int  _purePaint( FlowParams*, bool fast ) ;
    void _prepareColormap(        FlowParams* );
    int  _drawAStreamAsLines(     const std::vector<flow::Particle>&,
                                  const FlowParams* ) const;
    int  _drawAStreamAsTubes(     const std::vector<flow::Particle>&,
                                  const FlowParams* ) const;
    void _restoreGLState() const;

    int  _getAGrid( const FlowParams* params,           // Input
                    int               timestep,         // Input
                    std::string&      varName,          // Input
                    Grid**            gridpp  ) const;  // Output

    // Update values of _cache_* and _state_* member variables.
    void _updateFlowCacheAndStates( const FlowParams* );

    // A function to populate particle properties.
    // If useAsColor == true, then this calculated property will be stored in a field
    //    of a Particle that will be used for coloring the particle.
    // If useAsColor == false, then this property is simply kept by the Particle without
    //    any impact to the visualization.
    int  _populateParticleProperties( const std::string& varname,
                                      const FlowParams*  params,
                                      bool  useAsColor );


    // Color the last particle in a stream
    int  _colorLastParticle();


#ifndef WIN32
    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;
#endif

};  // End of class FlowRenderer

};  // End of namespace VAPoR

#endif 
