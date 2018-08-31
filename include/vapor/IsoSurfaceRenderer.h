#ifndef ISOSURFACERENDERER_H
#define ISOSURFACERENDERER_H

#include "vapor/RayCaster.h"
#include "vapor/IsoSurfaceParams.h"

namespace VAPoR {

class RENDER_API IsoSurfaceRenderer : public RayCaster {
public:
    IsoSurfaceRnderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);

    static std::string GetClassType() { return ("IsoSurface"); }

protected:
    void _loadShaders();
};

};    // namespace VAPoR

#endif
