#include "vapor/RegularGrid.h"


#include <vapor/VolumeRectilinear.h>
#include <vector>
#include <array>
#include <algorithm>
#include <vapor/StretchedGrid.h>
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

static VolumeAlgorithmRegistrar<VolumeRectilinear>    registration;
static VolumeAlgorithmRegistrar<VolumeRectilinearIso> registration2;

VolumeRectilinear::VolumeRectilinear(GLManager *gl, VolumeRenderer *renderer) : VolumeRegular(gl, renderer), _useHighPrecisionTriangleRoutine(false)
{
    _coordLUTTexture.Generate(GL_LINEAR);
    _minTexture.Generate(GL_NEAREST);
    _maxTexture.Generate(GL_NEAREST);
    _BBLevelDimTexture.Generate(GL_NEAREST);
}

VolumeRectilinear::~VolumeRectilinear() {}

int VolumeRectilinear::LoadData(const Grid *grid)
{
    if (VolumeRegular::LoadData(grid) < 0) return -1;

    _useHighPrecisionTriangleRoutine = false;
    _gridHasInvertedCoordinateSystemHandiness = !grid->HasInvertedCoordinateSystemHandiness();

    const auto dims = grid->GetDimensions();
    const auto w = dims[0], h = dims[1], d = dims[2];
    _coordDims[0] = w;
    _coordDims[1] = h;
    _coordDims[2] = d;

    vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    vec3  extLengths = dataMax - dataMin;

    Progress::Start("Load coord data", 6);

    GLint max2DTexDim;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max2DTexDim);
    max2DTexDim /= 100;
    vector<float> coordLUTData(max2DTexDim * 3);

    auto coords = array<vector<float>, 3>();
    array<vector<double>, 3> srcCoords;

    auto rgrid = dynamic_cast<const StretchedGrid*>(grid);
    if (rgrid) {
        srcCoords = {
            rgrid->GetXCoords(),
            rgrid->GetYCoords(),
            rgrid->GetZCoords(),
        };
    } else {
        for (int i = 0; i < 3; i++) {
            srcCoords[i] = vector<double>(dims[i]);
            for (int j = 0; j < dims[i]; j++) {
                auto idx = DimsType();
                auto coord = CoordType();
                idx[i] = j;
                grid->GetUserCoordinates(idx, coord);
                srcCoords[i][j] = coord[i];
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        Progress::Update(1+i);
        coords[i].resize(dims[i]);
        _coordSigns[i] = coords[i][0] < coords[i][dims[i]-1] ? 1 : -1;
        for (int j = 0; j < dims[i]; j++) {
            coords[i][j] = (srcCoords[i][j] - dataMin[i]) / extLengths[i];
        }

        for (int j = 0; j < dims[i]-1; j++) {
            int start = coords[i][j]*max2DTexDim;
            int end = coords[i][j+1]*max2DTexDim;
            if (end < start)
                swap(start, end);
            for (int k = start; k < end; k++)
                coordLUTData[i*max2DTexDim + k] = (j + (k-start)/(float)(end-start))/(float)(dims[i]-1);
        }
    }

    Progress::Update(4);
    _coordLUTTexture.TexImage(GL_R32F, max2DTexDim, 3, 0, GL_RED, GL_FLOAT, coordLUTData.data());
    Progress::Finish();

    return 0;
}

std::string VolumeRectilinear::_addDefinitionsToShader(std::string shaderName) const
{
    shaderName = VolumeRegular::_addDefinitionsToShader(shaderName);

    if (_useHighPrecisionTriangleRoutine) shaderName += ":USE_INTEL_TRI_ISECT";

    if (_gridHasInvertedCoordinateSystemHandiness) shaderName += ":INVERT_GRID_COORD_SYS_HAND";

    GLManager::Vendor vendor = GLManager::GetVendor();

    if (vendor == GLManager::Vendor::Nvidia || vendor == GLManager::Vendor::AMD || vendor == GLManager::Vendor::Mesa) shaderName += ":NVIDIA";

    shaderName += ":BB_LEVELS " + std::to_string(3);

    return shaderName;
}

ShaderProgram *VolumeRectilinear::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeRectilinearDVR")); }

void VolumeRectilinear::SetUniforms(const ShaderProgram *s) const
{
    VolumeRegular::SetUniforms(s);

    s->SetUniform("coordDims", *(glm::ivec3 *)&_coordDims);
    s->SetUniform("coordSigns", *(glm::ivec3 *)&_coordSigns);

    s->SetSampler("coordLUT", _coordLUTTexture);
    s->SetSampler("boxMins", _minTexture);
    s->SetSampler("boxMaxs", _maxTexture);
    s->SetSampler("levelDims", _BBLevelDimTexture);
}

float VolumeRectilinear::GuestimateFastModeSpeedupFactor() const { return 2; }

int VolumeRectilinear::CheckHardwareSupport(const Grid *grid) const
{
    int ret = VolumeRegular::CheckHardwareSupport(grid);
    if (ret < 0)
        return ret;

    long freeKB = oglGetFreeMemory();
    if (freeKB >= 0) {
        auto dims = grid->GetDimensions();
        long estimatedMinimumB = dims[0]*dims[1]*dims[2] * sizeof(float) * 4;
        long estimatedMinimumKB = estimatedMinimumB/1024;
        if (freeKB < estimatedMinimumKB) {
            Wasp::MyBase::SetErrMsg("Not enough GPU RAM free (%liMB free, need at least %liMB)\n", freeKB/1024, estimatedMinimumKB/1024);
            return -1;
        }
    }

    return 0;
}

ShaderProgram *VolumeRectilinearIso::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeRectilinearIso")); }

void VolumeRectilinearIso::SetUniforms(const ShaderProgram *shader) const { VolumeRectilinear::SetUniforms(shader); }
