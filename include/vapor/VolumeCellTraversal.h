#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeCellTraversal : public VolumeRegular {
  public:
    VolumeCellTraversal(GLManager *gl);
    ~VolumeCellTraversal();

    static std::string GetName() { return "Cell Traversal"; }
    static Type GetType() { return Type::DVR; }
    virtual bool IsSlow() { return true; }

    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const;
    virtual void SetUniforms(const ShaderProgram *shader) const;
    virtual float GuestimateFastModeSpeedupFactor() const;

  private:
    Texture3D _coordTexture;
    Texture2DArray _minTexture;
    Texture2DArray _maxTexture;
    Texture2D _BBLevelDimTexture;

    int _coordDims[3];
    int _BBLevels;
    bool _useHighPrecisionTriangleRoutine;
    bool _nvidiaGPU;

    bool _needsHighPrecisionTriangleRoutine(const Grid *grid);
    static bool _need32BitForCoordinates(const Grid *grid);

  protected:
    int _getHeuristicBBLevels() const;
    std::string _addDefinitionsToShader(std::string shaderName) const;
};

class VolumeCellTraversalIso : public VolumeCellTraversal {
  public:
    VolumeCellTraversalIso(GLManager *gl) : VolumeCellTraversal(gl) {}
    static std::string GetName() { return "Iso Cell Traversal"; }
    static Type GetType() { return Type::Iso; }
    virtual ShaderProgram *GetShader() const;
    virtual void SetUniforms(const ShaderProgram *shader) const;
};

} // namespace VAPoR
