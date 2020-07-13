#pragma once

#include <vapor/VolumeAlgorithm.h>
#include <vapor/Texture.h>
#include <vapor/OSPRay.h>

namespace VAPoR {
    
    //! \class VolumeOSPRay
    //! \ingroup Public_Render
    //!
    //! \brief OSPRay volume rendering adapter
    //!
    //! \author Stanislaw Jaroszynski
    //! \date July, 2020
    //!
    
    class VolumeOSPRay : public VolumeAlgorithm {
    public:
        VolumeOSPRay(GLManager *gl, VolumeRenderer *renderer);
        ~VolumeOSPRay();
        
        static std::string GetName() { return "OSPRay"; }
        static Type        GetType() { return Type::DVR; }
        virtual bool RequiresChunkedRendering() { return false; }
        
        virtual void SaveDepthBuffer(bool fast);
        virtual int Render(bool fast);
        virtual int LoadData(const Grid *grid);
        virtual int LoadSecondaryData(const Grid *grid) {return 0;}
        virtual void DeleteSecondaryData() {}
        virtual ShaderProgram *GetShader() const;
        virtual void SetUniforms(const ShaderProgram *shader) const;
        virtual float GuestimateFastModeSpeedupFactor() const;
        virtual void GetFinalBlendingMode(int *src, int *dst);
        
    protected:
        virtual bool _isIso() const { return false; }
        
    private:
        std::vector<size_t> _dataDimensions;
        std::vector<float> _depthData;
        std::vector<unsigned char> _backplateData;
        Texture2D _ospRenderTexture;
        Texture2D _ospWriteDepthTexture;
        struct {glm::vec3 min, max;} _clipBox = {glm::vec3(0), glm::vec3(1)};
        
        struct {
            bool usePT;
        } _cache;
        
        OSPRenderer         _ospRenderer     = nullptr;
        OSPWorld            _ospWorld        = nullptr;
        OSPCamera           _ospCamera       = nullptr;
        OSPTransferFunction _ospTF           = nullptr;
        OSPInstance         _ospInstance     = nullptr;
        OSPVolumetricModel  _ospVolumeModel  = nullptr;
        OSPLight            _ospLightDistant = nullptr;
        OSPGeometry         _ospIso          = nullptr;
        OSPGeometricModel   _ospIsoModel     = nullptr;
        
        void _setupRenderer(bool fast);
        void _setupCamera();
        void _setupIso();
        void _loadTF();
        void _applyTransform();
        void _copyDepth();
        void _copyBackplate();
        
        float _guessSamplingRateScalar(const Grid *grid) const;
        OSPVolume _loadVolumeRegular(const Grid *grid);
        OSPVolume _loadVolumeStructured(const Grid *grid);
        OSPVolume _loadVolumeUnstructured(const Grid *grid);
        OSPVolume _loadVolumeTest(const Grid *grid);
    };
    
    
    //! \class VolumeOSPRayIso
    //! \ingroup Public_Render
    //!
    //! \brief Regular grid isosurface rendering algorithm
    //!
    //! \author Stanislaw Jaroszynski
    //! \date Feburary, 2019
    //!
    //! Renders isosurfaces by ray tracing. This does the same CPU side tasks
    //! as the volume renderer but it provides different GLSL code.
    
    class VolumeOSPRayIso : public VolumeOSPRay {
    public:
        VolumeOSPRayIso(GLManager *gl, VolumeRenderer *renderer):VolumeOSPRay(gl, renderer){}
        static std::string GetName() { return "Iso OPSRay"; }
        static Type        GetType() { return Type::Iso; }
    protected:
        bool _isIso() const { return true; }
    };

}

