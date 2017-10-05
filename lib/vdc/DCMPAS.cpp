#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <cassert>
#include <stdio.h>

#ifdef _WINDOWS
#define _USE_MATH_DEFINES
#pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DCMPAS.h>

using namespace VAPoR;
using namespace std;

namespace {

// MPAS variable and dimension names are hard-wired in the NetCDF
// file :-(

// Dimension names
//
const string timeDimName = "Time";
const string nVertLevelsDimName = "nVertLevels";
const string nVertLevelsP1DimName = "nVertLevelsP1";
const string nCellsDimName = "nCells";
const string nVerticesDimName = "nVertices";
const string nEdgesDimName = "nEdges";
const string maxEdgesDimName = "maxEdges";
const string maxEdges2DimName = "maxEdges2";
const string vertexDegreeDimName = "vertexDegree";

const vector<string> requiredDimNames = {
    timeDimName,
    nVertLevelsDimName,
    nVertLevelsP1DimName,
    nCellsDimName,
    nVerticesDimName,
    nEdgesDimName,
    maxEdgesDimName,
    maxEdges2DimName,
    vertexDegreeDimName};

// Horizontal coordinate variable names
//
const string latCellVarName = "latCell";
const string lonCellVarName = "lonCell";
const string xCellVarName = "xCell"; // Cartesian coord - not used
const string yCellVarName = "yCell"; // Cartesian coord - not used
const string zCellVarName = "zCell"; // Cartesian coord - not used

const string latVertexVarName = "latVertex";
const string lonVertexVarName = "lonVertex";
const string xVertexVarName = "xVertex"; // Cartesian coord - not used
const string yVertexVarName = "yVertex"; // Cartesian coord - not used
const string zVertexVarName = "zVertex"; // Cartesian coord - not used

const string latEdgeVarName = "latEdge";
const string lonEdgeVarName = "lonEdge";
const string xEdgeVarName = "xEdge"; // Cartesian coord - not used
const string yEdgeVarName = "yEdge"; // Cartesian coord - not used
const string zEdgeVarName = "zEdge"; // Cartesian coord - not used

const vector<string> requiredHorizCoordVarNames = {
    latCellVarName,
    lonCellVarName,
    latVertexVarName,
    lonVertexVarName,
    latEdgeVarName,
    lonEdgeVarName};

// Vertical coordinate variables
//
const string zGridVarName = "zgrid";

// Time coordinate variables
//
const string xTimeVarName = "xtime";

// Auxilliary variables (connectivity variables and others)
//
const string cellsOnVertexVarName = "cellsOnVertex";
const string verticesOnCellVarName = "verticesOnCell";
const string verticesOnEdge = "verticesOnEdge";
const string edgesOnCellVarName = "edgesOnCell";
const string nEdgesOnCellVarName = "nEdgesOnCell";

const vector<string> connectivityVarNames = {
    cellsOnVertexVarName,
    verticesOnCellVarName,
    verticesOnEdge,
    edgesOnCellVarName};

const vector<string> requiredAuxVarNames = {
    cellsOnVertexVarName,
    verticesOnCellVarName,
    verticesOnEdge,
    edgesOnCellVarName,
    nEdgesOnCellVarName};

// Meshes: dual mesh (triangle) and primal (hexagonal cell)
//
const string mesh2DTriName = "mesh2DTri";
const string mesh3DTriName = "mesh3DTri";
const string mesh3DP1TriName = "mesh3DTri";
const string mesh2DCellName = "mesh2DCell";
const string mesh3DCellName = "mesh3DCell";
const string mesh3DP1CellName = "mesh3DCell";

// Attributes
//
const string coreNameAttr = "core_name";
const string onASphereAttr = "on_a_sphere";

// Derived variable names. These don't appear in the MPAS output.
// They are derived at run-time by the DCMPAS class
//
const string lonVertexVarNameDerived = lonVertexVarName + "X";
const string latVertexVarNameDerived = latVertexVarName + "Y";
const string lonCellVarNameDerived = lonCellVarName + "X";
const string latCellVarNameDerived = latCellVarName + "Y";
const string lonEdgeVarNameDerived = lonEdgeVarName + "X";
const string latEdgeVarNameDerived = latEdgeVarName + "Y";
const string zGridM1VarName = "zgridM1";

const vector<string> requiredAttrNames = {
    coreNameAttr,
    onASphereAttr};

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a) {
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++)
        ntotal *= a[i];
    return (ntotal);
}

// Return the name of the mesh for a specific set of dimension names
//
string get_mesh_name(vector<string> dimnames) {
    assert(dimnames.size() == 1 || dimnames.size() == 2);

    if (dimnames[0] == nCellsDimName) {
        if ((dimnames.size() == 2) && (dimnames[1] == nVertLevelsDimName)) {

            // 3D dual mesh, unstaggered
            //
            return (mesh3DTriName);
        } else if ((dimnames.size() == 2) && (dimnames[1] == nVertLevelsP1DimName)) {
            // 3D dual mesh, staggered
            //
            return (mesh3DP1TriName);
        } else {
            // 2D dual mesh
            //
            return (mesh2DTriName);
        }
    }

    if (dimnames[0] == nVerticesDimName) {
        if ((dimnames.size() == 2) && (dimnames[1] == nVertLevelsDimName)) {

            // 3D primal mesh, unstaggered
            //
            return (mesh3DCellName);
        } else if ((dimnames.size() == 2) && (dimnames[1] == nVertLevelsP1DimName)) {
            // 3D primal mesh, staggered
            //
            return (mesh3DP1CellName);
        } else {

            // 2D primal mesh
            //
            return (mesh2DCellName);
        }
    }
    return ("");
}

// Is 'varname' a connectivity variable?
//
bool is_connectivity_var(string varname) {
    return (
        find(
            connectivityVarNames.begin(),
            connectivityVarNames.end(),
            varname) != connectivityVarNames.end());
}
}; // namespace

DCMPAS::DCMPAS() {
    _ncdfc = NULL;
    _ovr_fd = -1;
    _ovr_varname.clear();
    _proj4String.clear();
    _proj4API = NULL;
    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
    _derivedVars.clear();
    _cellVars.clear();
    _pointVars.clear();
    _edgeVars.clear();
}

DCMPAS::~DCMPAS() {
    if (_ncdfc)
        delete _ncdfc;
    if (_proj4API)
        delete _proj4API;

    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i])
            delete _derivedVars[i];
    }
    _derivedVars.clear();
}

int DCMPAS::Initialize(const vector<string> &files) {

    _proj4API = NULL;
    _proj4String.clear();

    // Use UDUnits for unit conversion
    //
    int rc = _udunits.Initialize();
    if (rc < 0) {
        SetErrMsg(
            "Failed to initialize udunits2 library : %s",
            _udunits.GetErrMsg().c_str());
        return (-1);
    }

    // Create proj4 instance to project MPAS geographic coordinates to
    // PCS coordinates. Currently VAPOR only works on Cartesian coordinates,
    // not geographic (e.g. lat, lon, level)
    //
    _proj4API = _create_proj4api(
        -180.0, 180.0, -90.0, 90.0, _proj4String);
    if (!_proj4API)
        return (-1);

    NetCDFCollection *ncdfc = new NetCDFCollection();

    // Initialize NetCDFCollection class
    //
    vector<string> time_dimnames(1, timeDimName);
    rc = ncdfc->Initialize(files, time_dimnames, vector<string>());
    if (rc < 0) {
        SetErrMsg("Failed to initialize netCDF data collection for reading");
        return (-1);
    }

    // Create derived coordinate variables.
    //
    rc = _InitDerivedVars(ncdfc);
    if (rc < 0) {
        SetErrMsg("Failed to created required derived coordinate variables");
        return (-1);
    }

    // Make sure NetCDF file(s) have everything we need
    //
    rc = _CheckRequiredFields(ncdfc);
    if (rc < 0)
        return (-1);

    //
    //  Get the dimensions of the grid.
    //	Initializes members: _dimsMap
    //
    rc = _InitDimensions(ncdfc);
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    rc = _InitCoordvars(ncdfc);
    if (rc < 0) {
        return (-1);
    }

    rc = _InitMeshes(ncdfc);
    if (rc < 0)
        return (-1);

    //
    // Identify data and coordinate variables. Sets up members:
    // Initializes members: _dataVarsMap, _coordVarsMap, _meshMap
    //
    rc = _InitAuxVars(ncdfc);
    if (rc < 0)
        return (-1);

    // Finally, get metdata for all data variables
    //
    rc = _InitDataVars(ncdfc);
    if (rc < 0)
        return (-1);

    _ncdfc = ncdfc;

    return (0);
}

bool DCMPAS::GetDimension(
    string dimname, DC::Dimension &dimension) const {
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end())
        return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCMPAS::GetDimensionNames() const {
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) {
        names.push_back(itr->first);
    }

    return (names);
}

vector<string> DCMPAS::GetMeshNames() const {
    vector<string> mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) {
        mesh_names.push_back(itr->first);
    }
    return (mesh_names);
}

bool DCMPAS::GetMesh(
    string mesh_name, DC::Mesh &mesh) const {

    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end())
        return (false);

    mesh = itr->second;
    return (true);
}

bool DCMPAS::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const {

    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) {
        return (false);
    }

    cvar = itr->second;
    return (true);
}

bool DCMPAS::GetDataVarInfo(string varname, DC::DataVar &datavar) const {

    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) {
        return (false);
    }

    datavar = itr->second;
    return (true);
}

bool DCMPAS::GetAuxVarInfo(string varname, DC::AuxVar &auxvar) const {

    map<string, DC::AuxVar>::const_iterator itr;

    itr = _auxVarsMap.find(varname);
    if (itr == _auxVarsMap.end()) {
        return (false);
    }

    auxvar = itr->second;
    return (true);
}

bool DCMPAS::GetBaseVarInfo(string varname, DC::BaseVar &var) const {
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

std::vector<string> DCMPAS::GetDataVarNames() const {
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) {
        names.push_back(itr->first);
    }
    return (names);
}

vector<string> DCMPAS::GetDataVarNames(int ndim, bool spatial) const {

    // Storage of unstrucured data is linearized in 2D. Thus a 3D
    // variable has 2 NetCDF dimensions, etc.
    //
    vector<string> varnames;
    for (int i = 0; i < _cellVars.size(); i++) {
        vector<string> dimnames = _GetSpatialDimNames(_ncdfc, _cellVars[i]);
        if (dimnames.size() + 1 == ndim) {
            varnames.push_back(_cellVars[i]);
        }
    }
    for (int i = 0; i < _pointVars.size(); i++) {
        vector<string> dimnames = _GetSpatialDimNames(_ncdfc, _pointVars[i]);
        if (dimnames.size() + 1 == ndim) {
            varnames.push_back(_pointVars[i]);
        }
    }
    for (int i = 0; i < _edgeVars.size(); i++) {
        vector<string> dimnames = _GetSpatialDimNames(_ncdfc, _edgeVars[i]);
        if (dimnames.size() + 1 == ndim) {
            varnames.push_back(_edgeVars[i]);
        }
    }

    return (varnames);
}

std::vector<string> DCMPAS::GetCoordVarNames() const {
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) {
        names.push_back(itr->first);
    }
    return (names);
}

std::vector<string> DCMPAS::GetAuxVarNames() const {
    map<string, DC::AuxVar>::const_iterator itr;

    vector<string> names;
    for (itr = _auxVarsMap.begin(); itr != _auxVarsMap.end(); ++itr) {
        names.push_back(itr->first);
    }
    return (names);
}

// Oops. Need to implement GetAtt() methods. Oh well. Not currently
// used by vaporgui
//
bool DCMPAS::GetAtt(
    string varname, string attname, vector<double> &values) const {
    values.clear();
    return (false);
}

bool DCMPAS::GetAtt(
    string varname, string attname, vector<long> &values) const {
    values.clear();
    return (false);
}

bool DCMPAS::GetAtt(
    string varname, string attname, string &values) const {
    values.clear();
    return (false);
}

std::vector<string> DCMPAS::GetAttNames(string varname) const {
    vector<string> names;
    return (names);
}

DC::XType DCMPAS::GetAttType(string varname, string attname) const {
    return (DC::FLOAT);
}

int DCMPAS::GetDimLensAtLevel(
    string varname, int, std::vector<size_t> &dims_at_level,
    std::vector<size_t> &bs_at_level) const {
    dims_at_level.clear();
    bs_at_level.clear();

    bool ok = GetVarDimLens(varname, true, dims_at_level);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }
    bs_at_level = dims_at_level;

    return (0);
}

// Read the MPAS nEdgesOnCell auxiliary variable and store it for
// use later
//
int DCMPAS::_read_nEdgesOnCell(size_t ts) {

    DC::Dimension dimension;
    bool ok = GetDimension(nCellsDimName, dimension);
    if (!ok) {
        SetErrMsg(
            "Invalid MPAS auxiliary variable: %s",
            nEdgesOnCellVarName.c_str());
        return (-1);
    }

    int *buf = (int *)_nEdgesOnCellBuf.Alloc(
        dimension.GetLength() * sizeof(*buf));

    int fd = _ncdfc->OpenRead(ts, nEdgesOnCellVarName);
    if (fd < 0)
        return (fd);

    int rc = _ncdfc->Read(buf, fd);
    if (rc < 0)
        return (fd);

    return (_ncdfc->Close(fd));
}

// Read a floating point variable (data or coordinate) into a SmartBuf
//
int DCMPAS::_readVarToSmartBuf(
    size_t ts, string varname, Wasp::SmartBuf &smartBuf) {
    vector<size_t> dims;
    bool ok = GetVarDimLens(varname, true, dims);
    assert(ok);

    size_t n = vproduct(dims);
    float *buf = (float *)smartBuf.Alloc(n * sizeof(*buf));

    int fd = _ncdfc->OpenRead(ts, varname);
    if (fd < 0)
        return (fd);

    int rc = _ncdfc->Read(buf, fd);
    if (rc < 0)
        return (fd);

    return (_ncdfc->Close(fd));
}

// Read the longitude coordinates for cell and vertices. Need these
// to split the periodic mesh
//
int DCMPAS::_readCoordinates(size_t ts) {

    int rc = _readVarToSmartBuf(ts, lonCellVarName, _lonCellSmartBuf);
    if (rc < 0)
        return (rc);

    rc = _readVarToSmartBuf(ts, lonVertexVarName, _lonVertexSmartBuf);
    if (rc < 0)
        return (rc);

    return (0);
}

// MPAS uses an auxiliary array (the variable nEdgesOnCelll) to
// indicate the number of elements (edges or vertices) in
// multi-dimensional arrays with varying dimension lengths. But DC uses
// a padding flag: -1. So using nEdgesOnCell we pad the fixed size
// DC connectivity array with -1.
//
void DCMPAS::_addMissingFlag(int *data) const {

    int *nEdgesOnCell = (int *)_nEdgesOnCellBuf.GetBuf();

    DC::Dimension dimension;
    bool ok = GetDimension(nCellsDimName, dimension);
    assert(ok);

    size_t nCells = dimension.GetLength();

    ok = GetDimension(maxEdgesDimName, dimension);
    assert(ok);

    size_t nMaxEdges = dimension.GetLength();

    // Add padding
    //
    for (size_t j = 0; j < nCells; j++) {
        for (int i = nEdgesOnCell[j]; i < nMaxEdges; i++) {
            data[j + i * nMaxEdges] = -1;
        }
    }
}

// MPAS data on sphere is periodic. But we're projecting the geographic
// data to Cartesian coordinates. We need split the cells that straddle
// the split location, here chose to be 180 (-180) degrees
//
void DCMPAS::_splitOnBoundary(string varname, int *connData) const {

    vector<size_t> connDims;
    bool ok = GetVarDimLens(varname, true, connDims);
    assert(ok && connDims.size() == 2);

    // Dimensions for vertex lon
    //
    vector<size_t> lonVertexDims;
    ok = GetVarDimLens(lonVertexVarName, true, lonVertexDims);
    assert(ok && lonVertexDims.size() == 1);

    // Dimensions for cell lon
    //
    vector<size_t> lonCellDims;
    ok = GetVarDimLens(lonCellVarName, true, lonCellDims);
    assert(ok && lonCellDims.size() == 1);

    float *lonBuf1 = NULL;
    float *lonBuf2 = NULL;
    if (connDims[1] == lonVertexDims[0]) {
        lonBuf1 = (float *)_lonVertexSmartBuf.GetBuf();
        lonBuf2 = (float *)_lonCellSmartBuf.GetBuf();
    } else if (connDims[1] == lonCellDims[0]) {
        lonBuf1 = (float *)_lonCellSmartBuf.GetBuf();
        lonBuf2 = (float *)_lonVertexSmartBuf.GetBuf();
    } else {
        assert(0);
    }

    // For each cell (vertex) in the connectivity array make sure
    // that the cell's (vertices') coordinates and all of it's neighboring
    // vertices (cells) are on the same side of the split line for
    // longitude If not, introduce a split
    // by changing the index of the offending vertices (cells) to -2,
    // the boundary marker. This code assumes the valid lon coordiates
    // are 0.0 .. 2*M_PI, as
    // per the MPAS Mesh Specification, Version 1.0 (Oct. 8, 2015) document.
    //
    int n = connDims[0];
    for (size_t j = 0; j < connDims[1]; j++) {
        double lon1 = lonBuf1[j];

        // Ha ha. despite MPAS documentation longitude may run -pi to pi
        //
        if (lon1 > M_PI)
            lon1 -= 2 * M_PI;
        for (size_t i = 0; i < n && connData[j * n + i] >= 0; i++) {
            size_t index = connData[j * n + i] - 1; // Arrg. Index starts from 1!!
            double lon2 = lonBuf2[index];

            // Ha ha. despite MPAS documentation longitude may run -pi to pi
            //
            if (lon2 > M_PI)
                lon2 -= 2 * M_PI;

            // Ugh. Test for cell stradling -pi and pi
            //
            if (fabs(lon1 - lon2) > M_PI * 0.5) {
                connData[j * n + i] = -2;
            }
        }
    }
}

int DCMPAS::OpenVariableRead(
    size_t ts, string varname) {
    DCMPAS::CloseVariable();

    _ovr_fd = _ncdfc->OpenRead(ts, varname);

    // Special handling for some auxiliary variables
    //
    _ovr_varname = varname;
    if (_ovr_varname == verticesOnCellVarName) {
        if (_read_nEdgesOnCell(ts) < 0)
            return (-1);
    }

    if (is_connectivity_var(_ovr_varname)) {
        if (_readCoordinates(ts) < 0)
            return (-1);
    }

    return (_ovr_fd);
}

int DCMPAS::CloseVariable() {
    if (_ovr_fd < 0)
        return (0);
    int rc = _ncdfc->Close(_ovr_fd);
    _ovr_fd = -1;
    return (rc);
}

int DCMPAS::Read(float *data) {
    return (_ncdfc->Read(data, _ovr_fd));
}

int DCMPAS::Read(int *data) {
    int rc = _ncdfc->Read(data, _ovr_fd);

    // Special handling for some auxiliary variables
    //
    if (_ovr_varname == verticesOnCellVarName) {
        _addMissingFlag(data);
    }

    if (is_connectivity_var(_ovr_varname)) {
        _splitOnBoundary(_ovr_varname, data);
    }

    return (rc);
}

int DCMPAS::ReadSlice(float *slice) {

    return (_ncdfc->ReadSlice(slice, _ovr_fd));
}

int DCMPAS::ReadRegion(
    const vector<size_t> &min, const vector<size_t> &max, float *region) {

    // Need to reverse coordinate ordering for NetCDFCollection API, which
    // orders coordinates from slowest to fastest. DC class expects order
    // from fastest to slowest
    //
    vector<size_t> ncdf_start = min;
    reverse(ncdf_start.begin(), ncdf_start.end());

    vector<size_t> ncdf_max = max;
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) {
        ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1);
    }

    return (_ncdfc->Read(ncdf_start, ncdf_count, region, _ovr_fd));
}

int DCMPAS::ReadRegionBlock(
    const vector<size_t> &min, const vector<size_t> &max, float *region) {
    //return(DCMPAS::ReadRegion(min, max, region));
    return (DCMPAS::Read(region));
}

int DCMPAS::GetVar(string varname, float *data) {

    vector<size_t> dimlens;
    bool ok = GetVarDimLens(varname, true, dimlens);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    int nts = 1; // num time steps
    bool time_varying = IsTimeVarying(varname);
    if (time_varying) {
        nts = GetNumTimeSteps(varname);
    }

    // Number of grid points for variable
    //
    size_t sz = 1;
    for (int i = 0; i < dimlens.size(); i++) {
        sz *= dimlens[i];
    }

    //
    // Read one time step at a time
    //
    float *ptr = data;
    for (int ts = 0; ts < nts; ts++) {
        int rc = DCMPAS::OpenVariableRead(ts, varname);
        if (rc < 0)
            return (-1);

        rc = DCMPAS::Read(ptr);
        if (rc < 0)
            return (-1);

        rc = DCMPAS::CloseVariable();
        if (rc < 0)
            return (-1);

        ptr += sz; // Advance buffer past current time step
    }
    return (0);
}

int DCMPAS::GetVar(
    size_t ts, string varname, float *data) {
    int rc = DCMPAS::OpenVariableRead(ts, varname);
    if (rc < 0)
        return (-1);

    rc = DCMPAS::Read(data);
    if (rc < 0)
        return (-1);

    rc = DCMPAS::CloseVariable();
    if (rc < 0)
        return (-1);

    return (0);
}

bool DCMPAS::VariableExists(
    size_t ts, string varname, int, int) const {
    return (_ncdfc->VariableExists(ts, varname));
}

// Create proj4 class instance to convert from geographic to PCS
// coordinates
//
Proj4API *DCMPAS::_create_proj4api(
    double lonmin, double lonmax, double latmin, double latmax,
    string &proj4string) const {
    proj4string.clear();

    double lon_0 = (lonmin + lonmax) / 2.0;
    double lat_0 = (latmin + latmax) / 2.0;
    ostringstream oss;
    oss << " +lon_0=" << lon_0 << " +lat_0=" << lat_0;
    proj4string = "+proj=eqc +ellps=WGS84" + oss.str();

    Proj4API *proj4api = new Proj4API();

    int rc = proj4api->Initialize("", proj4string);
    if (rc < 0) {
        SetErrMsg("Invalid map projection : %s", proj4string.c_str());
        return (NULL);
    }
    return (proj4api);
}

int DCMPAS::_InitCoordvars(
    NetCDFCollection *ncdfc) {

    string time_dim_name = "";

    // longitude coordinates, native (those found in MPAS output files), and
    // derived (those created on-the-fly by the DCMPAS class. The latter
    // are longitude coordinate variables mapped to PCS coordinates (e.g
    // meters on the ground)
    //
    vector<string> cvars = {lonCellVarName, lonVertexVarName, lonEdgeVarName};
    for (int i = 0; i < cvars.size(); i++) {
        vector<bool> periodic(true);
        vector<string> dimnames;

        string units = "degrees_east";
        int axis = 0;
        string name = cvars[i];
        dimnames = ncdfc->GetDimNames(name);
        assert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(
            name, units, DC::FLOAT, periodic, axis, false,
            dimnames, vector<size_t>(), time_dim_name);

        // Now do derived (meters on the ground) coordinate variable
        // Here we're just adding it's metdata to the _coordVarsMap. Later
        // we'll actually derive it.
        //
        units = "meters";
        name += "X";
        dimnames = ncdfc->GetDimNames(name);
        assert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(
            name, units, DC::FLOAT, periodic, axis, false,
            dimnames, vector<size_t>(), time_dim_name);
    }

    // LAtitude coordinate variables
    //
    cvars = {latCellVarName, latVertexVarName, latEdgeVarName};
    for (int i = 0; i < cvars.size(); i++) {
        vector<bool> periodic(false);
        vector<string> dimnames;

        string units = "degrees_north";
        int axis = 1;
        string name = cvars[i];
        dimnames = ncdfc->GetDimNames(name);
        assert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(
            name, units, DC::FLOAT, periodic, axis, false,
            dimnames, vector<size_t>(), time_dim_name);

        units = "meters";
        name += "Y";
        dimnames = ncdfc->GetDimNames(name);
        assert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(
            name, units, DC::FLOAT, periodic, axis, false,
            dimnames, vector<size_t>(), time_dim_name);
    }

    vector<bool> periodic(false);
    vector<string> dimnames;

    // Vertical coordinate variables, native and derived
    //
    string units = "meters";
    int axis = 2;
    string name = zGridVarName;
    dimnames = ncdfc->GetDimNames(name);
    assert(dimnames.size() == 2);

    _coordVarsMap[name] = CoordVar(
        name, units, DC::FLOAT, periodic, axis, false,
        dimnames, vector<size_t>(), time_dim_name);

    units = "meters";
    name = zGridM1VarName;
    dimnames = ncdfc->GetDimNames(name);
    assert(dimnames.size() == 2);

    _coordVarsMap[name] = CoordVar(
        name, units, DC::FLOAT, periodic, axis, false,
        dimnames, vector<size_t>(), time_dim_name);

    // Need a derived time coordinate variable. The native MPAS
    // time coordinate variable is a formatted ASCII string like
    // in WRF :-(
    //

    units = "seconds";
    axis = 3;
    name = xTimeVarName + "T";
    dimnames.clear();

    _coordVarsMap[name] = CoordVar(
        name, units, DC::FLOAT, periodic, axis, false,
        dimnames, vector<size_t>(), timeDimName);

    return (0);
}

// Create all of the derived coordinate variables
//
int DCMPAS::_InitDerivedVars(
    NetCDFCollection *ncdfc) {
    int rc = _InitHorizontalCoordinatesDerived(ncdfc);

    if (rc < 0)
        return (-1);

    rc = _InitVerticalCoordinatesDerived(ncdfc);

    // Create the X derived variable class object
    //
    string nativeTimeVar = xTimeVarName;
    DerivedVarWRFTime *derivedT;
    derivedT = new DerivedVarWRFTime(
        ncdfc, &_udunits, nativeTimeVar);
    _derivedVars.push_back(derivedT);

    // Install the derived variable on the NetCDFCollection class. Then
    // all NetCDFCollection methods will treat the derived variable as
    // if it existed in the MPAS data set.
    //
    string name = nativeTimeVar + "T";
    ncdfc->InstallDerivedVar(name, derivedT);

    if (rc < 0)
        return (-1);

    return (0);
}

int DCMPAS::_InitHorizontalCoordinatesDerived(
    NetCDFCollection *ncdfc) {
    vector<pair<string, string>> coordpairs;
    coordpairs.push_back(make_pair(lonCellVarName, latCellVarName));
    coordpairs.push_back(make_pair(lonVertexVarName, latVertexVarName));
    coordpairs.push_back(make_pair(lonEdgeVarName, latEdgeVarName));

    // Create a pair of derived horizontal coordinate variables
    // in Cartographic
    // coordiantes for each lat-lon pair
    //

    // Create derived variables and find min and max lat-lon coordinate
    // extents.
    //
    for (int i = 0; i < coordpairs.size(); i++) {

        // Get dimension names for each coordinate pair.
        //
        vector<string> londimnames = ncdfc->GetDimNames(coordpairs[i].first);
        vector<string> latdimnames = ncdfc->GetDimNames(coordpairs[i].second);

        if (londimnames.size() > 1 && londimnames != latdimnames) {
            SetErrMsg(
                "Invalid coordinate variable pair : %s, %s",
                coordpairs[i].first.c_str(), coordpairs[i].second.c_str());
            return (-1);
        }
        reverse(londimnames.begin(), londimnames.end()); // DC order
        reverse(latdimnames.begin(), latdimnames.end()); // DC order

        // Time varying coordinates are't currently supported by MPAS, so this
        // code is a no-op
        //
        string lon_time_dim_name;
        if (ncdfc->IsTimeVarying(coordpairs[i].first)) {
            lon_time_dim_name = londimnames.back();
            londimnames.pop_back();
        }

        string lat_time_dim_name;
        if (ncdfc->IsTimeVarying(coordpairs[i].second)) {
            lat_time_dim_name = latdimnames.back();
            latdimnames.pop_back();
        }

        // Create the X derived variable class object
        //
        string name = coordpairs[i].first + "X";

        DerivedVarHorizontal *derivedX;
        derivedX = new DerivedVarHorizontal(
            ncdfc, coordpairs[i].first, coordpairs[i].second,
            _proj4API, true, true, false);
        _derivedVars.push_back(derivedX);

        // Install the derived variable on the NetCDFCollection class. Then
        // all NetCDFCollection methods will treat the derived variable as
        // if it existed in the MPAS data set.
        //
        ncdfc->InstallDerivedVar(name, derivedX);

        name = coordpairs[i].second + "Y";
        DerivedVarHorizontal *derivedY;
        derivedY = new DerivedVarHorizontal(
            ncdfc, coordpairs[i].first, coordpairs[i].second,
            _proj4API, false, true, false);
        _derivedVars.push_back(derivedY);

        ncdfc->InstallDerivedVar(name, derivedY);
    }

    return (0);
}

int DCMPAS::_InitVerticalCoordinatesDerived(
    NetCDFCollection *ncdfc) {
    string zGridVarStaggered = zGridVarName;
    string zGridVarDerived = zGridM1VarName;
    string zDimName = nVertLevelsP1DimName;

    DerivedVarVertical *derivedZ;
    derivedZ = new DerivedVarVertical(
        ncdfc, zGridVarStaggered, zDimName);
    _derivedVars.push_back(derivedZ);

    // Install the derived variable on the NetCDFCollection class. Then
    // all NetCDFCollection methods will treat the derived variable as
    // if it existed in the MPAS data set.
    //
    ncdfc->InstallDerivedVar(zGridVarDerived, derivedZ);

    return (0);
}

int DCMPAS::_CheckRequiredFields(
    NetCDFCollection *ncdfc) const {

    vector<string>::const_iterator itr;

    // Check for dimensions
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    for (int i = 0; i < requiredDimNames.size(); i++) {
        string s = requiredDimNames[i];

        itr = find(dimnames.begin(), dimnames.end(), s);
        if (itr == dimnames.end()) {
            SetErrMsg("Missing required dimension \"%s\"", s.c_str());
            return (-1);
        }
    }

    // Check for horizontal coordinate variables and auxiallary vars
    //
    vector<string> varnames;
    for (int ndim = 1; ndim < 3; ndim++) {
        vector<string> v = ncdfc->GetVariableNames(ndim, true);
        varnames.insert(varnames.end(), v.begin(), v.end());
    }

    for (int i = 0; i < requiredHorizCoordVarNames.size(); i++) {
        string s = requiredHorizCoordVarNames[i];

        itr = find(varnames.begin(), varnames.end(), s);
        if (itr == varnames.end()) {
            SetErrMsg("Missing required dimension \"%s\"", s.c_str());
            return (-1);
        }
    }

    for (int i = 0; i < requiredAuxVarNames.size(); i++) {
        string s = requiredAuxVarNames[i];

        itr = find(varnames.begin(), varnames.end(), s);
        if (itr == varnames.end()) {
            SetErrMsg("Missing required dimension \"%s\"", s.c_str());
            return (-1);
        }
    }

    // Check for required global attrs
    //
    vector<string> attnames = ncdfc->GetAttNames("");
    for (int i = 0; i < requiredAttrNames.size(); i++) {
        string s = requiredAttrNames[i];

        itr = find(attnames.begin(), attnames.end(), s);
        if (itr == attnames.end()) {
            SetErrMsg("Missing required dimension \"%s\"", s.c_str());
            return (-1);
        }
    }

    return (0);
}

// Get Space and time dimensions from MPAS data set. Initialize
// _dimsMap
//
int DCMPAS::_InitDimensions(
    NetCDFCollection *ncdfc) {
    _dimsMap.clear();

    // Get dimension names and lengths for all dimensions in the
    // MPAS data set.
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    vector<size_t> dimlens = ncdfc->GetDims();
    assert(dimnames.size() == dimlens.size());

    for (int i = 0; i < dimnames.size(); i++) {

        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }

    return (0);
}

// Given a data variable name return the variable's dimensions and
// associated coordinate variables. The coordinate variable names
// returned is for the derived coordinate variables expressed in
// Cartographic coordinates, not the native geographic coordinates
// found in the MPAS file.
//
// The order of the returned vectors
// is significant.
//
int DCMPAS::_GetVarCoordinates(
    NetCDFCollection *ncdfc, string varname,
    vector<string> &sdimnames,
    vector<string> &scoordvars,
    string &time_dim_name,
    string &time_coordvar) {
    sdimnames.clear();
    scoordvars.clear();
    time_dim_name.clear();
    time_coordvar.clear();

    vector<string> dimnames = ncdfc->GetDimNames(varname);
    assert(dimnames.size() >= 1);

    if (ncdfc->IsTimeVarying(varname)) {
        time_dim_name = dimnames[0];
        time_coordvar = xTimeVarName + "T";
        dimnames.erase(dimnames.begin());
    }

    if (find(_cellVars.begin(), _cellVars.end(), varname) != _cellVars.end()) {
        sdimnames.push_back(nCellsDimName);
        scoordvars.push_back(lonCellVarNameDerived);
        scoordvars.push_back(latCellVarNameDerived);
    } else if (find(_pointVars.begin(), _pointVars.end(), varname) != _pointVars.end()) {
        sdimnames.push_back(nVerticesDimName);
        scoordvars.push_back(lonVertexVarNameDerived);
        scoordvars.push_back(latVertexVarNameDerived);
    } else if (find(_edgeVars.begin(), _edgeVars.end(), varname) != _edgeVars.end()) {
        sdimnames.push_back(nEdgesDimName);
        scoordvars.push_back(lonEdgeVarNameDerived);
        scoordvars.push_back(latEdgeVarNameDerived);
    } else {
        assert(0);
    }

    // 3D variable?
    //
    if (dimnames.size() > 1) {
        sdimnames.push_back(dimnames[1]);
        if (dimnames[1] == nVertLevelsDimName) {
            scoordvars.push_back(zGridM1VarName);
        } else {
            scoordvars.push_back(zGridVarName);
        }
    }

    return (0);
}

int DCMPAS::_InitMeshes(
    NetCDFCollection *ncdfc) {

    // Max vertices or edges per cell
    //
    DC::Dimension dimension;
    bool ok = GetDimension(maxEdgesDimName, dimension);
    assert(ok);

    //
    // Dual meshes (triangle mesh)
    // N.B. for the dual node the meanings of cellsOnVertexVarName,
    // and verticesOnCellVarName are reversed.
    //
    // 2D, layered, and layered with staggered dimensions
    //

    vector<string> coordvars = {lonCellVarNameDerived, latCellVarNameDerived};
    _meshMap[mesh2DTriName] = Mesh(
        mesh2DTriName, 3, dimension.GetLength(),
        nCellsDimName, nVerticesDimName, coordvars,
        cellsOnVertexVarName, verticesOnCellVarName);

    coordvars = {lonCellVarNameDerived, latCellVarNameDerived, zGridM1VarName};
    _meshMap[mesh3DTriName] = Mesh(
        mesh3DTriName, 3, dimension.GetLength(),
        nCellsDimName, nVerticesDimName, nVertLevelsDimName, coordvars,
        cellsOnVertexVarName, verticesOnCellVarName);

    coordvars = {lonCellVarNameDerived, latCellVarNameDerived, zGridVarName};
    _meshMap[mesh3DP1TriName] = Mesh(
        mesh3DP1TriName, 3, dimension.GetLength(),
        nCellsDimName, nVerticesDimName, nVertLevelsP1DimName, coordvars,
        cellsOnVertexVarName, verticesOnCellVarName);

    //
    // Primal meshes (hexagonal mesh)
    //
    // 2D, layered, and layered with staggered dimensions
    //

    coordvars = {lonVertexVarNameDerived, latVertexVarNameDerived};
    _meshMap[mesh2DCellName] = Mesh(
        mesh2DCellName, dimension.GetLength(), 3, nVerticesDimName,
        nCellsDimName, coordvars, verticesOnCellVarName, cellsOnVertexVarName);

    coordvars = {lonVertexVarNameDerived, latVertexVarNameDerived, zGridM1VarName};
    _meshMap[mesh3DCellName] = Mesh(
        mesh3DCellName, dimension.GetLength(), 3, nVerticesDimName,
        nCellsDimName, nVertLevelsDimName, coordvars, verticesOnCellVarName,
        cellsOnVertexVarName);

    coordvars = {lonVertexVarNameDerived, latVertexVarNameDerived, zGridVarName};
    _meshMap[mesh3DP1CellName] = Mesh(
        mesh3DP1CellName, dimension.GetLength(), 3, nVerticesDimName,
        nCellsDimName, nVertLevelsP1DimName, coordvars, verticesOnCellVarName,
        cellsOnVertexVarName);

    return (0);
}

int DCMPAS::_InitAuxVars(
    NetCDFCollection *ncdfc) {
    _auxVarsMap.clear();

    vector<bool> periodic(3, false);
    //
    // Get names of variables  in the MPAS data set that have 1 or 2
    // spatial dimensions
    //
    vector<string> vars;
    for (int i = 1; i < 3; i++) {
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to _dataVarsMap
    //
    for (int i = 0; i < vars.size(); i++) {

        // variable type must be float or int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeInt(type))) {

            continue;
        }

        vector<string> dimnames = _GetSpatialDimNames(ncdfc, vars[i]);
        if (!dimnames.size())
            continue;

        _auxVarsMap[vars[i]] = AuxVar(
            vars[i], "", DC::INT32, "", vector<size_t>(),
            vector<size_t>(), periodic, dimnames);

        // IDs in MPAS files start from 1, not 0 :-(
        //
        if (
            vars[i] == cellsOnVertexVarName ||
            vars[i] == verticesOnCellVarName ||
            vars[i] == verticesOnEdge ||
            vars[i] == edgesOnCellVarName) {
            _auxVarsMap[vars[i]].SetOffset(-1);
        }
    }

    return (0);
}

int DCMPAS::_InitDataVars(
    NetCDFCollection *ncdfc) {
    _dataVarsMap.clear();

    vector<bool> periodic(3, false);
    //
    // Get names of variables  in the MPAS data set that have 1 or 2
    // spatial dimensions
    //
    vector<string> vars;
    //	for (int i=1; i<3; i++) {
    for (int i = 1; i < 2; i++) { // ONLY 2D VARIABLES SUPPORTED!!!
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to _dataVarsMap
    //
    for (int i = 0; i < vars.size(); i++) {

        // variable type must be float or int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeFloat(type))) {

            continue;
        }

        vector<string> sdimnames;
        vector<string> scoordvars;
        string time_dim_name;
        string time_coordvar;

        vector<string> dimnames = _GetSpatialDimNames(ncdfc, vars[i]);
        if (!dimnames.size())
            continue;

        if (dimnames[0] == nCellsDimName) {
            _cellVars.push_back(vars[i]);
        } else if (dimnames[0] == nVerticesDimName) {
            _pointVars.push_back(vars[i]);
        } else if (dimnames[0] == nEdgesDimName) {
            // not supported yet
            //
            //_edgeVars.push_back(vars[i]);
            continue;
        } else {
            continue;
        }

        string meshname = get_mesh_name(dimnames);
        if (meshname.empty())
            continue;

        int rc = _GetVarCoordinates(
            ncdfc, vars[i], sdimnames, scoordvars, time_dim_name, time_coordvar);
        if (rc < 0) {
            SetErrMsg("Invalid variable : %s", vars[i].c_str());
            return (-1);
        }

        string units;
        ncdfc->GetAtt(vars[i], "units", units);
        if (!_udunits.ValidUnit(units)) {
            units = "";
        }

        double mv;
        bool has_missing = ncdfc->GetMissingValue(vars[i], mv);

        if (!has_missing) {
            _dataVarsMap[vars[i]] = DataVar(
                vars[i], units, DC::FLOAT, periodic, meshname,
                vector<size_t>(), time_coordvar, DC::Mesh::NODE);
        } else {
            _dataVarsMap[vars[i]] = DataVar(
                vars[i], units, DC::FLOAT, periodic, meshname,
                vector<size_t>(), time_coordvar, DC::Mesh::NODE,
                mv);
        }
    }

    return (0);
}

vector<string> DCMPAS::_GetSpatialDimNames(
    NetCDFCollection *ncdfc, string varname) const {
    vector<string> v = ncdfc->GetSpatialDimNames(varname);
    reverse(v.begin(), v.end());
    return (v);
}

//////////////////////////////////////////////////////////////////////
//
// Class definitions for derived coordinate variables
//
//////////////////////////////////////////////////////////////////////

//
// MPAS's native horizontal coordinate system is geographic. Need to
// project from geographic to Cartographic using the Proj4 API
//
// N.B. This class **should** handle structured grids as well as
// unstructured and could replace the structured version used by WRF, etc.
//
DCMPAS::DerivedVarHorizontal::DerivedVarHorizontal(
    NetCDFCollection *ncdfc, string lonname, string latname,
    Proj4API *proj4API, bool xflag, bool uGridFlag, bool degrees) : DerivedVar(ncdfc) {

    _lonname = lonname;
    _latname = latname;
    _xflag = xflag;
    _uGridFlag = uGridFlag;
    _degreesFlag = degrees;
    _oneDFlag = false;
    _time_dim = 1;
    _time_dim_name.clear();
    _sdims.clear();
    _sdimnames.clear();
    _is_open = false;
    _lonbuf = NULL;
    _latbuf = NULL;
    _proj4API = proj4API;
    _lonfd = -1;
    _latfd = -1;
    _is_open = false;

    vector<string> lonDimNames = ncdfc->GetDimNames(lonname);
    vector<size_t> lonDimLens = ncdfc->GetDims(lonname);
    vector<string> latDimNames = ncdfc->GetDimNames(latname);
    vector<size_t> latDimLens = ncdfc->GetDims(latname);

    if (ncdfc->IsTimeVarying(lonname) || ncdfc->IsTimeVarying(latname)) {
        assert(ncdfc->IsTimeVarying(lonname) && ncdfc->IsTimeVarying(latname));
        assert(lonDimNames[0] == latDimNames[0]);
        assert(lonDimLens[0] == latDimLens[0]);

        _time_dim = lonDimLens[0];
        _time_dim_name = lonDimNames[0];
        lonDimLens.erase(lonDimLens.begin());
        lonDimNames.erase(lonDimNames.begin());
        latDimLens.erase(latDimLens.begin());
        latDimNames.erase(latDimNames.begin());
    }

    if (uGridFlag) {
        assert(lonDimLens.size() == 1);
        _sdims = lonDimLens;
        _sdimnames = lonDimNames;
    } else {
        assert(lonDimLens.size() >= 1 && lonDimLens.size() <= 2);
        if (lonDimLens.size() == 2) {
            _sdims = lonDimLens;
            _sdimnames = lonDimNames;
        } else {
            _sdims.push_back(latDimLens[0]);
            _sdims.push_back(lonDimLens[0]);
            _sdimnames.push_back(latDimNames[0]);
            _sdimnames.push_back(lonDimNames[0]);
            _oneDFlag = true;
        }
    }

    size_t n = vproduct(lonDimLens);
    _lonbuf = new float[n];

    n = vproduct(latDimLens);
    _latbuf = new float[n];
}

DCMPAS::DerivedVarHorizontal::~DerivedVarHorizontal() {

    if (_lonbuf)
        delete[] _lonbuf;
    if (_latbuf)
        delete[] _latbuf;
}

int DCMPAS::DerivedVarHorizontal::Open(size_t ts) {

    if (_is_open)
        return (-1); // Only one variable open at a time

    _lonfd = -1;
    _latfd = -1;

    int fd = _ncdfc->OpenRead(ts, _lonname);
    if (fd < 0) {
        SetErrMsg("Can't read %s variable", _lonname.c_str());
        return (-1);
    }
    _lonfd = fd;

    fd = _ncdfc->OpenRead(ts, _latname);
    if (fd < 0) {
        SetErrMsg("Can't read %s variable", _latname.c_str());
        return (-1);
    }
    _latfd = fd;

    _is_open = true;

    return (0);
}

int DCMPAS::DerivedVarHorizontal::Read(float *buf, int) {

    if (!_is_open) {
        SetErrMsg("Invalid operation");
        return (-1);
    }

    int rc = _ncdfc->Read(_lonbuf, _lonfd);
    if (rc < 0) {
        SetErrMsg("Can't read %s variable", _lonname.c_str());
        return (-1);
    }

    rc = _ncdfc->Read(_latbuf, _latfd);
    if (rc < 0) {
        SetErrMsg("Can't read %s variable", _latname.c_str());
        return (-1);
    }

    if (_oneDFlag) {
        int ny = _sdims[0];
        int nx = _sdims[1];

        for (int j = 1; j < ny; j++) {
            for (int i = 0; i < nx; i++) {
                _lonbuf[j * nx + i] = _lonbuf[i];
            }
        }

        for (int j = ny - 1; j >= 0; j--) {
            for (int i = 0; i < nx; i++) {
                _latbuf[j * nx + i] = _latbuf[j];
            }
        }
    }

    size_t n = vproduct(_sdims);

    // Convert from radians to degrees if necessary
    //
    if (!_degreesFlag) {
        for (size_t i = 0; i < n; i++) {
            _lonbuf[i] = _lonbuf[i] * 180.0 / M_PI;
            _latbuf[i] = _latbuf[i] * 180.0 / M_PI;
        }
    }

    rc = _proj4API->Transform(_lonbuf, _latbuf, n);
    if (rc < 0)
        return (-1);

    float *ptr = _xflag ? _lonbuf : _latbuf;

    for (int i = 0; i < n; i++) {
        buf[i] = ptr[i];
    }

    return (0);
}

int DCMPAS::DerivedVarHorizontal::ReadSlice(
    float *slice, int) {
    return (DCMPAS::DerivedVarHorizontal::Read(slice, 0));
}

int DCMPAS::DerivedVarHorizontal::SeekSlice(
    int offset, int whence, int) {
    return (0);
}

int DCMPAS::DerivedVarHorizontal::Close(int) {
    if (!_is_open)
        return (0);

    int rc = 0;
    if (_lonfd > 0 && _ncdfc->Close(_lonfd) < 0)
        rc = -1;
    if (_latfd > 0 && _ncdfc->Close(_latfd) < 0)
        rc = -1;
    _is_open = false;

    return (rc);
}

//
//
DCMPAS::DerivedVarVertical::DerivedVarVertical(
    NetCDFCollection *ncdfc, string staggeredVarName,
    string zDimNameUnstaggered) : DerivedVar(ncdfc) {

    _staggeredVarName = staggeredVarName;
    _time_dim = 1;
    _time_dim_name.clear();
    _sdims.clear();
    _sdimnames.clear();
    _is_open = false;
    _buf = NULL;
    _fd = -1;
    _is_open = false;

    vector<string> dimNames = ncdfc->GetDimNames(staggeredVarName);
    vector<size_t> dimLens = ncdfc->GetDims(staggeredVarName);

    assert(dimNames.size() == 2);
    assert(dimLens.size() == 2);
    assert(dimLens[1] >= 2);

    _sdims = dimLens;
    _sdims[1] = _sdims[1] - 1; // Substract one for non-staggered dim
    _sdimnames = dimNames;
    _sdimnames[1] = zDimNameUnstaggered;

    // Allocate buffer large enough for staggered variable
    //
    size_t n = vproduct(dimLens);
    _buf = new float[n];
}

DCMPAS::DerivedVarVertical::~DerivedVarVertical() {

    if (_buf)
        delete[] _buf;
}

int DCMPAS::DerivedVarVertical::Open(size_t ts) {

    if (_is_open)
        return (-1); // Only one variable open at a time

    _fd = -1;

    int fd = _ncdfc->OpenRead(ts, _staggeredVarName);
    if (fd < 0) {
        SetErrMsg("Can't read %s variable", _staggeredVarName.c_str());
        return (-1);
    }
    _fd = fd;
    _is_open = true;

    return (0);
}

int DCMPAS::DerivedVarVertical::Read(float *buf, int) {

    if (!_is_open) {
        SetErrMsg("Invalid operation");
        return (-1);
    }

    int rc = _ncdfc->Read(_buf, _fd);
    if (rc < 0) {
        SetErrMsg("Can't read %s variable", _staggeredVarName.c_str());
        return (-1);
    }

    // Resample staggered to unstaggered grid
    //
    size_t ny = _sdims[0];
    size_t nx = _sdims[1];
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {

            // _buf has dimensions (nx+1) * ny
            //
            buf[j * nx + i] = (_buf[j * nx + i] + _buf[j * nx + i + 1]) * 0.5;
        }
    }

    return (0);
}

int DCMPAS::DerivedVarVertical::ReadSlice(
    float *slice, int) {
    return (DCMPAS::DerivedVarVertical::Read(slice, 0));
}

int DCMPAS::DerivedVarVertical::SeekSlice(
    int offset, int whence, int) {
    return (0);
}

int DCMPAS::DerivedVarVertical::Close(int) {
    if (!_is_open)
        return (0);

    int rc = 0;
    if (_fd > 0 && _ncdfc->Close(_fd) < 0)
        rc = -1;
    _is_open = false;

    return (rc);
}

//
//
DCMPAS::DerivedVarWRFTime::DerivedVarWRFTime(
    NetCDFCollection *ncdfc, const UDUnits *udunits,
    string wrfTimeVar) : DerivedVar(ncdfc) {

    _udunits = udunits;
    _wrfTimeVar = wrfTimeVar;
    _time_dim = 1;
    _time_dim_name.clear();
    _is_open = false;
    _buf = NULL;
    _fd = -1;
    _is_open = false;

    vector<string> dimNames = ncdfc->GetDimNames(wrfTimeVar);
    vector<size_t> dimLens = ncdfc->GetDims(wrfTimeVar);

    assert(dimNames.size() == 2);
    assert(dimLens.size() == 2);

    _time_dim = dimLens[0];
    _time_dim_name = dimNames[0];

    // Allocate buffer large enough for staggered variable
    //
    _buf = new char[dimLens[1]];
}

DCMPAS::DerivedVarWRFTime::~DerivedVarWRFTime() {

    if (_buf)
        delete[] _buf;
}

int DCMPAS::DerivedVarWRFTime::Open(size_t ts) {

    if (_is_open)
        return (-1); // Only one variable open at a time

    _fd = -1;

    int fd = _ncdfc->OpenRead(ts, _wrfTimeVar);
    if (fd < 0) {
        SetErrMsg("Can't read %s variable", _wrfTimeVar.c_str());
        return (-1);
    }
    _fd = fd;
    _is_open = true;

    return (0);
}

int DCMPAS::DerivedVarWRFTime::Read(float *buf, int) {

    if (!_is_open) {
        SetErrMsg("Invalid operation");
        return (-1);
    }

    int rc = _ncdfc->Read(_buf, _fd);
    if (rc < 0) {
        SetErrMsg("Can't read %s variable", _wrfTimeVar.c_str());
        return (-1);
    }

    int year, mon, mday, hour, min, sec;
    const char *format = "%4d-%2d-%2d_%2d:%2d:%2d";
    rc = sscanf(_buf, format, &year, &mon, &mday, &hour, &min, &sec);
    if (rc != 6) {
        // Try altnernate, undocumented format. Sigh :-(
        //
        const char *format = "%4d-%5d_%2d:%2d:%2d";
        rc = sscanf(
            _buf, format, &year, &mday, &hour, &min, &sec);
        mon = 1;
        if (rc != 5) {
            SetErrMsg("Unrecognized time stamp: %s", buf);
            return (-1);
        }
    }

    buf[0] = _udunits->EncodeTime(year, mon, mday, hour, min, sec);

    return (0);
}

int DCMPAS::DerivedVarWRFTime::ReadSlice(
    float *slice, int) {
    return (DCMPAS::DerivedVarWRFTime::Read(slice, 0));
}

int DCMPAS::DerivedVarWRFTime::SeekSlice(
    int offset, int whence, int) {
    return (0);
}

int DCMPAS::DerivedVarWRFTime::Close(int) {
    if (!_is_open)
        return (0);

    int rc = 0;
    if (_fd > 0 && _ncdfc->Close(_fd) < 0)
        rc = -1;
    _is_open = false;

    return (rc);
}
