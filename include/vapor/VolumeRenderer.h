#pragma once

#include <vapor/Renderer.h>
#include <vapor/Texture.h>
#include <vapor/Framebuffer.h>
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
    
    virtual void _setShaderUniforms(const ShaderProgram *shader, const bool fast) const;
    void _drawScreenQuad();
    void _drawScreenQuadChuncked();
    void _generateChunkedRenderMesh(const float chunks);
    bool _wasTooSlowForFastRender() const;
    void _computeNewFramebufferRatio();
    bool _shouldUseChunkedRender() const;
    virtual bool _usingColorMapData() const;
    void _saveOriginalViewport();
    void _restoreOriginalViewport();
    void _initializeFramebuffer(bool fast);
    int _renderFramebufferToDisplay();
    void _initializeAlgorithm();
    int _loadData();
    int _loadSecondaryData();
    void _loadTF();
    glm::vec3 _getVolumeScales() const;
    void _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool _needToSetDefaultAlgorithm() const;
    
    unsigned int _VAO = NULL;
    unsigned int _VBO = NULL;
    unsigned int _VAOChunked = NULL;
    unsigned int _VBOChunked = NULL;
    VolumeAlgorithm *_algorithm = NULL;
    Texture1D _LUTTexture;
    Texture2D _depthTexture;
    Framebuffer _framebuffer;
    
    int _nChunks;
    double _lastRenderTime;
    bool _lastRenderWasFast;
    int _originalViewport[4];
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
