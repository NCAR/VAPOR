#pragma once

#include <vapor/Renderer.h>
#include <vapor/Texture.h>
#include <vapor/Framebuffer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>

namespace VAPoR {

class RENDER_API VolumeRenderer : public Renderer {
public:
    VolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr);
    VolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string paramsType, std::string classType, std::string &instName, DataMgr *dataMgr);
    ~VolumeRenderer();

    static std::string GetClassType() { return ("Volume"); }

protected:
    int  _initializeGL();
    int  _paintGL(bool fast);
    void _clearCache(){};

    virtual void        _setShaderUniforms(const ShaderProgram *shader, const bool fast) const;
    void                _drawScreenQuad();
    void                _drawScreenQuadChuncked();
    void                _generateChunkedRenderMesh(const float chunks);
    bool                _wasTooSlowForFastRender() const;
    void                _computeNewFramebufferRatio();
    bool                _shouldUseChunkedRender(bool fast) const;
    virtual bool        _usingColorMapData() const;
    void                _saveOriginalViewport();
    void                _restoreOriginalViewport();
    void                _initializeFramebuffer(bool fast);
    int                 _renderFramebufferToDisplay();
    int                 _initializeAlgorithm();
    int                 _loadData();
    int                 _loadSecondaryData();
    virtual void        _getLUTFromTF(const MapperFunction *tf, float *LUT) const;
    void                _loadTF();
    glm::vec3           _getVolumeScales() const;
    void                _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool                _needToSetDefaultAlgorithm() const;

    unsigned int     _VAO = (int)NULL;
    unsigned int     _VBO = (int)NULL;
    unsigned int     _VAOChunked = (int)NULL;
    unsigned int     _VBOChunked = (int)NULL;
    VolumeAlgorithm *_algorithm = nullptr;
    Texture1D        _LUTTexture;
    Texture2D        _depthTexture;
    Framebuffer      _framebuffer;

    int                 _nChunks;
    double              _lastRenderTime;
    bool                _lastRenderWasFast;
    int                 _originalViewport[4];
    int                 _framebufferSize[2];
    float               _framebufferRatio;
    float               _previousFramebufferRatio;
    std::vector<double> _dataMinExt;
    std::vector<double> _dataMaxExt;

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

        std::vector<double> minExt;
        std::vector<double> maxExt;

        bool needsUpdate;
    } _cache;
};

};    // namespace VAPoR
