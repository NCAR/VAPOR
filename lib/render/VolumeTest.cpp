#include <vapor/VolumeTest.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using std::vector;

using namespace VAPoR;

int VolumeTest::LoadData(const Grid *grid)
{
    vector<size_t> dims = grid->GetDimensions();
    float *data = new float[dims[0]*dims[1]*dims[2]];
    
    size_t i = 0;
    auto end = grid->cend();
    for (auto it = grid->cbegin(); it != end; ++it, ++i) {
        data[i] = *it;
    }
    
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);
    
    delete [] data;
    return 0;
}

ShaderProgram *VolumeTest::GetShader(ShaderManager *sm)
{
    return sm->GetShader("ray");
}
