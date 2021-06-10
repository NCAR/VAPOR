#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include "vapor/VAssert.h"
#include <cassert>
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DCUtils.h>
#include <vapor/DCWRF.h>
#include <vapor/DerivedVar.h>
#include <vapor/utils.h>
#include <netcdf.h>

using namespace VAPoR;
using namespace std;

namespace {
#ifdef UNUSED_FUNCTION
bool mycompare(const pair<int, float> &a, const pair<int, float> &b) { return (a.second < b.second); }
#endif

float read_scalar_float_attr(NetCDFCollection *ncdfc, string varname, string attrname, float default_value)
{
    vector<double> dvalues;
    ncdfc->GetAtt(varname, attrname, dvalues);
    if (dvalues.size() != 1) return (default_value);

    return ((float)dvalues[0]);
}

};    // namespace

DCWRF::DCWRF()
{
    _ncdfc = NULL;

    _dx = 0.0;
    _dy = 0.0;
    _cen_lat = 0.0;
    _cen_lon = 0.0;
    _true_lat1 = 0.0;
    _true_lat2 = 0.0;
    _pole_lat = 90.0;
    _pole_lon = 0.0;
    _grav = 9.81;
    _radius = 0.0;
    _p2si = 1.0;
    _mapProj = 0;

    _proj4String.clear();

    _derivedVars.clear();
    _derivedTime = NULL;

    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
}

DCWRF::~DCWRF()
{
    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i]) delete _derivedVars[i];
    }
    if (_derivedTime) delete _derivedTime;

    if (_ncdfc) delete _ncdfc;
}

int DCWRF::initialize(const vector<string> &files, const std::vector<string> &options)
{
    NetCDFCollection *ncdfc = new NetCDFCollection();

    // Initialize the NetCDFCollection class. Need to specify the name
    // of the time dimension ("Time" for WRF), and time coordinate variable
    // names (N/A for WRF)
    //
    vector<string> time_dimnames;
    vector<string> time_coordvars;
    time_dimnames.push_back("Time");
    int rc = ncdfc->Initialize(files, time_dimnames, time_coordvars);
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

    // Get required and optional global attributes  from WRF files.
    // Initializes members: _dx, _dy, _cen_lat, _cen_lon, _pole_lat,
    // _pole_lon, _grav, _radius, _p2si
    //
    rc = _InitAtts(ncdfc);
    if (rc < 0) return (-1);

    //
    //  Get the dimensions of the grid.
    //	Initializes members: _dimsMap
    //
    rc = _InitDimensions(ncdfc);
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    // Set up map projection transforms
    // Initializes members: _proj4String, _mapProj
    //
    rc = _InitProjection(ncdfc, _radius);
    if (rc < 0) { return (-1); }

    // Set up the horizontal coordinate variables
    //
    // Initializes members: _coordVarMap
    //
    rc = _InitHorizontalCoordinates(ncdfc);
    if (rc < 0) { return (-1); }

    // Set up the vertical coordinate variable. WRF data set doesn't
    // provide one.
    //
    // Initializes members: _coordVarMap
    //
    rc = _InitVerticalCoordinates(ncdfc);
    if (rc < 0) { return (-1); }

    // Set up user time coordinate derived variable . Time must be
    // in seconds.
    // Initializes members: _coordVarsMap
    //
    rc = _InitTime(ncdfc);
    if (rc < 0) { return (-1); }

    //
    // Identify data and coordinate variables. Sets up members:
    // Initializes members: _dataVarsMap, _meshMap, _coordVarsMap
    //
    rc = _InitVars(ncdfc);
    if (rc < 0) return (-1);

    _ncdfc = ncdfc;

    return (0);
}

bool DCWRF::getDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCWRF::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCWRF::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCWRF::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCWRF::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCWRF::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCWRF::getBaseVarInfo(string varname, DC::BaseVar &var) const
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

std::vector<string> DCWRF::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCWRF::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

template<class T> bool DCWRF::_getAttTemplate(string varname, string attname, T &values) const
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

bool DCWRF::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCWRF::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCWRF::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCWRF::getAttNames(string varname) const
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

DC::XType DCWRF::getAttType(string varname, string attname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}
int DCWRF::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    bool ok;
    if (_dvm.IsCoordVar(varname)) {
        return (_dvm.GetDimLensAtLevel(varname, 0, dims_at_level, bs_at_level, -1));
    } else {
        ok = GetVarDimLens(varname, true, dims_at_level, -1);
    }
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

string DCWRF::getMapProjection() const { return (_proj4String); }

int DCWRF::openVariableRead(size_t ts, string varname)
{
    if (ts >= _ncdfc->GetNumTimeSteps()) {
        SetErrMsg("Time step out of range : %d", ts);
        return (-1);
    }
    ts = _derivedTime->TimeLookup(ts);

    int  fd;
    bool derivedFlag;
    if (_dvm.IsCoordVar(varname)) {
        fd = _dvm.OpenVariableRead(ts, varname);
        derivedFlag = true;
    } else {
        fd = _ncdfc->OpenRead(ts, varname);
        derivedFlag = false;
    }
    if (fd < 0) return (-1);

    WRFFileObject *w = new WRFFileObject(ts, varname, 0, 0, fd, derivedFlag);

    return (_fileTable.AddEntry(w));
}

int DCWRF::closeVariable(int fd)
{
    WRFFileObject *w = (WRFFileObject *)_fileTable.GetEntry(fd);

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

template<class T> int DCWRF::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    VAssert(min.size() == max.size());

    WRFFileObject *w = (WRFFileObject *)_fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    if (w->GetDerivedFlag()) { return (_dvm.ReadRegion(aux, min, max, region)); }

    vector<size_t> ncdf_start = min;
    reverse(ncdf_start.begin(), ncdf_start.end());

    vector<size_t> ncdf_max = max;
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    return (_ncdfc->Read(ncdf_start, ncdf_count, region, aux));
}

bool DCWRF::variableExists(size_t ts, string varname, int, int) const
{
    if (ts >= _ncdfc->GetNumTimeSteps()) { return (false); }
    ts = _derivedTime->TimeLookup(ts);

    if (_dvm.IsCoordVar(varname)) { return (_dvm.VariableExists(ts, varname, 0, 0)); }
    return (_ncdfc->VariableExists(ts, varname));
}

vector<size_t> DCWRF::_GetSpatialDims(NetCDFCollection *ncdfc, string varname) const
{
    vector<size_t> dims = ncdfc->GetSpatialDims(varname);
    reverse(dims.begin(), dims.end());
    return (dims);
}

vector<string> DCWRF::_GetSpatialDimNames(NetCDFCollection *ncdfc, string varname) const
{
    vector<string> v = ncdfc->GetSpatialDimNames(varname);
    reverse(v.begin(), v.end());
    return (v);
}

//
// Read select attributes from the WRF files. Most of the attributes are
// needed for map projections
//
int DCWRF::_InitAtts(NetCDFCollection *ncdfc)
{
    _dx = read_scalar_float_attr(ncdfc, "", "DX", _dx);
    _dy = read_scalar_float_attr(ncdfc, "", "DY", _dy);
    _cen_lat = read_scalar_float_attr(ncdfc, "", "CEN_LAT", _cen_lat);
    _cen_lon = read_scalar_float_attr(ncdfc, "", "CEN_LON", _cen_lon);
    _true_lat1 = read_scalar_float_attr(ncdfc, "", "TRUELAT1", _true_lat1);
    _true_lat2 = read_scalar_float_attr(ncdfc, "", "TRUELAT2", _true_lat2);
    _pole_lat = read_scalar_float_attr(ncdfc, "", "POLE_LAT", _pole_lat);
    _pole_lon = read_scalar_float_attr(ncdfc, "", "POLE_LON", _pole_lon);

    //
    // "PlanetWRF" attributes
    //
    // RADIUS is the radius of the planet
    //
    // P2SI is the number of SI seconds in an planetary solar day
    // divided by the number of SI seconds in an earth solar day
    //
    vector<double> dvalues;
    ncdfc->GetAtt("", "G", dvalues);
    if (dvalues.size() == 1) {
        _grav = dvalues[0];

        _radius = read_scalar_float_attr(ncdfc, "", "RADIUS", _radius);
        _p2si = read_scalar_float_attr(ncdfc, "", "P2SI", _p2si);
    }

    return (0);
}

//
// Generate a Proj4 projection string for whatever map projection is used
// by the data. Map projection type is indicated by map_proj
// The Proj4 string will be used to transform from geographic coordinates
// measured in degrees to Cartographic coordinates in meters.
//
int DCWRF::_GetProj4String(NetCDFCollection *ncdfc, float radius, int map_proj, string &projstring)
{
    projstring.clear();
    ostringstream oss;

    vector<double> dvalues;
    switch (map_proj) {
    case 0: {    // Lat Lon

        double lon_0 = _cen_lon;
        double lat_0 = _cen_lat;
        oss << " +lon_0=" << lon_0 << " +lat_0=" << lat_0;
        projstring = "+proj=eqc +ellps=WGS84" + oss.str();

    } break;
    case 1: {    // Lambert
        ncdfc->GetAtt("", "STAND_LON", dvalues);
        if (dvalues.size() != 1) {
            SetErrMsg("Error reading required attribute : STAND_LON");
            return (-1);
        }
        float lon0 = dvalues[0];

        // Construct the projection string:
        projstring = "+proj=lcc";
        projstring += " +lon_0=";
        oss.str("");
        oss << (double)lon0;
        projstring += oss.str();

        projstring += " +lat_1=";
        oss.str("");
        oss << (double)_true_lat1;
        projstring += oss.str();

        projstring += " +lat_2=";
        oss.str("");
        oss << (double)_true_lat2;
        projstring += oss.str();

        break;
    }

    case 2: {    // Polar stereographic (pure north or south)
        projstring = "+proj=stere";

        // Determine whether north or south pole (lat_ts is pos or neg)

        float lat0;
        if (_true_lat1 < 0.)
            lat0 = -90.0;
        else
            lat0 = 90.0;

        projstring += " +lat_0=";
        oss.str("");
        oss << (double)lat0;
        projstring += oss.str();

        projstring += " +lat_ts=";
        oss.str("");
        oss << (double)_true_lat1;
        projstring += oss.str();

        ncdfc->GetAtt("", "STAND_LON", dvalues);
        if (dvalues.size() != 1) {
            SetErrMsg("Error reading required attribute : STAND_LON");
            return (-1);
        }
        float lon0 = dvalues[0];

        projstring += " +lon_0=";
        oss.str("");
        oss << (double)lon0;
        projstring += oss.str();

        break;
    }

    case (3): {    // Mercator

        ncdfc->GetAtt("", "STAND_LON", dvalues);
        if (dvalues.size() != 1) {
            SetErrMsg("Error reading required attribute : STAND_LON");
            return (-1);
        }
        float lon0 = dvalues[0];

        // Construct the projection string:
        projstring = "+proj=merc";

        projstring += " +lon_0=";
        oss.str("");
        oss << (double)lon0;
        projstring += oss.str();

        projstring += " +lat_ts=";
        oss.str("");
        oss << (double)_true_lat1;
        projstring += oss.str();

        break;
    }

    case (6): {    // lat-long, possibly rotated, possibly cassini

        // See if this is a regular cylindrical equidistant projection
        // with the pole in the default location
        //
        if (_pole_lat == 90.0 && _pole_lon == 0.0) {
            double        lon_0 = _cen_lon;
            double        lat_0 = _cen_lat;
            ostringstream oss;
            oss << " +lon_0=" << lon_0 << " +lat_0=" << lat_0;
            projstring = "+proj=eqc +ellps=WGS84" + oss.str();
        } else {
            //
            // Assume arbitrary pole displacement. Probably should
            // check for cassini projection (transverse cylindrical)
            // but general rotated cyl. equidist. projection should work
            //
            ncdfc->GetAtt("", "STAND_LON", dvalues);
            if (dvalues.size() != 1) {
                SetErrMsg("Error reading required attribute : STAND_LON");
                return (-1);
            }
            float lon0 = dvalues[0];

            projstring = "+proj=ob_tran";
            projstring += " +o_proj=eqc";
            projstring += " +to_meter=0.0174532925199";

            projstring += " +o_lat_p=";
            oss.str("");
            oss << (double)_pole_lat;
            projstring += oss.str();
            projstring += "d";    // degrees, not radians

            projstring += " +o_lon_p=";
            oss.str("");
            oss << (double)(180. - _pole_lon);
            projstring += oss.str();
            projstring += "d";    // degrees, not radians

            projstring += " +lon_0=";
            oss.str("");
            oss << (double)(-lon0);
            projstring += oss.str();
            projstring += "d";    // degrees, not radians
        }

        break;
    }
    default: {
        SetErrMsg("Unsupported MAP_PROJ value : %d", _mapProj);
        return -1;
    }
    }

    if (projstring.empty()) return (0);

    //
    // PlanetWRF data if radius is not zero
    //
    if (radius > 0) {    // planetWRf (not on earth)
        projstring += " +ellps=sphere";
        stringstream ss;
        ss << radius;
        projstring += " +a=" + ss.str() + " +es=0";
    } else {
        projstring += " +ellps=WGS84";
    }

    return (0);
}

// Check to see if the horizontal coordinates, XLONG and XLAT, are
// constant value. For simplicity, only check the
// first two elements of each array.
//
// N.B. older WRF files (pre 4.x) don't have an attribute that identifies
// the files as being "idealized"; for idealized cases the horizontal
// coordinates are initialized to constant values.
//
bool DCWRF::_isConstantValuedVariable(NetCDFCollection *ncdfc, string varname) const
{
    bool enabled = EnableErrMsg(false);

    vector<float> data;

    vector<size_t> dims = ncdfc->GetSpatialDims(varname);

    vector<size_t> start(dims.size(), 0);
    vector<size_t> count = dims;

    // Edge case.
    //
    if (Wasp::VProduct(dims) < 2) return (true);

    data.resize(Wasp::VProduct(dims));

    int fd = ncdfc->OpenRead(0, varname);
    if (fd < 0) {
        EnableErrMsg(enabled);
        return (false);
    }

    int rc = ncdfc->Read(start.data(), count.data(), data.data(), fd);
    if (rc < 0) {
        EnableErrMsg(enabled);
        return (false);
    }

    ncdfc->Close(fd);

    float a0 = data[0];
    float epsilon = 0.000001;
    for (size_t i = 1; i < Wasp::VProduct(dims); i++) {
        if (!Wasp::NearlyEqual(a0, data[i], epsilon)) {
            EnableErrMsg(enabled);
            return (false);
        }
    }

    EnableErrMsg(enabled);

    return (true);
}

bool DCWRF::_isIdealized(NetCDFCollection *ncdfc) const
{
    // Version 4.0 of WRF introduced "IDEAL_CASE" attribute.
    // For earlier versions we look for hints
    //
    vector<long> l;
    ncdfc->GetAtt("", "IDEAL_CASE", l);
    if (l.size() && l[0] != 0) return (true);

    string s;
    ncdfc->GetAtt("", "MAP_PROJ_CHAR", s);
    if (Wasp::StrCmpNoCase(s, "Cartesian") == 0) return (true);

    ncdfc->GetAtt("", "SIMULATION_INITIALIZATION_TYPE", s);
    if (Wasp::StrCmpNoCase(s, "IDEALIZED DATA") == 0) return (true);

    // Pre version 4.x WRF did not have an attribute to identify
    // idealized cases. However, these cases have constant-valued
    // XLONG and XLAT coordinates. N.B. The staggered grid horizontal
    // coordinates (XLONG_U, XLAT_V, etc.) may not exist.
    //
    vector<string> cvars = {"XLONG", "XLAT"};
    for (auto &c : cvars) {
        if (_isConstantValuedVariable(ncdfc, c)) return (true);
    }

    if (_isWRFSFIRE(ncdfc) && _cen_lat == 0.0 && _cen_lon == 0.0 && _true_lat1 == 0.0 && _true_lat2 == 0.0) { return (true); }

    return (false);
}

bool DCWRF::_isWRFSFIRE(NetCDFCollection *ncdfc) const
{
    if (!ncdfc->VariableExists("FXLONG")) return (false);

    if (!ncdfc->VariableExists("FXLAT")) return (false);

    return (true);
}

//
// Set up map projection stuff
//
int DCWRF::_InitProjection(NetCDFCollection *ncdfc, float radius)
{
    _proj4String.clear();
    _mapProj = 0;

    if (_isIdealized(ncdfc)) return (0);

    vector<long> ivalues;
    ncdfc->GetAtt("", "MAP_PROJ", ivalues);
    if (ivalues.size() != 1) {
        SetErrMsg("Error reading required attribute : MAP_PROJ");
        return (-1);
    }
    _mapProj = ivalues[0];

    int rc = _GetProj4String(ncdfc, radius, _mapProj, _proj4String);
    if (rc < 0) return (rc);

    return (0);
}

DerivedCoordVar_CF2D *DCWRF::_makeDerivedHorizontalIdealized(NetCDFCollection *ncdfc, string name, string &timeDimName, vector<string> &spaceDimNames)
{
    timeDimName.clear();
    spaceDimNames.clear();

    timeDimName = ncdfc->GetTimeDimName(name);
    spaceDimNames = ncdfc->GetSpatialDimNames(name);
    reverse(spaceDimNames.begin(), spaceDimNames.end());

    vector<size_t> dimLens = ncdfc->GetSpatialDims(name);
    reverse(dimLens.begin(), dimLens.end());
    VAssert(dimLens.size() == 2);

    vector<float> data;
    data.resize(dimLens[0] * dimLens[1]);

    string units = "km";
    float  offset = 0.0;
    int    axis = 0;
    if (name == "XLONG") {
        axis = 0;
        offset = _dx / 2.0;
    } else if (name == "XLAT") {
        axis = 1;
        offset = _dy / 2.0;
    } else if (name == "XLONG_U") {
        axis = 0;
        offset = 0.0;
    } else if (name == "XLAT_U") {
        axis = 1;
        offset = _dy / 2.0;
    } else if (name == "XLONG_V") {
        axis = 0;
        offset = _dx / 2.0;
    } else if (name == "XLAT_V") {
        axis = 1;
        offset = 0.0;
    } else {
        return (NULL);
    }

    if (axis == 0) {
        for (size_t j = 0; j < dimLens[1]; j++) {
            for (size_t i = 0; i < dimLens[0]; i++) { data[j * dimLens[0] + i] = i * _dx + offset; }
        }
    } else {
        for (size_t j = 0; j < dimLens[1]; j++) {
            for (size_t i = 0; i < dimLens[0]; i++) { data[j * dimLens[0] + i] = j * _dy + offset; }
        }
    }

    DerivedCoordVar_CF2D *derivedVar = new DerivedCoordVar_CF2D(name, spaceDimNames, dimLens, axis, units, data);

    int rc = derivedVar->Initialize();
    if (rc < 0) return (NULL);

    _dvm.AddCoordVar(derivedVar);

    return (derivedVar);
}

DerivedCoordVar_Staggered *DCWRF::_makeDerivedHorizontalStaggered(NetCDFCollection *ncdfc, string name, string &timeDimName, vector<string> &spaceDimNames)
{
    timeDimName.clear();
    spaceDimNames.clear();

    int    stagDim;
    string stagDimName;
    string inName;
    string dimName;
    if (name == "XLONG_U") {
        stagDim = 0;
        stagDimName = "west_east_stag";
        inName = "XLONG";
        dimName = "west_east";
    } else if (name == "XLAT_U") {
        stagDim = 0;
        stagDimName = "west_east_stag";
        inName = "XLAT";
        dimName = "west_east";
    } else if (name == "XLONG_V") {
        stagDim = 1;
        stagDimName = "south_north_stag";
        inName = "XLONG";
        dimName = "south_north";
    } else if (name == "XLAT_V") {
        stagDim = 1;
        stagDimName = "south_north_stag";
        inName = "XLAT";
        dimName = "south_north";
    } else {
        return (NULL);
    }

    timeDimName = ncdfc->GetTimeDimName(inName);
    spaceDimNames = ncdfc->GetSpatialDimNames(inName);
    reverse(spaceDimNames.begin(), spaceDimNames.end());

    spaceDimNames[stagDim] = stagDimName;

    DerivedCoordVar_Staggered *derivedVar = new DerivedCoordVar_Staggered(name, stagDimName, this, inName, dimName);
    int                        rc = derivedVar->Initialize();
    if (rc < 0) return (NULL);

    _dvm.AddCoordVar(derivedVar);

    return (derivedVar);
}

int DCWRF::_InitHorizontalCoordinatesHelper(NetCDFCollection *ncdfc, string name, int axis)
{
    VAssert(axis == 0 || axis == 1);

    DerivedVar *derivedVar = NULL;

    string         timeDimName;
    vector<string> spaceDimNames;

    //
    // Set up coordinate units identifier
    //
    string units;
    if (_proj4String.empty()) {
        if (_isWRFSFIRE(ncdfc)) {
            // For *idealized* WRF-SFIRE model runs the 'units' attribute
            // say degrees, but it's actually meters. Clever.
            //
            units = "meters";
        } else {
            units = "km";
        }
    } else {
        units = axis == 0 ? "degrees_east" : "degrees_north";
    }

    if (ncdfc->VariableExists(name) && !_proj4String.empty()) {
        timeDimName = ncdfc->GetTimeDimName(name);

        spaceDimNames = ncdfc->GetSpatialDimNames(name);
        reverse(spaceDimNames.begin(), spaceDimNames.end());
    } else if (ncdfc->VariableExists(name) && _proj4String.empty() && _isConstantValuedVariable(ncdfc, name)) {
        // For idealized case we need to synthesize Cartesian coordinates
        //
        derivedVar = _makeDerivedHorizontalIdealized(ncdfc, name, timeDimName, spaceDimNames);
        if (!derivedVar) return (-1);
    } else if (ncdfc->VariableExists(name) && _proj4String.empty() && _isWRFSFIRE(ncdfc) && !_isConstantValuedVariable(ncdfc, name)) {
        // Idealized WRF-SFIRE cases do have coordinate variables that
        // are already represented in meters
        //
        timeDimName = ncdfc->GetTimeDimName(name);

        spaceDimNames = ncdfc->GetSpatialDimNames(name);
        reverse(spaceDimNames.begin(), spaceDimNames.end());

    } else {
        // Ugh. Older WRF files don't have coordinate variables for
        // staggered dimensions, so we need to derive them.
        //
        derivedVar = _makeDerivedHorizontalStaggered(ncdfc, name, timeDimName, spaceDimNames);
        if (!derivedVar) return (-1);
    }

    // Finally, add the variable to _coordVarsMap. Probably don't
    // need to do this here. Could do this when we process native WRF
    // variables later. Sigh
    //
    vector<bool> periodic(2, false);
    _coordVarsMap[name] = CoordVar(name, units, DC::FLOAT, periodic, axis, false, spaceDimNames, timeDimName);

    int rc = DCUtils::CopyAtt(*ncdfc, name, _coordVarsMap[name]);
    if (rc < 0) return (-1);

    if (derivedVar) { _derivedVars.push_back(derivedVar); }

    return (0);
}

//
// Set up horizontal coordinates
//
int DCWRF::_InitHorizontalCoordinates(NetCDFCollection *ncdfc)
{
    _coordVarsMap.clear();

    // XLONG and XLAT must have same dimensionality
    //
    vector<size_t> latlondims = ncdfc->GetDims("XLONG");
    vector<size_t> dummy = ncdfc->GetDims("XLAT");
    if (latlondims.size() != 3 || dummy != latlondims) {
        SetErrMsg("Invalid coordinate variable : %s", "XLONG");
        return (-1);
    }

    // "XLONG" coordinate, unstaggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLONG", 0);

    // "XLAT" coordinate, unstaggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLAT", 1);

    // "XLONG_U" coordinate, staggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLONG_U", 0);

    // "XLAT_U" coordinate, staggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLAT_U", 1);

    // "XLONG_V" coordinate, staggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLONG_V", 0);

    // "XLAT_V" coordinate, staggered
    //
    (void)_InitHorizontalCoordinatesHelper(ncdfc, "XLAT_V", 1);

    if (_isWRFSFIRE(ncdfc)) {
        // "FXLONG" coordinate, unstaggered
        //
        (void)_InitHorizontalCoordinatesHelper(ncdfc, "FXLONG", 0);

        // "FXLAT" coordinate, staggered
        //
        (void)_InitHorizontalCoordinatesHelper(ncdfc, "FXLAT", 1);
    }

    return (0);
}

DerivedCoordVar_CF1D *DCWRF::_InitVerticalCoordinatesHelper(string varName, string dimName)
{
    DerivedCoordVar_CF1D *derivedVar;

    vector<string> dimNames = {dimName};
    string         units = "";
    int            axis = 2;

    derivedVar = new DerivedCoordVar_CF1D(varName, this, dimName, axis, units);
    (void)derivedVar->Initialize();

    _dvm.AddCoordVar(derivedVar);

    vector<bool> periodic(1, false);
    string       time_dim_name = "";

    CoordVar cvar(varName, units, DC::FLOAT, periodic, axis, false, dimNames, time_dim_name);

    if (varName == "bottom_top" || varName == "bottom_top_stag") {
        cvar.SetAttribute(Attribute("standard_name", DC::XType::TEXT, "wrf_terrain"));

        string formula_terms = "PH: PH PHB: PHB";
        cvar.SetAttribute(Attribute("formula_terms", DC::XType::TEXT, formula_terms));
    }

    _coordVarsMap[varName] = cvar;

    return (derivedVar);
}

//
// Create 1D derived variables expressing the vertical coordinates
// in unitless grid index coordinates.
//
int DCWRF::_InitVerticalCoordinates(NetCDFCollection *ncdfc)
{
    // Create 1D vertical coordinate variable for each "vertical" dimension
    //
    // First do ones we know about. These require special treatment because
    // they may be transformed to other units
    //
    string name = "bottom_top";
    if (_dimsMap.find(name) != _dimsMap.end()) { _derivedVars.push_back(_InitVerticalCoordinatesHelper(name, name)); }

    name = "bottom_top_stag";
    if (_dimsMap.find(name) != _dimsMap.end()) { _derivedVars.push_back(_InitVerticalCoordinatesHelper(name, name)); }

    // Now handle dimensions that are  not documented . I.e. everything
    // else in the 3rd dimension position
    //
    vector<string> vars = ncdfc->GetVariableNames(3, true);

    for (int i = 0; i < vars.size(); i++) {
        vector<string> dimnames = ncdfc->GetSpatialDimNames(vars[i]);
        assert(dimnames.size() == 3);
        reverse(dimnames.begin(), dimnames.end());
        name = dimnames[2];
        if (_dimsMap.find(name) == _dimsMap.end()) continue;

        // no duplicates
        //
        if (_coordVarsMap.find(name) != _coordVarsMap.end()) continue;

        _derivedVars.push_back(_InitVerticalCoordinatesHelper(name, name));
    }

    return (0);
}

// Create a derived variable for the time coordinate. Time in WRF data
// is an array of formatted time strings. The DC class requires that
// time be expressed as seconds represented as floats.
//
int DCWRF::_InitTime(NetCDFCollection *ncdfc)
{
    _derivedTime = NULL;

    // Create and install the Time coordinate variable
    //

    string derivedName = "Time";
    string wrfVarName = "Times";
    string dimName = "Time";
    _derivedTime = new DerivedCoordVar_WRFTime(derivedName, ncdfc, wrfVarName, dimName, _p2si);

    int rc = _derivedTime->Initialize();
    if (rc < 0) return (-1);

    _dvm.AddCoordVar(_derivedTime);

    DC::CoordVar cvarInfo;
    (void)_dvm.GetCoordVarInfo(derivedName, cvarInfo);

    _coordVarsMap[derivedName] = cvarInfo;

    return (0);
}

// Get Space and time dimensions from WRF data set. Initialize
// _dimsMap
//
int DCWRF::_InitDimensions(NetCDFCollection *ncdfc)
{
    _dimsMap.clear();

    // Get dimension names and lengths for all dimensions in the
    // WRF data set.
    //
    vector<string> dimnames = ncdfc->GetDimNames();
    vector<size_t> dimlens = ncdfc->GetDims();
    VAssert(dimnames.size() == dimlens.size());

    // WRF files use reserved names for dimensions. The time dimension
    // is always named "Time", etc.
    // Dimensions are expressed in the DC::Dimension class as a
    // combination of name, and length.
    //
    string timedimname = "Time";
    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }

    if ((_dimsMap.find("west_east") == _dimsMap.end()) || (_dimsMap.find("west_east_stag") == _dimsMap.end()) || (_dimsMap.find("south_north") == _dimsMap.end())
        || (_dimsMap.find("south_north_stag") == _dimsMap.end()) ||
        // (_dimsMap.find("bottom_top") == _dimsMap.end()) ||
        // (_dimsMap.find("bottom_top_stag") == _dimsMap.end()) ||
        (_dimsMap.find("Time") == _dimsMap.end())) {
        SetErrMsg("Missing dimension");
        return (-1);
    }
    return (0);
}

// Given a data variable name return the variable's dimension names and
// associated coordinate variables. The coordinate variable names
// returned is for the derived coordinate variables expressed in
// Cartographic coordinates, not the native geographic coordinates
// found in the WRF file.
//
// The order of the returned vectors
// is significant.
//
bool DCWRF::_GetVarCoordinates(NetCDFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar

)
{
    sdimnames.clear();
    scoordvars.clear();
    time_dim_name.clear();
    time_coordvar.clear();

    // Order of dimensions in WRF files is reverse of DC convention
    //
    vector<string> dimnames = ncdfc->GetDimNames(varname);
    reverse(dimnames.begin(), dimnames.end());

    // Deal with time dimension first
    //
    if (dimnames.size() == 1) {
        if (dimnames[0].compare("Time") != 0) { return (false); }
        time_dim_name = "Time";
        time_coordvar = "Time";
        return (true);
    }

    // only handle 2d, 3d, and 4d variables
    //
    if (dimnames.size() < 2) return (false);

    if (dimnames[0].compare("west_east") == 0 && dimnames[1].compare("south_north") == 0) {
        scoordvars.push_back("XLONG");
        scoordvars.push_back("XLAT");
    } else if (dimnames[0].compare("west_east_stag") == 0 && dimnames[1].compare("south_north") == 0) {
        scoordvars.push_back("XLONG_U");
        scoordvars.push_back("XLAT_U");
    } else if (dimnames[0].compare("west_east") == 0 && dimnames[1].compare("south_north_stag") == 0) {
        scoordvars.push_back("XLONG_V");
        scoordvars.push_back("XLAT_V");
    } else if (_isWRFSFIRE(ncdfc) && dimnames[0].compare("west_east_subgrid") == 0 && dimnames[1].compare("south_north_subgrid") == 0) {
        scoordvars.push_back("FXLONG");
        scoordvars.push_back("FXLAT");
    } else {
        return (false);
    }

    if (dimnames.size() > 2 && dimnames[2] != "Time") { scoordvars.push_back(dimnames[2]); }

    sdimnames = dimnames;

    if (dimnames.size() == 2) { return (true); }

    if (sdimnames.back().compare("Time") == 0) {
        time_dim_name = "Time";
        time_coordvar = "Time";
        sdimnames.pop_back();    // Oops. Remove time dimension
    }
    return (true);
}

// Collect metadata for all data variables found in the WRF data
// set. Initialize the _dataVarsMap member
//
int DCWRF::_InitVars(NetCDFCollection *ncdfc)
{
    _dataVarsMap.clear();
    _meshMap.clear();

    //
    // Get names of variables  in the WRF data set that have 1 2 or 3
    // spatial dimensions
    //
    vector<string> vars;
    for (int i = 1; i < 4; i++) {
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to _dataVarsMap
    //
    for (int i = 0; i < vars.size(); i++) {
        // variable type must be float or int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeFloat(type) || NetCDFSimple::IsNCTypeFloat(type))) continue;

        // If variables are in _coordVarsMap then they are coordinate, not
        // data, variables
        //
        if (_coordVarsMap.find(vars[i]) != _coordVarsMap.end()) continue;

        vector<string> sdimnames;
        vector<string> scoordvars;
        string         time_dim_name;
        string         time_coordvar;

        bool ok = _GetVarCoordinates(ncdfc, vars[i], sdimnames, scoordvars, time_dim_name, time_coordvar);

        // Must have a coordinate variable for each dimension!
        //
        if (sdimnames.size() != scoordvars.size()) { continue; }

        if (!ok) continue;
        // if (! ok) {
        //	SetErrMsg("Invalid variable : %s", vars[i].c_str());
        //	return(-1);
        //}

        Mesh mesh("", sdimnames, scoordvars);

        // Create new mesh. We're being lazy here and probably should only
        // createone if it doesn't ready exist
        //
        _meshMap[mesh.GetName()] = mesh;

        string units;
        ncdfc->GetAtt(vars[i], "units", units);
        if (!_udunits.ValidUnit(units)) { units = ""; }

        vector<bool> periodic(3, false);
        DC::DataVar  dvar = DataVar(vars[i], units, DC::FLOAT, periodic, mesh.GetName(), time_coordvar, DC::Mesh::NODE);

        int rc = DCUtils::CopyAtt(*ncdfc, vars[i], dvar);
        if (rc < 0) return (-1);

        _dataVarsMap[vars[i]] = dvar;
    }

    return (0);
}
