#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DCMelanie.h>
#include <vapor/DCUtils.h>

using namespace VAPoR;

DCMelanie::DCMelanie()
{
    _ncdfc = NULL;

    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
    _coordVarKeys.clear();
    _derivedVars.clear();
}

DCMelanie::~DCMelanie()
{
    if (_ncdfc) delete _ncdfc;

    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i]) delete _derivedVars[i];
    }
    _derivedVars.clear();
}

static void ReplaceAll(string *s, char a, char b)
{
    for (auto &c : *s) c = c == a ? b : c;
}

int DCMelanie::initialize(const vector<string> &paths, const std::vector<string> &options)
{
    NetCDFCFCollection *ncdfc = new NetCDFCFCollection();

    // Initialize the NetCDFCFCollection class.
    //
    int rc = ncdfc->Initialize(paths);
    if (rc < 0) {
        SetErrMsg("Failed to initialize netCDF data collection for reading");
        return (-1);
    }

    // Use UDUnits for unit conversion
    //
    rc = _udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", _udunits.GetErrMsg().c_str());
        return (-1);
    }

    auto particleAttributes = ncdfc->GetVariableNames(1, true);
    auto particleVecs = ncdfc->GetVariableNames(2, true);

    for (auto &n : particleAttributes) n = sanitizeVarName(n);
    for (auto &n : particleVecs) n = sanitizeVarName(n);

    string         particlePositions = "Position";
    vector<string> coords = {
        particlePositions + "_x",
        particlePositions + "_y",
        particlePositions + "_z",
    };
    {
        auto positionPos = find(particleVecs.begin(), particleVecs.end(), particlePositions);
        VAssert(positionPos != particleVecs.end());
        particleVecs.erase(positionPos);
    }

    //    printf("Particle Coordinates\n");
    //    printf("\t%s\n", particlePositions.c_str());
    //    printf("Particle Vecs\n");
    //    for (auto s : particleVecs)
    //        printf("\t%s\n", s.c_str());
    //    printf("Particle Attributes\n");
    //    for (auto s : particleAttributes)
    //        printf("\t%s\n", s.c_str());


    // Dimensions
    // ==================
    const string particlesDim = "phony_dim_0";
    const string axesDim = "phony_dim_1";
    auto         dimNames = ncdfc->GetDimNames();
    auto         dimLens = ncdfc->GetDims();
    string       timeDim = "";
    assert(dimNames.size() == dimLens.size());
    for (int i = 0; i < dimNames.size(); i++) _dimsMap[dimNames[i]] = DC::Dimension(dimNames[i], dimLens[i]);

    // Coord Vars
    // ==================
    string timeCoordVar = "";
    {
        vector<bool> periodic(false);
        for (int i = 0; i < 3; i++) {
            auto c = coords[i];
            auto axis = i;
            _coordVarsMap[c] = CoordVar(c, "", DC::FLOAT, periodic, axis, false, {"phony_dim_0"}, timeDim);
        }
    }

    // Aux Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (auto v : {nodeFaceVar, faceNodeVar}) _auxVarsMap[v] = AuxVar(v, "", DC::INT32, "", vector<size_t>(), periodic, {particlesDim});
    }

    // Mesh
    // ==================
    DC::Mesh mesh("particles", 1, 1, "phony_dim_0", "phony_dim_0", coords);
    mesh.SetNodeFaceVar(nodeFaceVar);
    mesh.SetFaceNodeVar(faceNodeVar);
    _meshMap[mesh.GetName()] = mesh;


    // Data Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (string v : particleVecs) {
            auto dims = ncdfc->GetDimNames(getOriginalVarName(v));
            if (STLUtils::Contains(dims, axesDim)) {
                for (auto axis : {"_x", "_y", "_z"}) _dataVarsMap[v + axis] = DC::DataVar(v + axis, "", DC::FLOAT, periodic, mesh.GetName(), timeCoordVar, DC::Mesh::NODE);
            } else {
                _dataVarsMap[v] = DC::DataVar(v, "", DC::FLOAT, periodic, mesh.GetName(), timeCoordVar, DC::Mesh::NODE);
            }
        }
    }

    _ncdfc = ncdfc;

    return (0);
}


bool DCMelanie::getDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCMelanie::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCMelanie::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCMelanie::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCMelanie::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCMelanie::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCMelanie::getBaseVarInfo(string varname, DC::BaseVar &var) const
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


std::vector<string> DCMelanie::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}


std::vector<string> DCMelanie::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

template<class T> bool DCMelanie::_getAttTemplate(string varname, string attname, T &values) const
{
    //    printf("%s(%s, %s, %s&)\n", __func__, varname.c_str(), attname.c_str(), TypeToChar(values));

    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (status);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (status);

    att.GetValues(values);

    return (true);
}

bool DCMelanie::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCMelanie::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCMelanie::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCMelanie::getAttNames(string varname) const
{
    //    printf("%s(%s) = ", __func__, varname.c_str());

    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) {
        //        printf("{} (ERR)\n");
        return (vector<string>());
    }

    vector<string> names;

    const std::map<string, Attribute> &         atts = var.GetAttributes();
    std::map<string, Attribute>::const_iterator itr;
    for (itr = atts.begin(); itr != atts.end(); ++itr) { names.push_back(itr->first); }

    //    printf("%s\n", ToStr(names).c_str());
    return(names);
}

DC::XType DCMelanie::getAttType(string varname, string attname) const
{
    //    printf("%s(%s, %s)\n", __func__, varname.c_str(), attname.c_str());
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}

int DCMelanie::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    VAssert(0);
    bool ok = GetVarDimLens(varname, true, dims_at_level, 0);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}


int DCMelanie::openVariableRead(size_t ts, string varname)
{
    //    printf("%s(%li, %s) = ", __func__, ts, varname.c_str());

    if (STLUtils::Contains(fakeVars, varname)) {
        int ret = fakeVarsFileCounter++;
        //        printf("(fake) %i\n", ret);
        _fdMap[ret] = varname;
        return ret;
    }

    string ncdfName = varname;
    if (STLUtils::EndsWith(ncdfName, "_x") || STLUtils::EndsWith(ncdfName, "_y") || STLUtils::EndsWith(ncdfName, "_z")) {
        ncdfName = ncdfName.substr(0, ncdfName.length() - 2);
        //        printf("<removing _axis>\n\t");
        //        printf("%s(%li, %s) = ", __func__, ts, ncdfName.c_str());
    }

    ncdfName = getOriginalVarName(ncdfName);

    int aux = _ncdfc->OpenRead(ts, ncdfName);
    if (!(aux < 0)) {
        FileTable::FileObject *f = new FileTable::FileObject(ts, ncdfName, 0, 0, aux);
        aux = _fileTable.AddEntry(f);
    }

    //    printf("%i\n", aux);
    _fdMap[aux] = varname;
    return aux;
}


int DCMelanie::closeVariable(int fd)
{
    //    printf("%s(%i)\n", __func__, fd);
    _fdMap.erase(_fdMap.find(fd));

    if (fd >= fakeVarsFileCounterStart) {
        fakeVarsFileCounter--;
        return 0;
    }

    DC::FileTable::FileObject *w = _fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    int rc = _ncdfc->Close(aux);

    _fileTable.RemoveEntry(fd);

    return (rc);
}

// min -> max is inclusive

template<class T> int DCMelanie::_readRegionTemplate(int fd, const vector<size_t> &min_, const vector<size_t> &max_, T *region)
{
    vector<size_t> min = min_;
    vector<size_t> max = max_;
    string         varname = _fdMap[fd];
    //    printf("%s(%i(%s), %s, %s, %s*)\n", __func__, fd, varname.c_str(), ToStr(min).c_str(), ToStr(max).c_str(), TypeToChar(*region));

    VAssert(min.size() == 1);

    bool fake = fd >= fakeVarsFileCounterStart;

    if (fake) {
        //        printf("\t (Fake)\n");
        for (size_t i = min[0]; i <= max[0]; i++) region[i] = i / (double)(max[0] - 1);
        return 0;
    }

    int axis = -1;
    if (STLUtils::EndsWith(varname, "_x")) axis = 0;
    if (STLUtils::EndsWith(varname, "_y")) axis = 1;
    if (STLUtils::EndsWith(varname, "_z")) axis = 2;
    if (axis != -1) {
        //        printf("\t Read data from header\n");
        //        for (size_t i = min[0]; i <= max[0]; i++)
        //            region[i] = Particle_Position[i*3+axis];
        //        return 0;


        //        min.push_back(axis);
        //        max.push_back(axis);
        min.push_back(min[0]);
        max.push_back(max[0]);
        min[0] = axis;
        max[0] = axis;

        //        printf("\t %s(%i(%s), %s, %s, %s*)\n", __func__, fd, varname.substr(0,varname.length()-2).c_str(), ToStr(min).c_str(), ToStr(max).c_str(), TypeToChar(*region));
    }

    FileTable::FileObject *w = (FileTable::FileObject *)_fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    vector<size_t> ncdf_start = min;
    reverse(ncdf_start.begin(), ncdf_start.end());

    vector<size_t> ncdf_max = max;
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    return (_ncdfc->Read(ncdf_start, ncdf_count, region, aux));
}

bool DCMelanie::variableExists(size_t ts, string varname, int, int) const
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

    //    if (found == false)
    //        printf("WARNING %s(%li, %s) = %s\n", __func__, ts, varname.c_str(), found?"true":"false");
    return found;
}


std::vector<string> DCMelanie::getAuxVarNames() const
{
    vector<string> names(_auxVarsMap.size());
    for (const auto &it : _auxVarsMap) names.push_back(it.first);
    return names;
}

bool DCMelanie::getAuxVarInfo(string varname, DC::AuxVar &var) const
{
    const auto &it = _auxVarsMap.find(varname);
    if (it == _auxVarsMap.end()) return false;
    var = it->second;
    return true;
}

string DCMelanie::sanitizeVarName(const string &name)
{
    assert(_sanitizedToOriginalMap.count(name) == 0);

    string sanitizedName = name;
    ReplaceAll(&sanitizedName, ' ', '_');
    _sanitizedToOriginalMap[sanitizedName] = name;

    return sanitizedName;
}

string DCMelanie::getOriginalVarName(const string &name) const
{
    auto it = _sanitizedToOriginalMap.find(name);
    if (it == _sanitizedToOriginalMap.end()) return name;
    return it->second;
}
