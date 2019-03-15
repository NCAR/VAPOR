#pragma once

#include <vapor/VolumeAlgorithm.h>

namespace VAPoR {

class VolumeRegular : public VolumeAlgorithm {
  public:
    VolumeRegular(GLManager *gl);
    ~VolumeRegular();

    static std::string GetName() { return "Regular"; }
    static Type GetType() { return Type::DVR; }

    virtual int LoadData(const Grid *grid);
    virtual int LoadSecondaryData(const Grid *grid);
    virtual void DeleteSecondaryData();
    virtual ShaderProgram *GetShader() const;
    virtual void SetUniforms() const;

  protected:
    unsigned int _dataTexture;
    unsigned int _missingTexture;
    bool _hasMissingData;

    std::vector<size_t> dataDimensions;

    bool _hasSecondData;
    unsigned int _dataTexture2;
    unsigned int _missingTexture2;
    bool _hasMissingData2;

    static int _loadDataDirect(const Grid *grid, const unsigned int dataTexture, const unsigned int missingTexture, bool *hasMissingData);
    static void _initializeTexture(unsigned int &texture);
};

class VolumeRegularIso : public VolumeRegular {
  public:
    VolumeRegularIso(GLManager *gl) : VolumeRegular(gl) {}
    static std::string GetName() { return "Iso Regular"; }
    static Type GetType() { return Type::Iso; }
    virtual ShaderProgram *GetShader() const;
    virtual void SetUniforms() const;
};

} // namespace VAPoR
