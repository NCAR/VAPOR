#include <vapor/VolumeResampled.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using std::vector;

using namespace VAPoR;

int VolumeResampled::LoadData(const Grid *grid)
{
    const vector<size_t> dims = grid->GetDimensions();
    const size_t w = dims[0], h = dims[1], d = dims[2];
    float *data = new float[dims[0]*dims[1]*dims[2]];
    
    vector<double> min, max;
    grid->GetUserExtents(min, max);
    
    for (int z = 0; z < d; z++) {
        printf("Resampling... %i/%li\n", z, d);
        const float zSamplePos = (z + 0.5f)/(float)d * (max[2]-min[2]) + min[2];
        for (int y = 0; y < h; y++) {
            const float ySamplePos = (y + 0.5f)/(float)h * (max[1]-min[1]) + min[1];
            for (int x = 0; x < w; x++) {
                const float xSamplePos = (x + 0.5f)/(float)w * (max[0]-min[0]) + min[0];
                data[z*w*h + y*w + x] = grid->GetValue(xSamplePos, ySamplePos, zSamplePos);
            }
        }
    }
    
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);
    
    delete [] data;
    return 0;
}
