#include <vapor/VolumeCellTraversal.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
// #include <glm/integer.hpp>

using std::vector;

using namespace VAPoR;

VolumeCellTraversal::VolumeCellTraversal()
{
    glGenTextures(1, &zCoordTexture);
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &xyCoordTexture);
    glBindTexture(GL_TEXTURE_2D, xyCoordTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

VolumeCellTraversal::~VolumeCellTraversal()
{
    if (zCoordTexture)  glDeleteTextures(1, &zCoordTexture);
    if (xyCoordTexture) glDeleteTextures(1, &xyCoordTexture);
}

int VolumeCellTraversal::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0)
        return -1;
    
    vector<size_t> dims = grid->GetDimensions();
    const int w = dims[0], h = dims[1], d = dims[2];
    coordDims[0] = w;
    coordDims[1] = h;
    coordDims[2] = d;
    
    float *data = new float[w*h*d*3];
    
    // auto coord = grid->ConstCoordBegin(); // broken
    
    vector <size_t> indices = {0,0,0};
    vector <double> coords;
    
    for (int z = 0; z < d; z++) {
        printf("%i/%i\n", z, d);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                
                indices[0] = x;
                indices[1] = y;
                indices[2] = z;
                grid->GetUserCoordinates(indices, coords);
                const float cx = coords[0];
                const float cy = coords[1];
                const float cz = coords[2];
                
                data[3 * (z*w*h + y*w + x)]   = cx;
                data[3 * (z*w*h + y*w + x)+1] = cy;
                data[3 * (z*w*h + y*w + x)+2] = cz;
            }
        }
    }
    
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, dims[0], dims[1], dims[2], 0, GL_RGB, GL_FLOAT, data);
    
    delete [] data;
    return 0;
}

ShaderProgram *VolumeCellTraversal::GetShader(ShaderManager *sm)
{
    ShaderProgram *s = sm->GetShader("rayCellTraversal");
    if (!s) return nullptr;
    s->Bind();
    
    s->SetUniform("coordDims", *(glm::ivec3*)&coordDims);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    
    s->SetUniform("coords", 2);
    
    return s;
}
