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
#include <vapor/DCCF.h>
#include <vapor/DCUtils.h>

using namespace VAPoR;
using namespace std;

namespace {

#ifdef UNUSED_FUNCTION
// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}
#endif

};    // namespace

DCCF::DCCF()
{
    _ncdfc = NULL;

    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _auxVarsMap.clear();
    _meshMap.clear();
    _derivedVars.clear();
}

DCCF::~DCCF()
{
    if (_ncdfc) delete _ncdfc;

    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i]) delete _derivedVars[i];
    }
    _derivedVars.clear();
}

int DCCF::initialize(const vector<string> &paths, const std::vector<string> &options)
{
    if (_ncdfc) delete _ncdfc;
    _ncdfc = nullptr;

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

    //
    //  Get the dimensions of the grid.
    //	Initializes members: _dimsMap
    //
    rc = initDimensions(ncdfc, _dimsMap);
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    rc = initCoordinates(ncdfc, _coordVarsMap);
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    //
    // Identify auxilliary variables.
    //
    rc = initAuxilliaryVars(ncdfc, _auxVarsMap);
    if (rc < 0) return (-1);

    //
    // Identify meshes.
    //
    rc = initMesh(ncdfc, _meshMap);
    if (rc < 0) return (-1);


    //
    // Identify data variables.
    //
    rc = initDataVars(ncdfc, _dataVarsMap);
    if (rc < 0) return (-1);

    _ncdfc = ncdfc;

    return (0);
}

bool DCCF::getDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCCF::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCCF::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCCF::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCCF::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCCF::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCCF::getAuxVarInfo(string varName, DC::AuxVar &auxVar) const
{
    map<string, DC::AuxVar>::const_iterator itr;

    itr = _auxVarsMap.find(varName);
    if (itr == _auxVarsMap.end()) { return (false); }

    auxVar = itr->second;
    return (true);
}

bool DCCF::getBaseVarInfo(string varname, DC::BaseVar &var) const
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

std::vector<string> DCCF::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCCF::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCCF::getAuxVarNames() const
{
    map<string, DC::AuxVar>::const_iterator itr;

    vector<string> names;
    for (itr = _auxVarsMap.begin(); itr != _auxVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

template<class T> bool DCCF::_getAttTemplate(string varname, string attname, T &values) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (status);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (status);

    att.GetValues(values);

    return (true);
}

bool DCCF::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCCF::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCCF::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCCF::getAttNames(string varname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (vector<string>());

    vector<string> names;

    const std::map<string, Attribute> &         atts = var.GetAttributes();
    std::map<string, Attribute>::const_iterator itr;
    for (itr = atts.begin(); itr != atts.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

DC::XType DCCF::getAttType(string varname, string attname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}

int DCCF::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

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

int DCCF::openVariableRead(size_t ts, string varname)
{
    int aux = _ncdfc->OpenRead(ts, varname);
    if (aux < 0) return (aux);

    FileTable::FileObject *f = new FileTable::FileObject(ts, varname, 0, 0, aux);
    return (_fileTable.AddEntry(f));
}

int DCCF::closeVariable(int fd)
{
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

template<class T> int DCCF::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
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

bool DCCF::variableExists(size_t ts, string varname, int, int) const { return (_ncdfc->VariableExists(ts, varname)); }

bool DCCF::_isUniform(NetCDFCFCollection *ncdfc, string varname)
{
    vector<size_t> dims = ncdfc->GetDims(varname);

    if (dims.size() != 1) return (false);

    if (dims[0] < 2) return (true);

    vector<float> buf(dims[0]);

    bool enabled = EnableErrMsg(false);
    int  fd = ncdfc->OpenRead(0, varname);
    if (fd < 0) {
        EnableErrMsg(enabled);
        return (false);
    }
    int rc = ncdfc->Read(buf.data(), fd);
    if (rc < 0) {
        EnableErrMsg(enabled);
        return (false);
    }
    ncdfc->Close(fd);
    EnableErrMsg(enabled);

    VAssert(buf.size() >= 2);

    float epsilon = 0.0001;
    float delta = buf[1] - buf[0];
    for (int i = 1; i < buf.size() - 1; i++) {
        if (!Wasp::NearlyEqual((buf[i + 1] - buf[i]), delta, epsilon)) { return (false); }
    }

    return (true);
}

int DCCF::addCoordvars(NetCDFCFCollection *ncdfc, const vector<string> &cvars, std::map<string, DC::CoordVar> &coordVarsMap)
{
    for (int i = 0; i < cvars.size(); i++) {
        // Get dimension names
        //
        vector<string> dimnames = ncdfc->GetDimNames(cvars[i]);
        reverse(dimnames.begin(), dimnames.end());    // DC order

        string time_dim_name;
        if (ncdfc->IsTimeVarying(cvars[i])) {
            time_dim_name = dimnames.back();
            dimnames.pop_back();
        }

        int axis = 0;
        if (ncdfc->IsLonCoordVar(cvars[i])) {
            axis = 0;
        } else if (ncdfc->IsLatCoordVar(cvars[i])) {
            axis = 1;
        } else if (ncdfc->IsVertCoordVar(cvars[i])) {
            axis = 2;
        } else if (ncdfc->IsTimeCoordVar(cvars[i])) {
            axis = 3;
        } else {
            continue;    // should this be a error condition?
        }

        string units;
        ncdfc->GetAtt(cvars[i], "units", units);
        if (!_udunits.ValidUnit(units)) { units = ""; }

        // See if the variable has uniform spacing. If so, set the uniform hint
        //
        bool uniformHint = _isUniform(ncdfc, cvars[i]);

        // Finally, add the variable to coordVarsMap.
        //
        vector<bool> periodic(false);
        coordVarsMap[cvars[i]] = CoordVar(cvars[i], units, DC::FLOAT, periodic, axis, uniformHint, dimnames, time_dim_name);

        int rc = DCUtils::CopyAtt(*ncdfc, cvars[i], coordVarsMap[cvars[i]]);
        if (rc < 0) return (-1);
    }

    return (0);
}

int DCCF::initCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap)
{
    coordVarsMap.clear();

    // Set up the horizontal coordinate variables
    //
    int rc = _initHorizontalCoordinates(ncdfc, coordVarsMap);
    if (rc < 0) { return (-1); }

    // Set up the vertical coordinate variables.
    //
    rc = _initVerticalCoordinates(ncdfc, coordVarsMap);
    if (rc < 0) { return (-1); }

    // Set up user time coordinate variable.
    //
    rc = _initTimeCoordinates(ncdfc, coordVarsMap);
    if (rc < 0) { return (-1); }

    return (0);
}

int DCCF::_initHorizontalCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap)
{
    vector<string> latVars = ncdfc->GetLatCoordVars();
    int            rc = addCoordvars(ncdfc, latVars, coordVarsMap);
    if (rc < 0) return (-1);

    vector<string> lonVars = ncdfc->GetLonCoordVars();
    rc = addCoordvars(ncdfc, lonVars, coordVarsMap);
    if (rc < 0) return (-1);

    return (0);
}

//
//
int DCCF::_initVerticalCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap)
{
    vector<string> vertVars = ncdfc->GetVertCoordVars();
    int            rc = addCoordvars(ncdfc, vertVars, coordVarsMap);

    return (rc);
}

int DCCF::_initTimeCoordinates(NetCDFCFCollection *ncdfc, std::map<string, DC::CoordVar> &coordVarsMap)
{
    // add native time coordinate variables
    //
    int rc = addCoordvars(ncdfc, ncdfc->GetTimeCoordVars(), coordVarsMap);
    if (rc < 0) return (-1);

    return (0);
}

// Get Space and time dimensions from CF data set. Initialize
// _dimsMap
//
int DCCF::initDimensions(NetCDFCFCollection *ncdfc, std::map<string, DC::Dimension> &dimsMap)
{
    dimsMap.clear();

    // Get dimension names and lengths for all dimensions in the
    // CF data set.
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    vector<size_t> dimlens = ncdfc->GetDims();
    VAssert(dimnames.size() == dimlens.size());

    //
    // Find all dimensions and their associated axis (X,Y,Z,T). From
    // CF 1.6:
    //
    // All of a variable's dimensions that are latitude, longitude, vertical,
    // or time dimensions (see Section 1.2, “Terminology”) must have
    // corresponding coordinate variables, i.e., one-dimensional variables
    // with the same name as the dimension (see examples in Chapter 4,
    // Coordinate Types ). This is the only method of associating dimensions
    // with coordinates that is supported by COARDS.
    //
    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        dimsMap[dimnames[i]] = dim;
    }

    return (0);
}

// Given a data variable name return the variable's dimensions and
// associated coordinate variables. The coordinate variable names
// returned is for the derived coordinate variables expressed in
// Cartographic coordinates, not the native geographic coordinates
// found in the CF file.
//
// The order of the returned vectors
// is significant.
//
int DCCF::getVarCoordinates(NetCDFCFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar) const
{
    sdimnames.clear();
    scoordvars.clear();
    time_dim_name.clear();
    time_coordvar.clear();

    int rc = ncdfc->GetVarCoordVarNames(varname, scoordvars);
    if (rc < 0) return (-1);

    reverse(scoordvars.begin(), scoordvars.end());    // DC dimension order

    sdimnames = ncdfc->GetDimNames(varname);
    reverse(sdimnames.begin(), sdimnames.end());    // DC order

    // Coordinate variables returned by ncdfc are assumed to be lat, lon,
    // height, time. For each native coordinate variable a derived
    // coordinate variable has been created in Cartographic coordinates with
    // elevation expressed in meters, and time in seconds
    //
    for (int i = 0; i < scoordvars.size(); i++) {
        string xcv;
        if (ncdfc->IsLonCoordVar(scoordvars[i])) {
            xcv = scoordvars[i];
        } else if (ncdfc->IsLatCoordVar(scoordvars[i])) {
            xcv = scoordvars[i];
        } else if (ncdfc->IsVertCoordVar(scoordvars[i])) {
            xcv = scoordvars[i];
        } else if (ncdfc->IsTimeCoordVar(scoordvars[i])) {
            xcv = scoordvars[i];
        }

        // Rank of Cartographic coordinates must be less than or
        // equal to that of the data variable, or we have to use
        // the native coordinates
        //
        if (ncdfc->GetDimNames(xcv).size() <= sdimnames.size()) { scoordvars[i] = xcv; }
    }

    if (ncdfc->IsTimeVarying(varname)) {
        time_dim_name = sdimnames.back();
        sdimnames.pop_back();

        time_coordvar = scoordvars.back();
        scoordvars.pop_back();
    }

    return (0);
}

// Initialize the meshMap
//
int DCCF::initMesh(NetCDFCFCollection *ncdfc, std::map<string, DC::Mesh> &meshMap)
{
    //
    // Get names of variables  in the CF data set that have 0 to 3
    // spatial dimensions
    //
    vector<string> vars;
    for (int i = 0; i < 4; i++) {
        vector<string> v = ncdfc->GetDataVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable determine the mesh
    //
    for (int i = 0; i < vars.size(); i++) {
        // variable type must be float or int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeFloat(type) || NetCDFSimple::IsNCTypeInt(type))) continue;

        vector<string> sdimnames;
        vector<string> scoordvars;
        string         time_dim_name;
        string         time_coordvar;

        int rc = getVarCoordinates(ncdfc, vars[i], sdimnames, scoordvars, time_dim_name, time_coordvar);
        if (rc < 0) {
            SetErrMsg("Invalid variable : %s", vars[i].c_str());
            return (-1);
        }

        Mesh mesh("", sdimnames, scoordvars);

        // Create new mesh. We're being lazy here and probably should only
        // createone if it doesn't ready exist
        //
        meshMap[mesh.GetName()] = mesh;
    }

    return (0);
}

int DCCF::initAuxilliaryVars(NetCDFCFCollection *ncdfc, std::map<string, DC::AuxVar> &auxVarsMap)
{
    auxVarsMap.clear();
    return (0);
}


// Collect metadata for all data variables found in the CF data
// set. Initialize dataVarsMap member
//
int DCCF::initDataVars(NetCDFCFCollection *ncdfc, std::map<string, DC::DataVar> &dataVarsMap)
{
    vector<bool> periodic(3, false);
    //
    // Get names of variables  in the CF data set that have 0 to 3
    // spatial dimensions
    //
    vector<string> vars;
    for (int i = 0; i < 4; i++) {
        vector<string> v = ncdfc->GetDataVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to dataVarsMap
    //
    for (auto varName : vars) {
        // A variable can't be both a data variable and a coordinate variable
        //
        auto itr = _coordVarsMap.find(varName);
        if (itr != _coordVarsMap.end()) continue;

        // variable type must be float or int
        //
        int type = ncdfc->GetXType(varName);
        if (!(NetCDFSimple::IsNCTypeFloat(type) || NetCDFSimple::IsNCTypeInt(type))) continue;

        vector<string> sdimnames;
        vector<string> scoordvars;
        string         time_dim_name;
        string         time_coordvar;

        int rc = getVarCoordinates(ncdfc, varName, sdimnames, scoordvars, time_dim_name, time_coordvar);
        if (rc < 0) {
            SetErrMsg("Invalid variable : %s", varName.c_str());
            return (-1);
        }

        string meshName = DC::Mesh::MakeMeshName(sdimnames);

        string units;
        ncdfc->GetAtt(varName, "units", units);
        if (!_udunits.ValidUnit(units)) { units = ""; }

        double mv;
        bool   has_missing = ncdfc->GetMissingValue(varName, mv);

        if (!has_missing) {
            dataVarsMap[varName] = DataVar(varName, units, DC::FLOAT, periodic, meshName, time_coordvar, DC::Mesh::NODE);
        } else {
            dataVarsMap[varName] = DataVar(varName, units, DC::FLOAT, periodic, meshName, time_coordvar, DC::Mesh::NODE, mv);
        }

        rc = DCUtils::CopyAtt(*ncdfc, varName, dataVarsMap[varName]);
        if (rc < 0) return (-1);
    }

    return (0);
}
