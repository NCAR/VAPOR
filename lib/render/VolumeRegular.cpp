#include <vapor/VolumeRegular.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>

using std::vector;

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeRegular> registration;

VolumeRegular::VolumeRegular(GLManager *gl) : VolumeAlgorithm(gl),
                                              _dataTexture(NULL),
                                              _missingTexture(NULL),
                                              _hasSecondData(false),
                                              _dataTexture2(NULL),
                                              _missingTexture2(NULL) {
    _initializeTexture(_dataTexture);
    _initializeTexture(_missingTexture);
}

VolumeRegular::~VolumeRegular() {
    if (_dataTexture)
        glDeleteTextures(1, &_dataTexture);
    if (_missingTexture)
        glDeleteTextures(1, &_missingTexture);
    if (_dataTexture2)
        glDeleteTextures(1, &_dataTexture2);
    if (_missingTexture2)
        glDeleteTextures(1, &_missingTexture2);
}

int VolumeRegular::LoadData(const Grid *grid) {
    printf("Loading data...\n");
    _dataDimensions = grid->GetDimensions();
    _hasSecondData = false;
    return _loadDataDirect(grid, _dataTexture, _missingTexture, &_hasMissingData);
}

int VolumeRegular::LoadSecondaryData(const Grid *grid) {
    printf("Loading secondary data...\n");
    _hasSecondData = false;
    if (_dataDimensions != grid->GetDimensions()) {
        return -1;
    }
    _initializeTexture(_dataTexture2);
    _initializeTexture(_missingTexture2);
    int ret = _loadDataDirect(grid, _dataTexture2, _missingTexture2, &_hasMissingData2);
    if (ret >= 0)
        _hasSecondData = true;
    return ret;
}

void VolumeRegular::DeleteSecondaryData() {
    _hasSecondData = false;
    if (_dataTexture2)
        glDeleteTextures(1, &_dataTexture2);
    if (_missingTexture2)
        glDeleteTextures(1, &_missingTexture2);
    _dataTexture2 = NULL;
    _missingTexture2 = NULL;
}

int VolumeRegular::_loadDataDirect(const Grid *grid, const unsigned int dataTexture, const unsigned int missingTexture, bool *hasMissingData) {
    const vector<size_t> dims = grid->GetDimensions();
    const size_t nVerts = dims[0] * dims[1] * dims[2];
    float *data = new float[nVerts];

    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) {
        data[i] = *dataIt;
    }

    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 0, 0, 0, 0, GL_RED, GL_FLOAT, NULL); // Fix driver bug with re-uploading large textures
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);

    *hasMissingData = grid->HasMissingData();
    if (*hasMissingData) {
        printf("Loading missing data...\n");
        const float missingValue = grid->GetMissingValue();
        unsigned char *missingMask = new unsigned char[nVerts];
        memset(missingMask, 0, nVerts);

        for (size_t i = 0; i < nVerts; i++)
            if (data[i] == missingValue)
                missingMask[i] = 255;

        glBindTexture(GL_TEXTURE_3D, missingTexture);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 0, 0, 0, 0, GL_RED, GL_UNSIGNED_BYTE, NULL); // Fix driver bug with re-uploading large textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, dims[0], dims[1], dims[2], 0, GL_RED, GL_UNSIGNED_BYTE, missingMask);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        delete[] missingMask;
    }

    delete[] data;
    return 0;
}

void VolumeRegular::_initializeTexture(unsigned int &texture) {
    if (texture)
        return;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

ShaderProgram *VolumeRegular::GetShader() const {
    return _glManager->shaderManager->GetShader("VolumeDVR");
}

void VolumeRegular::SetUniforms(int *nextTextureUnit) const {
    ShaderProgram *s = GetShader();

    glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
    glBindTexture(GL_TEXTURE_3D, _dataTexture);
    s->SetUniform("data", *nextTextureUnit);
    (*nextTextureUnit)++;

    s->SetUniform("hasMissingData", _hasMissingData);

    glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
    glBindTexture(GL_TEXTURE_3D, _missingTexture);
    s->SetUniform("missingMask", *nextTextureUnit);
    (*nextTextureUnit)++;

    if (_hasSecondData) {
        glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
        glBindTexture(GL_TEXTURE_3D, _dataTexture2);
        s->SetUniform("data2", *nextTextureUnit);
        (*nextTextureUnit)++;

        s->SetUniform("hasMissingData2", _hasMissingData2);

        glActiveTexture(GL_TEXTURE0 + *nextTextureUnit);
        glBindTexture(GL_TEXTURE_3D, _missingTexture2);
        s->SetUniform("missingMask2", *nextTextureUnit);
        (*nextTextureUnit)++;
    }
}

static VolumeAlgorithmRegistrar<VolumeRegularIso> registrationIso;

ShaderProgram *VolumeRegularIso::GetShader() const {
    return _glManager->shaderManager->GetShader("VolumeISO");
}

void VolumeRegularIso::SetUniforms(int *nextTextureUnit) const {
    VolumeRegular::SetUniforms(nextTextureUnit);
    GetShader()->SetUniform("useColormapData", _hasSecondData);
}
