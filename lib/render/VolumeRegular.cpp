#include <vapor/VolumeRegular.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>

using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeRegular> registration;

VolumeRegular::VolumeRegular(GLManager *gl) :
VolumeAlgorithm(gl),
_hasSecondData(false)
{
    _data.Generate();
    _missing.Generate();
}

VolumeRegular::~VolumeRegular()
{
}

int VolumeRegular::LoadData(const Grid *grid)
{
    printf("Loading data...\n");
    _dataDimensions = grid->GetDimensions();
    _hasSecondData = false;
    return _loadDataDirect(grid, &_data, &_missing, &_hasMissingData);
}

int VolumeRegular::LoadSecondaryData(const Grid *grid)
{
    printf("Loading secondary data...\n");
    _hasSecondData = false;
    if (_dataDimensions != grid->GetDimensions()) {
        return -1;
    }
    if (!_data2.Initialized()) _data2.Generate();
    if (!_missing2.Initialized()) _missing2.Generate();
    int ret = _loadDataDirect(grid, &_data2, &_missing2, &_hasMissingData2);
    if (ret >= 0)
        _hasSecondData = true;
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
    const vector<size_t> dims = grid->GetDimensions();
    const size_t nVerts = dims[0]*dims[1]*dims[2];
    float *data = new float[nVerts];
    
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) {
        data[i] = *dataIt;
    }
    
    dataTexture->TexImage(GL_R32F, dims[0], dims[1], dims[2], GL_RED, GL_FLOAT, data);
    
    *hasMissingData = grid->HasMissingData();
    if (*hasMissingData) {
        printf("Loading missing data...\n");
        const float missingValue = grid->GetMissingValue();
        unsigned char *missingMask = new unsigned char[nVerts];
        memset(missingMask, 0, nVerts);
        
        for (size_t i = 0; i < nVerts; i++)
            if (data[i] == missingValue)
                missingMask[i] = 255;
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        missingTexture->TexImage(GL_R8, dims[0], dims[1], dims[2], GL_RED, GL_UNSIGNED_BYTE, missingMask);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        
        delete [] missingMask;
    }
    
    delete [] data;
    return 0;
}

ShaderProgram *VolumeRegular::GetShader() const
{
    return _glManager->shaderManager->GetShader("VolumeDVR");
}

void VolumeRegular::SetUniforms(int *nextTextureUnit) const
{
    ShaderProgram *s = GetShader();
    
    glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
    _data.Bind();
    s->SetUniform("data", *nextTextureUnit);
    (*nextTextureUnit)++;
    
    s->SetUniform("hasMissingData", _hasMissingData);
    
    glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
    _missing.Bind();
    s->SetUniform("missingMask", *nextTextureUnit);
    (*nextTextureUnit)++;
    
    if (_hasSecondData) {
        glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
        _data2.Bind();
        s->SetUniform("data2", *nextTextureUnit);
        (*nextTextureUnit)++;
        
        s->SetUniform("hasMissingData2", _hasMissingData2);
        
        glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
        _missing2.Bind();
        s->SetUniform("missingMask2", *nextTextureUnit);
        (*nextTextureUnit)++;
    }
}

float VolumeRegular::GuestimateFastModeSpeedupFactor() const
{
    return 5;
}


static VolumeAlgorithmRegistrar<VolumeRegularIso> registrationIso;

ShaderProgram *VolumeRegularIso::GetShader() const
{
    return _glManager->shaderManager->GetShader("VolumeISO");
}

void VolumeRegularIso::SetUniforms(int *nextTextureUnit) const
{
    VolumeRegular::SetUniforms(nextTextureUnit);
    GetShader()->SetUniform("useColormapData", _hasSecondData);
}
