#pragma once

#include <vapor/Renderer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

namespace VAPoR {

class RENDER_API VolumeRenderer : public Renderer {
public:
    VolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);
    VolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr);
    ~VolumeRenderer();

    static std::string GetClassType() { return ("NEW_Volume"); }

protected:
    int  _initializeGL();
    int  _paintGL(bool fast);
    void _clearCache(){};

    int                 _loadData();
    virtual bool        _usingColorMapData() const;
    int                 _loadSecondaryData();
    void                _loadTF();
    glm::vec3           _getVolumeScales() const;
    void                _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool                _needToSetDefaultAlgorithm() const;

    VolumeAlgorithm *algorithm;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int VAO2 = 0;
    unsigned int VBO2 = 0;
    unsigned int LUTTexture;
    unsigned int depthTexture;

    double lastRenderTime;

    struct Cache {
        std::string var = "";
        size_t      ts = -1;
        int         refinement;
        int         compression;

        bool        useColorMapVar = false;
        std::string colorMapVar = "";

        MapperFunction *    tf = nullptr;
        std::vector<double> mapRange;
        std::vector<float>  constantColor;

        std::string algorithmName = "";

        bool needsUpdate;
    } cache;
};

};    // namespace VAPoR
