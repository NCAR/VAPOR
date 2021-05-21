#include <vapor/DerivedParticleDensity.h>
#include <vapor/STLUtils.h>
#include <vapor/DataMgrUtils.h>
#include <float.h>
#include <glm/glm.hpp>

using namespace VAPoR;
using glm::ivec3;
using glm::vec3;

// ===================================
//       DerivedParticleDensity
// ===================================

DerivedParticleDensity::DerivedParticleDensity(string varName, DC *dc, string meshName, DataMgr *dataMgr) : DerivedDataVar(varName), _dc(dc), _meshName(meshName), _dataMgr(dataMgr) {}

int DerivedParticleDensity::Initialize()
{
    auto   dataVarNames = _dataMgr->GetDataVarNames(3);
    string dataVar;
    for (auto dv : dataVarNames) {
        vector<string> coordNames;
        _dataMgr->GetVarCoordVars(dv, true, coordNames);
        if (STLUtils::Contains(coordNames, string("Position_x")) && STLUtils::Contains(coordNames, string("Position_y")) && STLUtils::Contains(coordNames, string("Position_z"))) {
            dataVar = dv;
            break;
        }
    }

    if (dataVar.empty()) {
        assert(0);
        return -1;
    }

    _dataVar = dataVar;

    return 0;
}

int DerivedParticleDensity::OpenVariableRead(size_t ts, int level, int lod)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);
    return _fileTable.AddEntry(f);
}

int DerivedParticleDensity::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);
    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return -1;
    }
    _fileTable.RemoveEntry(fd);
    delete f;
    return 0;
}

int DerivedParticleDensity::ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);
    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return -1;
    }


    vector<size_t> dims;
    _dataMgr->GetDimLens(_derivedVarName, dims, f->GetTS());
    if (dims.size() != 3) {
        assert(0);
        return -1;
    }

    Grid *g = _dataMgr->GetVariable(f->GetTS(), _dataVar, -1, -1, true);
    if (!g) {
        assert(0);
        return -1;
    }

    size_t       realNP = SIZE_T_MAX;
    vector<long> values;
    _dc->GetAtt("", "realNP", values);
    if (!values.empty()) realNP = values[0];

    int    xd = dims[0], yd = dims[1], zd = dims[2];
    float *data = new float[xd * yd * zd];
    compute(g, data, xd, yd, zd, realNP);
    delete g;

    //    assert(max[0]-min[0]+1 == xd);
    //    assert(max[1]-min[1]+1 == yd);
    //    assert(max[2]-min[2]+1 == zd);

    //    for (int z = 0; z < zd; z++)
    //        for (int y = 0; y < yd; y++)
    //            for (int x = 0; x < xd; x++)
    //                if (x > 10 && x < 20 && y > 10 && y < 20 && z > 10 && z < 20)
    //                    data[z*yd*xd + y*xd + x] = 1;
    //                else
    //                    data[z*yd*xd + y*xd + x] = 0;

    int oxd = 1 + max[0] - min[0], oyd = 1 + max[1] - min[1], ozd = 1 + max[2] - min[2];
    //    printf("Read(%s, %s, %s)\n", C(_derivedVarName), C(min), C(max));

    // Copy subvolume
    for (int z = 0; z < ozd; z++)
        for (int y = 0; y < oyd; y++)
            for (int x = 0; x < oxd; x++) region[z * oyd * oxd + y * oxd + x] = data[(min[2] + z) * yd * xd + (min[1] + y) * xd + (min[0] + x)];
    //                region[z*oyd*oxd + y*oxd + x] = y % 3 ? 0 : 1;

    //    for (size_t i = 0; i < nbins; i++)
    //        region[i] = data[i];

    delete[] data;
    return 0;
}

void DerivedParticleDensity::compute(Grid *inGrid, float *output, int xd, int yd, int zd, size_t realNP) const
{
    DblArr3 minud, maxud;
    inGrid->GetUserExtents(minud, maxud);
    vec3 minu(minud[0], minud[1], minud[2]);
    vec3 maxu(maxud[0], maxud[1], maxud[2]);
    vec3 diffu = maxu - minu;

    vec3    binDimsF(xd, yd, zd);
    ivec3   binDimsI(xd, yd, zd);
    size_t  nbins = xd * yd * zd;
    size_t *bins = new size_t[nbins];
    for (size_t i = 0; i < nbins; i++) bins[i] = 0;

    auto it = inGrid->ConstCoordBegin();
    auto end = inGrid->ConstCoordEnd();
    for (size_t j = 0; it != end && j < realNP; ++it, ++j) {
        vec3  c((*it)[0], (*it)[1], (*it)[2]);
        vec3  t = (c - minu) / diffu;
        ivec3 i = t * binDimsF;
        i = glm::clamp(i, ivec3(0), binDimsI - ivec3(1, 1, 1));
        bins[i.z * yd * xd + i.y * xd + i.x]++;
    }

    for (size_t i = 0; i < nbins; i++) output[i] = bins[i];

    delete[] bins;
}

int DerivedParticleDensity::ReadRegionBlock(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region) { return ReadRegion(fd, min, max, region); }

int DerivedParticleDensity::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    DC::Mesh mesh;
    _dataMgr->GetMesh(_meshName, mesh);
    DC::Dimension dim;
    for (auto dimName : mesh.GetDimNames()) {
        _dataMgr->GetDimension(dimName, dim, -1);
        dims_at_level.push_back(dim.GetLength());
    }

    //    dims_at_level = {128, 128, 128};
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);
    return 0;
}

bool DerivedParticleDensity::VariableExists(size_t ts, int reflevel, int lod) const { return true; }

bool DerivedParticleDensity::GetBaseVarInfo(DC::BaseVar &var) const
{
    DC::DataVar dvar;
    GetDataVarInfo(dvar);
    var = dvar;
    return true;
}

bool DerivedParticleDensity::GetDataVarInfo(DC::DataVar &dvar) const
{
    if (_dataVar.empty()) return false;

    dvar.SetName(_derivedVarName);
    dvar.SetMeshName(_meshName);
    dvar.SetUnits("");
    dvar.SetXType(DC::FLOAT);
    dvar.SetWName("");
    dvar.SetCRatios(vector<size_t>());
    dvar.SetPeriodic(vector<bool>(3, false));

    //    dvar.SetHasMissing(true);
    //    dvar.SetMissingValue(-1);

    DC::DataVar dv;
    _dc->GetDataVarInfo(_dataVar, dv);
    dvar.SetTimeCoordVar(dv.GetTimeCoordVar());

    return true;
}

vector<string> DerivedParticleDensity::GetInputs() const { return {_dataVar}; }


// ===================================
//       DerivedParticleAverage
// ===================================


DerivedParticleAverage::DerivedParticleAverage(string varName, DC *dc, string meshName, DataMgr *dataMgr, string inputVar) : DerivedParticleDensity(varName, dc, meshName, dataMgr)
{
    _dataVar = inputVar;
}

void DerivedParticleAverage::compute(Grid *inGrid, float *output, int xd, int yd, int zd, size_t realNP) const
{
    DblArr3 minud, maxud;
    inGrid->GetUserExtents(minud, maxud);
    vec3 minu(minud[0], minud[1], minud[2]);
    vec3 maxu(maxud[0], maxud[1], maxud[2]);
    vec3 diffu = maxu - minu;

    vec3    binDimsF(xd, yd, zd);
    ivec3   binDimsI(xd, yd, zd);
    size_t  nbins = xd * yd * zd;
    size_t *bins = new size_t[nbins];
    double *accum = new double[nbins];
    for (size_t i = 0; i < nbins; i++) {
        bins[i] = 0;
        accum[i] = 0;
    }

    auto it = inGrid->ConstCoordBegin();
    auto dit = inGrid->cbegin();
    auto end = inGrid->ConstCoordEnd();
    for (size_t j = 0; it != end && j < realNP; ++it, ++dit, ++j) {
        vec3  c((*it)[0], (*it)[1], (*it)[2]);
        vec3  t = (c - minu) / diffu;
        ivec3 i = t * binDimsF;
        i = glm::clamp(i, ivec3(0), binDimsI - ivec3(1, 1, 1));
        size_t index = i.z * yd * xd + i.y * xd + i.x;
        bins[index]++;
        accum[index] += *dit;
    }

    for (size_t i = 0; i < nbins; i++) output[i] = accum[i] / (float)bins[i];

    delete[] accum;
    delete[] bins;
}


// ===================================
//        DerivedCoordVar1DSpan
// ===================================


DerivedCoordVar1DSpan::DerivedCoordVar1DSpan(string derivedVarName, DC *dc, string dimName, int axis, string units, float minExt, float maxExt)
: DerivedCoordVar_CF1D(derivedVarName, dc, dimName, axis, units), _dc(dc), _minExt(minExt), _maxExt(maxExt)
{
    VAssert(maxExt >= minExt);
}

DerivedCoordVar1DSpan::DerivedCoordVar1DSpan(string derivedVarName, DC *dc, string dimName, int axis, string units, string inputCoordVar)
: DerivedCoordVar1DSpan(derivedVarName, dc, dimName, axis, units, 0, 1)
{
    _inputCoordVar = inputCoordVar;
}

int DerivedCoordVar1DSpan::ReadRegion(int fd, const std::vector<size_t> &min, const std::vector<size_t> &max, float *region)
{
    VAssert(min.size() == 1);
    VAssert(max.size() == 1);
    long ts = -1;

    if (!_derivedVarName.empty()) {
        if (!_dc->IsCoordVar(_inputCoordVar)) {
            assert(0);
            return -1;
        }

        DC::CoordVar cvInfo;
        _dc->GetCoordVarInfo(_inputCoordVar, cvInfo);
        auto dims = cvInfo.GetDimNames();
        if (dims.size() != 1) {
            assert(0);
            return -1;
        }

        DC::CoordVar myInfo;
        GetCoordVarInfo(myInfo);
        if (myInfo.GetAxis() != cvInfo.GetAxis()) {
            assert(0);
            return -1;
        }

        DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);
        if (!f) {
            assert(0);
            return -1;
        }

        DC::Dimension dim;
        _dc->GetDimension(dims[0], dim, f->GetTS());
        int fd = _dc->OpenVariableRead(f->GetTS(), _inputCoordVar);
        if (fd < 0) {
            assert(0);
            return -1;
        }
        size_t nCoords = dim.GetLength();
        float *coords = new float[nCoords];
        ts = f->GetTS();

        _dc->ReadRegion(fd, {0}, {nCoords - 1}, coords);
        float min = FLT_MAX;
        float max = -FLT_MAX;
        for (size_t i = 0; i < nCoords; i++) {
            min = std::min(min, coords[i]);
            max = std::max(max, coords[i]);
        }
        _minExt = min;
        _maxExt = max;

        _dc->CloseVariable(fd);
        delete[] coords;
    }

    DC::CoordVar cv;
    GetCoordVarInfo(cv);
    auto dims = cv.GetDimNames();
    if (dims.size() != 1) {
        assert(0);
        return -1;
    }
    DC::Dimension dim;
    _dc->GetDimension(dims[0], dim, ts);
    size_t dimLen = dim.GetLength();

    size_t n = 1 + max[0] - min[0];
    if (n == 1) {
        *region = (_maxExt + _minExt) / 2.f;
    } else {
        float diff = _maxExt - _minExt;
        for (size_t i = 0; i < n; i++) {
            size_t ri = i + min[0];
            float  t = ri / (float)(dimLen - 1);
            region[i] = t * diff + _minExt;
        }
    }

    return 0;
}

vector<string> DerivedCoordVar1DSpan::GetInputs() const
{
    if (!_inputCoordVar.empty())
        return {_inputCoordVar};
    else
        return {};
}
