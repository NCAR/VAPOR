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

void IsoSurfaceRenderer::_3rdPassSpecialHandling( bool fast, int castingMode )
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
}

