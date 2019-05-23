#include <vapor/VolumeTest2.h>
#include <vector>
#include <vapor/glutil.h>
#include <vapor/GLManager.h>
#include <glm/glm.hpp>

using glm::ivec3;
using glm::vec3;
using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeTest2> registration;

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

static ivec3 GetFaceFromFaceIndex(int i)
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

static int GetFaceIndexFromFace(const ivec3 face)
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

static void GetFaceCoordinateIndices(const ivec3 &cell, const ivec3 &face, ivec3 &i0, ivec3 &i1, ivec3 &i2, ivec3 &i3)
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

static vec3 GetCoordAtIndex(const ivec3 &index, const vec3 *data, const ivec3 &dims)
{
    const int w = dims.x;
    const int h = dims.y;
    const int d = dims.z;
    const int x = index.x;
    const int y = index.y;
    const int z = index.z;
    assert(x >= 0 && x < w && y >= 0 && y < h && z >= 0 && z < d);
    return data[(z * w * h + y * w + x)];
}

static void GetFaceVertices(const ivec3 &cellIndex, const ivec3 &face, const vec3 *data, const ivec3 &dims, vec3 &v0, vec3 &v1, vec3 &v2, vec3 &v3)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    v0 = GetCoordAtIndex(i0, data, dims);
    v1 = GetCoordAtIndex(i1, data, dims);
    v2 = GetCoordAtIndex(i2, data, dims);
    v3 = GetCoordAtIndex(i3, data, dims);
}

static void GetFaceVertices(const ivec3 &cellIndex, int face, const vec3 *data, const ivec3 &dims, vec3 &v0, vec3 &v1, vec3 &v2, vec3 &v3)
{
    GetFaceVertices(cellIndex, GetFaceFromFaceIndex(face), data, dims, v0, v1, v2, v3);
}

VolumeTest2::VolumeTest2(GLManager *gl) : VolumeRegular(gl)
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

VolumeTest2::~VolumeTest2()
{
    if (zCoordTexture) glDeleteTextures(1, &zCoordTexture);
    if (xyCoordTexture) glDeleteTextures(1, &xyCoordTexture);
}

#define EPSILON 1.19e-07

static bool IsInsideCell(const vec3 &p, const ivec3 &cellIndex, const vec3 *coords, const ivec3 dims)
{
    for (int face = 0; face < 6; face++) {
        vec3 v0, v1, v2, v3;
        GetFaceVertices(cellIndex, face, coords, dims, v0, v1, v2, v3);
        vec3 n = cross(v1 - v0, v2 - v0);
        if (glm::length(n) < EPSILON) {
            if (glm::dot(n, p - v0) > 0) return false;
        }
    }
    return true;
}

int VolumeTest2::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0) return -1;

    vector<size_t> dims = grid->GetDimensions();
    const int      w = dims[0], h = dims[1], d = dims[2];
    const size_t   nCoords = (size_t)w * h * d;
    vector<double> minExt, maxExt;
    grid->GetUserExtents(minExt, maxExt);
    const float minX = minExt[0];
    const float minY = minExt[1];
    const float minZ = minExt[2];
    const float maxX = maxExt[0];
    const float maxY = maxExt[1];
    const float maxZ = maxExt[2];

    // float *xy = new float[dims[0]*dims[1]];
    float *data = new float[w * h * d * 3];
    for (int i = 0; i < w * h * d * 3; i++) data[i] = -1;

    vec3 *coords = new vec3[nCoords];

    auto coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nCoords; ++i, ++coord) {
        coords[i][0] = (*coord)[0];
        coords[i][1] = (*coord)[1];
        coords[i][2] = (*coord)[2];
    }

    for (int z = 0; z < d; z++) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {}
        }
    }

    glBindTexture(GL_TEXTURE_3D, zCoordTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, dims[0], dims[1], dims[2], 0, GL_RGB, GL_FLOAT, data);

    delete[] data;
    return 0;
}

ShaderProgram *VolumeTest2::GetShader() const
{
    ShaderProgram *s = _glManager->shaderManager->GetShader("ray2");
    if (!s) return nullptr;
    s->Bind();

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, zCoordTexture);

    s->SetUniform("coordLUT", 2);

    return s;
}
