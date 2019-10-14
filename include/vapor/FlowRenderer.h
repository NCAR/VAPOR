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

class RENDER_API FlowRenderer final : public Renderer
{
public:

    FlowRenderer(  const ParamsMgr*  pm, 
                   std::string&      winName, 
                   std::string&      dataSetName,
                   std::string&      instName, 
                   DataMgr*          dataMgr);

    // Rule of five
    ~FlowRenderer();
    FlowRenderer( const FlowRenderer& )             = delete;
    FlowRenderer( const FlowRenderer&& )            = delete;
    FlowRenderer& operator=( const FlowRenderer& )  = delete;
    FlowRenderer& operator=( const FlowRenderer&& ) = delete;

    static std::string GetClassType()
    {
        return ("Flow");
    }
    
private:

    // Define two enums for this class use only
    enum class FlowStatus
    {
        SIMPLE_OUTOFDATE,   // When variable name or compression is out of date,
        TIME_STEP_OOD,      // Existing particles are good, but need to advect more
        UPTODATE            // Everything is up-to-date
    };
    // The following two enums are stored in params, so we use a native format
    // which is long in this case to avoid casting error.
    enum class SeedGenMode : long   
    {
        UNIFORM     = 0,
        RANDOM      = 1,
        RANDOM_BIAS = 2,
        LIST        = 3
    };
    enum class FlowDir : long
    {
        FORWARD     = 0,
        BACKWARD    = 1,
        BI_DIR      = 2
    };

    // C++ stuff: pure virtual functions from Renderer
    int  _initializeGL()        override;
    int  _paintGL( bool fast )  override;
    void _clearCache()          override {};

    // Member variables
    flow::Advection     _advection;
    flow::VaporField    _velocityField;
    flow::VaporField    _colorField;
    std::vector<float>  _colorMap;
    std::vector<double> _timestamps;
    float               _colorMapRange[3];   // min, max, and their diff

    bool                _advectionComplete          = false;
    bool                _coloringComplete           = false;

    // A few variables to keep the current advection states.
    // Some of them are initialized to be at an illegal state.
    int                 _cache_refinementLevel      = 0;;
    int                 _cache_compressionLevel     = 0;;
    float               _cache_velocityMltp         = 1.0;;
    bool                _cache_isSteady             = false;
    long                _cache_steadyNumOfSteps     = 0;
    size_t              _cache_currentTS            = 0;
    bool                _cache_periodic[3]          { false, false, false };
    float               _cache_rake[6]              { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
    long                _cache_rakeNumOfSeeds[4]    { 1, 1, 1, 1 };
    std::string         _cache_rakeBiasVariable;
    float               _cache_rakeBiasStrength     = 0.0f;
    int                 _cache_seedInjInterval      = 0;
    SeedGenMode         _cache_seedGenMode          = SeedGenMode::UNIFORM;
    std::string         _cache_seedInputFilename;
    FlowDir             _cache_flowDir              = FlowDir::FORWARD;
    FlowStatus          _velocityStatus             = FlowStatus::SIMPLE_OUTOFDATE;
    FlowStatus          _colorStatus                = FlowStatus::SIMPLE_OUTOFDATE;

    // This Advection class is only used in bi-directional advection mode 
    std::unique_ptr<flow::Advection> _2ndAdvection;


    // Member variables for OpenGL
    const  GLint        _colorMapTexOffset;
    ShaderProgram*      _shader             = nullptr;
    GLuint              _vertexArrayId      = 0; 
    GLuint              _vertexBufferId     = 0; 
    GLuint              _colorMapTexId      = 0;

    //
    // Member functions
    //
    void _printFlowStatus( const std::string& prefix, FlowStatus stat  ) const;
    int  _genSeedsRakeUniform(      std::vector<flow::Particle>& seeds ) const;
    int  _genSeedsRakeRandom(       std::vector<flow::Particle>& seeds ) const;
    int  _genSeedsRakeRandomBiased( std::vector<flow::Particle>& seeds ) const;

    int  _renderFromAnAdvection( const flow::Advection*, FlowParams*, bool fast );
    void _prepareColormap(       FlowParams* );
    void _particleHelper1( std::vector<float>& vec, const flow::Particle& p, bool singleColor ) const;
    int  _drawALineStrip( const float* buf, size_t numOfParts, bool singleColor ) const; 
    void _restoreGLState() const;

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

    void _dupSeedsNewTime( std::vector<flow::Particle>& seeds,
                           size_t  firstN,                      // First N particles to duplicate
                           float   newTime )   const;           // New time to assign to particles


#ifndef WIN32
    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;
#endif

};  // End of class FlowRenderer

};  // End of namespace VAPoR

#endif 
