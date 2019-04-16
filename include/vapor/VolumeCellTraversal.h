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
    virtual void SetUniforms(int *nextTextureUnit) const;

  private:
    unsigned int _minTexture;
    unsigned int _maxTexture;
    unsigned int _BBLevelDimTexture;
    unsigned int _coordTexture;

    int _coordDims[3];
    int _BBLevels;
    bool _useHighPrecisionTriangleRoutine;

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
    virtual void SetUniforms(int *nextTextureUnit) const;
};

} // namespace VAPoR
