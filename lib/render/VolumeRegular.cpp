#include <vapor/VolumeRegular.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>
#include <vapor/Progress.h>

using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeRegular> registration;

VolumeRegular::VolumeRegular(GLManager *gl, VolumeRenderer *renderer) : VolumeGLSL(gl, renderer), _hasSecondData(false)
{
    _data.Generate();
    _missing.Generate();
}

VolumeRegular::~VolumeRegular() {}

int VolumeRegular::LoadData(const Grid *grid)
{
    VolumeGLSL::LoadData(grid);
    auto tmp = grid->GetDimensions();
    _dataDimensions = {tmp[0], tmp[1], tmp[2]};
    _hasSecondData = false;
    return _loadDataDirect(grid, &_data, &_missing, &_hasMissingData);
}

int VolumeRegular::LoadSecondaryData(const Grid *grid)
{
    _hasSecondData = false;
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    if (_dataDimensions != dims) {
        Wasp::MyBase::SetErrMsg("Secondary (color mapped) variable has different grid from primary variable");
        return -1;
    }
    if (!_data2.Initialized()) _data2.Generate();
    if (!_missing2.Initialized()) _missing2.Generate();
    int ret = _loadDataDirect(grid, &_data2, &_missing2, &_hasMissingData2);
    if (ret >= 0) _hasSecondData = true;
    return ret;
}

void VolumeRegular::DeleteSecondaryData()
{
    _hasSecondData = false;
    _data2.Delete();
    _missing2.Delete();
}

int VolumeRegular::_loadDataDirect(const Grid *grid, Texture3D *dataTexture, Texture3D *missingTexture, bool *hasMissingData)
{
    auto         dims = grid->GetDimensions();
    const size_t nVerts = dims[0] * dims[1] * dims[2];
    float *      data = new float[nVerts];
    if (!data) {
        Wasp::MyBase::SetErrMsg("Could not allocate enough RAM to load data");
        return -1;
    }

    Progress::Start("Load volume data", nVerts, true);
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) {
        Progress::Update(i);
        if (Progress::Cancelled()) {
            delete[] data;
            return -1;
        }
        data[i] = *dataIt;
    }
    Progress::Finish();

    dataTexture->TexImage(GL_R32F, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, data);

    *hasMissingData = grid->HasMissingData();
    if (*hasMissingData) {
        const float    missingValue = grid->GetMissingValue();
        unsigned char *missingMask = new unsigned char[nVerts];
        memset(missingMask, 0, nVerts);

        for (size_t i = 0; i < nVerts; i++)
            if (data[i] == missingValue) missingMask[i] = 255;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        missingTexture->TexImage(GL_R8, dims[0], dims[1], dims[2], GL_RED, GL_UNSIGNED_BYTE, missingMask);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        delete[] missingMask;
    }

    delete[] data;
    return 0;
}

ShaderProgram *VolumeRegular::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeDVR")); }

void VolumeRegular::SetUniforms(const ShaderProgram *s) const
{
    s->SetUniform("hasMissingData", _hasMissingData);

    s->SetSampler("data", _data);
    s->SetSampler("missingMask", _missing);

    s->SetUniform("useColormapData", _hasSecondData);
    if (_hasSecondData) {
        s->SetUniform("hasMissingData2", _hasMissingData2);

        s->SetSampler("data2", _data2);
        s->SetSampler("missing2", _missing2);
    }
}

float VolumeRegular::GuestimateFastModeSpeedupFactor() const { return 5; }

std::string VolumeRegular::_addDefinitionsToShader(std::string shaderName) const
{
    if (_hasSecondData) shaderName += ":USE_SECOND_DATA";

    return shaderName;
}

static VolumeAlgorithmRegistrar<VolumeRegularIso> registrationIso;

ShaderProgram *VolumeRegularIso::GetShader() const { return _glManager->shaderManager->GetShader(_addDefinitionsToShader("VolumeIso")); }

void VolumeRegularIso::SetUniforms(const ShaderProgram *shader) const { VolumeRegular::SetUniforms(shader); }
