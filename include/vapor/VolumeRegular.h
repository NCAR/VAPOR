#pragma once

#include <vapor/VolumeAlgorithm.h>
#include <vapor/Texture.h>

namespace VAPoR {
    
    class VolumeRegular : public VolumeAlgorithm {
    public:
        VolumeRegular(GLManager *gl);
        ~VolumeRegular();
        
        static std::string GetName() { return "Regular"; }
        static Type        GetType() { return Type::DVR; }
        virtual bool IsSlow() { return false; }
        
        virtual int LoadData(const Grid *grid);
        virtual int LoadSecondaryData(const Grid *grid);
        virtual void DeleteSecondaryData();
        virtual ShaderProgram *GetShader() const;
        virtual void SetUniforms(const ShaderProgram *shader) const;
        virtual float GuestimateFastModeSpeedupFactor() const;
        
    protected:
        Texture3D _data;
        Texture3D _missing;
        bool _hasMissingData;
        
        std::vector<size_t> _dataDimensions;
        
        bool _hasSecondData;
        Texture3D _data2;
        Texture3D _missing2;
        bool _hasMissingData2;
        
        int _loadDataDirect(const Grid *grid, Texture3D *dataTexture, Texture3D *missingTexture, bool *hasMissingData);
    };
    
    
    class VolumeRegularIso : public VolumeRegular {
    public:
        VolumeRegularIso(GLManager *gl):VolumeRegular(gl){}
        static std::string GetName() { return "Iso Regular"; }
        static Type        GetType() { return Type::Iso; }
        virtual ShaderProgram *GetShader() const;
        virtual void SetUniforms(const ShaderProgram *shader) const;
    };
    
}

