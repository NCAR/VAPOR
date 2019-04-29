#pragma once

#include <vapor/Renderer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

namespace VAPoR
{

class RENDER_API VolumeRenderer : public Renderer
{
public:
    VolumeRenderer(const ParamsMgr* pm,
                std::string&        winName,
                std::string&        dataSetName,
                std::string&        instName,
                DataMgr*            dataMgr );
    VolumeRenderer(const ParamsMgr* pm,
                   std::string&     winName,
                   std::string&     dataSetName,
                   std::string      paramsType,
                   std::string      classType,
                   std::string&     instName,
                   DataMgr*         dataMgr );
    ~VolumeRenderer();

    static std::string GetClassType()
    {
        return ("NEW_Volume");
    }

protected:
    int _initializeGL();
    int _paintGL(bool fast);
    void _clearCache() {};
    
    void _generateChunkedRenderMesh(const float chunks);
    int _loadData();
    virtual bool _usingColorMapData() const;
    int _loadSecondaryData();
    void _loadTF();
    glm::vec3 _getVolumeScales() const;
    void _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool _needToSetDefaultAlgorithm() const;
    
    VolumeAlgorithm *_algorithm;
    
    unsigned int _VAO = 0;
    unsigned int _VBO = 0;
    unsigned int _VAOChunked = 0;
    unsigned int _VBOChunked = 0;
    unsigned int _framebuffer = 0;
    unsigned int _framebufferTexture = 0;
    unsigned int _framebufferDepthTexture = 0;
    unsigned int _LUTTexture;
    unsigned int _depthTexture;
    
    int _nChunks = 64;
    double _lastRenderTime;
    bool _lastRenderWasFast;
    int _framebufferSize[2];
    float _framebufferRatio;
    
    struct Cache {
        std::string var = "";
        size_t ts = -1;
        int refinement;
        int compression;
        
        bool useColorMapVar = false;
        std::string colorMapVar = "";
        
        MapperFunction *tf = nullptr;
        std::vector<double> mapRange;
        std::vector<float> constantColor;
        
        std::string algorithmName = "";
        
        bool needsUpdate;
    } _cache;
};


};
