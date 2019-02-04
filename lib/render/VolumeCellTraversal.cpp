#include <vapor/VolumeCellTraversal.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>
#include <vapor/ShaderManager.h>
// #include <glm/integer.hpp>

using glm::ivec2;
using glm::ivec3;
using glm::vec3;
using std::vector;

using namespace VAPoR;

#define MAX_LEVELS 32

#define FI_LEFT  0
#define FI_RIGHT 1
#define FI_UP    2
#define FI_DOWN  3
#define FI_FRONT 4
#define FI_BACK  5

#define F_LEFT  ivec3(-1, 0, 0)
#define F_RIGHT ivec3(1, 0, 0)
#define F_UP    ivec3(0, 0, 1)
#define F_DOWN  ivec3(0, 0, -1)
#define F_FRONT ivec3(0, -1, 0)
#define F_BACK  ivec3(0, 1, 0)

ivec3 GetFaceFromFaceIndex(int i)
{
    if (i == FI_LEFT) return F_LEFT;
    if (i == FI_RIGHT) return F_RIGHT;
    if (i == FI_UP) return F_UP;
    if (i == FI_DOWN) return F_DOWN;
    if (i == FI_FRONT) return F_FRONT;
    if (i == FI_BACK) return F_BACK;
    assert(0);
    return F_LEFT;
}

int GetFaceIndexFromFace(const ivec3 face)
{
    if (face == F_LEFT) return FI_LEFT;
    if (face == F_RIGHT) return FI_RIGHT;
    if (face == F_UP) return FI_UP;
    if (face == F_DOWN) return FI_DOWN;
    if (face == F_FRONT) return FI_FRONT;
    if (face == F_BACK) return FI_BACK;
    assert(0);
    return 0;
}

void GetFaceCoordinateIndices(const ivec3 &cell, const ivec3 &face, ivec3 &i0, ivec3 &i1, ivec3 &i2, ivec3 &i3)
{
    // CCW
    if (face == F_DOWN) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 1, 0);
        i2 = cell + ivec3(1, 1, 0);
        i3 = cell + ivec3(1, 0, 0);
    } else if (face == F_UP) {
        i0 = cell + ivec3(0, 0, 1);
        i1 = cell + ivec3(1, 0, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(0, 1, 1);
    } else if (face == F_LEFT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 0, 1);
        i2 = cell + ivec3(0, 1, 1);
        i3 = cell + ivec3(0, 1, 0);
    } else if (face == F_RIGHT) {
        i0 = cell + ivec3(1, 0, 0);
        i1 = cell + ivec3(1, 1, 0);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 0, 1);
    } else if (face == F_FRONT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(1, 0, 0);
        i2 = cell + ivec3(1, 0, 1);
        i3 = cell + ivec3(0, 0, 1);
    } else if (face == F_BACK) {
        i0 = cell + ivec3(0, 1, 0);
        i1 = cell + ivec3(0, 1, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 1, 0);
    }
}

vec3 GetCoordAtIndex(const ivec3 &index, const float *data, const ivec3 &dims)
{
    const int w = dims.x;
    const int h = dims.y;
    const int d = dims.z;
    const int x = index.x;
    const int y = index.y;
    const int z = index.z;
    assert(x >= 0 && x < w && y >= 0 && y < h && z >= 0 && z < d);
    return vec3(data[3 * (z * w * h + y * w + x)], data[3 * (z * w * h + y * w + x) + 1], data[3 * (z * w * h + y * w + x) + 2]);
}

void GetFaceVertices(const ivec3 &cellIndex, const ivec3 &face, const float *data, const ivec3 &dims, vec3 &v0, vec3 &v1, vec3 &v2, vec3 &v3)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    v0 = GetCoordAtIndex(i0, data, dims);
    v1 = GetCoordAtIndex(i1, data, dims);
    v2 = GetCoordAtIndex(i2, data, dims);
    v3 = GetCoordAtIndex(i3, data, dims);
}

bool ComputeSideBBoxes(ivec3 side, int fastDim, int slowDim, vec3 *boxMins, vec3 *boxMaxs, float *coordData, const ivec3 &cellDims, const ivec3 &coordDims, const int bd, const int sd)
{
    ivec3 index = (side + 1) / 2 * (cellDims - 1);
    int   sideID = GetFaceIndexFromFace(side);

    for (index[slowDim] = 0; index[slowDim] < cellDims[slowDim]; index[slowDim]++) {
        for (index[fastDim] = 0; index[fastDim] < cellDims[fastDim]; index[fastDim]++) {
            vec3 v0, v1, v2, v3;
            GetFaceVertices(index, side, coordData, coordDims, v0, v1, v2, v3);

            boxMins[sideID * bd * sd + index[slowDim] * sd + index[fastDim]] = glm::min(v0, glm::min(v1, glm::min(v2, v3)));
            boxMaxs[sideID * bd * sd + index[slowDim] * sd + index[fastDim]] = glm::max(v0, glm::max(v1, glm::max(v2, v3)));
        }
    }
    return false;
}

#define BBTYPE GL_TEXTURE_2D_ARRAY

VolumeCellTraversal::VolumeCellTraversal(GLManager *gl) : VolumeRegular(gl)
{
    glGenTextures(1, &coordTexture);
    glBindTexture(GL_TEXTURE_3D, coordTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &minTexture);
    glBindTexture(BBTYPE, minTexture);
    glTexParameteri(BBTYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(BBTYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(BBTYPE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(BBTYPE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &maxTexture);
    glBindTexture(BBTYPE, maxTexture);
    glTexParameteri(BBTYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(BBTYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(BBTYPE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(BBTYPE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &BBLevelDimTexture);
    glBindTexture(GL_TEXTURE_2D, BBLevelDimTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    float BL = -1;
    float data[] = {BL, BL, 0, 0, 1, BL, 1, 0, BL, 1, 0, 1,

                    BL, 1,  0, 1, 1, BL, 1, 0, 1,  1, 1, 1};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

VolumeCellTraversal::~VolumeCellTraversal()
{
    if (coordTexture) glDeleteTextures(1, &coordTexture);
    if (minTexture) glDeleteTextures(1, &minTexture);
    if (maxTexture) glDeleteTextures(1, &maxTexture);
    if (BBLevelDimTexture) glDeleteTextures(1, &BBLevelDimTexture);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

#include <vapor/VolumeRenderer.h>

int VolumeCellTraversal::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0) return -1;

    vector<size_t> dims = grid->GetDimensions();
    const int      w = dims[0], h = dims[1], d = dims[2];
    coordDims[0] = w;
    coordDims[1] = h;
    coordDims[2] = d;

    float *data = new float[w * h * d * 3];

    // auto coord = grid->ConstCoordBegin(); // broken

    vector<size_t> indices = {0, 0, 0};
    vector<double> coords;

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

                data[3 * (z * w * h + y * w + x)] = cx;
                data[3 * (z * w * h + y * w + x) + 1] = cy;
                data[3 * (z * w * h + y * w + x) + 2] = cz;
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, coordTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, dims[0], dims[1], dims[2], 0, GL_RGB, GL_FLOAT, data);

    vector<size_t> cellDims = {dims[0] - 1, dims[1] - 1, dims[2] - 1};
    vector<size_t> cellDimsSorted = cellDims;
    std::sort(cellDimsSorted.begin(), cellDimsSorted.end());
    int bbDim = cellDimsSorted[2];
    int bd = bbDim;
    int sd = bbDim;

    vec3 *boxMins = new vec3[bd * sd * 6];
    vec3 *boxMaxs = new vec3[bd * sd * 6];
    memset(boxMins, 0, sizeof(vec3) * bd * sd * 6);
    memset(boxMaxs, 0, sizeof(vec3) * bd * sd * 6);

    int cw = w - 1;
    int ch = h - 1;
    int cd = d - 1;

    ComputeSideBBoxes(F_LEFT, 1, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    ComputeSideBBoxes(F_RIGHT, 1, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    ComputeSideBBoxes(F_UP, 0, 1, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    ComputeSideBBoxes(F_DOWN, 0, 1, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    ComputeSideBBoxes(F_FRONT, 0, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    ComputeSideBBoxes(F_BACK, 0, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);

    int levels = 1;
    int size = bd;
    while ((size = size >> 1)) levels++;
    levels = min(levels, MAX_LEVELS);
    BBLevels = levels;

    glBindTexture(BBTYPE, minTexture);
    // glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GL_RGB16F, sd, bd, 6);
    // glTexSubImage3D(BBTYPE, 0, 0, 0, 0, sd, bd, 6, GL_RGB, GL_FLOAT, boxMins);
    glTexImage3D(BBTYPE, 0, GL_RGB16F, sd, bd, 6, 0, GL_RGB, GL_FLOAT, boxMins);

    glBindTexture(BBTYPE, maxTexture);
    // glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, GL_RGB16F, sd, bd, 6);
    // glTexSubImage3D(BBTYPE, 0, 0, 0, 0, sd, bd, 6, GL_RGB, GL_FLOAT, boxMaxs);
    glTexImage3D(BBTYPE, 0, GL_RGB16F, sd, bd, 6, 0, GL_RGB, GL_FLOAT, boxMaxs);

    int   sizes[levels];
    ivec2 mipDims[levels][6];
    vec3 *minMip[levels];
    vec3 *maxMip[levels];
    sizes[0] = bd;
    minMip[0] = boxMins;
    maxMip[0] = boxMaxs;

    mipDims[0][FI_LEFT] = ivec2(ch, cd);
    mipDims[0][FI_RIGHT] = ivec2(ch, cd);
    mipDims[0][FI_UP] = ivec2(cw, ch);
    mipDims[0][FI_DOWN] = ivec2(cw, ch);
    mipDims[0][FI_FRONT] = ivec2(cw, cd);
    mipDims[0][FI_BACK] = ivec2(cw, cd);

    for (int level = 1; level < levels; level++) {
        int ms = bd >> level;
        int mUpS = sizes[level - 1];

        sizes[level] = ms;
        minMip[level] = new vec3[ms * ms * 6];
        maxMip[level] = new vec3[ms * ms * 6];

        for (int z = 0; z < 6; z++) {
            int mUpW = mipDims[level - 1][z][0];
            int mUpH = mipDims[level - 1][z][1];

            int mW = max(1, mUpW >> 1);
            int mH = max(1, mUpH >> 1);
            mipDims[level][z][0] = mW;
            mipDims[level][z][1] = mH;

            for (int y = 0; y < mH; y++) {
                for (int x = 0; x < mW; x++) {
                    vec3 v0 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2 + 1];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + x * 2 + 1];
                    vec3 v3 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + x * 2];

                    minMip[level][z * ms * ms + y * ms + x] = glm::min(v0, glm::min(v1, glm::min(v2, v3)));

                    v0 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2 + 1];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + x * 2 + 1];
                    v3 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + x * 2];

                    maxMip[level][z * ms * ms + y * ms + x] = glm::max(v0, glm::max(v1, glm::max(v2, v3)));
                }
            }
            if (mUpW % 2 == 1) {
                for (int y = 0; y < mH; y++) {
                    vec3 v0 = minMip[level][z * ms * ms + y * ms + mW - 1];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + mUpW - 1];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + mUpW - 1];
                    minMip[level][z * ms * ms + y * ms + mW - 1] = glm::min(v0, glm::min(v1, v2));

                    v0 = maxMip[level][z * ms * ms + y * ms + mW - 1];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + mUpW - 1];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + 1) * mUpS + mUpW - 1];
                    maxMip[level][z * ms * ms + y * ms + mW - 1] = glm::max(v0, glm::max(v1, v2));
                }
            }
            if (mUpH % 2 == 1) {
                for (int x = 0; x < mW; x++) {
                    vec3 v0 = minMip[level][z * ms * ms + (mH - 1) * ms + x];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2 + 1];
                    minMip[level][z * ms * ms + (mH - 1) * ms + x] = glm::min(v0, glm::min(v1, v2));

                    v0 = maxMip[level][z * ms * ms + (mH - 1) * ms + x];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2 + 1];
                    maxMip[level][z * ms * ms + (mH - 1) * ms + x] = glm::max(v0, glm::max(v1, v2));
                }
            }
        }

        glBindTexture(BBTYPE, minTexture);
        // glTexSubImage3D(BBTYPE, level, 0, 0, 0, ms, ms, 6, GL_RGB, GL_FLOAT, minMip[level]);
        glTexImage3D(BBTYPE, level, GL_RGB16F, ms, ms, 6, 0, GL_RGB, GL_FLOAT, minMip[level]);
        glBindTexture(BBTYPE, maxTexture);
        // glTexSubImage3D(BBTYPE, level, 0, 0, 0, ms, ms, 6, GL_RGB, GL_FLOAT, maxMip[level]);
        glTexImage3D(BBTYPE, level, GL_RGB16F, ms, ms, 6, 0, GL_RGB, GL_FLOAT, maxMip[level]);
    }

    GL_ERR_BREAK();
    glBindTexture(GL_TEXTURE_2D, BBLevelDimTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, levels, 6, 0, GL_RG, GL_INT, mipDims);
    GL_ERR_BREAK();

    for (int level = 1; level < levels; level++) {
        delete[] minMip[level];
        delete[] maxMip[level];
    }

    GL_ERR_BREAK();

    delete[] data;
    delete[] boxMins;
    delete[] boxMaxs;
    return 0;
}

ShaderProgram *VolumeCellTraversal::GetShader(ShaderManager *sm)
{
    ShaderProgram *s = sm->GetShader("rayCellTraversal");
    if (!s) return nullptr;
    s->Bind();

    s->SetUniform("coordDims", *(glm::ivec3 *)&coordDims);
    s->SetUniform("BBLevels", BBLevels);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, coordTexture);
    s->SetUniform("coords", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(BBTYPE, minTexture);
    s->SetUniform("boxMins", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(BBTYPE, maxTexture);
    s->SetUniform("boxMaxs", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, BBLevelDimTexture);
    s->SetUniform("levelDims", 5);

    GL_ERR_BREAK();
    return s;
}
