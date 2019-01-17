#include "vapor/IsoSurfaceRenderer.h"

#define GLERROR     -5

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<IsoSurfaceRenderer> registrar( IsoSurfaceRenderer::GetClassType(), 
                                                        IsoSurfaceParams::GetClassType() );


IsoSurfaceRenderer::IsoSurfaceRenderer( const ParamsMgr*    pm,
                                        std::string&        winName,
                                        std::string&        dataSetName,
                                        std::string&        instName,
                                        DataMgr*            dataMgr )
                  : RayCaster(  pm,
                                winName,
                                dataSetName,
                                IsoSurfaceParams::GetClassType(),
                                IsoSurfaceRenderer::GetClassType(),
                                instName,
                                dataMgr )
{ }

int IsoSurfaceRenderer::_load3rdPassShaders()
{
    ShaderProgram *shader  = nullptr;
    if( (shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode1")) )
        _3rdPassMode1Shader  = shader;
    else
        return GLERROR;

    if( (shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode2")) )
        _3rdPassMode2Shader  = shader;
    else 
        return GLERROR;

    return 0;   // Success
}

void IsoSurfaceRenderer::_3rdPassSpecialHandling( bool fast, int castingMode, bool use2ndVar )
{
    IsoSurfaceParams*   params    = dynamic_cast<IsoSurfaceParams*>( GetActiveParams() );
    std::vector<double> isoValues = params->GetIsoValues();
    std::vector<bool>   isoFlags  = params->GetEnabledIsoValueFlags();

    // Special handling for IsoSurface: pass in *normalized* iso values.
    std::vector<float>  validValues;
    for( int i = 0; i < isoFlags.size(); i++ )
    {
        if( isoFlags[i] )
            validValues.push_back( float(isoValues[i]) );
    }
    int numOfIsoValues  = (int)validValues.size();
    for( int i = numOfIsoValues; i < 4; i++ )
        validValues.push_back( 0.0f );
    
    _3rdPassShader->SetUniform("numOfIsoValues", numOfIsoValues);
    _3rdPassShader->SetUniformArray("isoValues", 4, validValues.data());

    _3rdPassShader->SetUniform( "use2ndVar", int(use2ndVar) );
    if( use2ndVar )
    {
        glActiveTexture(  GL_TEXTURE0 +             _2ndVarDataTexOffset );
        glBindTexture(    GL_TEXTURE_3D,            _2ndVarDataTexId );
        _3rdPassShader->SetUniform("secondVarData", _2ndVarDataTexOffset );

        glActiveTexture(  GL_TEXTURE0 +             _2ndVarMaskTexOffset );
        glBindTexture(    GL_TEXTURE_3D,            _2ndVarMaskTexId );
        _3rdPassShader->SetUniform("secondVarMask", _2ndVarMaskTexOffset );
        
        glBindTexture(    GL_TEXTURE_3D,        0 );
    }
}

void IsoSurfaceRenderer::_colormapSpecialHandling( RayCasterParams* params )
{
    VAPoR::MapperFunction* mapperFunc = params->RenderParams::GetMapperFunc 
                                        ( params->GetColorMapVariableName() );
    mapperFunc->makeLut( _colorMap );
    assert( _colorMap.size()  % 4 == 0 );
    std::vector<double> range = mapperFunc->getMinMaxMapValue();
    _colorMapRange[0]         = float(range[0]);
    _colorMapRange[1]         = float(range[1]);
    _colorMapRange[2]         = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ?
                                (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f ;
}

bool IsoSurfaceRenderer::_use2ndVariable( const RayCasterParams* params ) const
{
    if( params->UseSingleColor() )
        return false;
    else
        return true;
}
