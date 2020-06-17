#pragma once

#include <vapor/Renderer.h>
#include <vapor/Texture.h>
#include <vapor/Framebuffer.h>
#include <vapor/VolumeAlgorithm.h>
#include <glm/fwd.hpp>
#include <vapor/OSPRay.h>

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
        return ("Volume");
    }

protected:
    int _initializeGL();
    int _paintGL(bool fast);
    void _clearCache() {};
    
    virtual std::string _getColorbarVariableName() const;
    
    virtual void _setShaderUniforms(const ShaderProgram *shader, const bool fast) const;
    void _drawScreenQuad();
    void _drawScreenQuadChuncked();
    void _generateChunkedRenderMesh(const float chunks);
    bool _wasTooSlowForFastRender() const;
    void _computeNewFramebufferRatio();
    bool _shouldUseChunkedRender(bool fast) const;
    virtual bool _usingColorMapData() const;
    void _saveOriginalViewport();
    void _restoreOriginalViewport();
    void _initializeFramebuffer(bool fast);
    int _renderFramebufferToDisplay();
    int _initializeAlgorithm();
    int _loadData();
    int _loadSecondaryData();
    virtual void _getLUTFromTF(const MapperFunction *tf, float *LUT) const;
    void _loadTF();
    void _loadTF(Texture1D *texture, MapperFunction *tf, MapperFunction **cacheTF);
    glm::vec3 _getVolumeScales() const;
    void _getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const;
    virtual std::string _getDefaultAlgorithmForGrid(const Grid *grid) const;
    bool _needToSetDefaultAlgorithm() const;
    
    unsigned int _VAO = (int)NULL;
    unsigned int _VBO = (int)NULL;
    unsigned int _VAOChunked = (int)NULL;
    unsigned int _VBOChunked = (int)NULL;
    VolumeAlgorithm *_algorithm = nullptr;
    Texture1D _LUTTexture;
    Texture1D _LUT2Texture;
    Texture2D _depthTexture;
    Framebuffer _framebuffer;
    
    int _nChunks;
    double _lastRenderTime;
    bool _lastRenderWasFast;
    int _originalViewport[4];
    int _originalFramebuffer;
    int _framebufferSize[2];
    float _framebufferRatio;
    float _previousFramebufferRatio;
    std::vector<double> _dataMinExt;
    std::vector<double> _dataMaxExt;
    
    struct Cache {
        std::string var = "";
        size_t ts = -1;
        int refinement;
        int compression;
        
        bool useColorMapVar = false;
        std::string colorMapVar = "";
        
        MapperFunction *tf = nullptr;
        std::vector<float> constantColor;
        MapperFunction *tf2 = nullptr;
        
        std::string algorithmName = "";

        std::vector<double> minExt;
        std::vector<double> maxExt;
        
        bool ospEnabled;
        int ospMaxCells;
        int ospTestCellId;
        bool ospPT;
        bool osp_force_regular;
        bool osp_test_volume;
        
        bool needsUpdate;
    } _cache;
    
    Texture2D _ospRenderTexture;
    
    OSPRenderer         _ospRenderer     = nullptr;
    OSPWorld            _ospWorld        = nullptr;
    OSPCamera           _ospCamera       = nullptr;
    OSPTransferFunction _ospTF           = nullptr;
    OSPInstance         _ospInstance     = nullptr;
    OSPVolumetricModel  _ospVolumeModel  = nullptr;
    OSPLight            _ospLightDistant = nullptr;
    
    bool _ospEnabled();
    int _ospInit();
    int _ospRender(bool fast);
    void _ospSetupRenderer(bool fast);
    void _ospSetupCamera();
    void _ospLoadTF();
    void _ospApplyTransform();
    int _ospLoadData(const Grid *grid);
    float _ospGuessSamplingRateScalar(const Grid *grid) const;
    OSPVolume _ospLoadVolumeRegular(const Grid *grid);
    OSPVolume _ospLoadVolumeStructured(const Grid *grid);
    OSPVolume _ospLoadVolumeUnstructured(const Grid *grid);
    OSPVolume _ospLoadVolumeTest(const Grid *grid);
    void _ospDelete();
};


};
