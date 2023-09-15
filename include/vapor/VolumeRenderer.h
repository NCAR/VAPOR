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

    virtual std::string _getColorbarVariableName() const;

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
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool                _needToSetDefaultAlgorithm() const;

    unsigned int     _VAO = (int)NULL;
    unsigned int     _VBO = (int)NULL;
    unsigned int     _VAOChunked = (int)NULL;
    unsigned int     _VBOChunked = (int)NULL;
    VolumeAlgorithm *_algorithm = nullptr;
    Framebuffer      _framebuffer;

    int                 _nChunks;
    double              _lastRenderTime;
    bool                _lastRenderWasFast;
    int                 _originalViewport[4];
    int                 _originalFramebuffer;
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

        MapperFunction *   tf = nullptr;
        std::vector<float> constantColor;
        MapperFunction *   tf2 = nullptr;

        std::string algorithmName = "";

        std::vector<double> minExt;
        std::vector<double> maxExt;

        int  ospMaxCells;
        int  ospTestCellId;
        bool ospPT;
        bool osp_force_regular;
        bool osp_test_volume;
        bool osp_decompose;
        bool osp_enable_clipping;

        bool needsUpdate;
    } _cache;

    friend class VolumeAlgorithm;
};

};    // namespace VAPoR
