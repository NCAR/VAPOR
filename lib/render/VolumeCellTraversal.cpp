#include <vapor/VolumeCellTraversal.h>
#include <vector>
#include <array>
#include <algorithm>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>
#include <vapor/ShaderManager.h>
#include <vapor/Progress.h>

#ifndef FLT16_MAX
    #define FLT16_MAX 6.55E4
#endif

using glm::ivec2;
using glm::ivec3;
using glm::vec3;
using std::array;
using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeCellTraversal>    registration;
static VolumeAlgorithmRegistrar<VolumeCellTraversalIso> registration2;

#define MAX_LEVELS 12

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

static int GetFaceIndexFromFace(const ivec3 &face)
{
    if (face == F_LEFT) return FI_LEFT;
    if (face == F_RIGHT) return FI_RIGHT;
    if (face == F_UP) return FI_UP;
    if (face == F_DOWN) return FI_DOWN;
    if (face == F_FRONT) return FI_FRONT;
    if (face == F_BACK) return FI_BACK;
    VAssert(0);
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
    } else {
        VAssert(!"Invalid face enum");
        i0 = ivec3(0);
        i1 = ivec3(0);
        i2 = ivec3(0);
        i3 = ivec3(0);
    }
}

static vec3 GetCoordAtIndex(const ivec3 &index, const float *data, const ivec3 &dims)
{
    const int w = dims.x;
    const int h = dims.y;
    const int d = dims.z;
    const int x = index.x;
    const int y = index.y;
    const int z = index.z;
    VAssert(x >= 0 && x < w && y >= 0 && y < h && z >= 0 && z < d);
    return vec3(data[3 * (z * w * h + y * w + x)], data[3 * (z * w * h + y * w + x) + 1], data[3 * (z * w * h + y * w + x) + 2]);
}

static void GetFaceVertices(const ivec3 &cellIndex, const ivec3 &face, const float *data, const ivec3 &dims, vec3 &v0, vec3 &v1, vec3 &v2, vec3 &v3)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    v0 = GetCoordAtIndex(i0, data, dims);
    v1 = GetCoordAtIndex(i1, data, dims);
    v2 = GetCoordAtIndex(i2, data, dims);
    v3 = GetCoordAtIndex(i3, data, dims);
}

static bool ComputeSideBBoxes(ivec3 side, int fastDim, int slowDim, vec3 *boxMins, vec3 *boxMaxs, float *coordData, const ivec3 &cellDims, const ivec3 &coordDims, const int bd, const int sd)
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

VolumeCellTraversal::VolumeCellTraversal(GLManager *gl, VolumeRenderer *renderer) : VolumeRegular(gl, renderer), _useHighPrecisionTriangleRoutine(false)
{
    _coordTexture.Generate(GL_NEAREST);
    _minTexture.Generate(GL_NEAREST);
    _maxTexture.Generate(GL_NEAREST);
    _BBLevelDimTexture.Generate(GL_NEAREST);
}

VolumeCellTraversal::~VolumeCellTraversal() {}

int VolumeCellTraversal::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0) return -1;

    _useHighPrecisionTriangleRoutine = _needsHighPrecisionTriangleRoutine(grid);
    _gridHasInvertedCoordinateSystemHandiness = !grid->HasInvertedCoordinateSystemHandiness();

    auto         dims = grid->GetDimensions();
    const int    w = dims[0], h = dims[1], d = dims[2];
    const size_t nCoords = (size_t)w * h * d;
    _coordDims[0] = w;
    _coordDims[1] = h;
    _coordDims[2] = d;

    float *data = new float[nCoords * 3];
    if (!data) {
        Wasp::MyBase::SetErrMsg("Could not allocate enough RAM to load data coordinates");
        return -1;
    }

    Progress::Start("Load coord data", nCoords);
    auto coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nCoords; ++i, ++coord) {
        Progress::Update(i);
        data[i * 3] = (*coord)[0];
        data[i * 3 + 1] = (*coord)[1];
        data[i * 3 + 2] = (*coord)[2];
    }
    Progress::Finish();

    _coordTexture.TexImage(GL_RGB32F, dims[0], dims[1], dims[2], GL_RGB, GL_FLOAT, data);

    // ---------------------------------------
    // Compute bounding boxes for outside faces
    // ---------------------------------------

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

    Progress::Start("Compute acceleration data", 6);
    ComputeSideBBoxes(F_LEFT, 1, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Update(1);
    ComputeSideBBoxes(F_RIGHT, 1, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Update(2);
    ComputeSideBBoxes(F_UP, 0, 1, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Update(3);
    ComputeSideBBoxes(F_DOWN, 0, 1, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Update(4);
    ComputeSideBBoxes(F_FRONT, 0, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Update(5);
    ComputeSideBBoxes(F_BACK, 0, 2, boxMins, boxMaxs, data, ivec3(cw, ch, cd), ivec3(w, h, d), bd, sd);
    Progress::Finish();

    int levels = 1;
    int size = bd;
    while ((size = size >> 1)) levels++;
    levels = min(levels, MAX_LEVELS);
    _BBLevels = levels;

    _minTexture.TexImage(GL_RGB32F, sd, bd, 6, GL_RGB, GL_FLOAT, boxMins);
    _maxTexture.TexImage(GL_RGB32F, sd, bd, 6, GL_RGB, GL_FLOAT, boxMaxs);

    // ---------------------------------------
    // Compute mipmap for acceleration tree
    //
    // Each subsequent mipmip level contains outermost bounds of all the bounding
    // boxes it encompases from the previous level.
    // ---------------------------------------

    vector<int>             sizes(levels);
    vector<array<ivec2, 6>> mipDims(levels);
    vector<vec3 *>          minMip(levels);
    vector<vec3 *>          maxMip(levels);
    sizes[0] = bd;
    minMip[0] = boxMins;
    maxMip[0] = boxMaxs;

    mipDims[0][FI_LEFT] = ivec2(ch, cd);
    mipDims[0][FI_RIGHT] = ivec2(ch, cd);
    mipDims[0][FI_UP] = ivec2(cw, ch);
    mipDims[0][FI_DOWN] = ivec2(cw, ch);
    mipDims[0][FI_FRONT] = ivec2(cw, cd);
    mipDims[0][FI_BACK] = ivec2(cw, cd);

    Progress::Start("Generate tree", levels);
    for (int level = 1; level < levels; level++) {
        Progress::Update(level);
        int ms = bd >> level;
        int mUpS = sizes[level - 1];

        sizes[level] = ms;
        minMip[level] = new vec3[ms * ms * 6];
        maxMip[level] = new vec3[ms * ms * 6];

        for (int z = 0; z < 6; z++) {
            int mUpW = mipDims[level - 1][z][0];
            int mUpH = mipDims[level - 1][z][1];

            int mW = std::max(1, mUpW >> 1);
            int mH = std::max(1, mUpH >> 1);
            mipDims[level][z][0] = mW;
            mipDims[level][z][1] = mH;

            // At each higher mipmap, the dimensions halve. So each pixel maps to at least 4
            // pixels at the previous level. However if the previous level dim was 1, it
            // no longer halves so we set the increment to 0. This resamples the same pixel twice
            // for simplicities sake.
            int ix = 1, iy = 1;
            if (mUpW == 1) ix = 0;
            if (mUpH == 1) iy = 0;

            for (int y = 0; y < mH; y++) {
                for (int x = 0; x < mW; x++) {
                    vec3 v0 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2 + ix];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + x * 2 + ix];
                    vec3 v3 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + x * 2];

                    // glm::min and glm::max are component-wise
                    minMip[level][z * ms * ms + y * ms + x] = glm::min(v0, glm::min(v1, glm::min(v2, v3)));

                    v0 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + x * 2 + ix];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + x * 2 + ix];
                    v3 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + x * 2];

                    maxMip[level][z * ms * ms + y * ms + x] = glm::max(v0, glm::max(v1, glm::max(v2, v3)));
                }
            }
            // If the upper level is odd, the top and right border pixels map to 6 pixels in the previous
            // level which are accounted for here.
            if (mUpW % 2 == 1) {
                for (int y = 0; y < mH; y++) {
                    vec3 v0 = minMip[level][z * ms * ms + y * ms + mW - 1];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + mUpW - 1];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + mUpW - 1];
                    minMip[level][z * ms * ms + y * ms + mW - 1] = glm::min(v0, glm::min(v1, v2));

                    v0 = maxMip[level][z * ms * ms + y * ms + mW - 1];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (y * 2) * mUpS + mUpW - 1];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (y * 2 + iy) * mUpS + mUpW - 1];
                    maxMip[level][z * ms * ms + y * ms + mW - 1] = glm::max(v0, glm::max(v1, v2));
                }
            }
            if (mUpH % 2 == 1) {
                for (int x = 0; x < mW; x++) {
                    vec3 v0 = minMip[level][z * ms * ms + (mH - 1) * ms + x];
                    vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2];
                    vec3 v2 = minMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2 + ix];
                    minMip[level][z * ms * ms + (mH - 1) * ms + x] = glm::min(v0, glm::min(v1, v2));

                    v0 = maxMip[level][z * ms * ms + (mH - 1) * ms + x];
                    v1 = maxMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2];
                    v2 = maxMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + x * 2 + ix];
                    maxMip[level][z * ms * ms + (mH - 1) * ms + x] = glm::max(v0, glm::max(v1, v2));
                }
            }
            // If the both upper dims are odd, the top-right pixel maps to 9 pixels in the previous level.
            // 8 of these were already accounted for. This accounts for the last pixel in the corner
            if (mUpW % 2 == 1 && mUpH % 2 == 1) {
                vec3 v0 = minMip[level][z * ms * ms + (mH - 1) * ms + mW - 1];
                vec3 v1 = minMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + mUpW - 1];
                minMip[level][z * ms * ms + (mH - 1) * ms + mW - 1] = glm::min(v0, v1);

                v0 = maxMip[level][z * ms * ms + (mH - 1) * ms + mW - 1];
                v1 = maxMip[level - 1][z * mUpS * mUpS + (mUpH - 1) * mUpS + mUpW - 1];
                maxMip[level][z * ms * ms + (mH - 1) * ms + mW - 1] = glm::max(v0, v1);
            }
        }

        _minTexture.TexImage(GL_RGB32F, ms, ms, 6, GL_RGB, GL_FLOAT, minMip[level], level);
        _maxTexture.TexImage(GL_RGB32F, ms, ms, 6, GL_RGB, GL_FLOAT, maxMip[level], level);
    }
    Progress::Finish();

    _BBLevelDimTexture.TexImage(GL_RG32I, 6, levels, 0, GL_RG_INTEGER, GL_INT, mipDims.data());

    for (int level = 1; level < levels; level++) {
        delete[] minMip[level];
        delete[] maxMip[level];
    }

    delete[] data;
    delete[] boxMins;
    delete[] boxMaxs;
    return 0;
}

// System: CISL-VAPOR
// Vendor: NVIDIA
// Dataset: Lee Orf tornado Max/Min resolutions
//
//                  Render Times
// Levels  Compile   Max    Min
//   9        64     0.05    -
//   8        24     0.05    -
//   7        9.5    0.04    -
//   6        4      0.05   0.01
//   5        1.8    0.067  0.01
//   4        0.9    0.22   0.012
//   3        0.56    -     0.022

int VolumeCellTraversal::_getHeuristicBBLevels() const
{
    int levels = _BBLevels;

    if (levels == 12)
        levels -= 4;
    else if (levels >= 9)
        levels -= 3;
    else if (levels >= 7)
        levels -= 2;
    else if (levels >= 2)
        levels -= 1;

    // Nvidia's loop optimizer has an exponential Big-O complexity in
    // relation to nesting and anything over 6 takes too long to compile.
    GLManager::Vendor vendor = GLManager::GetVendor();
    if (vendor == GLManager::Vendor::Nvidia)
        if (levels > 6) levels = 6;

    return levels;
}

std::string VolumeCellTraversal::_addDefinitionsToShader(std::string shaderName) const
{
    shaderName = VolumeRegular::_addDefinitionsToShader(shaderName);

    if (_useHighPrecisionTriangleRoutine) shaderName += ":USE_INTEL_TRI_ISECT";

    if (_gridHasInvertedCoordinateSystemHandiness) shaderName += ":INVERT_GRID_COORD_SYS_HAND";

    GLManager::Vendor vendor = GLManager::GetVendor();

    if (vendor == GLManager::Vendor::Nvidia || vendor == GLManager::Vendor::AMD || vendor == GLManager::Vendor::Mesa) shaderName += ":NVIDIA";

    shaderName += ":BB_LEVELS " + std::to_string(_getHeuristicBBLevels());

    return shaderName;
}

ShaderProgram *VolumeCellTraversal::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeCellDVR")); }

void VolumeCellTraversal::SetUniforms(const ShaderProgram *s) const
{
    VolumeRegular::SetUniforms(s);

    s->SetUniform("coordDims", *(glm::ivec3 *)&_coordDims);

    s->SetSampler("coords", _coordTexture);
    s->SetSampler("boxMins", _minTexture);
    s->SetSampler("boxMaxs", _maxTexture);
    s->SetSampler("levelDims", _BBLevelDimTexture);
}

float VolumeCellTraversal::GuestimateFastModeSpeedupFactor() const { return 2; }

bool VolumeCellTraversal::_needsHighPrecisionTriangleRoutine(const Grid *grid)
{
    vector<double> extentsMin, extentsMax;
    grid->GetUserExtents(extentsMin, extentsMax);
    vector<double> lengths = {
        extentsMax[0] - extentsMin[0],
        extentsMax[1] - extentsMin[1],
        extentsMax[2] - extentsMin[2],
    };
    double minLength = min(lengths[0], min(lengths[1], lengths[2]));
    double maxLength = max(lengths[0], max(lengths[1], lengths[2]));
    double ratio = maxLength / minLength;

    if (ratio > 10000)
        return true;
    else
        return false;
}

// Needs more consideration for frequency
bool VolumeCellTraversal::_need32BitForCoordinates(const Grid *grid)
{
    vector<double> minExts, maxExts;
    grid->GetUserExtents(minExts, maxExts);
    double minExt = min(minExts[0], min(minExts[1], minExts[2]));
    double maxExt = max(maxExts[0], max(maxExts[1], maxExts[2]));
    if (minExt <= -FLT16_MAX || maxExt >= FLT16_MAX) return true;
    return false;
}

ShaderProgram *VolumeCellTraversalIso::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeCellIso")); }

void VolumeCellTraversalIso::SetUniforms(const ShaderProgram *shader) const { VolumeCellTraversal::SetUniforms(shader); }
