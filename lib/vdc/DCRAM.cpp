#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <cassert>
#include <functional>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DCRAM.h>
#include <vapor/DCUtils.h>

using namespace VAPoR;

DCRAM::DCRAM()
{
    _dimsMap.clear();
    _coordVarsMap.clear();
    _meshMap.clear();
}

DCRAM::~DCRAM()
{
    for (const auto &it : _dataMap)
        delete [] it.second;
    _dataMap.clear();
}

int DCRAM::initialize(const vector<string> &paths, const std::vector<string> &options)
{
#ifdef DCRAM_GENERATE_TEST_DATA
    // Dimensions
    // ==================
    string particlesDim;
    string axisDim;
    auto   dimNames = ncdfc->GetDimNames();
    auto   dimLens = ncdfc->GetDims();
    auto   dimIsTimeVarying = ncdfc->GetDimsAreTimeVarying();

    _dimsMap["dimX"] = DC::Dimension("dimX", 32);
    _dimsMap["dimY"] = DC::Dimension("dimY", 32);
    _dimsMap["dimZ"] = DC::Dimension("dimZ", 32);
    
    // Coords
    // ==================
    const vector<bool> periodic(false);
    _coordVarsMap["coordX"] = CoordVar("coordX", "m", DC::FLOAT, periodic, /*axis=x*/0, /*uniformHint=*/true, {"dimX"}, /*timeDim*/"");
    _coordVarsMap["coordY"] = CoordVar("coordY", "m", DC::FLOAT, periodic, /*axis=y*/1, /*uniformHint=*/true, {"dimY"}, /*timeDim*/"");
    _coordVarsMap["coordZ"] = CoordVar("coordZ", "m", DC::FLOAT, periodic, /*axis=z*/2, /*uniformHint=*/true, {"dimZ"}, /*timeDim*/"");

    // Mesh
    // ==================
    vector<string> dims = {"dimX", "dimY", "dimZ"};
    vector<string> coords = {"coordX", "coordY", "coordZ"};
    DC::Mesh mesh("test_mesh", dims, coords);
    _meshMap[mesh.GetName()] = mesh;

    // Data Vars
    // ==================
    {
    vector<bool> periodic(3, false);
    _dataVarsMap["sphere"] = DC::DataVar("sphere", "", DC::FLOAT, periodic, mesh.GetName(), /*timeCoordVar*/"", DC::Mesh::NODE);
    _dataVarsMap["empty"]  = DC::DataVar("empty",  "", DC::FLOAT, periodic, mesh.GetName(), /*timeCoordVar*/"", DC::Mesh::NODE);
    _dataVarsMap["xval"]   = DC::DataVar("xval",   "", DC::FLOAT, periodic, mesh.GetName(), /*timeCoordVar*/"", DC::Mesh::NODE);
    }
#endif

    return 0;
}

void DCRAM::Test()
{
#ifdef DCRAM_GENERATE_TEST_DATA
    // ==========================
    // New data on existing grid
    // ==========================
    vector<bool> periodic(3, false);
    _dataVarsMap["sine"] = DC::DataVar("sine", "", DC::FLOAT, periodic, "test_mesh", /*timeCoordVar*/"", DC::Mesh::NODE);
    
    
    // ==========================
    // New data on new grid
    // ==========================
    
    _dimsMap["dim2X"] = DC::Dimension("dim2X", 64);
    _dimsMap["dim2Y"] = DC::Dimension("dim2Y", 64);
    _dimsMap["dim2Z"] = DC::Dimension("dim2Z", 64);
    
    // Coords
    // ==================
    _coordVarsMap["coord2X"] = CoordVar("coord2X", "m", DC::FLOAT, periodic, /*axis=x*/0, /*uniformHint=*/true, {"dim2X"}, /*timeDim*/"");
    _coordVarsMap["coord2Y"] = CoordVar("coord2Y", "m", DC::FLOAT, periodic, /*axis=y*/1, /*uniformHint=*/true, {"dim2Y"}, /*timeDim*/"");
    _coordVarsMap["coord2Z"] = CoordVar("coord2Z", "m", DC::FLOAT, periodic, /*axis=z*/2, /*uniformHint=*/true, {"dim2Z"}, /*timeDim*/"");

    // Mesh
    // ==================
    vector<string> dims = {"dim2X", "dim2Y", "dim2Z"};
    vector<string> coords = {"coord2X", "coord2Y", "coord2Z"};
    DC::Mesh mesh("test_mesh2", dims, coords);
    _meshMap[mesh.GetName()] = mesh;
    
    _dataVarsMap["grid"] = DC::DataVar("grid", "", DC::FLOAT, periodic, "test_mesh2", /*timeCoordVar*/"", DC::Mesh::NODE);
    _dataVarsMap["xval2"] = DC::DataVar("xval2", "", DC::FLOAT, periodic, "test_mesh2", /*timeCoordVar*/"", DC::Mesh::NODE);
    
    
    // ==========================
    // New data on curve grid
    // ==========================
    
    _dimsMap["curveDimX"] = DC::Dimension("curveDimX", 8);
    _dimsMap["curveDimY"] = DC::Dimension("curveDimY", 8);
    _dimsMap["curveDimZ"] = DC::Dimension("curveDimZ", 8);
    
    // Coords
    // ==================
    _coordVarsMap["curveCoordX"] = CoordVar("curveCoordX", "m", DC::FLOAT, periodic, /*axis=x*/0, /*uniformHint=*/false, {"curveDimX", "curveDimY"}, /*timeDim*/"");
    _coordVarsMap["curveCoordY"] = CoordVar("curveCoordY", "m", DC::FLOAT, periodic, /*axis=y*/1, /*uniformHint=*/false, {"curveDimX", "curveDimY"}, /*timeDim*/"");
    _coordVarsMap["curveCoordZ"] = CoordVar("curveCoordZ", "m", DC::FLOAT, periodic, /*axis=z*/2, /*uniformHint=*/false, {"curveDimX", "curveDimY", "curveDimZ"}, /*timeDim*/"");

    // Mesh
    // ==================
    vector<string> curveDims = {"curveDimX", "curveDimY", "curveDimZ"};
    vector<string> curveCoords = {"curveCoordX", "curveCoordY", "curveCoordZ"};
    DC::Mesh curveMesh("curveMesh", curveDims, curveCoords);
    _meshMap[curveMesh.GetName()] = curveMesh;
    
    _dataVarsMap["curveData"] = DC::DataVar("curveData", "", DC::FLOAT, periodic, curveMesh.GetName(), /*timeCoordVar*/"", DC::Mesh::NODE);
#endif
}


void DCRAM::AddDimension(const DC::Dimension &dim)
{
    _dimsMap[dim.GetName()] = dim;
}


void DCRAM::AddMesh(const DC::Mesh &mesh)
{
    _meshMap[mesh.GetName()] = mesh;
}


void DCRAM::AddCoordVar(const DC::CoordVar &var, const float *buf)
{
    _coordVarsMap[var.GetName()] = var;
    
    size_t size = 1;
    auto dimNames = var.GetDimNames();
    for (auto name : dimNames) {
        DC::Dimension dim;
        getDimension(name, dim);
        size *= dim.GetLength();
    }
    copyVarData(var, buf, size);
}


void DCRAM::AddDataVar(const DC::DataVar &var, const float *buf)
{
    _dataVarsMap[var.GetName()] = var;
    
    size_t size = 1;
    vector<size_t> dimLens;
    GetMeshDimLens(var.GetMeshName(), dimLens);
    for (auto len : dimLens)
        size *= len;
    copyVarData(var, buf, size);
}


void DCRAM::copyVarData(const DC::BaseVar &var, const float *buf, const size_t size)
{
    if (_dataMap.count(var.GetName()))
        delete [] _dataMap[var.GetName()];
    
    float *copy = new float[size];
    memcpy(copy, buf, sizeof(float)*size);
    _dataMap[var.GetName()] = copy;
}


bool DCRAM::getDimension(string dimname, DC::Dimension &dimension) const
{
    return getDimension(dimname, dimension, -1);
}

bool DCRAM::getDimension(string dimname, DC::Dimension &dimension, long ts) const
{
    map<string, DC::Dimension>::const_iterator itr;
    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);
    dimension = itr->second;

    if (dimension.IsTimeVarying()) {
#ifndef NDEBUG
        printf("WARNING: RAM dimension '%s' is time varying", dimname.c_str());
#endif
    }

    return true;
}

std::vector<string> DCRAM::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCRAM::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCRAM::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCRAM::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCRAM::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCRAM::getBaseVarInfo(string varname, DC::BaseVar &var) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr != _coordVarsMap.end()) {
        var = itr->second;
        return (true);
    }

    map<string, DC::DataVar>::const_iterator itr1 = _dataVarsMap.find(varname);
    if (itr1 != _dataVarsMap.end()) {
        var = itr1->second;
        return (true);
    }

    return (false);
}


std::vector<string> DCRAM::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}


std::vector<string> DCRAM::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

template<class T> bool DCRAM::_getAttTemplate(string varname, string attname, T &values) const
{
    DC::BaseVar var;
    bool status = getBaseVarInfo(varname, var);
    if (!status)
        return false;

    const std::map<string, Attribute> &atts = var.GetAttributes();
    
    if (!atts.count(attname))
        return false;
    
    atts.at(attname).GetValues(values);

    return true;
}

bool DCRAM::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCRAM::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCRAM::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCRAM::getAttNames(string varname) const
{
    if (varname.empty()) {
        return {};
    } else {
        DC::BaseVar var;
        bool        status = getBaseVarInfo(varname, var);
        if (!status) {
            return (vector<string>());
        }

        vector<string> names;

        const std::map<string, Attribute> &         atts = var.GetAttributes();
        std::map<string, Attribute>::const_iterator itr;
        for (itr = atts.begin(); itr != atts.end(); ++itr) { names.push_back(itr->first); }
        return (names);
    }
}

DC::XType DCRAM::getAttType(string varname, string attname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}

int DCRAM::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    VAssert(0);
    return -1;
}

int DCRAM::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level, long ts) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    bool ok = GetVarDimLens(varname, true, dims_at_level, ts);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}


int DCRAM::openVariableRead(size_t ts, string varname)
{
    int ret = _fakeVarsFileCounter++;
    _fdMap[ret] = varname;
    return ret;
}


int DCRAM::closeVariable(int fd)
{
    auto fdIt = _fdMap.find(fd);
    if (fdIt != _fdMap.end()) _fdMap.erase(fdIt);

    if (fd >= _fakeVarsFileCounterStart) {
        _fakeVarsFileCounter--;
        return 0;
    }

    assert(0);
    return -1;
}

// min -> max is inclusive

#include <glm/glm.hpp>
using glm::vec3;
using glm::vec2;

template<class T> int DCRAM::_readRegionTemplate(int fd, const vector<size_t> &min_, const vector<size_t> &max_, T *region)
{
    vector<size_t> min = min_;
    vector<size_t> max = max_;
    string         varname = _fdMap[fd];
//    printf("%s(%i(%s), %s, %s, %s*)\n", __func__, fd, varname.c_str(), S(min).c_str(), S(max).c_str(), TypeToChar(*region));
    
    bool fake = fd >= _fakeVarsFileCounterStart;

    const size_t w = 1 + max[0] - min[0];
    const size_t h = 1 + max[1] - min[1];
    const size_t d = 1 + max[2] - min[2];
    vec3 s(w,h,d);
#ifdef DCRAM_GENERATE_TEST_DATA
    vec3 c = s/2.f;
#endif
    
    auto var3d = [=](function<float(vec3 p)> f)
    {
        for (size_t z = min[2]; z <= max[2]; z++)
            for (size_t y = min[1]; y <= max[1]; y++)
                for (size_t x = min[0]; x <= max[0]; x++)
                    region[z*w*h+y*w+x] = f(vec3(x,y,z));
        return 0;
    };
    
    auto var2d = [=](function<float(vec2 p)> f)
    {
        for (size_t y = min[1]; y <= max[1]; y++)
            for (size_t x = min[0]; x <= max[0]; x++)
                region[y*w+x] = f(vec2(x,y));
        return 0;
    };
    
    if (_dataMap.count(varname)) {
        float *data = _dataMap[varname];
        vector<size_t> dimLens;
        GetVarDimLens(varname, true, dimLens);
        assert((dimLens.size() == min.size()) && (min.size() == max.size()));
        int nDims = dimLens.size();
        size_t rw=0, rh=0, sx=0, sy=0, sz=0;
        rw = dimLens[0];
        sx = min[0];
        if (nDims > 1) {
            rh = dimLens[1];
            sy = min[1];
        }
        if (nDims > 2) {
            sz = min[2];
        }
        
        if (nDims == 1) {
            for (size_t x = min[0]; x <= max[0]; x++)
                region[(x-sx)] = data[x];
        }
        else if (nDims == 2) {
            for (size_t y = min[1]; y <= max[1]; y++)
                for (size_t x = min[0]; x <= max[0]; x++)
                    region[(y-sy)*w+(x-sx)] = data[y*rw+x];
        }
        else if (nDims == 3) {
            for (size_t z = min[2]; z <= max[2]; z++)
                for (size_t y = min[1]; y <= max[1]; y++)
                    for (size_t x = min[0]; x <= max[0]; x++)
                        region[(z-sz)*w*h+(y-sy)*w+(x-sx)] = data[z*rw*rh+y*rw+x];
        }
        return 0;
    }

#ifdef DCRAM_GENERATE_TEST_DATA
    
    if (varname == "empty")
        return var3d([=](vec3 p) { return 0; });
    
    if (varname == "sphere")
        return var3d([=](vec3 p) { return glm::distance(p, c); });
    
    if (varname == "sine")
        return var3d([=](vec3 p) { return sinf(p.x/(float)w*36); });
    
    if (varname == "grid")
        return var3d([=](vec3 p) { return (int)p.x/8%2 == (int)p.y/8%2 == (int)p.z/8%2; });
    
    if (varname == "xval" || varname == "xval2")
        return var3d([=](vec3 p) { return p.x; });
    
    
    
    if (varname == "curveCoordX")
        return var2d([=](vec2 p) { return cosf(p.y/(float)h*M_PI) * (p.x+(w+1)); });
    
    if (varname == "curveCoordY")
        return var2d([=](vec2 p) { return sinf(p.y/(float)h*M_PI) * (p.x+(w+1)); });
    
    if (varname == "curveCoordZ")
        return var3d([=](vec3 p) { return p.z; });
    
    if (varname == "curveData")
        return var3d([=](vec3 p) { return glm::distance(p, c); });

#endif
    
    
    if (fake) {
        // Generate Regular Coords
        for (size_t i = min[0]; i <= max[0]; i++) region[i] = i / (double)(max[0] - 1);
        return 0;
    }
    
    VAssert(0);
    return -1;
}

bool DCRAM::variableExists(size_t ts, string varname, int, int) const
{
    bool found = false;

    for (const auto &it : _dataVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    for (const auto &it : _coordVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    for (const auto &it : _auxVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    return found;
}


std::vector<string> DCRAM::getAuxVarNames() const
{
    vector<string> names(_auxVarsMap.size());
    for (const auto &it : _auxVarsMap) names.push_back(it.first);
    return names;
}

bool DCRAM::getAuxVarInfo(string varname, DC::AuxVar &var) const
{
    const auto &it = _auxVarsMap.find(varname);
    if (it == _auxVarsMap.end()) return false;
    var = it->second;
    return true;
}
