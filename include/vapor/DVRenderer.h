#ifndef DVRENDERER_H
#define DVRENDERER_H

#include "vapor/RayCaster.h"
#include "vapor/DVRParams.h"

namespace VAPoR {

class RENDER_API DVRenderer : public RayCaster {
  public:
    DVRenderer(const ParamsMgr *pm,
               std::string &winName,
               std::string &dataSetName,
               std::string &instName,
               DataMgr *dataMgr);

    static std::string GetClassType() {
        return ("Volume");
    }

  protected:
    int _load3rdPassShaders();
    void _3rdPassSpecialHandling(bool fast, int castMode);
};

}; // namespace VAPoR

#endif
