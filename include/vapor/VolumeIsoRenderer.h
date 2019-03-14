#pragma once

#include <vapor/Renderer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

using std::string;
using std::vector;

namespace VAPoR {

class RENDER_API VolumeIsoRenderer : public Renderer {
  public:
    VolumeIsoRenderer(const ParamsMgr *pm,
                      std::string &winName,
                      std::string &dataSetName,
                      std::string &instName,
                      DataMgr *dataMgr);
    ~VolumeIsoRenderer();

    static std::string GetClassType() {
        return ("VolumeIso");
    }

  protected:
    int _initializeGL();
    int _paintGL(bool fast);
    void _clearCache(){};

    int _loadData();
    void _loadTF();
    glm::vec3 _getVolumeScales() const;
    void _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;

    VolumeAlgorithm *algorithm;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int LUTTexture;
    unsigned int depthTexture;

    struct Cache {
        string var;
        size_t ts;
        int refinement;
        int compression;

        MapperFunction *tf = nullptr;
        vector<double> mapRange;

        string algorithmName;

        bool needsUpdate;
    } cache;
};

}; // namespace VAPoR
