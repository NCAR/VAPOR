#pragma once

#include <vapor/Renderer.h>
#include <vapor/VolumeAlgorithm.h>

using std::string;
using std::vector;

namespace VAPoR {

class RENDER_API VolumeRenderer : public Renderer {
public:
    VolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);
    ~VolumeRenderer();

    static std::string GetClassType() { return ("Volume2"); }

protected:
    int  _initializeGL();
    int  _paintGL(bool fast);
    void _clearCache(){};

    void _loadData();
    void _loadTF();

    VolumeAlgorithm *algorithm;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int dataTexture;
    unsigned int LUTTexture;
    unsigned int depthTexture;

    struct Cache {
        string var;
        size_t ts;
        int    refinement;
        int    compression;

        MapperFunction *tf = nullptr;
        vector<double>  mapRange;

        string algorithmName;

        bool needsUpdate;
    } cache;
};

};    // namespace VAPoR
