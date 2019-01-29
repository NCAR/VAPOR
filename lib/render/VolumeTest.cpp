#include <vapor/VolumeTest.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using std::vector;

using namespace VAPoR;

VolumeTest::VolumeTest()
{
    glGenTextures(1, &zCoordTexture);
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &xyCoordTexture);
    glBindTexture(GL_TEXTURE_2D, xyCoordTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

VolumeTest::~VolumeTest()
{
    if (zCoordTexture) glDeleteTextures(1, &zCoordTexture);
    if (xyCoordTexture) glDeleteTextures(1, &xyCoordTexture);
}

int VolumeTest::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0) return -1;

    vector<size_t> dims = grid->GetDimensions();
    float *        xy = new float[dims[0] * dims[1]];

    auto end = grid->ConstCoordEnd();
    for (auto it = grid->ConstCoordBegin(); it != end; ++it) {
        // it.
    }

    return 0;
}

ShaderProgram *VolumeTest::GetShader(ShaderManager *sm) { return sm->GetShader("ray"); }
