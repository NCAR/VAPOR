#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

class VolumeCellTraversal : public VolumeRegular {
  public:
    VolumeCellTraversal(GLManager *gl);
    ~VolumeCellTraversal();

    static std::string GetName() { return "Cell Traversal"; }

    virtual int LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const;
    virtual void SetUniforms() const;

  private:
    unsigned int minTexture;
    unsigned int maxTexture;
    unsigned int BBLevelDimTexture;
    unsigned int coordTexture;

    unsigned int VAO;
    unsigned int VBO;

    int coordDims[3];
    int BBLevels;
    bool useHighPrecisionTriangleRoutine;

    bool NeedsHighPrecisionTriangleRoutine(const Grid *grid);
    static bool Need32BitForCoordinates(const Grid *grid);

  protected:
    std::string AddDefinitionsToShader(std::string shaderName) const;
};

class IsoCellTraversal : public VolumeCellTraversal {
  public:
    IsoCellTraversal(GLManager *gl) : VolumeCellTraversal(gl) {}
    static std::string GetName() { return "Iso Cell Traversal"; }
    virtual ShaderProgram *GetShader() const;
};

} // namespace VAPoR
