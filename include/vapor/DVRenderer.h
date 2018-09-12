#ifndef DVRENDERER_H
#define DVRENDERER_H

#include "vapor/RayCaster.h"
#include "vapor/DVRParams.h"

namespace VAPoR {

class RENDER_API DVRenderer : public RayCaster {
public:
    DVRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);

    static std::string GetClassType() { return ("VolumeRenderer"); }

protected:
    void _loadShaders();
    void _3rdPassSpecialHandling(bool);
};

};    // namespace VAPoR

#endif
