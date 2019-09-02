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

    // A few variables to keep the current advection states
    int                 _cache_refinementLevel;
    int                 _cache_compressionLevel;
    float               _cache_velocityMltp;
    bool                _cache_isSteady;
    long                _cache_steadyNumOfSteps;
    size_t              _cache_currentTS;
    bool                _cache_periodic[3];
    float               _cache_rake[6];
    long                _cache_rakeNumOfSeeds[4];
    std::string         _cache_rakeBiasVariable;
    float               _cache_rakeBiasStrength;
    int                 _cache_numOfPastTimeSteps;

    // A few different modes to generate advection seeds:
    //   0 - programmatical
    //   1 - reading a list of seeds
    //   2 - uniformly generate 
    //   3 - randomly generate
    //   4 - randomly generate with bias
    long                _cache_seedGenMode;
    std::string         _cache_seedInputFilename;

    // A few different ways to integrate a flow line in steady mode:
    //   0 - forward
    //   1 - backward
    //   2 - bi-directional
    long                _cache_flowDirection;

    // This Advection class is only used in bi-directional advection mode 
    flow::Advection*    _2ndAdvection;

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
    int  _genSeedsXY(               std::vector<flow::Particle>& seeds, float timeVal ) const;
    int  _genSeedsRakeUniform(      std::vector<flow::Particle>& seeds, float timeVal ) const;
    int  _genSeedsRakeRandom(       std::vector<flow::Particle>& seeds, float timeVal ) const;
    int  _genSeedsRakeRandomBiased( std::vector<flow::Particle>& seeds, float timeVal ) const;

    int  _renderFromAnAdvection( const flow::Advection*, FlowParams*, bool fast );
    void _prepareColormap(       FlowParams* );
    void _particleHelper1( std::vector<float>& vec, const flow::Particle& p, bool singleColor ) const;
    int  _drawALineSeg( const float* buf, size_t numOfParts, bool singleColor ) const; 
    void _restoreGLState() const;

    int  _getAGrid( const FlowParams* params,           // Input
                    int               timestep,         // Input
                    std::string&      varName,          // Input
                    Grid**            gridpp  ) const;  // Output

    // Update values of _cache_* and _state_* member variables.
    void _updateFlowCacheAndStates( const FlowParams* );

    void _updatePeriodicity( flow::Advection* advc );

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
