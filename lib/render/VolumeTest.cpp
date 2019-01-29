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
    if (zCoordTexture)  glDeleteTextures(1, &zCoordTexture);
    if (xyCoordTexture) glDeleteTextures(1, &xyCoordTexture);
}

int VolumeTest::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0)
        return -1;
    
    vector<size_t> dims = grid->GetDimensions();
    const int w = dims[0], h = dims[1], d = dims[2];
    vector<double> minExt, maxExt;
    grid->GetUserExtents(minExt, maxExt);
    const float minX = minExt[0];
    const float minY = minExt[1];
    const float minZ = minExt[2];
    const float maxX = maxExt[0];
    const float maxY = maxExt[1];
    const float maxZ = maxExt[2];
    
    // float *xy = new float[dims[0]*dims[1]];
    float *data = new float[w*h*d*3];
    for (int i = 0; i < w*h*d*3; i++)
        data[i] = -1;
    
    // auto coord = grid->ConstCoordBegin(); // broken
    // auto end = grid->ConstCoordEnd();
    for (int z = 0; z < d; z++) {
        printf("%i/%i\n", z, d);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                // data[coord] = x,y,z;
                
                // float cx = (*coord)[0];
                // float cy = (*coord)[1];
                // float cz = (*coord)[2];
                double dcx, dcy, dcz;
                float cx, cy, cz;
                grid->GetUserCoordinates(x, y, z, dcx, dcy, dcz);
                cx = dcx;
                cy = dcy;
                cz = dcz;
                // printf("[%i, %i, %i] = %.2f\t%.2f\t%.2f\n", x, y, z, dcx, dcy, dcz);
                
                int dx = (cx - minX)/(maxX - minX) * (w-1);
                int dy = (cy - minY)/(maxY - minY) * (h-1);
                int dz = (cz - minZ)/(maxZ - minZ) * (d-1);
                
                // assert(dx>=0 && dx<w);
                // assert(dy>=0 && dy<h);
                // assert(dz>=0 && dz<d);
                
                data[3 * (dz*w*h + dy*w + dx)] = x/(float)w;
                data[3 * (dz*w*h + dy*w + dx)+1] = y/(float)h;
                data[3 * (dz*w*h + dy*w + dx)+2] = z/(float)d;
                // printf("Set data[%i, %i, %i] = %i, %i, %i\n", dx, dy, dz, x,y,z);
                
                // assert(coord != end);
                // ++coord;
            }
        }
    }
    
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, dims[0], dims[1], dims[2], 0, GL_RGB, GL_FLOAT, data);
    
    delete [] data;
    return 0;
}

ShaderProgram *VolumeTest::GetShader(ShaderManager *sm)
{
    ShaderProgram *s = sm->GetShader("ray2");
    if (!s) return nullptr;
    s->Bind();
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    
    s->SetUniform("coordLUT", 2);
    
    return s;
}
