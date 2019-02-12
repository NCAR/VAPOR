#include <vapor/VolumeRegular.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>

using std::vector;

using namespace VAPoR;

VolumeRegular::VolumeRegular(GLManager *gl) : VolumeAlgorithm(gl), dataTexture(NULL)
{
    glGenTextures(1, &dataTexture);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

VolumeRegular::~VolumeRegular()
{
    if (dataTexture) glDeleteTextures(1, &dataTexture);
}

int VolumeRegular::LoadData(const Grid *grid)
{
    vector<size_t> dims = grid->GetDimensions();
    float *        data = new float[dims[0] * dims[1] * dims[2]];

    size_t i = 0;
    auto   end = grid->cend();
    for (auto it = grid->cbegin(); it != end; ++it, ++i) { data[i] = *it; }

    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);

    delete[] data;
    return 0;
}

ShaderProgram *VolumeRegular::GetShader() const { return _glManager->shaderManager->GetShader("ray"); }

void VolumeRegular::SetUniforms() const
{
    ShaderProgram *s = GetShader();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    s->SetUniform("data", 0);
}
