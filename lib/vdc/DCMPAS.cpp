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

vector<string> requiredDimNames = {"nVertLevels", "nCells", "Time", "nEdges", "nVertices", "maxEdges", "maxEdges2", "vertexDegree"};

vector<string> requiredHorizCoordVarNames = {
    "latCell",   "lonCell",
    "xCell",      // Cartesian coord - not used
    "yCell",      // Cartesian coord - not used
    "yVertex",    // Cartesian coord - not used
    "latVertex", "lonVertex",
    "xVertex",    // Cartesian coord - not used
    "yVertex",    // Cartesian coord - not used
    "yVertex",    // Cartesian coord - not used
    "latEdge",   "lonEdge",
    "xEdge",      // Cartesian coord - not used
    "yVertex",    // Cartesian coord - not used
    "yEdge"       // Cartesian coord - not used
};

lat, lon, x, y, z
}    // namespace
Vertex, and the coordinates of the edges(the points where Delaunay triangle faces intersect Voronoi cell faces) are given by{lat, lon, x, y, z} Edge.
}
;
{lat, lon, x, y, z} Vertex, and the coordinates of the edges(the points where Delaunay triangle faces intersect Voronoi cell faces) are given by{lat, lon, x, y, z} Edge.

                                // Product of elements in a vector
                                //
                                size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}
}
;

DCMPAS::DCMPAS()
{
    _ncdfc = NULL;

    _ovr_fd = -1;

    _proj4Strings.clear();
    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
    _coordVarKeys.clear();
    _derivedVars.clear();
}

DCMPAS::~DCMPAS()
{
    if (_ncdfc) delete _ncdfc;
    for (int i = 0; i < _proj4APIs.size(); i++) {
        if (_proj4APIs[i]) delete _proj4APIs[i];
    }
    _proj4APIs.clear();

    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i]) delete _derivedVars[i];
    }
    _derivedVars.clear();
}

int DCMPAS::Initialize(const vector<string> &files)
{
    NetCDFCollection *ncdfc = new NetCDFCollection();

    // Initialize NetCDFCollection class
    //
    vector<string> time_dimnames(1, "Time");
    int            rc = ncdfc->Initialize(files, time_dimnames, vector<string>());
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
    rc = _InitDimensions(ncdfc);
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    // Set up the horizontal coordinate variables
    // These are derived variables that provide horizontal coordinates
    // in **Cartographic coordinates**. I.e. geographic coordinate
    // variables found in MPAS data are projected to cartographic
    // coordinates using Proj4
    //
    // Initializes members: _coordVarMap, _proj4APIs, _proj4Strings;
    //
    rc = _InitHorizontalCoordinates(ncdfc);
    if (rc < 0) { return (-1); }

    // Set up the ELEVATION coordinate variable. MPAS pressure based
    // coordinate system is transformed to meters by generated derived
    // variables.
    //
    // Initializes members: _coordVarMap
    //
    rc = _InitVerticalCoordinates(ncdfc);
    if (rc < 0) { return (-1); }

    // Set up user time coordinate derived variable . Time must be
    // in seconds.
    // Initializes members: _coordVarsMap
    //
    rc = _InitTimeCoordinates(ncdfc);
    if (rc < 0) { return (-1); }

    //
    // Identify data and coordinate variables. Sets up members:
    // Initializes members: _dataVarsMap, _coordVarsMap, _meshMap
    //
    rc = _InitVars(ncdfc);
    if (rc < 0) return (-1);

    _ncdfc = ncdfc;

    return (0);
}

bool DCMPAS::GetDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCMPAS::GetDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCMPAS::GetMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCMPAS::GetMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCMPAS::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCMPAS::GetDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCMPAS::GetBaseVarInfo(string varname, DC::BaseVar &var) const
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

std::vector<string> DCMPAS::GetDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

vector<string> DCMPAS::GetDataVarNames(int ndim, bool spatial) const
{
    // 3D VARIABLES NOT CURRENTLY SUPPORTED
    //
    if ((spatial && ndim > 2) || (!spatial && ndim > 3)) {
        cerr << "DCMPAS::GetDataVarNames() : 3D VARIABLES NOT SUPPORTED" << endl;
        return (vector<string>());
    }

    return (DC::GetDataVarNames(ndim, spatial));
}

std::vector<string> DCMPAS::GetCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

bool DCMPAS::GetAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();
    return (false);
}

bool DCMPAS::GetAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();
    return (false);
}

bool DCMPAS::GetAtt(string varname, string attname, string &values) const
{
    values.clear();
    return (false);
}

std::vector<string> DCMPAS::GetAttNames(string varname) const
{
    vector<string> names;
    return (names);
}

DC::XType DCMPAS::GetAttType(string varname, string attname) const { return (DC::FLOAT); }

int DCMPAS::GetDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
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

string DCMPAS::GetMapProjection(string varname) const
{
    // See if the named variable alread has a projection string defined .
    // If so, use it.
    //
    string proj4string;
    if (_ncdfc->GetMapProjectionProj4(varname, proj4string)) { return (proj4string); }

    // No projection string defined in the NetCDF file for this
    // variable. So we use a synthesized one
    //

    // Find coordinate lat-lon coordinate pair for this variable
    //
    map<string, string>::const_iterator itr = _coordVarKeys.find(varname);
    if (itr == _coordVarKeys.end()) return ("");

    string key = itr->second;

    // Now get projection for this coordinate pair
    //
    map<string, string>::const_iterator itr1 = _proj4Strings.find(key);
    if (itr1 == _proj4Strings.end()) return ("");

    proj4string = itr1->second;
    return (proj4string);
}

string DCMPAS::GetMapProjection() const
{
    if (!_proj4Strings.empty()) {
        map<string, string>::const_iterator itr = _proj4Strings.begin();
        return (itr->second);
    }

    return ("");
}

int DCMPAS::OpenVariableRead(size_t ts, string varname)
{
    DCMPAS::CloseVariable();

    _ovr_fd = _ncdfc->OpenRead(ts, varname);
    return (_ovr_fd);
}

int DCMPAS::CloseVariable()
{
    if (_ovr_fd < 0) return (0);
    int rc = _ncdfc->Close(_ovr_fd);
    _ovr_fd = -1;
    return (rc);
}

int DCMPAS::Read(float *data) { return (_ncdfc->Read(data, _ovr_fd)); }

int DCMPAS::ReadSlice(float *slice) { return (_ncdfc->ReadSlice(slice, _ovr_fd)); }

int DCMPAS::ReadRegion(const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    vector<size_t> ncdf_start = min;
    reverse(ncdf_start.begin(), ncdf_start.end());

    vector<size_t> ncdf_max = max;
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    return (_ncdfc->Read(ncdf_start, ncdf_count, region, _ovr_fd));
}

int DCMPAS::ReadRegionBlock(const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    // return(DCMPAS::ReadRegion(min, max, region));
    return (DCMPAS::Read(region));
}

int DCMPAS::GetVar(string varname, float *data)
{
    vector<size_t> dimlens;
    bool           ok = GetVarDimLens(varname, true, dimlens);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    int  nts = 1;    // num time steps
    bool time_varying = IsTimeVarying(varname);
    if (time_varying) { nts = GetNumTimeSteps(varname); }

    // Number of grid points for variable
    //
    size_t sz = 1;
    for (int i = 0; i < dimlens.size(); i++) { sz *= dimlens[i]; }

    //
    // Read one time step at a time
    //
    float *ptr = data;
    for (int ts = 0; ts < nts; ts++) {
        int rc = DCMPAS::OpenVariableRead(ts, varname);
        if (rc < 0) return (-1);

        rc = DCMPAS::Read(ptr);
        if (rc < 0) return (-1);

        rc = DCMPAS::CloseVariable();
        if (rc < 0) return (-1);

        ptr += sz;    // Advance buffer past current time step
    }
    return (0);
}

int DCMPAS::GetVar(size_t ts, string varname, float *data)
{
    int rc = DCMPAS::OpenVariableRead(ts, varname);
    if (rc < 0) return (-1);

    rc = DCMPAS::Read(data);
    if (rc < 0) return (-1);

    rc = DCMPAS::CloseVariable();
    if (rc < 0) return (-1);

    return (0);
}

bool DCMPAS::VariableExists(size_t ts, string varname, int, int) const { return (_ncdfc->VariableExists(ts, varname)); }

int DCMPAS::_get_latlon_coordvars(NetCDFCollection *ncdfc, string dvar, string &loncvar, string &latcvar) const
{
    loncvar.clear();
    latcvar.clear();

    vector<string> cvars;
    int            rc = ncdfc->GetVarCoordVarNames(dvar, cvars);
    if (rc < 0) return (-1);

    for (int i = 0; i < cvars.size(); i++) {
        if (ncdfc->IsLatCoordVar(cvars[i])) {
            assert(latcvar.empty());
            latcvar = cvars[i];
        } else if (ncdfc->IsLonCoordVar(cvars[i])) {
            assert(loncvar.empty());
            loncvar = cvars[i];
        }
    }
    return (0);
}

int DCMPAS::_get_latlon_extents(NetCDFCollection *ncdfc, string latlon, bool lonflag, float &min, float &max)
{
    vector<size_t> dims = ncdfc->GetSpatialDims(latlon);
    reverse(dims.begin(), dims.end());    // DC dimension order
    assert(dims.size() >= 1 && dims.size() <= 2);

    float *buf = (float *)_buf.Alloc(vproduct(dims) * sizeof(*buf));

    int fd = ncdfc->OpenRead(0, latlon);
    if (fd < 0) {
        SetErrMsg("Can't Read variable %s", latlon.c_str());
        return (-1);
    }

    int rc = ncdfc->Read(buf, fd);
    if (rc < 0) {
        SetErrMsg("Can't Read variable %s", latlon.c_str());
        ncdfc->Close(fd);
        return (-1);
    }
    ncdfc->Close(fd);

    //
    // Precondition longitude coordinates so that there are no
    // discontinuities (e.g. jumping 360 to 0, or -180 to 180)
    //
    if (lonflag) {
        if (dims.size() == 2) {
            GeoUtil::ShiftLon(buf, dims[0], dims[1], buf);
            GeoUtil::LonExtents(buf, dims[0], dims[1], min, max);
        } else {
            GeoUtil::ShiftLon(buf, dims[0], buf);
            GeoUtil::LonExtents(buf, dims[0], min, max);
        }
    } else {
        if (dims.size() == 2) {
            GeoUtil::LatExtents(buf, dims[0], dims[1], min, max);
        } else {
            GeoUtil::LatExtents(buf, dims[0], min, max);
        }
    }
    return (0);
}

int DCMPAS::_get_coord_pair_extents(NetCDFCollection *ncdfc, string lon, string lat, double &lonmin, double &lonmax, double &latmin, double &latmax)
{
    lonmin = lonmax = latmin = latmax = 0.0;

    float lonmin_f, lonmax_f;
    int   rc = _get_latlon_extents(ncdfc, lon, true, lonmin_f, lonmax_f);
    if (rc < 0) return (-1);

    float latmin_f, latmax_f;
    rc = _get_latlon_extents(ncdfc, lat, false, latmin_f, latmax_f);
    if (rc < 0) return (-1);

    lonmin = lonmin_f;
    lonmax = lonmax_f;
    latmin = latmin_f;
    latmax = latmax_f;

    return (0);
}

Proj4API *DCMPAS::_create_proj4api(double lonmin, double lonmax, double latmin, double latmax, string &proj4string) const
{
    proj4string.clear();

    double        lon_0 = (lonmin + lonmax) / 2.0;
    double        lat_0 = (latmin + latmax) / 2.0;
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

int DCMPAS::_AddCoordvars(NetCDFCollection *ncdfc, const vector<string> &cvars)
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

        // Finally, add the variable to _coordVarsMap.
        //
        vector<bool> periodic(false);
        _coordVarsMap[cvars[i]] = CoordVar(cvars[i], units, DC::FLOAT, periodic, axis, false, dimnames, vector<size_t>(), time_dim_name);
    }

    return (0);
}

int DCMPAS::_InitHorizontalCoordinatesDerived(NetCDFCollection *ncdfc, const vector<pair<string, string>> &coordpairs)
{
    // Create derived variables and find min and max lat-lon coordinate
    // extents.
    //
    for (int i = 0; i < coordpairs.size(); i++) {
        // Get dimension names for each coordinate pair.
        //
        vector<string> londimnames = ncdfc->GetDimNames(coordpairs[i].first);
        vector<string> latdimnames = ncdfc->GetDimNames(coordpairs[i].second);

        if (londimnames.size() > 1 && londimnames != latdimnames) {
            SetErrMsg("Invalid coordinate variable pair : %s, %s", coordpairs[i].first.c_str(), coordpairs[i].second.c_str());
            return (-1);
        }
        reverse(londimnames.begin(), londimnames.end());    // DC order
        reverse(latdimnames.begin(), latdimnames.end());    // DC order

        vector<DC::Dimension> londims;
        for (int j = 0; j < londimnames.size(); j++) {
            assert(_dimsMap.find(londimnames[j]) != _dimsMap.end());
            londims.push_back(_dimsMap[londimnames[j]]);
        }

        vector<DC::Dimension> latdims;
        for (int j = 0; j < latdimnames.size(); j++) {
            assert(_dimsMap.find(latdimnames[j]) != _dimsMap.end());
            latdims.push_back(_dimsMap[latdimnames[j]]);
        }

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
        derivedX = new DerivedVarHorizontal(ncdfc, coordpairs[i].first, coordpairs[i].second, londims, _proj4APIs[i], true);
        _derivedVars.push_back(derivedX);

        // Install the derived variable on the NetCDFCollection class. Then
        // all NetCDFCollection methods will treat the derived variable as
        // if it existed in the MPAS data set.
        //
        ncdfc->InstallDerivedCoordVar(name, derivedX, 0);

        // Finally, add the variable to _coordVarsMap. Probably don't
        // need to do this here. Could do this when we process native MPAS
        // variables later. Sigh
        //
        vector<bool> periodic(3, false);
        _coordVarsMap[name] = CoordVar(name, "meters", DC::FLOAT, periodic, 0, false, londimnames, vector<size_t>(), lon_time_dim_name);

        name = coordpairs[i].second + "Y";

        DerivedVarHorizontal *derivedY;
        derivedY = new DerivedVarHorizontal(ncdfc, coordpairs[i].first, coordpairs[i].second, latdims, _proj4APIs[i], false);
        _derivedVars.push_back(derivedY);

        ncdfc->InstallDerivedCoordVar(name, derivedY, 1);

        _coordVarsMap[name] = CoordVar(name, "meters", DC::FLOAT, periodic, 1, false, latdimnames, vector<size_t>(), lat_time_dim_name

        );
    }

    return (0);
}

//
// Create derived variables expressing the horizontal coordinates
// in Cartographic coordinates in meters.
//
// The derived variables are named lonX, latY, where "lat" are the names
// of the latitude coordinate (geographic coordinates), and "lon" are the
// names of the longitude coordinate. E.g. TLAT => TLATY, ULON => ULONX
//
// Initializes _proj4APIs, _proj4Strings, _coordVarKeys
//
int DCMPAS::_InitHorizontalCoordinates(NetCDFCollection *ncdfc)
{
    _proj4APIs = NULL;
    _proj4String.clear();

    // Check for required horizontal coordinate variables
    //
    vector<string> varnames = ncdfc->GetVarNames(1, true);
    for (int i = 0; i < requiredHorizCoordVarNames.size(); i++) {
        if (_dimsMap.find(requiredHorizCoordVarNames[i]) == _dimsMap.end()) {
            SetErrMsg("Missing required horizontal coordinate variable \"%s\",
                      requiredHorizCoordVarNames[i]
                          .c_str());
            return (-1);
        }
    }

    _proj4API = _create_proj4api(-180.0, 180.0, -90.0, 90.0, _proj4string);
    if (!_proj4API) return (-1);

    vector<pair<string, string>> coordpairs;
    coordpairs.push_back(make_pair("lonCell", "latCell"));
    coordpairs.push_back(make_pair("lonVertex", "latVertex"));
    coordpairs.push_back(make_pair("lonEdge", "latEdge"));

    // Create a pair of derived horizontal coordinate variables
    // in Cartographic
    // coordiantes for each lat-lon pair
    //
    int rc = _InitHorizontalCoordinatesDerived(ncdfc, coordpairs);
    if (rc < 0) return (-1);

    // Add native coordinate variables
    //
    vector<string> cvars = {"lonCell", "latCell", "lonVertex", "latVertex", "lonEdge", "latEdge"};
    rc = _AddCoordvars(ncdfc, cvars);
    if (rc < 0) return (-1);

    return (0);
}

int DCMPAS::_get_vertical_coordvar(NetCDFCollection *ncdfc, string dvar, string &cvar)
{
    cvar.clear();

    vector<string> cvars;
    int            rc = ncdfc->GetVarCoordVarNames(dvar, cvars);
    if (rc < 0) return (-1);

    for (int i = 0; i < cvars.size(); i++) {
        if (ncdfc->IsVertCoordVar(cvars[i])) {
            assert(cvar.empty());
            cvar = cvars[i];
        }
    }
    return (0);
}

int DCMPAS::_InitVerticalCoordinatesDerived(NetCDFCollection *ncdfc, const vector<string> &cvars)
{
    vector<bool> periodic(3, false);
    for (int i = 0; i < cvars.size(); i++) {
        NetCDFSimple::Variable varinfo;
        (void)ncdfc->GetVariableInfo(cvars[i], varinfo);

#ifdef DEAD
        string standard_name;
        varinfo.GetAtt("standard_name", standard_name);
        if (standard_name.empty()) continue;

        string formula_terms;
        varinfo.GetAtt("formula_terms", formula_terms);
        if (formula_terms.empty()) continue;
#endif

        string name = cvars[i] + "Z";
        int    rc = ncdfc->InstallStandardVerticalConverter(cvars[i], name, "meters");
        if (rc < 0) return (-1);

        // Get dimension names for the *derived* variable, and
        // then set up a Dimenions vector
        //
        vector<string> dimnames = ncdfc->GetDimNames(name);
        reverse(dimnames.begin(), dimnames.end());

        vector<DC::Dimension> dims;
        for (int j = 0; j < dimnames.size(); j++) {
            assert(_dimsMap.find(dimnames[j]) != _dimsMap.end());
            dims.push_back(_dimsMap[dimnames[j]]);
        }

        string time_dim_name;
        if (ncdfc->IsTimeVarying(cvars[i])) {
            time_dim_name = dimnames.back();
            dimnames.pop_back();
        }

        // Finally, add the variable to _coordVarsMap. Probably don't
        // need to do this here. Could do this when we process native MPAS
        // variables later. Sigh
        //
        _coordVarsMap[name] = CoordVar(name, "meters", DC::FLOAT, periodic, 2, false, dimnames, vector<size_t>(), time_dim_name);
    }

    return (0);
}

//
// Create derived variables expressing the vertical coordinates
// in meters. MPAS uses a Arakawa C grid (staggered
// grid). Hence, there are separate vertical coordinates for U, V, W, and
// all other variables.
//
// The derived variables are named ELEVATION, ELEVATIONU, ELEVATIONV,
// ELEVATIONW.
//
int DCMPAS::_InitVerticalCoordinates(NetCDFCollection *ncdfc)
{
    //
    // Get names of data variables  in the MPAS data set that have 1, 2 or 3
    // spatial dimensions
    //
    vector<string> dvars;
    for (int i = 1; i < 4; i++) {
        vector<string> v = ncdfc->GetDataVariableNames(i, true);
        dvars.insert(dvars.end(), v.begin(), v.end());
    }

    // Now get all of the vertical coordinate variable names
    // for each of the 1D, 2D and 3D data variables
    //
    vector<string> cvars;
    for (int i = 0; i < dvars.size(); i++) {
        string vertcvar;

        int rc = _get_vertical_coordvar(ncdfc, dvars[i], vertcvar);
        if (rc < 0) return (-1);

        if (vertcvar.empty()) continue;

        cvars.push_back(vertcvar);
    }

    // Remove duplicates
    //
    sort(cvars.begin(), cvars.end());
    vector<string>::iterator last;
    last = unique(cvars.begin(), cvars.end());
    cvars.erase(last, cvars.end());

    // Create a new derived vertical coordinate variable for each native
    // vertical coordinate variable using the NetCDFCollection class
    // built-in standard vertical coordinate converts
    //
    int rc = _InitVerticalCoordinatesDerived(ncdfc, cvars);
    if (rc < 0) return (-1);

    rc = _AddCoordvars(ncdfc, cvars);
    if (rc < 0) return (-1);

    return (0);
}

int DCMPAS::_get_time_coordvar(NetCDFCollection *ncdfc, string dvar, string &cvar)
{
    cvar.clear();

    vector<string> cvars;
    int            rc = ncdfc->GetVarCoordVarNames(dvar, cvars);
    if (rc < 0) return (-1);

    for (int i = 0; i < cvars.size(); i++) {
        if (ncdfc->IsTimeCoordVar(cvars[i])) {
            assert(cvar.empty());
            cvar = cvars[i];
        }
    }
    return (0);
}

int DCMPAS::_InitTimeCoordinatesDerived(NetCDFCollection *ncdfc, const vector<string> &cvars)
{
    vector<bool> periodic(1, false);
    for (int i = 0; i < cvars.size(); i++) {
        // Get dimension names for time variable
        //
        vector<string> dimnames = ncdfc->GetDimNames(cvars[i]);

        if (dimnames.size() != 1) {
            SetErrMsg("Invalid time coordinate variable : %s", cvars[i].c_str());
            return (-1);
        }

        vector<DC::Dimension> dims;
        for (int j = 0; j < dimnames.size(); j++) {
            assert(_dimsMap.find(dimnames[j]) != _dimsMap.end());
            dims.push_back(_dimsMap[dimnames[j]]);
        }

        // Create the time derived variable class object
        //
        string name = cvars[i] + "T";
        int    rc = ncdfc->InstallStandardTimeConverter(cvars[i], name, "seconds");
        if (rc < 0) return (-1);

        // Finally, add the variable to _coordVarsMap. Probably don't
        // need to do this here. Could do this when we process native MPAS
        // variables later. Sigh
        //
        _coordVarsMap[name] = CoordVar(name, "seconds", DC::FLOAT, periodic, 3, false, vector<string>(), vector<size_t>(), dimnames[0]);
    }

    return (0);
}

// Create a derived variable for the time coordinate. Time in MPAS data
// is an array of formatted time strings. The DC class requires that
// time be expressed as seconds represented as floats.
//
int DCMPAS::_InitTimeCoordinates(NetCDFCollection *ncdfc)
{
    //
    // Get names of data variables  in the MPAS data set that have 1, 2 or 3
    // spatial dimensions
    //
    vector<string> dvars;
    for (int i = 1; i < 4; i++) {
        vector<string> v = ncdfc->GetDataVariableNames(i, true);
        dvars.insert(dvars.end(), v.begin(), v.end());
    }

    // Now get all of the time coordinate variable names
    // for each of the 1D, 2D and 3D data variables
    //
    vector<string> cvars;
    for (int i = 0; i < dvars.size(); i++) {
        string timecvar;

        int rc = _get_time_coordvar(ncdfc, dvars[i], timecvar);
        if (rc < 0) return (-1);

        if (timecvar.empty()) continue;

        cvars.push_back(timecvar);
    }
    // Remove duplicates
    //
    sort(cvars.begin(), cvars.end());
    vector<string>::iterator last;
    last = unique(cvars.begin(), cvars.end());
    cvars.erase(last, cvars.end());

    // create time coordinate variables that map time units to seconds
    //
    int rc = _InitTimeCoordinatesDerived(ncdfc, cvars);
    if (rc < 0) return (-1);

// Don't add native time coordiate variable because all of the
// time variables are aggregated into a single 'global' time
// variable
//
#ifdef DEAD
    // add native time coordinate variables
    //
    rc = _AddCoordvars(ncdfc, cvars);
    if (rc < 0) return (-1);
#endif

    return (0);
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
    assert(dimnames.size() == dimlens.size());

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }

    for (int i = 0; i < requiredDimNames.size(); i++) {
        if (_dimsMap.find(requiredDimNames[i]) == _dimsMap.end()) {
            SetErrMsg("Missing required dimension \"%s\",
                      requiredDimNames[i]
                          .c_str());
            return (-1);
        }
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
int DCMPAS::_GetVarCoordinates(NetCDFCollection *ncdfc, string varname, vector<string> &sdimnames, vector<string> &scoordvars, string &time_dim_name, string &time_coordvar)
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
    // elevtion expressed in meters, and time in seconds
    //
    for (int i = 0; i < scoordvars.size(); i++) {
        string xcv;
        if (ncdfc->IsLonCoordVar(scoordvars[i])) {
            xcv = scoordvars[i] + "X";
        } else if (ncdfc->IsLatCoordVar(scoordvars[i])) {
            xcv = scoordvars[i] + "Y";
        } else if (ncdfc->IsVertCoordVar(scoordvars[i])) {
            xcv = scoordvars[i] + "Z";
        } else if (ncdfc->IsTimeCoordVar(scoordvars[i])) {
            xcv = scoordvars[i] + "T";
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

// Collect metadata for all data variables found in the MPAS data
// set. Initialize the _dataVarsMap member
//
int DCMPAS::_InitVars(NetCDFCollection *ncdfc)
{
    _dataVarsMap.clear();
    _meshMap.clear();

    vector<bool> periodic(3, false);
    //
    // Get names of variables  in the MPAS data set that have 1 2 or 3
    // spatial dimensions
    //
    vector<string> vars;
    for (int i = 1; i < 4; i++) {
        vector<string> v = ncdfc->GetDataVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to _dataVarsMap
    //
    for (int i = 0; i < vars.size(); i++) {
        cout << "Var i " << i << " " << vars[i] << endl;

        // variable type must be float or int
        //
        int type = ncdfc->GetXType(vars[i]);
        if (!(NetCDFSimple::IsNCTypeFloat(type) || NetCDFSimple::IsNCTypeInt(type))) continue;

        vector<string> sdimnames;
        vector<string> scoordvars;
        string         time_dim_name;
        string         time_coordvar;

        int rc = _GetVarCoordinates(ncdfc, vars[i], sdimnames, scoordvars, time_dim_name, time_coordvar);
        if (rc < 0) {
            SetErrMsg("Invalid variable : %s", vars[i].c_str());
            return (-1);
        }

        string units;
        ncdfc->GetAtt(vars[i], "units", units);
        if (!_udunits.ValidUnit(units)) { units = ""; }

        Mesh mesh("", sdimnames, scoordvars);

        // Create new mesh. We're being lazy here and probably should only
        // createone if it doesn't ready exist
        //
        _meshMap[mesh.GetName()] = mesh;

        double mv;
        bool   has_missing = ncdfc->GetMissingValue(vars[i], mv);

        if (!has_missing) {
            _dataVarsMap[vars[i]] = DataVar(vars[i], units, DC::FLOAT, periodic, mesh.GetName(), vector<size_t>(), time_coordvar, DC::Mesh::VOLUME);
        } else {
            _dataVarsMap[vars[i]] = DataVar(vars[i], units, DC::FLOAT, periodic, mesh.GetName(), vector<size_t>(), time_coordvar, DC::Mesh::VOLUME, mv);
        }
    }

    return (0);
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
DCMPAS::DerivedVarHorizontal::DerivedVarHorizontal(NetCDFCollection *ncdfc, string lonname, string latname, const vector<DC::Dimension> &dims, Proj4API *proj4API, bool xflag) : DerivedVar(ncdfc)
{
    assert(dims.size() >= 1 && dims.size() <= 3);

    _lonname = lonname;
    _latname = latname;
    _xflag = xflag;
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

    // NetCDF dimension order
    //
    vector<DC::Dimension> dimsncdf = dims;
    reverse(dimsncdf.begin(), dimsncdf.end());

    if (ncdfc->IsTimeVarying(lonname) || ncdfc->IsTimeVarying(latname)) {
        assert(ncdfc->IsTimeVarying(lonname) && ncdfc->IsTimeVarying(latname));
        _time_dim = dimsncdf[0].GetLength();
        _time_dim_name = dimsncdf[0].GetName();
        dimsncdf.erase(dimsncdf.begin());
    }
    assert(dimsncdf.size() >= 1 && dimsncdf.size() <= 2);

    _sdims.clear();
    _sdimnames.clear();
    for (int i = 0; i < dimsncdf.size(); i++) {
        _sdims.push_back(dimsncdf[i].GetLength());
        _sdimnames.push_back(dimsncdf[i].GetName());
    }

    size_t n = vproduct(_sdims);
    _latbuf = new float[n];
    _lonbuf = new float[n];

    _is_open = false;
}

DCMPAS::DerivedVarHorizontal::~DerivedVarHorizontal()
{
    if (_lonbuf) delete[] _lonbuf;
    if (_latbuf) delete[] _latbuf;
}

int DCMPAS::DerivedVarHorizontal::Open(size_t ts)
{
    if (_is_open) return (-1);    // Only one variable open at a time

    _lonfd = -1;
    _latfd = -1;

    // if 2d open longitude  and latitude variables
    // if 1d open only corresponding geographic coordinate varible for
    // Cartographic axis

    if (_xflag || _sdims.size() > 1) {
        int fd = _ncdfc->OpenRead(ts, _lonname);
        if (fd < 0) {
            SetErrMsg("Can't read %s variable", _lonname.c_str());
            return (-1);
        }
        _lonfd = fd;
    }

    if (!_xflag || _sdims.size() > 1) {
        int fd = _ncdfc->OpenRead(ts, _latname);
        if (fd < 0) {
            SetErrMsg("Can't read %s variable", _latname.c_str());
            return (-1);
        }
        _latfd = fd;
    }
    _is_open = true;

    return (0);
}

int DCMPAS::DerivedVarHorizontal::Read(float *buf, int)
{
    if (!_is_open) {
        SetErrMsg("Invalid operation");
        return (-1);
    }
    size_t n = vproduct(_sdims);
    for (int i = 0; i < n; i++) {
        _lonbuf[i] = 0.0;
        _latbuf[i] = 0.0;
    }

    if (_lonfd >= 0) {
        int rc = _ncdfc->Read(_lonbuf, _lonfd);
        if (rc < 0) {
            SetErrMsg("Can't read %s variable", _lonname.c_str());
            return (-1);
        }
    }

    if (_latfd >= 0) {
        int rc = _ncdfc->Read(_latbuf, _latfd);
        if (rc < 0) {
            SetErrMsg("Can't read %s variable", _latname.c_str());
            return (-1);
        }
    }

    int rc = _proj4API->Transform(_lonbuf, _latbuf, n);
    if (rc < 0) return (-1);

    float *ptr = _xflag ? _lonbuf : _latbuf;

    for (int i = 0; i < n; i++) { buf[i] = ptr[i]; }

    return (0);
}

int DCMPAS::DerivedVarHorizontal::ReadSlice(float *slice, int) { return (DCMPAS::DerivedVarHorizontal::Read(slice, 0)); }

int DCMPAS::DerivedVarHorizontal::SeekSlice(int offset, int whence, int) { return (0); }

int DCMPAS::DerivedVarHorizontal::Close(int)
{
    if (!_is_open) return (0);

    int rc = 0;
    if (_lonfd > 0 && _ncdfc->Close(_lonfd) < 0) rc = -1;
    if (_latfd > 0 && _ncdfc->Close(_latfd) < 0) rc = -1;
    _is_open = false;

    return (rc);
}
