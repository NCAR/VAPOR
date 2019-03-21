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
#include "vapor/ScalarField.h"

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

    enum class UpdateStatus
    {
        SIMPLE_OUTOFDATE,   // When variable name or compression is out of date,
        UPTODATE            // Everything is up-to-date
    };

protected:
    // C++ stuff: pure virtual functions from Renderer
    int  _initializeGL();
    int  _paintGL( bool fast );
    void _clearCache() {};

    // Member variables
    flow::Advection     _advection;
    flow::ScalarField*  _colorField;
    std::vector<float>  _colorMap;
    float               _colorMapRange[3];   // min, max, and their diff
    bool                _advectionComplete;

    // A few variables to keep the current advection states
    size_t              _cache_currentTS;
    float               _cache_time;        // Actual time value at current time step
    int                 _cache_refinementLevel;
    int                 _cache_compressionLevel;
    bool                _cache_isSteady;
    UpdateStatus        _velocityStatus;
    UpdateStatus        _scalarStatus;

    // Member variables for OpenGL
    const  GLint        _colorMapTexOffset;
    ShaderProgram*      _shader;
    GLuint              _vertexArrayId; 
    GLuint              _vertexBufferId; 
    GLuint              _colorMapTexId;

    //
    // Member functions
    //
    int  _useSteadyVAPORField( const FlowParams* );
    int  _useSteadyColorField( const FlowParams* );

    int  _useUnsteadyVAPORField( const FlowParams* );
    int  _useUnsteadyColorField( const FlowParams* );
    //int  _addTimestepUnsteadyVAPORField( const FlowParams* );

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
