#include "vapor/VolumeIsoRenderer.h"
#include <vapor/VolumeIsoParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

#include <vapor/VolumeRegular.h>
#include <vapor/VolumeCellTraversal.h>


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using namespace VAPoR;

static RendererRegistrar<VolumeIsoRenderer> registrar( VolumeIsoRenderer::GetClassType(),
                                                VolumeIsoParams::GetClassType() );


VolumeIsoRenderer::VolumeIsoRenderer( const ParamsMgr*    pm,
                        std::string&        winName,
                        std::string&        dataSetName,
                        std::string&        instName,
                        DataMgr*            dataMgr )
          : VolumeRenderer(  pm,
                        winName,
                        dataSetName,
                        VolumeIsoParams::GetClassType(),
                        VolumeIsoRenderer::GetClassType(),
                        instName,
                        dataMgr )
{
}

VolumeIsoRenderer::~VolumeIsoRenderer()
{
}

bool VolumeIsoRenderer::_usingColorMapData() const
{
    return !GetActiveParams()->UseSingleColor();
}

std::string VolumeIsoRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    const RegularGrid* regular = dynamic_cast<const RegularGrid*>(grid);
    if (regular)
        return VolumeRegularIso::GetName();
    return VolumeCellTraversalIso::GetName();
}

