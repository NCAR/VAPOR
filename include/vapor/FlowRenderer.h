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
    
protected:
    virtual std::string _getColorbarVariableName() const override;
    
private:

    // Define two enums for this class use only
    enum class FlowStatus
    {
        SIMPLE_OUTOFDATE,   // When variable name or compression is out of date,
        TIME_STEP_OOD,      // Existing particles are good, but need to advect more
        UPTODATE            // Everything is up-to-date
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
    float               _cache_velocityMltp         = 1.0f;
    bool                _cache_isSteady             = false;
    long                _cache_steadyNumOfSteps     = 0;
    size_t              _cache_currentTS            = 0;
    std::vector<bool>   _cache_periodic             { false, false, false };
    std::vector<float>  _cache_rake                 { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
    std::vector<long>   _cache_gridNumOfSeeds       { 5, 5, 5};
    long                _cache_randNumOfSeeds       = 5;
    int                 _cache_seedInjInterval      = 0;
    float               _cache_rakeBiasStrength     = 0.0f;
    double              _cache_deltaT               = 0.05;
    FlowSeedMode        _cache_seedGenMode          = FlowSeedMode::UNIFORM;
    FlowDir             _cache_flowDir              = FlowDir::FORWARD;
    FlowStatus          _velocityStatus             = FlowStatus::SIMPLE_OUTOFDATE;
    FlowStatus          _colorStatus                = FlowStatus::SIMPLE_OUTOFDATE;
    FlowStatus          _renderStatus               = FlowStatus::SIMPLE_OUTOFDATE;
    std::string         _cache_rakeBiasVariable;
    std::string         _cache_seedInputFilename;

    // This Advection class is only used in bi-directional advection mode 
    std::unique_ptr<flow::Advection> _2ndAdvection;


    // Member variables for OpenGL
    const  GLint        _colorMapTexOffset;
    ShaderProgram*      _shader             = nullptr;
    GLuint              _vertexArrayId      = 0; 
    GLuint              _vertexBufferId     = 0; 
    GLuint              _colorMapTexId      = 0;
    
    unsigned int _VAO = 0;
    unsigned int _VBO = 0;
    vector<int> _streamSizes;

    //
    // Member functions
    //
    void _printFlowStatus( const std::string& prefix, FlowStatus stat  ) const;
    int  _genSeedsRakeUniform(      std::vector<flow::Particle>& seeds ) const;
    int  _genSeedsRakeRandom(       std::vector<flow::Particle>& seeds ) const;
    int  _genSeedsRakeRandomBiased( std::vector<flow::Particle>& seeds ) const;
    int  _genSeedsFromList(         std::vector<flow::Particle>& seeds ) const;

    int  _renderFromAnAdvectionLegacy( const flow::Advection*, FlowParams*, bool fast );
    int  _renderAdvection(const flow::Advection* adv);
    int  _renderAdvectionHelper(bool renderDirection = false);
    void _prepareColormap(       FlowParams* );
    void _particleHelper1( std::vector<float>& vec, const flow::Particle& p, bool singleColor ) const;
    int  _drawALineStrip( const float* buf, size_t numOfParts, bool singleColor ) const;
    void _restoreGLState() const;
    glm::vec3 _getScales();

    // Update values of _cache_* and _state_* member variables.
    int _updateFlowCacheAndStates( const FlowParams* );

    int _updateAdvectionPeriodicity( flow::Advection* advc );

    int _outputFlowLines();

    void _dupSeedsNewTime( std::vector<flow::Particle>& seeds,
                           size_t  firstN,                      // First N particles to duplicate
                           double  newTime )   const;           // New time to assign to particles


#ifndef WIN32
    double _getElapsedSeconds( const struct timeval* begin, const struct timeval* end ) const;
#endif

};  // End of class FlowRenderer

};  // End of namespace VAPoR

#endif 
