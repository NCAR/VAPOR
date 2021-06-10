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
#include <vapor/DCUtils.h>
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

const vector<string> requiredDimNames = {timeDimName, nCellsDimName, nVerticesDimName, nEdgesDimName, maxEdgesDimName, maxEdges2DimName, vertexDegreeDimName};

const vector<string> optionalVertDimNames = {nVertLevelsDimName};

// Horizontal coordinate variable names
//
const string latCellVarName = "latCell";
const string lonCellVarName = "lonCell";
const string xCellVarName = "xCell";    // Cartesian coord - not used
const string yCellVarName = "yCell";    // Cartesian coord - not used
const string zCellVarName = "zCell";    // Cartesian coord - not used

const string latVertexVarName = "latVertex";
const string lonVertexVarName = "lonVertex";
const string xVertexVarName = "xVertex";    // Cartesian coord - not used
const string yVertexVarName = "yVertex";    // Cartesian coord - not used
const string zVertexVarName = "zVertex";    // Cartesian coord - not used

const string latEdgeVarName = "latEdge";
const string lonEdgeVarName = "lonEdge";
const string xEdgeVarName = "xEdge";    // Cartesian coord - not used
const string yEdgeVarName = "yEdge";    // Cartesian coord - not used
const string zEdgeVarName = "zEdge";    // Cartesian coord - not used

const vector<string> requiredHorizCoordVarNames = {latCellVarName, lonCellVarName, latVertexVarName, lonVertexVarName, latEdgeVarName, lonEdgeVarName};

// Vertical coordinate variables
//
const string zGridP1VarName = "zgrid";    // atmosphere vertical coordinate
const string zTopVarName = "zTop";        // ocean vertical coordinate

const vector<string> optionalVertCoordVarNames = {zGridP1VarName, zTopVarName};

// Time coordinate variables
//
const string xTimeVarName = "xtime";

// Auxilliary variables (connectivity variables and others)
//
const string cellsOnVertexVarName = "cellsOnVertex";
const string verticesOnCellVarName = "verticesOnCell";
const string verticesOnEdge = "verticesOnEdge";
const string edgesOnVertexVarName = "edgesOnVertex";
const string edgesOnCellVarName = "edgesOnCell";
const string nEdgesOnCellVarName = "nEdgesOnCell";
const string angleEdgeVarName = "angleEdge";

// Special data variables
//
const string uNormalVarName = "u";
const string uTangentialVarName = "v";

const vector<string> connectivityVarNames = {cellsOnVertexVarName, verticesOnCellVarName, verticesOnEdge, edgesOnCellVarName};

const vector<string> requiredAuxVarNames = {cellsOnVertexVarName, verticesOnCellVarName, verticesOnEdge, edgesOnCellVarName, nEdgesOnCellVarName};

// Meshes: dual mesh (triangle) and primal (hexagonal cell)
//
const string mesh2DTriName = "mesh2DTri";
const string mesh3DTriName = "mesh3DTri";
const string mesh3DP1TriName = "mesh3DTriP1";
const string mesh2DCellName = "mesh2DCell";
const string mesh3DCellName = "mesh3DCell";
const string mesh3DP1CellName = "mesh3DCellP1";

// Attributes
//
const string coreNameAttr = "core_name";
const string onASphereAttr = "on_a_sphere";

// Derived MPAS-A variable names. These don't appear in the MPAS output.
// They are derived at run-time by the DCMPAS class
//
const string zGridVertP1VarName = "zgridVert";
const string zGridVarName = "zgridM1";
const string zGridVertVarName = "zgridVertM1";

// Derived MPAS-O variable names. These don't appear in the MPAS output.
// They are derived at run-time by the DCMPAS class
//
const string zTopVertVarName = "zTopVert";

const vector<string> requiredAttrNames = {
    //		coreNameAttr,
    onASphereAttr};

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

// Return the name of the mesh for a specific set of dimension names
//
string get_mesh_name(vector<string> dimnames)
{
    VAssert(dimnames.size() == 1 || dimnames.size() == 2);

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
bool is_connectivity_var(string varname) { return (find(connectivityVarNames.begin(), connectivityVarNames.end(), varname) != connectivityVarNames.end()); }

bool is_lat_or_lon(string varname) { return (find(requiredHorizCoordVarNames.begin(), requiredHorizCoordVarNames.end(), varname) != requiredHorizCoordVarNames.end()); }

void rad2degrees(float *buf, size_t n)
{
    for (size_t i = 0; i < n; i++) { buf[i] = buf[i] * 180.0 / M_PI; }
}

// MPAS orders 3D data with vertical axis varying fastest. VAPOR
// wants horizontal dimensions varying fastest. Return true if this is
// a 3D variable with transposed horizontal and vertical dimensions
//
bool isTransposed(NetCDFCollection *ncdfc, string varname)
{
    vector<string> v = ncdfc->GetSpatialDimNames(varname);
    if (v.size() != 2) return (false);

    if ((v[1] == nVertLevelsDimName || v[1] == nVertLevelsP1DimName)) { return (true); }

    return (false);
}

bool isEdgeVariable(NetCDFCollection *ncdfc, string varname)
{
    vector<string> v = ncdfc->GetSpatialDimNames(varname);

    if (v[0] == nEdgesDimName) { return (true); }

    return (false);
}
template<class T> int _xgetVar(NetCDFCollection *ncdfc, size_t ts, string varname, T *buf)
{
    int fd = ncdfc->OpenRead(ts, varname);
    if (fd < 0) return (fd);

    int rc = ncdfc->Read(buf, fd);
    if (rc < 0) return (fd);

    return (ncdfc->Close(fd));
}
};    // namespace

DCMPAS::DCMPAS()
{
    _ncdfc = NULL;
    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
    _cellVars.clear();
    _pointVars.clear();
    _edgeVars.clear();
    _hasVertical = false;
}

DCMPAS::~DCMPAS()
{
    if (_ncdfc) delete _ncdfc;

    vector<string> names = _dvm.GetDataVarNames();
    for (int i = 0; i < names.size(); i++) {
        if (_dvm.GetVar(names[i])) delete _dvm.GetVar(names[i]);
    }
}

int DCMPAS::initialize(const vector<string> &files, const std::vector<string> &options)
{
    // Use UDUnits for unit conversion
    //
    int rc = _udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", _udunits.GetErrMsg().c_str());
        return (-1);
    }

    NetCDFCollection *ncdfc = new NetCDFCollection();

    // Initialize NetCDFCollection class
    //
    vector<string> time_dimnames(1, timeDimName);
    rc = ncdfc->Initialize(files, time_dimnames, vector<string>());
    if (rc < 0) {
        SetErrMsg("Failed to initialize netCDF data collection for reading");
        return (-1);
    }

    // Make sure NetCDF file(s) have everything we need
    //
    rc = _CheckRequiredFields(ncdfc);
    if (rc < 0) return (-1);

    _hasVertical = _HasVertical(ncdfc);

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
    if (rc < 0) { return (-1); }

    // Create derived coordinate variables.
    //
    rc = _InitDerivedVars(ncdfc);
    if (rc < 0) {
        SetErrMsg("Failed to created required derived coordinate variables");
        return (-1);
    }

    rc = _InitMeshes(ncdfc);
    if (rc < 0) return (-1);

    //
    // Identify data and coordinate variables. Sets up members:
    // Initializes members: _dataVarsMap, _coordVarsMap, _meshMap
    //
    rc = _InitAuxVars(ncdfc);
    if (rc < 0) return (-1);

    // Finally, get metdata for all data variables
    //
    rc = _InitDataVars(ncdfc);
    if (rc < 0) return (-1);

    // If u and v variables are present create derived Uzonal and Umeridional
    // variables.
    //
    if (ncdfc->VariableExists(uNormalVarName) && ncdfc->VariableExists(uTangentialVarName)) {
        for (auto varname : {"Uzonal", "Umeridional"}) {
            bool                   zonalFlag = (varname == string("Uzonal"));
            DerivedZonalMeridonal *derivedVar = new DerivedZonalMeridonal(varname, this, ncdfc, uNormalVarName, uTangentialVarName, zonalFlag);

            rc = derivedVar->Initialize();
            if (rc < 0) {
                delete (derivedVar);
                return (-1);
            }

            _dvm.AddDataVar(derivedVar);

            DC::DataVar dvarInfo;
            (void)derivedVar->GetDataVarInfo(dvarInfo);
            _dataVarsMap[varname] = dvarInfo;
        }
    }

    _ncdfc = ncdfc;

    return (0);
}

template<class T> bool DCMPAS::_getAttTemplate(string varname, string attname, T &values) const
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

bool DCMPAS::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCMPAS::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCMPAS::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCMPAS::getAttNames(string varname) const
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

DC::XType DCMPAS::getAttType(string varname, string attname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}

bool DCMPAS::getDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCMPAS::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCMPAS::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCMPAS::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCMPAS::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCMPAS::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCMPAS::getAuxVarInfo(string varname, DC::AuxVar &auxvar) const
{
    map<string, DC::AuxVar>::const_iterator itr;

    itr = _auxVarsMap.find(varname);
    if (itr == _auxVarsMap.end()) { return (false); }

    auxvar = itr->second;
    return (true);
}

bool DCMPAS::getBaseVarInfo(string varname, DC::BaseVar &var) const
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

std::vector<string> DCMPAS::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCMPAS::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCMPAS::getAuxVarNames() const
{
    map<string, DC::AuxVar>::const_iterator itr;

    vector<string> names;
    for (itr = _auxVarsMap.begin(); itr != _auxVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

int DCMPAS::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    if (_dvm.HasVar(varname)) { return (_dvm.GetDimLensAtLevel(varname, 0, dims_at_level, bs_at_level, -1)); }

    bool ok = GetVarDimLens(varname, true, dims_at_level, -1);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

// Read the MPAS nEdgesOnCell auxiliary variable and store it for
// use later
//
int DCMPAS::_read_nEdgesOnCell(size_t ts)
{
    DC::Dimension dimension;
    bool          ok = GetDimension(nCellsDimName, dimension, -1);
    if (!ok) {
        SetErrMsg("Invalid MPAS auxiliary variable: %s", nEdgesOnCellVarName.c_str());
        return (-1);
    }

    int *buf = (int *)_nEdgesOnCellBuf.Alloc(dimension.GetLength() * sizeof(*buf));

    int fd = _ncdfc->OpenRead(ts, nEdgesOnCellVarName);
    if (fd < 0) return (fd);

    int rc = _ncdfc->Read(buf, fd);
    if (rc < 0) return (fd);

    return (_ncdfc->Close(fd));
}

// Read a floating point variable (data or coordinate) into a SmartBuf
//
int DCMPAS::_readVarToSmartBuf(size_t ts, string varname, Wasp::SmartBuf &smartBuf)
{
    vector<size_t> dims;
    bool           ok = GetVarDimLens(varname, true, dims, -1);
    VAssert(ok);

    size_t n = vproduct(dims);
    float *buf = (float *)smartBuf.Alloc(n * sizeof(*buf));

    int fd = _ncdfc->OpenRead(ts, varname);
    if (fd < 0) return (fd);

    int rc = _ncdfc->Read(buf, fd);
    if (rc < 0) return (fd);

    if (is_lat_or_lon(varname)) { rad2degrees((float *)buf, n); }

    return (_ncdfc->Close(fd));
}

// Read the longitude coordinates for cell and vertices. Need these
// to split the periodic mesh
//
int DCMPAS::_readCoordinates(size_t ts)
{
    int rc = _readVarToSmartBuf(ts, lonCellVarName, _lonCellSmartBuf);
    if (rc < 0) return (rc);

    rc = _readVarToSmartBuf(ts, lonVertexVarName, _lonVertexSmartBuf);
    if (rc < 0) return (rc);

    return (0);
}

// MPAS uses an auxiliary array (the variable nEdgesOnCelll) to
// indicate the number of elements (edges or vertices) in
// multi-dimensional arrays with varying dimension lengths. But DC uses
// a padding flag: -1. So using nEdgesOnCell we pad the fixed size
// DC connectivity array with -1.
//
void DCMPAS::_addMissingFlag(int *data) const
{
    int *nEdgesOnCell = (int *)_nEdgesOnCellBuf.GetBuf();

    DC::Dimension dimension;
    bool          ok = GetDimension(nCellsDimName, dimension, -1);
    VAssert(ok);

    size_t nCells = dimension.GetLength();

    ok = GetDimension(maxEdgesDimName, dimension, -1);
    VAssert(ok);

    size_t nMaxEdges = dimension.GetLength();

    // Add padding
    //
    for (size_t j = 0; j < nCells; j++) {
        for (int i = nEdgesOnCell[j]; i < nMaxEdges; i++) { data[j * nMaxEdges + i] = -1; }
    }
}

// MPAS data on sphere is periodic. But we're projecting the geographic
// data to Cartesian coordinates. We need split the cells that straddle
// the split location, here chose to be 180 (-180) degrees
//
void DCMPAS::_splitOnBoundary(string varname, int *connData) const
{
    vector<size_t> connDims;
    bool           ok = GetVarDimLens(varname, true, connDims, -1);
    VAssert(ok && connDims.size() == 2);

    // Dimensions for vertex lon
    //
    vector<size_t> lonVertexDims;
    ok = GetVarDimLens(lonVertexVarName, true, lonVertexDims, -1);
    VAssert(ok && lonVertexDims.size() == 1);

    // Dimensions for cell lon
    //
    vector<size_t> lonCellDims;
    ok = GetVarDimLens(lonCellVarName, true, lonCellDims, -1);
    VAssert(ok && lonCellDims.size() == 1);

    // float *lonBuf1 = NULL;
    float *lonBuf2 = NULL;
    if (connDims[1] == lonVertexDims[0]) {
        // lonBuf1 = (float *) _lonVertexSmartBuf.GetBuf();
        lonBuf2 = (float *)_lonCellSmartBuf.GetBuf();
    } else if (connDims[1] == lonCellDims[0]) {
        // lonBuf1 = (float *) _lonCellSmartBuf.GetBuf();
        lonBuf2 = (float *)_lonVertexSmartBuf.GetBuf();
    } else {
        VAssert(0);
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
        // MPAS apparently uses a 0 to indicate cell boundaries. This is a undocumented feature
        // the we need to handle here
        //
        bool mpas_boundary_marker = false;
        for (size_t i = 0; i < n; i++) {
            if (connData[j * n + i] == 0) {
                for (size_t ii = 0; ii < n; ii++) { connData[j * n + ii] = -2; }
                mpas_boundary_marker = true;
                break;
            }
        }
        if (mpas_boundary_marker) continue;

        // Ha ha. despite MPAS documentation longitude may run -pi to pi
        //
        double lon1 = lonBuf2[connData[j * n] - 1];
        if (lon1 > 180.0) lon1 -= 360.0;
        for (size_t i = 1; i < n && connData[j * n + i] >= 0; i++) {
            size_t index = connData[j * n + i] - 1;    // Arrg. Index starts from 1!!

            double lon2 = lonBuf2[index];

            // Ha ha. despite MPAS documentation longitude may run -pi to pi
            //
            if (lon2 > 180.0) lon2 -= 360.0;

            // Ugh. Test for cell stradling -pi and pi
            //
            if (fabs(lon1 - lon2) > 180.0 * 0.5) { connData[j * n + i] = -2; }
        }
    }
}

int DCMPAS::openVariableRead(size_t ts, string varname, int, int)
{
    int  aux;
    bool derivedFlag;
    if (_dvm.HasVar(varname)) {
        aux = _dvm.OpenVariableRead(ts, varname);
        derivedFlag = true;
    } else {
        aux = _ncdfc->OpenRead(ts, varname);
        derivedFlag = false;

        // Special handling for some auxiliary variables
        //
        if (varname == verticesOnCellVarName) {
            if (_read_nEdgesOnCell(ts) < 0) return (-1);
        }

        if (is_connectivity_var(varname)) {
            if (_readCoordinates(ts) < 0) return (-1);
        }
    }

    MPASFileObject *w = new MPASFileObject(ts, varname, 0, 0, aux, derivedFlag);

    return (_fileTable.AddEntry(w));
}

int DCMPAS::closeVariable(int fd)
{
    MPASFileObject *w = (MPASFileObject *)_fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    int rc;
    if (w->GetDerivedFlag()) {
        rc = _dvm.CloseVariable(aux);
    } else {
        rc = _ncdfc->Close(aux);
    }
    _fileTable.RemoveEntry(fd);

    delete w;
    return (rc);
}

int DCMPAS::_readRegionTransposed(MPASFileObject *w, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 1 || min.size() == 2);
    VAssert(min.size() == max.size());

    int aux = w->GetAux();

    vector<size_t> ncdf_start = min;
    vector<size_t> ncdf_max = max;

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    float *buf = new float[vproduct(ncdf_count)];

    if (min.size() == 2) {
        int rc = _ncdfc->Read(ncdf_start, ncdf_count, buf, aux);
        if (rc < 0) return (-1);

        Wasp::Transpose(buf, region, ncdf_count[1], ncdf_count[0]);
    }
    // No transpose needed. 1D variable
    //
    else {
        int rc = _ncdfc->Read(ncdf_start, ncdf_count, region, aux);
        if (rc < 0) return (-1);
    }

    return (0);
}

int DCMPAS::_readRegionEdgeVariable(MPASFileObject *w, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 1 || min.size() == 2);
    VAssert(min.size() == max.size());

    vector<size_t> dims = _ncdfc->GetDims(edgesOnVertexVarName);
    int *          edgesOnVertex = new int[vproduct(dims)];
    int            rc = _xgetVar(_ncdfc, w->GetTS(), edgesOnVertexVarName, edgesOnVertex);
    if (rc < 0) {
        delete[] edgesOnVertex;
        return (-1);
    }

    size_t vertexDegree = dims[1];
    VAssert(vertexDegree == 3);

    string varname = w->GetVarname();

    // Don't need to reverse dims because we have to do a tranpose anyway
    //
    dims = _ncdfc->GetSpatialDims(varname);
    float *edgeVariable = new float[vproduct(dims)];

    vector<size_t> minAll, maxAll;
    for (int i = 0; i < dims.size(); i++) {
        minAll.push_back(0);
        maxAll.push_back(dims[i] - 1);
    }

    rc = _readRegionTransposed(w, minAll, maxAll, edgeVariable);
    if (rc < 0) {
        delete[] edgesOnVertex;
        delete[] edgeVariable;
        return (-1);
    }

    size_t j0 = min.size() == 2 ? min[1] : 0;
    size_t j1 = max.size() == 2 ? max[1] : 0;

    float wgt = 1.0 / (float)vertexDegree;
    for (size_t j = j0; j <= j1; j++) {
        for (size_t i = min[0], ii = 0; i <= max[0]; i++, ii++) {
            size_t vidx0 = edgesOnVertex[i * vertexDegree + 0] - 1;
            size_t vidx1 = edgesOnVertex[i * vertexDegree + 1] - 1;
            size_t vidx2 = edgesOnVertex[i * vertexDegree + 2] - 1;

            region[j * (max[0] - min[0] + 1) + ii] = edgeVariable[j * dims[0] + vidx0] * wgt + edgeVariable[j * dims[0] + vidx1] * wgt + edgeVariable[j * dims[0] + vidx2] * wgt;
        }
    }

    delete[] edgesOnVertex;
    delete[] edgeVariable;

    return (0);
}

template<class T> int DCMPAS::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    MPASFileObject *w = (MPASFileObject *)_fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int    aux = w->GetAux();
    string varname = w->GetVarname();

    if (w->GetDerivedFlag()) { return (_dvm.ReadRegion(aux, min, max, region)); }

    if (isEdgeVariable(_ncdfc, varname)) {
        VAssert((std::is_same<float *, T *>::value) == true);
        return (_readRegionEdgeVariable(w, min, max, (float *)region));

    } else if (isTransposed(_ncdfc, varname)) {
        VAssert((std::is_same<float *, T *>::value) == true);
        return (_readRegionTransposed(w, min, max, (float *)region));
    }

    // Need to reverse coordinate ordering for NetCDFCollection API, which
    // orders coordinates from slowest to fastest. DC class expects order
    // from fastest to slowest. Unless the variable is tranposed, then
    // we need to "untranspose" it.
    //
    vector<size_t> ncdf_start = min;
    vector<size_t> ncdf_max = max;

    reverse(ncdf_start.begin(), ncdf_start.end());
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    int rc = _ncdfc->Read(ncdf_start, ncdf_count, region, aux);
    if (rc < 0) return (-1);

    // If reading a coordinate variable need to convert from
    // radians to degrees :-(
    //
    if (is_lat_or_lon(varname)) { rad2degrees((float *)region, max[0] - min[0] + 1); }

    // Special handling for some auxiliary variables
    //
    if (varname == verticesOnCellVarName) { _addMissingFlag((int *)region); }

    if (is_connectivity_var(varname)) { _splitOnBoundary(varname, (int *)region); }

    return (0);
}

bool DCMPAS::variableExists(size_t ts, string varname, int, int) const
{
    if (_dvm.HasVar(varname)) { return (_dvm.VariableExists(ts, varname, 0, 0)); }

    return (_ncdfc->VariableExists(ts, varname));
}

int DCMPAS::_InitCoordvars(NetCDFCollection *ncdfc)
{
    string time_dim_name = "";

    // longitude coordinates, native (those found in MPAS output files), and
    // derived (those created on-the-fly by the DCMPAS class. The latter
    // are longitude coordinate variables mapped to PCS coordinates (e.g
    // meters on the ground)
    //
    vector<string> cvars = {lonCellVarName, lonVertexVarName, lonEdgeVarName};
    for (int i = 0; i < cvars.size(); i++) {
        vector<bool>   periodic(true);
        vector<string> dimnames;

        string units = "degrees_east";
        int    axis = 0;
        string name = cvars[i];
        dimnames = ncdfc->GetDimNames(name);
        VAssert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(name, units, DC::FLOAT, periodic, axis, false, dimnames, time_dim_name);

        int rc = DCUtils::CopyAtt(*ncdfc, name, _coordVarsMap[name]);
        if (rc < 0) return (-1);
    }

    // LAtitude coordinate variables
    //
    cvars = {latCellVarName, latVertexVarName, latEdgeVarName};
    for (int i = 0; i < cvars.size(); i++) {
        vector<bool>   periodic(false);
        vector<string> dimnames;

        string units = "degrees_north";
        int    axis = 1;
        string name = cvars[i];
        dimnames = ncdfc->GetDimNames(name);
        VAssert(dimnames.size() == 1);

        _coordVarsMap[name] = CoordVar(name, units, DC::FLOAT, periodic, axis, false, dimnames, time_dim_name);

        int rc = DCUtils::CopyAtt(*ncdfc, name, _coordVarsMap[name]);
        if (rc < 0) return (-1);
    }

    vector<bool>   periodic(false);
    vector<string> dimnames;

    if (_hasVertical && (_isAtmosphere(ncdfc) || _isOcean(ncdfc))) {
        string name;
        if (_isAtmosphere(ncdfc)) {
            name = zGridP1VarName;
        } else {
            name = zTopVarName;
        }

        // Vertical coordinate variables
        //
        string units = "meters";
        int    axis = 2;
        dimnames = ncdfc->GetSpatialDimNames(name);
        time_dim_name = ncdfc->GetTimeDimName(name);
        VAssert(dimnames.size() == 2);

        _coordVarsMap[name] = CoordVar(name, units, DC::FLOAT, periodic, axis, false, dimnames, time_dim_name);
        int rc = DCUtils::CopyAtt(*ncdfc, name, _coordVarsMap[name]);
        if (rc < 0) return (-1);
    }

    return (0);
}

// Create all of the derived coordinate variables
//
int DCMPAS::_InitDerivedVars(NetCDFCollection *ncdfc)
{
    int rc = _InitVerticalCoordinatesDerivedAtmosphere(ncdfc);
    if (rc < 0) return (-1);

    rc = _InitVerticalCoordinatesDerivedOcean(ncdfc);
    if (rc < 0) return (-1);

    DC::CoordVar cvarInfo;
    if (ncdfc->VariableExists(ncdfc->GetNumTimeSteps() - 1, xTimeVarName)) {
        // Create and install the Time coordinate variable
        //
        DerivedCoordVar_WRFTime *derivedVar = new DerivedCoordVar_WRFTime(timeDimName, ncdfc, xTimeVarName, timeDimName);

        rc = derivedVar->Initialize();
        if (rc < 0) return (-1);

        _dvm.AddCoordVar(derivedVar);

        // Time coordinate is a derived variable
        //
        bool ok = _dvm.GetCoordVarInfo(timeDimName, cvarInfo);
        VAssert(ok);
    } else {
        // Synthesize and install a Time coordinate variable
        //
        DerivedCoordVar_Time *derivedVar = new DerivedCoordVar_Time(timeDimName, timeDimName, ncdfc->GetNumTimeSteps());

        rc = derivedVar->Initialize();
        if (rc < 0) return (-1);

        _dvm.AddCoordVar(derivedVar);

        // Time coordinate is a derived variable
        //
        bool ok = _dvm.GetCoordVarInfo(timeDimName, cvarInfo);
        VAssert(ok);
    }

    _coordVarsMap[timeDimName] = cvarInfo;

    return (0);
}

int DCMPAS::_InitVerticalCoordinatesDerivedAtmosphere(NetCDFCollection *ncdfc)
{
    // MPAS-A only outputs a single vertical coordinate variable, zgrid,
    // which is the elevation of the staggered grid, primary (cell) mesh.
    //

    if (!_hasVertical) return (0);
    if (!_isAtmosphere(ncdfc)) return (0);

    DerivedCoordVar *derivedVar = NULL;

    derivedVar = new DerivedCoordVar_UnStaggered(zGridVarName, nVertLevelsDimName, this, zGridP1VarName, nVertLevelsP1DimName);

    int rc = derivedVar->Initialize();
    if (rc < 0) return (-1);
    _dvm.AddCoordVar(derivedVar);

    DC::CoordVar cvarInfo;
    bool         ok = _dvm.GetCoordVarInfo(zGridVarName, cvarInfo);
    VAssert(ok);
    _coordVarsMap[zGridVarName] = cvarInfo;

    derivedVar = new DerivedCoordVertFromCell(zGridVertP1VarName, nVerticesDimName, this, zGridP1VarName, cellsOnVertexVarName);

    rc = derivedVar->Initialize();
    if (rc < 0) return (-1);
    _dvm.AddCoordVar(derivedVar);

    ok = _dvm.GetCoordVarInfo(zGridVertP1VarName, cvarInfo);
    VAssert(ok);
    _coordVarsMap[zGridVertP1VarName] = cvarInfo;

    derivedVar = new DerivedCoordVertFromCell(zGridVertVarName, nVerticesDimName, this, zGridVarName, cellsOnVertexVarName);

    rc = derivedVar->Initialize();
    if (rc < 0) return (-1);
    _dvm.AddCoordVar(derivedVar);

    ok = _dvm.GetCoordVarInfo(zGridVertVarName, cvarInfo);
    VAssert(ok);
    _coordVarsMap[zGridVertVarName] = cvarInfo;

    return (0);
}

int DCMPAS::_InitVerticalCoordinatesDerivedOcean(NetCDFCollection *ncdfc)
{
    // MPAS-O only outputs a single vertical coordinate variable, zTop,
    // which is the elevation of the primary (cell) mesh.
    //

    if (!_hasVertical) return (0);
    if (!_isOcean(ncdfc)) return (0);

    DerivedCoordVar *derivedVar = NULL;

    derivedVar = new DerivedCoordVertFromCell(zTopVertVarName, nVerticesDimName, this, zTopVarName, cellsOnVertexVarName);

    int rc = derivedVar->Initialize();
    if (rc < 0) return (-1);
    _dvm.AddCoordVar(derivedVar);

    DC::CoordVar cvarInfo;
    bool         ok = _dvm.GetCoordVarInfo(zTopVertVarName, cvarInfo);
    VAssert(ok);
    _coordVarsMap[zTopVertVarName] = cvarInfo;

    return (0);
}

int DCMPAS::_CheckRequiredFields(NetCDFCollection *ncdfc) const
{
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

bool DCMPAS::_HasVertical(NetCDFCollection *ncdfc) const
{
    vector<string>::const_iterator itr;

    // Check for dimensions
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    for (int i = 0; i < optionalVertDimNames.size(); i++) {
        string s = optionalVertDimNames[i];

        itr = find(dimnames.begin(), dimnames.end(), s);
        if (itr == dimnames.end()) { return (false); }
    }

    vector<string> varnames;
    for (int ndim = 1; ndim < 3; ndim++) {
        vector<string> v = ncdfc->GetVariableNames(ndim, true);
        varnames.insert(varnames.end(), v.begin(), v.end());
    }

    for (int i = 0; i < optionalVertCoordVarNames.size(); i++) {
        string s = optionalVertCoordVarNames[i];

        itr = find(varnames.begin(), varnames.end(), s);
        if (itr != varnames.end()) { return (true); }
    }
    return (false);
}

// Get Space and time dimensions from MPAS data set. Initialize
// _dimsMap
//
int DCMPAS::_InitDimensions(NetCDFCollection *ncdfc)
{
    _dimsMap.clear();

    // Get dimension names and lengths for all dimensions in the
    // MPAS data set.
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    vector<size_t> dimlens = ncdfc->GetDims();
    VAssert(dimnames.size() == dimlens.size());

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }

    return (0);
}

// Given a data variable name return the variable's dimensions and
// associated coordinate variables.
//
// The order of the returned vectors
// is significant.
//
int DCMPAS::_GetVarCoordinates(NetCDFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar)
{
    sdimnames.clear();
    scoordvars.clear();
    time_dim_name.clear();
    time_coordvar.clear();

    vector<string> dimnames = ncdfc->GetDimNames(varname);
    VAssert(dimnames.size() >= 1);

    if (ncdfc->IsTimeVarying(varname)) {
        time_dim_name = dimnames[0];
        time_coordvar = timeDimName;
        dimnames.erase(dimnames.begin());
    }

    string verticalCellVarName;
    string verticalVertexVarName;
    if (_isAtmosphere(ncdfc)) {
        verticalCellVarName = zGridVarName;
        verticalVertexVarName = zGridVertVarName;
    } else {
        verticalCellVarName = zTopVarName;
        verticalVertexVarName = zTopVertVarName;
    }

    if (find(_cellVars.begin(), _cellVars.end(), varname) != _cellVars.end()) {
        sdimnames.push_back(nCellsDimName);
        scoordvars.push_back(lonCellVarName);
        scoordvars.push_back(latCellVarName);

        if (dimnames.size() > 1) {
            sdimnames.push_back(dimnames[1]);
            if (dimnames[1] == nVertLevelsDimName) {
                scoordvars.push_back(verticalCellVarName);
            } else {
                scoordvars.push_back(zGridP1VarName);
            }
        }
    } else if (find(_pointVars.begin(), _pointVars.end(), varname) != _pointVars.end()) {
        sdimnames.push_back(nVerticesDimName);
        scoordvars.push_back(lonVertexVarName);
        scoordvars.push_back(latVertexVarName);

        if (dimnames.size() > 1) {
            sdimnames.push_back(dimnames[1]);
            if (dimnames[1] == nVertLevelsDimName) {
                scoordvars.push_back(verticalVertexVarName);
            } else {
                scoordvars.push_back(zGridVertP1VarName);
            }
        }
    } else if (find(_edgeVars.begin(), _edgeVars.end(), varname) != _edgeVars.end()) {
        sdimnames.push_back(nEdgesDimName);
        scoordvars.push_back(lonEdgeVarName);
        scoordvars.push_back(latEdgeVarName);

        if (dimnames.size() > 1) {
            sdimnames.push_back(dimnames[1]);
            if (dimnames[1] == nVertLevelsDimName) {
                scoordvars.push_back(verticalCellVarName);
            } else {
                scoordvars.push_back(zGridP1VarName);
            }
        }
    } else {
        VAssert(0);
    }

    return (0);
}

int DCMPAS::_InitMeshes(NetCDFCollection *ncdfc)
{
    // Max vertices or edges per cell
    //
    DC::Dimension dimension;
    bool          ok = GetDimension(maxEdgesDimName, dimension, -1);
    VAssert(ok);

    //
    // Dual meshes (triangle mesh)
    // N.B. for the dual node the meanings of cellsOnVertexVarName,
    // and verticesOnCellVarName are reversed.
    //
    // 2D, layered, and layered with staggered dimensions
    //

    vector<string> coordvars = {lonCellVarName, latCellVarName};
    _meshMap[mesh2DTriName] = Mesh(mesh2DTriName, 3, dimension.GetLength(), nCellsDimName, nVerticesDimName, coordvars, cellsOnVertexVarName, verticesOnCellVarName);

    if ((_isAtmosphere(ncdfc) || _isOcean(ncdfc)) && _hasVertical) {
        if (_isAtmosphere(ncdfc)) {
            coordvars = {lonCellVarName, latCellVarName, zGridP1VarName};
            _meshMap[mesh3DP1TriName] = Mesh(mesh3DP1TriName, 3, dimension.GetLength(), nCellsDimName, nVerticesDimName, nVertLevelsP1DimName, coordvars, cellsOnVertexVarName, verticesOnCellVarName);

            coordvars = {lonCellVarName, latCellVarName, zGridVarName};
        } else {
            coordvars = {lonCellVarName, latCellVarName, zTopVarName};
        }

        _meshMap[mesh3DTriName] = Mesh(mesh3DTriName, 3, dimension.GetLength(), nCellsDimName, nVerticesDimName, nVertLevelsDimName, coordvars, cellsOnVertexVarName, verticesOnCellVarName);
    }

    //
    // Primal meshes (hexagonal mesh)
    //
    // 2D, layered, and layered with staggered dimensions
    //

    coordvars = {lonVertexVarName, latVertexVarName};
    _meshMap[mesh2DCellName] = Mesh(mesh2DCellName, dimension.GetLength(), 3, nVerticesDimName, nCellsDimName, coordvars, verticesOnCellVarName, cellsOnVertexVarName);

    if ((_isAtmosphere(ncdfc) || _isOcean(ncdfc)) && _hasVertical) {
        if (_isAtmosphere(ncdfc)) {
            coordvars = {lonVertexVarName, latVertexVarName, zGridVertP1VarName};
            _meshMap[mesh3DP1CellName] =
                Mesh(mesh3DP1CellName, dimension.GetLength(), 3, nVerticesDimName, nCellsDimName, nVertLevelsP1DimName, coordvars, verticesOnCellVarName, cellsOnVertexVarName);

            coordvars = {lonVertexVarName, latVertexVarName, zGridVertVarName};
        } else {
            coordvars = {lonVertexVarName, latVertexVarName, zTopVertVarName};
        }

        _meshMap[mesh3DCellName] = Mesh(mesh3DCellName, dimension.GetLength(), 3, nVerticesDimName, nCellsDimName, nVertLevelsDimName, coordvars, verticesOnCellVarName, cellsOnVertexVarName);
    }

    return (0);
}

int DCMPAS::_InitAuxVars(NetCDFCollection *ncdfc)
{
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
        // variable type must be int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeInt(type))) { continue; }

        vector<string> dimnames = _GetSpatialDimNames(ncdfc, vars[i]);
        if (!dimnames.size()) continue;

        _auxVarsMap[vars[i]] = AuxVar(vars[i], "", DC::INT32, "", vector<size_t>(), periodic, dimnames);

        // IDs in MPAS files start from 1, not 0 :-(
        //
        if (vars[i] == cellsOnVertexVarName || vars[i] == verticesOnCellVarName || vars[i] == verticesOnEdge || vars[i] == edgesOnCellVarName) { _auxVarsMap[vars[i]].SetOffset(-1); }
    }

    return (0);
}

int DCMPAS::_InitDataVars(NetCDFCollection *ncdfc)
{
    _dataVarsMap.clear();

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
        // variable type must be float
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeFloat(type))) { continue; }

        if (_isCoordVar(vars[i])) continue;

        vector<string> sdimnames;
        vector<string> scoordvars;
        string         time_dim_name;
        string         time_coordvar;

        vector<string> dimnames = _GetSpatialDimNames(ncdfc, vars[i]);
        if (!dimnames.size()) continue;

        if (dimnames[0] == nCellsDimName) {
            _cellVars.push_back(vars[i]);
        } else if (dimnames[0] == nVerticesDimName) {
            _pointVars.push_back(vars[i]);
        } else if (dimnames[0] == nEdgesDimName) {
            // No grid support for edge variables in VAPOR, so we turn
            // them into node-centered data
            //
            //_edgeVars.push_back(vars[i]);
            dimnames[0] = nVerticesDimName;
            _pointVars.push_back(vars[i]);
        } else {
            continue;
        }

        string meshname = get_mesh_name(dimnames);
        if (meshname.empty()) continue;

        int rc = _GetVarCoordinates(ncdfc, vars[i], sdimnames, scoordvars, time_dim_name, time_coordvar);
        if (rc < 0) {
            SetErrMsg("Invalid variable : %s", vars[i].c_str());
            return (-1);
        }

        string units;
        ncdfc->GetAtt(vars[i], "units", units);
        if (!_udunits.ValidUnit(units)) { units = ""; }

        double mv;
        bool   has_missing = ncdfc->GetMissingValue(vars[i], mv);

        if (!has_missing) {
            _dataVarsMap[vars[i]] = DataVar(vars[i], units, DC::FLOAT, periodic, meshname, time_coordvar, DC::Mesh::NODE);
        } else {
            _dataVarsMap[vars[i]] = DataVar(vars[i], units, DC::FLOAT, periodic, meshname, time_coordvar, DC::Mesh::NODE, mv);
        }

        rc = DCUtils::CopyAtt(*ncdfc, vars[i], _dataVarsMap[vars[i]]);
        if (rc < 0) return (-1);
    }

    return (0);
}

vector<string> DCMPAS::_GetSpatialDimNames(NetCDFCollection *ncdfc, string varname) const
{
    vector<string> v = ncdfc->GetSpatialDimNames(varname);
    if (v.size() == 0) return (v);

    if (!isTransposed(ncdfc, varname)) { reverse(v.begin(), v.end()); }

    return (v);
}

// Atmosphere core configuration ?
//
bool DCMPAS::_isAtmosphere(NetCDFCollection *ncdfc) const
{
    string value;
    ncdfc->GetAtt("", coreNameAttr, value);

    return (value == "" || value == "atmosphere");
}

// Ocean core configuration ?
//
bool DCMPAS::_isOcean(NetCDFCollection *ncdfc) const
{
    string value;
    ncdfc->GetAtt("", coreNameAttr, value);

    return (value == "ocean");
}

//
//
bool DCMPAS::_isCoordVar(string varname) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }
    return (true);
}

bool DCMPAS::_isDataVar(string varname) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }
    return (true);
}

//////////////////////////////////////////////////////////////////////
//
// Class definitions for derived coordinate variables
//
//////////////////////////////////////////////////////////////////////

DCMPAS::DerivedCoordVertFromCell::DerivedCoordVertFromCell(string derivedVarName, string derivedDimName, DC *dc, string inName, string cellsOnVertexName

                                                           )
: DerivedCoordVar(derivedVarName)
{
    _derivedDimName = derivedDimName;
    _dc = dc;
    _inName = inName;
    _cellsOnVertexName = cellsOnVertexName;
}

int DCMPAS::DerivedCoordVertFromCell::Initialize()
{
    // Set up CoordVarInfo for derived variable. Only difference between
    // it and native variable is dimension name for first dimension
    //
    bool status = _dc->GetCoordVarInfo(_inName, _coordVarInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _inName.c_str());
        return (-1);
    }

    vector<string> dimNames = _coordVarInfo.GetDimNames();
    VAssert(dimNames.size());

    dimNames[0] = _derivedDimName;

    _coordVarInfo.SetDimNames(dimNames);

    return (0);
}

bool DCMPAS::DerivedCoordVertFromCell::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DCMPAS::DerivedCoordVertFromCell::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DCMPAS::DerivedCoordVertFromCell::GetDimLensAtLevel(int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    int rc = _dc->GetDimLensAtLevel(_inName, -1, dims_at_level, bs_at_level, -1);
    if (rc < 0) return (-1);

    DC::Dimension dimension;
    bool          ok = _dc->GetDimension(_derivedDimName, dimension, -1);
    if (!ok) {
        SetErrMsg("Invalid dimension name : %s", _derivedDimName.c_str());
        return (-1);
    }

    dims_at_level[0] = dimension.GetLength();

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DCMPAS::DerivedCoordVertFromCell::OpenVariableRead(size_t ts, int, int)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, 0, 0, 0);

    return (_fileTable.AddEntry(f));
}

int DCMPAS::DerivedCoordVertFromCell::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;

    return (0);
}

float *DCMPAS::DerivedCoordVertFromCell::_getCellData()
{
    // Dimensions of input (cell) grid:
    //
    vector<size_t> inDims, dummy;
    int            rc = _dc->GetDimLensAtLevel(_inName, -1, inDims, dummy, -1);
    if (rc < 0) return (NULL);

    vector<size_t> inMin, inMax;
    for (int i = 0; i < inDims.size(); i++) {
        inMin.push_back(0);
        inMax.push_back(inDims[i] - 1);
    }

    float *buf = new float[vproduct(inDims)];

    rc = _getVar(_dc, 0, _inName, -1, -1, inMin, inMax, buf);
    if (rc < 0) {
        delete[] buf;
        return (NULL);
    }

    return (buf);
}

int *DCMPAS::DerivedCoordVertFromCell::_getCellsOnVertex(size_t i0, size_t i1, int &vertexDegree)
{
    vertexDegree = 0;

    vector<size_t> dims;
    bool           ok = _dc->GetVarDimLens(_cellsOnVertexName, true, dims, -1);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", _cellsOnVertexName.c_str());
        return (NULL);
    }

    int fd = _dc->OpenVariableRead(0, _cellsOnVertexName, 0, 0);

    vertexDegree = dims[0];

    int *buf = new int[(i1 - i0 + 1) * vertexDegree];

    vector<size_t> min, max;
    min.push_back(0);
    min.push_back(i0);
    max.push_back(vertexDegree - 1);
    max.push_back(i1);

    int rc = _dc->ReadRegion(fd, min, max, buf);
    if (rc < 0) {
        delete[] buf;
        return (NULL);
    }

    _dc->CloseVariable(fd);

    return (buf);
}

int DCMPAS::DerivedCoordVertFromCell::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);
    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    string varname = f->GetVarname();

    vector<size_t> inDims, dummy;
    int            rc = _dc->GetDimLensAtLevel(_inName, -1, inDims, dummy, -1);
    if (rc < 0) return (-1);

    float *cellData = _getCellData();
    if (!cellData) return (-1);

    int  vertexDegree;
    int *cellsOnVertex = _getCellsOnVertex(min[0], max[0], vertexDegree);

    // only handle triangles for dual mesh
    //
    VAssert(vertexDegree == 3);

    if (!cellsOnVertex) {
        delete[] cellData;
        return (-1);
    }

    size_t ny = min.size() >= 2 ? max[1] - min[1] + 1 : 1;
    size_t nx = min.size() >= 1 ? max[0] - min[0] + 1 : 1;

    // Interpolation weights. Assume interpolated sample is at geometric
    // center of triangle
    //
    float wgt0 = 1.0 / 3.0;
    float wgt1 = 1.0 / 3.0;
    float wgt2 = 1.0 / 3.0;

    int offset = -1;    // indexing in MPAS starts from -1
    for (size_t j = 0; j < ny; j++) {
        for (size_t i = 0; i < nx; i++) {
            float v0 = cellData[j * inDims[0] + cellsOnVertex[i * vertexDegree + 0] + offset];
            float v1 = cellData[j * inDims[0] + cellsOnVertex[i * vertexDegree + 1] + offset];
            float v2 = cellData[j * inDims[0] + cellsOnVertex[i * vertexDegree + 2] + offset];

            region[j * nx + i] = v0 * wgt0 + v1 * wgt1 + v2 * wgt2;
        }
    }

    delete[] cellData;
    delete[] cellsOnVertex;

    return (0);
}

bool DCMPAS::DerivedCoordVertFromCell::VariableExists(size_t ts, int, int) const { return (_dc->VariableExists(ts, _inName, -1, -1)); }

//////////////////////////////////////////////////////////////////////
//
// Class definitions for derived data variables
//
//////////////////////////////////////////////////////////////////////

DCMPAS::DerivedZonalMeridonal::DerivedZonalMeridonal(string derivedVarName, DC *dc, NetCDFCollection *ncdfc, string normalVarName, string tangentialVarName, bool zonalFlag

                                                     )
: DerivedDataVar(derivedVarName)
{
    _dc = dc;
    _ncdfc = ncdfc;
    _normalVarName = normalVarName;
    _tangentialVarName = tangentialVarName;
    _zonalFlag = zonalFlag;
}

int DCMPAS::DerivedZonalMeridonal::Initialize()
{
    // Set up DataVarInfo for derived variable. No difference between
    // it and native variables derived from
    //
    bool status = _dc->GetDataVarInfo(_normalVarName, _dataVarInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _normalVarName.c_str());
        return (-1);
    }

    return (0);
}

bool DCMPAS::DerivedZonalMeridonal::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _dataVarInfo;
    return (true);
}

bool DCMPAS::DerivedZonalMeridonal::GetDataVarInfo(DC::DataVar &cvar) const
{
    cvar = _dataVarInfo;
    return (true);
}

int DCMPAS::DerivedZonalMeridonal::GetDimLensAtLevel(int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    int rc = _dc->GetDimLensAtLevel(_normalVarName, -1, dims_at_level, bs_at_level, -1);
    if (rc < 0) return (-1);

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DCMPAS::DerivedZonalMeridonal::OpenVariableRead(size_t ts, int, int)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, 0, 0, 0);

    return (_fileTable.AddEntry(f));
}

int DCMPAS::DerivedZonalMeridonal::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;

    return (0);
}

int DCMPAS::DerivedZonalMeridonal::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 2);
    VAssert(min.size() == max.size());

    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);
    size_t                     ts = f->GetTS();

    vector<size_t> dims = _ncdfc->GetDims(edgesOnVertexVarName);
    vector<int>    edgesOnVertex(vproduct(dims));
    int            rc = _xgetVar(_ncdfc, ts, edgesOnVertexVarName, edgesOnVertex.data());
    if (rc < 0) return (-1);

    size_t vertexDegree = dims[1];

    dims = _ncdfc->GetDims(angleEdgeVarName);
    vector<float> angleEdge(vproduct(dims));
    rc = _xgetVar(_ncdfc, ts, angleEdgeVarName, angleEdge.data());
    if (rc < 0) return (-1);

    dims = _ncdfc->GetSpatialDims(_normalVarName);
    vector<size_t> ncdf_start = {0, min[1]};
    vector<size_t> ncdf_count = {dims[0], max[1] - min[1] + 1};

    vector<float> u(vproduct(ncdf_count));
    vector<float> v(vproduct(ncdf_count));
    vector<float> buf(vproduct(ncdf_count));

    int myfd = _ncdfc->OpenRead(ts, _normalVarName);
    if (rc < 0) return (-1);

    rc = _ncdfc->Read(ncdf_start, ncdf_count, buf.data(), myfd);
    if (rc < 0) return (-1);

    Wasp::Transpose(buf.data(), u.data(), ncdf_count[1], ncdf_count[0]);

    (void)_ncdfc->Close(myfd);

    myfd = _ncdfc->OpenRead(ts, _tangentialVarName);
    if (rc < 0) return (-1);

    rc = _ncdfc->Read(ncdf_start, ncdf_count, buf.data(), myfd);
    if (rc < 0) return (-1);

    Wasp::Transpose(buf.data(), v.data(), ncdf_count[1], ncdf_count[0]);

    (void)_ncdfc->Close(myfd);

    size_t j0 = min.size() == 2 ? min[1] : 0;
    size_t j1 = max.size() == 2 ? max[1] : 0;

    float wgt = 1.0 / (float)vertexDegree;
    for (size_t j = j0; j <= j1; j++) {
        for (size_t i = min[0], ii = 0; i <= max[0]; i++, ii++) {
            size_t vidx0 = edgesOnVertex[i * vertexDegree + 0] - 1;
            size_t vidx1 = edgesOnVertex[i * vertexDegree + 1] - 1;
            size_t vidx2 = edgesOnVertex[i * vertexDegree + 2] - 1;

            float alpha0 = angleEdge[vidx0];
            float alpha1 = angleEdge[vidx1];
            float alpha2 = angleEdge[vidx2];

            float u0, u1, u2;

            //
            // |Um| = |cos(alpha)    -sin(alpha)|   |u|
            // |  |   |                         | x | |
            // |Uz| = |sin(alpha)    cos(alpha) |   |v|
            //
            if (_zonalFlag) {
                u0 = (cos(alpha0) * u.data()[j * dims[0] + vidx0]) - (sin(alpha0) * v.data()[j * dims[0] + vidx0]);

                u1 = (cos(alpha1) * u.data()[j * dims[0] + vidx1]) - (sin(alpha1) * v.data()[j * dims[0] + vidx1]);

                u2 = (cos(alpha2) * u.data()[j * dims[0] + vidx2]) - (sin(alpha2) * v.data()[j * dims[0] + vidx2]);
            } else {
                u0 = (sin(alpha0) * u.data()[j * dims[0] + vidx0]) + (cos(alpha0) * v.data()[j * dims[0] + vidx0]);

                u1 = (sin(alpha1) * u.data()[j * dims[0] + vidx1]) + (cos(alpha1) * v.data()[j * dims[0] + vidx1]);

                u2 = (sin(alpha2) * u.data()[j * dims[0] + vidx2]) + (cos(alpha2) * v.data()[j * dims[0] + vidx2]);
            }

            region[j * (max[0] - min[0] + 1) + ii] = u0 * wgt + u1 * wgt + u2 * wgt;
        }
    }

    return (0);
}

bool DCMPAS::DerivedZonalMeridonal::VariableExists(size_t ts, int, int) const { return (_dc->VariableExists(ts, _normalVarName, -1, -1) && _dc->VariableExists(ts, _tangentialVarName, -1, -1)); }
