#include <vapor/VolumeResampled.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeResampled> registration;

int VolumeResampled::LoadData(const Grid *grid)
{
    VolumeRegular::LoadData(grid);
#define S 1
    const vector<size_t> dims = grid->GetDimensions();
    const size_t         w = dims[0] * S, h = dims[1] * S, d = dims[2] * S;
    float *              data = new float[w * h * d];

    vector<double> min, max;
    grid->GetUserExtents(min, max);

    for (int z = 0; z < d; z++) {
        printf("Resampling... %i/%li\n", z, d);
        const float zSamplePos = (z + 0.5f) / (float)d * (max[2] - min[2]) + min[2];
        for (int y = 0; y < h; y++) {
            const float ySamplePos = (y + 0.5f) / (float)h * (max[1] - min[1]) + min[1];
            for (int x = 0; x < w; x++) {
                const float xSamplePos = (x + 0.5f) / (float)w * (max[0] - min[0]) + min[0];
                data[z * w * h + y * w + x] = grid->GetValue(xSamplePos, ySamplePos, zSamplePos);
            }
        }
    }

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, w, h, d, 0, GL_RED, GL_FLOAT, data);

    _hasMissingData = grid->HasMissingData();
    if (_hasMissingData) {
        printf("Loading missing data...\n");
        const float    missingValue = grid->GetMissingValue();
        unsigned char *missingMask = new unsigned char[w * h * d];
        memset(missingMask, 0, w * h * d);

        for (size_t i = 0; i < w * h * d; i++)
            if (data[i] == missingValue) missingMask[i] = 255;

        glBindTexture(GL_TEXTURE_3D, _missingTexture);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 0, 0, 0, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);    // Fix driver bug with re-uploading large textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, dims[0], dims[1], dims[2], 0, GL_RED, GL_UNSIGNED_BYTE, missingMask);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        delete[] missingMask;
    }

    delete[] data;
    return 0;
}
