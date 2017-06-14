#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cassert>
#ifdef _WINDOWS
#include "vapor/udunits2.h"
#else
#include <udunits2.h>
#endif
#include <vapor/GetAppPath.h>
#include <vapor/NetCDFCFCollection.h>

using namespace VAPoR;
using namespace Wasp;
using namespace std;

NetCDFCFCollection::NetCDFCFCollection() : NetCDFCollection() {

    _coordinateVars.clear();
    _auxCoordinateVars.clear();
    _lonCoordVars.clear();
    _latCoordVars.clear();
    _vertCoordVars.clear();
    _timeCoordVars.clear();
    _missingValueMap.clear();
    _derivedVarsMap.clear();

    _udunit = NULL;
}
NetCDFCFCollection::~NetCDFCFCollection() {
    if (_udunit)
        delete _udunit;

    std::map<string, DerivedVar *>::iterator itr;
    for (itr = _derivedVarsMap.begin(); itr != _derivedVarsMap.end(); ++itr) {
        delete itr->second;
    }
    _derivedVarsMap.clear();
}

int NetCDFCFCollection::_Initialize(
    const vector<string> &files) {

    // Look for time coordinate variables. Must be 1D and have same
    // name as dimension. Need not be present in all files.
    //
    vector<string> tv;
    for (int j = 0; j < files.size(); j++) {

        vector<string> emptyvec;
        vector<string> file(1, files[j]);
        int rc = NetCDFCollection::Initialize(file, emptyvec, emptyvec);
        if (rc < 0)
            return (-1);

        // Get all 1D variables
        //
        vector<string> vars = NetCDFCollection::GetVariableNames(1, false);
        for (int i = 0; i < vars.size(); i++) {

            NetCDFSimple::Variable varinfo;
            (void)NetCDFCollection::GetVariableInfo(vars[i], varinfo);

            if (_IsCoordinateVar(varinfo) && _IsTimeCoordVar(varinfo)) {
                tv.push_back(vars[i]);
            }
        }
        if (tv.size())
            break; // Should we process all files???
    }

    //
    // Reinitialize the base class now that we know the time coordinate
    // variable. We're assuming that the time coordinate variable and
    // the time dimension have the same name. Thus we're not handling
    // the case where time variable could be a CF "auxilliary
    // coordinate variable".
    //
    int rc = NetCDFCollection::Initialize(files, tv, tv);
    if (rc < 0)
        return (-1);
    return (0);
}

int NetCDFCFCollection::Initialize(
    const vector<string> &files) {

    _coordinateVars.clear();
    _auxCoordinateVars.clear();
    _lonCoordVars.clear();
    _latCoordVars.clear();
    _vertCoordVars.clear();
    _timeCoordVars.clear();
    _missingValueMap.clear();

    if (_udunit)
        delete _udunit;
    _udunit = new UDUnits();
    int rc = _udunit->Initialize();
    if (rc < 0) {
        SetErrMsg(
            "Failed to initialized udunits2 library : %s",
            _udunit->GetErrMsg().c_str());
        return (0);
    }

    rc = _Initialize(files);
    if (rc < 0)
        return (-1);

    //
    // Identify all of the coordinate variables and
    // auxiliary coordinate variables. Not sure if we need to
    // make a distinction between "coordinate variable" and
    // "auxiliary coordinate variable".
    //
    // First look for "coordinate variables", which are 1D and have
    // the same name as their one dimension
    //
    vector<string> vars = NetCDFCollection::GetVariableNames(1, false);
    for (int i = 0; i < vars.size(); i++) {

        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(vars[i], varinfo);

        if (_IsCoordinateVar(varinfo)) {
            _coordinateVars.push_back(vars[i]);
        }
    }

    //
    // Get all the 1D, 2D 3D, and 4D variables
    vars.clear();
    vector<string> v = NetCDFCollection::GetVariableNames(1, false);
    vars.insert(vars.end(), v.begin(), v.end());

    v = NetCDFCollection::GetVariableNames(2, false);
    vars.insert(vars.end(), v.begin(), v.end());

    v = NetCDFCollection::GetVariableNames(3, false);
    vars.insert(vars.end(), v.begin(), v.end());

    v = NetCDFCollection::GetVariableNames(4, false);
    vars.insert(vars.end(), v.begin(), v.end());

    for (int i = 0; i < vars.size(); i++) {

        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(vars[i], varinfo);

        vector<string> coordattr = _GetCoordAttrs(varinfo);
        if (!coordattr.size())
            continue;

        for (int j = 0; j < coordattr.size(); j++) {

            //
            // Make sure the auxiliary coordinate variable
            // actually exists in the data collection and has not
            // already been identified as a 1D "coordinate variable".
            //
            if (NetCDFCollection::VariableExists(coordattr[j]) &&
                (find(
                     _coordinateVars.begin(), _coordinateVars.end(),
                     coordattr[j]) == _coordinateVars.end())) {

                _auxCoordinateVars.push_back(coordattr[j]);
            }
        }
    }

    //
    // sort and remove duplicate auxiliary coordinate variables
    //
    sort(_auxCoordinateVars.begin(), _auxCoordinateVars.end());
    vector<string>::iterator lasts;
    lasts = unique(_auxCoordinateVars.begin(), _auxCoordinateVars.end());
    _auxCoordinateVars.erase(lasts, _auxCoordinateVars.end());

    //
    // Lastly, determine the type of coordinate variable (lat, lon,
    // vertical, or time)
    //
    vector<string> cvars = _coordinateVars;
    cvars.insert(
        cvars.end(), _auxCoordinateVars.begin(), _auxCoordinateVars.end());

    for (int i = 0; i < cvars.size(); i++) {
        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(cvars[i], varinfo);

        if (_IsLonCoordVar(varinfo)) {
            _lonCoordVars.push_back(cvars[i]);
        } else if (_IsLatCoordVar(varinfo)) {
            _latCoordVars.push_back(cvars[i]);
        } else if (_IsVertCoordVar(varinfo)) {
            _vertCoordVars.push_back(cvars[i]);
        } else if (_IsTimeCoordVar(varinfo)) {
            _timeCoordVars.push_back(cvars[i]);
        }
    }

    _GetMissingValueMap(_missingValueMap);
    return (0);
}

vector<string> NetCDFCFCollection::GetDataVariableNames(
    int ndim, bool spatial) const {

    vector<string> tmp = NetCDFCollection::GetVariableNames(ndim, spatial);

    vector<string> varnames;
    for (int i = 0; i < tmp.size(); i++) {
        //
        // Strip off any coordinate variables
        //
        if (IsCoordVarCF(tmp[i]) || IsAuxCoordVarCF(tmp[i]))
            continue;

        vector<string> cvars;
        bool enable = EnableErrMsg(false);
        int rc = NetCDFCFCollection::GetVarCoordVarNames(tmp[i], cvars);
        EnableErrMsg(enable);
        SetErrCode(0);

        if (rc < 0)
            continue; // Doesn't have coordinate variables

        int myndim = cvars.size();
        if (spatial && IsTimeVarying(tmp[i])) {
            myndim--; // don't count time dimension
        }
        if (myndim != ndim)
            continue;

        varnames.push_back(tmp[i]);
    }

    return (varnames);
}

int NetCDFCFCollection::GetVarCoordVarNames(
    string var, vector<string> &cvars) const {
    cvars.clear();

    NetCDFSimple::Variable varinfo;

    int rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    if (rc < 0)
        return (rc);

    vector<string> dimnames = varinfo.GetDimNames();

    //
    // First look for auxiliary coordinate variables
    //
    bool hasTimeCoord = false;
    bool hasLatCoord = false;
    bool hasLonCoord = false;
    bool hasVertCoord = false;
    vector<string> tmpcvars;
    vector<string> auxcvars = _GetCoordAttrs(varinfo);
    for (int i = 0; i < auxcvars.size(); i++) {

        //
        // Make sure variable specified by "coordinate" attribute is in
        // fact an auxiliary coordinate variable (any coordinate variables
        // specified by the "coordinate" attribute should have already
        // been picked up above
        //
        if (NetCDFCFCollection::IsTimeCoordVar(auxcvars[i]) && !hasTimeCoord) {
            hasTimeCoord = true;
            tmpcvars.push_back(auxcvars[i]);
        }
        if (NetCDFCFCollection::IsLatCoordVar(auxcvars[i]) && !hasLatCoord) {
            hasLatCoord = true;
            tmpcvars.push_back(auxcvars[i]);
        }
        if (NetCDFCFCollection::IsLonCoordVar(auxcvars[i]) && !hasLonCoord) {
            hasLonCoord = true;
            tmpcvars.push_back(auxcvars[i]);
        }
        if (NetCDFCFCollection::IsVertCoordVar(auxcvars[i]) && !hasVertCoord) {
            hasVertCoord = true;
            tmpcvars.push_back(auxcvars[i]);
        }
    }

    //
    // Now see if any "coordinate variables" for which we haven't
    // already identified a coord var exist.
    //
    if (tmpcvars.size() != dimnames.size()) {
        for (int i = 0; i < dimnames.size(); i++) {
            if (NetCDFCFCollection::IsCoordVarCF(dimnames[i])) { // is a CF "coordinate variable"?
                if (NetCDFCFCollection::IsTimeCoordVar(dimnames[i]) && !hasTimeCoord) {
                    hasTimeCoord = true;
                    tmpcvars.push_back(dimnames[i]);
                }
                if (NetCDFCFCollection::IsLatCoordVar(dimnames[i]) && !hasLatCoord) {
                    hasLatCoord = true;
                    tmpcvars.push_back(dimnames[i]);
                }
                if (NetCDFCFCollection::IsLonCoordVar(dimnames[i]) && !hasLonCoord) {
                    hasLonCoord = true;
                    tmpcvars.push_back(dimnames[i]);
                }
                if (NetCDFCFCollection::IsVertCoordVar(dimnames[i]) && !hasVertCoord) {
                    hasVertCoord = true;
                    tmpcvars.push_back(dimnames[i]);
                }
            }
        }
    }

    //
    // If we still don't have lat and lon coordinate (or auxiliary)
    // variables for 'var' then we look for coordinate variables whose
    // dim names match the dim names of 'var'. Don't think this is
    // part of the CF 1.6 spec, but it seems necessary for ROMS data sets
    //
    //
    // If "coordinate variables" are specified for each dimension we're don
    //
    if (tmpcvars.size() != dimnames.size()) {
        if (!hasLatCoord) {
            vector<string> latcvs = NetCDFCFCollection::GetLatCoordVars();
            for (int i = 0; i < latcvs.size(); i++) {
                NetCDFSimple::Variable varinfo;
                NetCDFCollection::GetVariableInfo(latcvs[i], varinfo);
                vector<string> dns = varinfo.GetDimNames();

                if (dimnames.size() >= dns.size()) {
                    vector<string> tmp(dimnames.end() - dns.size(), dimnames.end());
                    if (tmp == dns) {
                        tmpcvars.push_back(latcvs[i]);
                        break;
                    }
                }
            }
        }

        if (!hasLonCoord) {
            vector<string> loncvs = NetCDFCFCollection::GetLonCoordVars();
            for (int i = 0; i < loncvs.size(); i++) {
                NetCDFSimple::Variable varinfo;
                NetCDFCollection::GetVariableInfo(loncvs[i], varinfo);
                vector<string> dns = varinfo.GetDimNames();

                if (dimnames.size() >= dns.size()) {
                    vector<string> tmp(dimnames.end() - dns.size(), dimnames.end());
                    if (tmp == dns) {
                        tmpcvars.push_back(loncvs[i]);
                        break;
                    }
                }
            }
        }
    }
    if (tmpcvars.size() != dimnames.size()) {
        SetErrMsg("Non-conforming CF variable : %s", var.c_str());
        return (-1);
    }

    //
    // Finally, order the coordinate variables from slowest to fastest
    // varying dimension
    //
    for (int i = 0; i < tmpcvars.size(); i++) {
        if (NetCDFCFCollection::IsTimeCoordVar(tmpcvars[i])) {
            cvars.push_back(tmpcvars[i]);
            break;
        }
    }
    for (int i = 0; i < tmpcvars.size(); i++) {
        if (NetCDFCFCollection::IsVertCoordVar(tmpcvars[i])) {
            cvars.push_back(tmpcvars[i]);
            break;
        }
    }
    for (int i = 0; i < tmpcvars.size(); i++) {
        if (NetCDFCFCollection::IsLatCoordVar(tmpcvars[i])) {
            cvars.push_back(tmpcvars[i]);
            break;
        }
    }
    for (int i = 0; i < tmpcvars.size(); i++) {
        if (NetCDFCFCollection::IsLonCoordVar(tmpcvars[i])) {
            cvars.push_back(tmpcvars[i]);
            break;
        }
    }
    assert(cvars.size() == tmpcvars.size());

    return (0);
}

bool NetCDFCFCollection::IsVertCoordVarUp(string cvar) const {
    NetCDFSimple::Variable varinfo;

    int rc = NetCDFCollection::GetVariableInfo(cvar, varinfo);
    if (rc < 0) {
        SetErrCode(0);
        return (false);
    }

    string s;
    varinfo.GetAtt("positive", s);

    if (StrCmpNoCase(s, "up") == 0)
        return (true);
    else
        return (false);
}

int NetCDFCFCollection::GetVarUnits(string var, string &units) const {
    units.clear();

    NetCDFSimple::Variable varinfo;

    int rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    if (rc < 0)
        return (rc);

    varinfo.GetAtt("units", units);
    return (0);
}

int NetCDFCFCollection::Convert(
    const string from, const string to, const double *src, double *dst, size_t n) const {
    bool status = _udunit->Convert(from, to, src, dst, n);

    if (!status) {
        SetErrMsg(
            "NetCDFCFCollection::Convert(%s , %s,,) : failed",
            from.c_str(), to.c_str());
        return (-1);
    }
    return (0);
}
int NetCDFCFCollection::Convert(
    const string from, const string to, const float *src, float *dst, size_t n) const {
    bool status = _udunit->Convert(from, to, src, dst, n);

    if (!status) {
        SetErrMsg(
            "NetCDFCFCollection::Convert(%s , %s,,) : failed",
            from.c_str(), to.c_str());
        return (-1);
    }
    return (0);
}

bool NetCDFCFCollection::GetMissingValue(string varname, double &mv) const {
    map<string, double>::const_iterator itr;

    if (NetCDFCollection::IsDerivedVar(varname))
        return (false);

    itr = _missingValueMap.find(varname);
    if (itr != _missingValueMap.end()) {
        mv = itr->second;
        return (true);
    }
    return (false);
}

int NetCDFCFCollection::OpenRead(size_t ts, string varname) {
    double mv;
    string mvattname;

    if (_GetMissingValue(varname, mvattname, mv)) {
        NetCDFCFCollection::SetMissingValueAttName(mvattname);
    }
    int fd = NetCDFCollection::OpenRead(ts, varname);

    NetCDFCFCollection::SetMissingValueAttName("");
    return (fd);
}

bool NetCDFCFCollection::IsVertDimensionless(string cvar) const {

    //
    // Return false if variable isn't a vertical coordinate variable at all
    //
    if (!IsVertCoordVar(cvar))
        return (false);

    //
    // If we get to here the cvar is a valid coordinate variable
    // so GetVariableInfo() should return successfully
    //
    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(cvar, varinfo);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty())
        return (false); // No coordinates attribute found

    return (!(_udunit->IsPressureUnit(unit) || _udunit->IsLengthUnit(unit)));
}

int NetCDFCFCollection::_parse_formula(
    string formula_terms, map<string, string> &parsed_terms) const {

    // Remove ":" to ease parsing. It's superflous
    //
    replace(formula_terms.begin(), formula_terms.end(), ':', ' ');

    string buf;                     // Have a buffer string
    stringstream ss(formula_terms); // Insert the string into a stream

    vector<string> tokens; // Create vector to hold our words

    while (ss >> buf) {
        tokens.push_back(buf);
    }

    if (tokens.size() % 2) {
        SetErrMsg("Invalid formula_terms string : %s", formula_terms.c_str());
        return (-1);
    }

    for (int i = 0; i < tokens.size(); i += 2) {
        parsed_terms[tokens[i]] = tokens[i + 1];
    }
    return (0);
}

int NetCDFCFCollection::InstallStandardVerticalConverter(
    string cvar, string newvar, string units) {
    if (!NetCDFCFCollection::IsVertCoordVar(cvar)) {
        SetErrMsg("Variable %s not a vertical coordinate variable", cvar.c_str());
        return (-1);
    }

    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(cvar, varinfo);

    string standard_name;
    varinfo.GetAtt("standard_name", standard_name);
    if (standard_name.empty()) {
        varinfo.GetAtt("long_name", standard_name);
    }

    string formula_terms;
    varinfo.GetAtt("formula_terms", formula_terms);

    string native_units;
    varinfo.GetAtt("units", native_units);

    map<string, string> terms_map;
    int rc = _parse_formula(formula_terms, terms_map);
    if (rc < 0)
        return (-1);

    NetCDFCollection::DerivedVar *derived_var;
    if (standard_name.compare("ocean_s_coordinate_g1") == 0) {
        derived_var = new DerivedVar_ocean_s_coordinate_g1(
            this, terms_map);
    } else if (standard_name.compare("ocean_s_coordinate_g2") == 0) {
        derived_var = new DerivedVar_ocean_s_coordinate_g2(
            this, terms_map);
    } else if (standard_name.compare("atmosphere_hybrid_sigma_pressure_coordinate") == 0) {
        derived_var = new DerivedVar_AHSPC(
            this, terms_map);
    } else if (
        (standard_name.compare("altitude") == 0) ||
        (standard_name.compare("model_level_number") == 0) ||
        (_udunit->IsLengthUnit(units))) { // noop

        // The "altitude" representation is already in units of distance
        // Setting up a bogus formula allows use to use the
        // DerivedVar_noop class and avoid special case handling
        //
        terms_map.clear();
        terms_map["z"] = cvar;

        derived_var = new DerivedVar_noop(this, terms_map, units);
    } else {
        SetErrMsg("Standard formula \"%s\" not supported", standard_name.c_str());
        return (-1);
    }

    // Uninstall any previous instance
    //
    NetCDFCFCollection::UninstallStandardVerticalConverter(newvar);

    NetCDFCFCollection::InstallDerivedCoordVar(newvar, derived_var, 2);

    _derivedVarsMap[newvar] = derived_var;

    return (0);
}

void NetCDFCFCollection::UninstallStandardVerticalConverter(string cvar) {

    std::map<string, DerivedVar *>::iterator itr = _derivedVarsMap.find(cvar);
    if (itr != _derivedVarsMap.end()) {
        NetCDFCFCollection::RemoveDerivedVar(cvar);
        delete itr->second;
        _derivedVarsMap.erase(itr);
    }
}

int NetCDFCFCollection::InstallStandardTimeConverter(
    string cvar, string newvar, string units) {
    if (!NetCDFCFCollection::IsTimeCoordVar(cvar)) {
        SetErrMsg("Variable %s not a time coordinate variable", cvar.c_str());
        return (-1);
    }

    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(cvar, varinfo);

    string native_units;
    varinfo.GetAtt("units", native_units);
    if (native_units.empty()) {
        SetErrMsg("Variable %s missing units attribute", cvar.c_str());
        return (-1);
    }

    NetCDFCollection::DerivedVar *derived_var;
    derived_var = new DerivedVarTime(
        this, cvar, native_units, units);

    // Uninstall any previous instance
    //
    NetCDFCFCollection::UninstallStandardTimeConverter(newvar);

    NetCDFCFCollection::InstallDerivedCoordVar(newvar, derived_var, 3);

    _derivedVarsMap[newvar] = derived_var;

    return (0);
}

void NetCDFCFCollection::UninstallStandardTimeConverter(string cvar) {

    std::map<string, DerivedVar *>::iterator itr = _derivedVarsMap.find(cvar);
    if (itr != _derivedVarsMap.end()) {
        NetCDFCFCollection::RemoveDerivedVar(cvar);
        delete itr->second;
        _derivedVarsMap.erase(itr);
    }
}

void NetCDFCFCollection::InstallDerivedCoordVar(
    string varname, DerivedVar *derivedVar, int axis) {
    NetCDFCollection::InstallDerivedVar(varname, derivedVar);

    vector<string> dims = derivedVar->GetSpatialDimNames();
    if (derivedVar->TimeVarying()) {
        dims.insert(dims.begin(), derivedVar->GetTimeDimName());
    }

    // Is the derived variable a CF "coordinate variable" or an "auxilliary
    // coordinate" variable?
    //
    if (dims.size() == 1 && dims[0] == varname) {
        _coordinateVars.push_back(varname);
    } else {
        _auxCoordinateVars.push_back(varname);
    }

    switch (axis) {
    case 0:
        _lonCoordVars.push_back(varname);
        break;
    case 1:
        _latCoordVars.push_back(varname);
        break;
    case 2:
        _vertCoordVars.push_back(varname);
        break;
    case 3:
        _timeCoordVars.push_back(varname);
        break;
    default:
        break;
    }
}

void NetCDFCFCollection::RemoveDerivedVar(string varname) {

    NetCDFCollection::RemoveDerivedVar(varname);

    vector<string>::iterator itr;

    itr = find(_lonCoordVars.begin(), _lonCoordVars.end(), varname);
    if (itr != _lonCoordVars.end())
        _lonCoordVars.erase(itr);

    itr = find(_latCoordVars.begin(), _latCoordVars.end(), varname);
    if (itr != _latCoordVars.end())
        _latCoordVars.erase(itr);

    itr = find(_vertCoordVars.begin(), _vertCoordVars.end(), varname);
    if (itr != _vertCoordVars.end())
        _vertCoordVars.erase(itr);

    itr = find(_timeCoordVars.begin(), _timeCoordVars.end(), varname);
    if (itr != _timeCoordVars.end())
        _timeCoordVars.erase(itr);

    itr = find(_auxCoordinateVars.begin(), _auxCoordinateVars.end(), varname);
    if (itr != _auxCoordinateVars.end())
        _auxCoordinateVars.erase(itr);

    itr = find(_coordinateVars.begin(), _coordinateVars.end(), varname);
    if (itr != _coordinateVars.end())
        _coordinateVars.erase(itr);
}

bool NetCDFCFCollection::GetMapProjectionProj4(
    string varname, string &proj4string) const {
    proj4string.clear();

    bool enable = EnableErrMsg(false);
    NetCDFSimple::Variable varinfo;
    int rc = NetCDFCollection::GetVariableInfo(varname, varinfo);
    EnableErrMsg(enable);
    SetErrCode(0);
    if (rc < 0)
        return (false);

    // If variable has a map projection  a NetCDF variable named
    // after the projection will exist that contains map projection
    // parameter attributes
    //
    string projection;
    varinfo.GetAtt("grid_mapping", projection);
    if (projection.empty())
        return (false); // No map projection found

    // Currently only support rotated_latitude_longitude
    //
    if (projection.compare("rotated_latitude_longitude") != 0)
        return (false);

    enable = EnableErrMsg(false);
    rc = NetCDFCollection::GetVariableInfo(projection, varinfo);
    EnableErrMsg(enable);
    SetErrCode(0);
    if (rc < 0)
        return (false);

    vector<double> lon0, pole_lat, pole_lon;
    varinfo.GetAtt("longitude_of_prime_meridian", lon0);
    varinfo.GetAtt("grid_north_pole_longitude", pole_lon);
    varinfo.GetAtt("grid_north_pole_latitude", pole_lat);

    if (lon0.size() != 1 || pole_lon.size() != 1 || pole_lat.size() != 1) {
        return (false); // Probably should return error
    }

    ostringstream oss;

    proj4string = "+ellps=WGS84 ";

    proj4string += "+proj=ob_tran";
    proj4string += " +o_proj=eqc";
    proj4string += " +to_meter=0.0174532925199";

    proj4string += " +o_lat_p=";
    oss.str("");
    oss << (double)pole_lat[0];
    proj4string += oss.str();
    proj4string += "d"; //degrees, not radians

    proj4string += " +o_lon_p=";
    oss.str("");
    //	oss << (double)(180. + pole_lon[0]);
    oss << (double)(-lon0[0]);
    proj4string += oss.str();
    proj4string += "d"; //degrees, not radians

    proj4string += " +lon_0=";
    oss.str("");
    //	oss << (double)(-lon0[0]);
    oss << (double)(180. + pole_lon[0]);
    proj4string += oss.str();
    proj4string += "d"; //degrees, not radians

    proj4string += " +no_defs";

    return (true);
}

void NetCDFCFCollection::FormatTimeStr(double seconds, string &str) const {

    int year, month, day, hour, minute, second;
    _udunit->DecodeTime(seconds, &year, &month, &day, &hour, &minute, &second);

    ostringstream oss;
    oss.fill('0');
    oss.width(4);
    oss << year;
    oss << "-";
    oss.width(2);
    oss << month;
    oss << "-";
    oss.width(2);
    oss << day;
    oss << " ";
    oss.width(2);
    oss << hour;
    oss << ":";
    oss.width(2);
    oss << minute;
    oss << ":";
    oss.width(2);
    oss << second;
    oss << " ";

    str = oss.str();
}

namespace VAPoR {
std::ostream &operator<<(
    std::ostream &o, const NetCDFCFCollection &ncdfcfc) {
    o << "NetCDFCFCollection" << endl;
    o << " _coordinateVars : ";
    for (int i = 0; i < ncdfcfc._coordinateVars.size(); i++) {
        o << ncdfcfc._coordinateVars[i] << " ";
    }
    o << endl;

    o << " _auxCoordinateVars : ";
    for (int i = 0; i < ncdfcfc._auxCoordinateVars.size(); i++) {
        o << ncdfcfc._auxCoordinateVars[i] << " ";
    }
    o << endl;

    o << " _lonCoordVars : ";
    for (int i = 0; i < ncdfcfc._lonCoordVars.size(); i++) {
        o << ncdfcfc._lonCoordVars[i] << " ";
    }
    o << endl;

    o << " _latCoordVars : ";
    for (int i = 0; i < ncdfcfc._latCoordVars.size(); i++) {
        o << ncdfcfc._latCoordVars[i] << " ";
    }
    o << endl;

    o << " _vertCoordVars : ";
    for (int i = 0; i < ncdfcfc._vertCoordVars.size(); i++) {
        o << ncdfcfc._vertCoordVars[i] << " ";
    }
    o << endl;

    o << " _timeCoordVars : ";
    for (int i = 0; i < ncdfcfc._timeCoordVars.size(); i++) {
        o << ncdfcfc._timeCoordVars[i] << " ";
    }
    o << endl;

    o << " _missingValueMap : ";
    std::map<string, double>::const_iterator itr;
    for (itr = ncdfcfc._missingValueMap.begin(); itr != ncdfcfc._missingValueMap.end(); ++itr) {
        o << itr->first << " " << itr->second << ", ";
    }
    o << endl;

    o << " Data Variables and coordinates :" << endl;
    for (int dim = 1; dim < 4; dim++) {
        vector<string> vars = ncdfcfc.GetDataVariableNames(dim, true);
        for (int i = 0; i < vars.size(); i++) {
            o << "  " << vars[i] << " : ";
            vector<string> cvars;
            int rc = ncdfcfc.GetVarCoordVarNames(vars[i], cvars);
            if (rc < 0)
                continue;
            for (int j = 0; j < cvars.size(); j++) {
                o << cvars[j] << " ";
            }
            o << endl;
        }
    }

    o << endl;
    o << endl;

    o << (const NetCDFCollection &)ncdfcfc;

    return (o);
}
}; // namespace VAPoR

bool NetCDFCFCollection::_IsCoordinateVar(
    const NetCDFSimple::Variable &varinfo) const {
    string varname = varinfo.GetName();
    vector<string> dimnames = varinfo.GetDimNames();

    if (dimnames.size() != 1)
        return (false);

    if (varname.compare(dimnames[0]) != 0)
        return (false);

    return (true);
}

vector<string> NetCDFCFCollection::_GetCoordAttrs(
    const NetCDFSimple::Variable &varinfo) const {
    vector<string> coordattrs;

    string s;
    varinfo.GetAtt("coordinates", s);
    if (s.empty())
        return (coordattrs); // No coordinates attribute found

    //
    // split the string using white space as the delimiter
    //
    stringstream ss(s);
    istream_iterator<std::string> begin(ss);
    istream_iterator<std::string> end;
    coordattrs.insert(coordattrs.begin(), begin, end);

    return (coordattrs);
}

bool NetCDFCFCollection::_IsLonCoordVar(
    const NetCDFSimple::Variable &varinfo) const {

    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "X") == 0)
        return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "longitude") == 0)
        return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty())
        return (false); // No coordinates attribute found

    return (_udunit->IsLonUnit(unit));
}

bool NetCDFCFCollection::_IsLatCoordVar(
    const NetCDFSimple::Variable &varinfo) const {
    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "Y") == 0)
        return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "latitude") == 0)
        return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty())
        return (false); // No coordinates attribute found

    return (_udunit->IsLatUnit(unit));
}

bool NetCDFCFCollection::_IsVertCoordVar(
    const NetCDFSimple::Variable &varinfo) const {

    if (varinfo.GetDimNames().size() < 1)
        return (false);

    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "Z") == 0)
        return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);

    if (StrCmpNoCase(s, "atmosphere_ln_pressure_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "atmosphere_sigma_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "atmosphere_hybrid_sigma_pressure_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "atmosphere_hybrid_height_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "atmosphere_sleve_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_sigma_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_double_sigma_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_double_sigma_coordinate") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate_g1") == 0)
        return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate_g2") == 0)
        return (true);
    if (StrCmpNoCase(s, "altitude") == 0)
        return (true);

    s.clear();
    varinfo.GetAtt("long_name", s);

    if (StrCmpNoCase(s, "model_level_number") == 0)
        return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty())
        return (false); // No coordinates attribute found

    return (_udunit->IsPressureUnit(unit) || _udunit->IsLengthUnit(unit));
}

bool NetCDFCFCollection::_IsTimeCoordVar(
    const NetCDFSimple::Variable &varinfo) const {
    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "T") == 0)
        return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "time") == 0)
        return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty())
        return (false); // No coordinates attribute found

    return (_udunit->IsTimeUnit(unit));
}

bool NetCDFCFCollection::_GetMissingValue(
    string varname,
    string &attname,
    double &mv) const {
    attname.clear();
    mv = 0.0;

    if (NetCDFCollection::IsDerivedVar(varname)) {
        return (NetCDFCollection::GetMissingValue(varname, mv));
    }

    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(varname, varinfo);

    vector<double> dvec;

    attname = "_FillValue";
    varinfo.GetAtt(attname, dvec);
    if (dvec.size()) {
        mv = dvec[0];
        return (true);
    } else {
        //
        // Use of "missing_value" is deprecated, but still
        // supported
        //
        attname = "missing_value";
        varinfo.GetAtt(attname, dvec);
        if (dvec.size()) {
            mv = dvec[0];
            return (true);
        }
    }
    return (false);
}

void NetCDFCFCollection::_GetMissingValueMap(
    map<string, double> &missingValueMap) const {
    missingValueMap.clear();

    //
    // Generate a map from all data variables with missing value
    // attributes to missing value values.
    //
    for (int d = 1; d < 5; d++) {
        vector<string> vars = NetCDFCFCollection::GetDataVariableNames(d, false);

        for (int i = 0; i < vars.size(); i++) {
            string attname;
            double mv;
            if (_GetMissingValue(vars[i], attname, mv)) {
                missingValueMap[vars[i]] = mv;
            }
        }
    }
}

NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::DerivedVar_ocean_s_coordinate_g1(
    NetCDFCFCollection *ncdfcf,
    const std::map<string, string> &formula_map) : DerivedVar(ncdfcf) {

    _dims.resize(3);
    _dimnames.resize(3);
    _s = NULL;
    _C = NULL;
    _eta = NULL;
    _depth = NULL;
    _depth_c = 0;
    _svar.clear();
    _Cvar.clear();
    _etavar.clear();
    _depthvar.clear();
    _depth_cvar.clear();
    _is_open = false;
    _ok = true;

    map<string, string>::const_iterator itr;
    itr = formula_map.find("s");
    if (itr != formula_map.end())
        _svar = itr->second;

    itr = formula_map.find("C");
    if (itr != formula_map.end())
        _Cvar = itr->second;

    itr = formula_map.find("eta");
    if (itr != formula_map.end())
        _etavar = itr->second;

    itr = formula_map.find("depth");
    if (itr != formula_map.end())
        _depthvar = itr->second;

    itr = formula_map.find("depth_c");
    if (itr != formula_map.end())
        _depth_cvar = itr->second;

    vector<size_t> dims_tmp;
    vector<string> dimnames_tmp;
    if (_ncdfc->VariableExists(_svar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_svar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_svar);
    } else if (_ncdfc->VariableExists(_Cvar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_Cvar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_Cvar);
    } else {
        _ok = false;
        return;
    }
    _dims[0] = dims_tmp[0];
    _dimnames[0] = dimnames_tmp[0];

    if (_ncdfc->VariableExists(_etavar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_etavar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_etavar);
    } else if (_ncdfc->VariableExists(_depthvar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_depthvar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_depthvar);
    } else {
        _ok = false;
        return;
    }
    _dims[1] = dims_tmp[0];
    _dims[2] = dims_tmp[1];
    _dimnames[1] = dimnames_tmp[0];
    _dimnames[2] = dimnames_tmp[1];

    _s = new float[_dims[0]];
    _C = new float[_dims[0]];
    _eta = new float[_dims[1] * _dims[2]];
    _depth = new float[_dims[1] * _dims[2]];
}

NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::~DerivedVar_ocean_s_coordinate_g1() {

    if (_s)
        delete[] _s;
    if (_C)
        delete[] _C;
    if (_eta)
        delete[] _eta;
    if (_depth)
        delete[] _depth;
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::Open(size_t) {

    if (_is_open)
        return (0); // Only open first time step
    if (!_ok) {
        SetErrMsg("Missing forumla terms");
        return (-1);
    }

    _slice_num = 0;

    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    int rc;
    double mv;

    int fd = _ncdfc->OpenRead(0, _svar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_s, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_svar, mv)) { // zero out any mv
        for (int i = 0; i < nz; i++) {
            if (_s[i] == mv)
                _s[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _Cvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_C, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_Cvar, mv)) {
        for (int i = 0; i < nz; i++) {
            if (_C[i] == mv)
                _C[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _etavar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_eta, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_etavar, mv)) {
        for (int i = 0; i < nx * ny; i++) {
            if (_eta[i] == mv)
                _eta[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _depthvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_depth, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_depthvar, mv)) {
        for (int i = 0; i < nx * ny; i++) {
            if (_depth[i] == mv)
                _depth[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _depth_cvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(&_depth_c, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);

    _is_open = true;
    return (0);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::Read(
    float *buf, int) {
    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    for (size_t z = 0; z < nz; z++) {
        for (size_t y = 0; y < ny; y++) {
            for (size_t x = 0; x < nx; x++) {
                buf[z * nx * ny + y * nx + x] =
                    _depth_c * _s[z] + (_depth[y * nx + x] - _depth_c) * _C[z] + _eta[y * nx + x] * (1 + (_depth_c * _s[z] + (_depth[y * nx + x] - _depth_c) * _C[z]) / _depth[y * nx + x]);
            }
        }
    }
    return (0);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::ReadSlice(
    float *slice, int) {
    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    if (_slice_num >= nz)
        return (0);

    size_t z = _slice_num;

    for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
            slice[y * nx + x] =
                _depth_c * _s[z] + (_depth[y * nx + x] - _depth_c) * _C[z] + _eta[y * nx + x] * (1 + (_depth_c * _s[z] + (_depth[y * nx + x] - _depth_c) * _C[z]) / _depth[y * nx + x]);
        }
    }

    _slice_num++;
    return (1);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g1::SeekSlice(
    int offset, int whence, int) {
    size_t nz = _dims[0];

    int slice = 0;
    if (whence == 0) {
        slice = offset;
    } else if (whence == 1) {
        slice = _slice_num + offset;
    } else if (whence == 2) {
        slice = offset + nz - 1;
    }
    if (slice < 0)
        slice = 0;
    if (slice > nz - 1)
        slice = nz - 1;

    _slice_num = slice;

    return (0);
}

NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::DerivedVar_ocean_s_coordinate_g2(
    NetCDFCFCollection *ncdfcf,
    const std::map<string, string> &formula_map) : DerivedVar(ncdfcf) {

    _dims.resize(3);
    _dimnames.resize(3);
    _s = NULL;
    _C = NULL;
    _eta = NULL;
    _depth = NULL;
    _depth_c = 0;
    _svar.clear();
    _Cvar.clear();
    _etavar.clear();
    _depthvar.clear();
    _depth_cvar.clear();
    _is_open = false;
    _ok = true;

    map<string, string>::const_iterator itr;
    itr = formula_map.find("s");
    if (itr != formula_map.end())
        _svar = itr->second;

    itr = formula_map.find("C");
    if (itr != formula_map.end())
        _Cvar = itr->second;

    itr = formula_map.find("eta");
    if (itr != formula_map.end())
        _etavar = itr->second;

    itr = formula_map.find("depth");
    if (itr != formula_map.end())
        _depthvar = itr->second;

    itr = formula_map.find("depth_c");
    if (itr != formula_map.end())
        _depth_cvar = itr->second;

    vector<size_t> dims_tmp;
    vector<string> dimnames_tmp;
    if (_ncdfc->VariableExists(_svar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_svar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_svar);
    } else if (_ncdfc->VariableExists(_Cvar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_Cvar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_Cvar);
    } else {
        _ok = false;
        return;
    }
    _dims[0] = dims_tmp[0];
    _dimnames[0] = dimnames_tmp[0];

    if (_ncdfc->VariableExists(_etavar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_etavar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_etavar);
    } else if (_ncdfc->VariableExists(_depthvar)) {
        dims_tmp = _ncdfc->GetSpatialDims(_depthvar);
        dimnames_tmp = _ncdfc->GetSpatialDimNames(_depthvar);
    } else {
        _ok = false;
        return;
    }
    _dims[1] = dims_tmp[0];
    _dims[2] = dims_tmp[1];
    _dimnames[1] = dimnames_tmp[0];
    _dimnames[2] = dimnames_tmp[1];

    _s = new float[_dims[0]];
    _C = new float[_dims[0]];
    _eta = new float[_dims[1] * _dims[2]];
    _depth = new float[_dims[1] * _dims[2]];
}

NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::~DerivedVar_ocean_s_coordinate_g2() {

    if (_s)
        delete[] _s;
    if (_C)
        delete[] _C;
    if (_eta)
        delete[] _eta;
    if (_depth)
        delete[] _depth;
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::Open(size_t) {

    if (_is_open)
        return (0); // Only open first time step
    if (!_ok) {
        SetErrMsg("Missing forumla terms");
        return (-1);
    }

    _slice_num = 0;

    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    int rc;
    double mv;

    int fd = _ncdfc->OpenRead(0, _svar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_s, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_svar, mv)) { // zero out any mv
        for (int i = 0; i < nz; i++) {
            if (_s[i] == mv)
                _s[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _Cvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_C, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_Cvar, mv)) {
        for (int i = 0; i < nz; i++) {
            if (_C[i] == mv)
                _C[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _etavar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_eta, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_etavar, mv)) {
        for (int i = 0; i < nx * ny; i++) {
            if (_eta[i] == mv)
                _eta[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _depthvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(_depth, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);
    if (_ncdfc->GetMissingValue(_depthvar, mv)) {
        for (int i = 0; i < nx * ny; i++) {
            if (_depth[i] == mv)
                _depth[i] = 0.0;
        }
    }

    fd = _ncdfc->OpenRead(0, _depth_cvar);
    if (fd < 0)
        return (-1);
    rc = _ncdfc->Read(&_depth_c, fd);
    if (rc < 0)
        return (-1);
    rc = _ncdfc->Close(fd);
    if (rc < 0)
        return (-1);

    _is_open = true;
    return (0);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::Read(
    float *buf, int) {
    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    for (size_t z = 0; z < nz; z++) {
        for (size_t y = 0; y < ny; y++) {
            for (size_t x = 0; x < nx; x++) {
                buf[z * nx * ny + y * nx + x] =
                    _eta[y * nx + x] + (_eta[y * nx + x] + _depth[y * nx + x]) * ((_depth_c * _s[z] +
                                                                                   _depth[y * nx + x] * _C[z]) /
                                                                                  (_depth_c + _depth[y * nx + x]));
            }
        }
    }
    return (0);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::ReadSlice(
    float *slice, int fd) {
    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    if (_slice_num >= nz)
        return (0);

    size_t z = _slice_num;

    for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
            slice[y * nx + x] =
                _eta[y * nx + x] + (_eta[y * nx + x] + _depth[y * nx + x]) * ((_depth_c * _s[z] +
                                                                               _depth[y * nx + x] * _C[z]) /
                                                                              (_depth_c + _depth[y * nx + x]));
        }
    }

    _slice_num++;
    return (1);
}

int NetCDFCFCollection::DerivedVar_ocean_s_coordinate_g2::SeekSlice(int offset, int whence, int) {
    size_t nz = _dims[0];

    int slice = 0;
    if (whence == 0) {
        slice = offset;
    } else if (whence == 1) {
        slice = _slice_num + offset;
    } else if (whence == 2) {
        slice = offset + nz - 1;
    }
    if (slice < 0)
        slice = 0;
    if (slice > nz - 1)
        slice = nz - 1;

    _slice_num = slice;

    return (0);
}

NetCDFCFCollection::DerivedVar_AHSPC::DerivedVar_AHSPC(
    NetCDFCFCollection *ncdfcf,
    const std::map<string, string> &formula_map) : DerivedVar(ncdfcf) {

    _dims.resize(3);
    _dimnames.resize(3);

    PS = NULL;   //Current surface pressure
    PHIS = NULL; //Surface Geopotential Height
    TV = NULL;   //Virtual Temperature
    P0 = 0;      //Pressure constant set by model code
    HYAM = NULL; //HYAM
    HYBM = NULL; //HYBM
    HYAI = NULL; //HYAI
    HYBI = NULL; //HYBI
    _Z3 = NULL;

    _is_open = false;
    _ok = true;

    vector<size_t> dims_tmp;
    vector<string> dimnames_tmp;

    // Get dimension size of our vertical coordinate variable
    if (_ncdfc->VariableExists("lev")) {
        dims_tmp = _ncdfc->GetSpatialDims("lev");
        dimnames_tmp = _ncdfc->GetSpatialDimNames("lev");
    } else {
        _ok = false;
        return;
    }

    _dims[0] = dims_tmp[0];
    _dimnames[0] = dimnames_tmp[0];

    // Get dimensions of the lat/lon coordinate variables used by PS(time,lat,lon)
    if (_ncdfc->VariableExists("PS")) {
        dims_tmp = _ncdfc->GetSpatialDims("PS");
        dimnames_tmp = _ncdfc->GetSpatialDimNames("PS");
    } else {
        _ok = false;
        return;
    }
    _dims[1] = dims_tmp[0];
    _dims[2] = dims_tmp[1];
    _dimnames[1] = dimnames_tmp[0];
    _dimnames[2] = dimnames_tmp[1];

    // If Z3 exists, use it.  Otherwise we will derive Z2.
    if (_ncdfc->VariableExists("Z3"))
        _Z3 = new float[_dims[0] * _dims[1] * _dims[2]];
    else {
        HYAI = new float[_dims[0] + 1];
        HYBI = new float[_dims[0] + 1];
        HYAM = new float[_dims[0]];
        HYBM = new float[_dims[0]];
        PS = new float[_dims[1] * _dims[2]];
        PHIS = new float[_dims[1] * _dims[2]];
        TV = new float[_dims[0] * _dims[1] * _dims[2]];
        _Z3 = new float[_dims[0] * _dims[1] * _dims[2]];
    }
}

NetCDFCFCollection::DerivedVar_AHSPC::~DerivedVar_AHSPC() {
    if (_Z3)
        delete[] _Z3;
    if (HYAI)
        delete[] HYAI;
    if (HYBI)
        delete[] HYBI;
    if (HYAM)
        delete[] HYAM;
    if (HYBM)
        delete[] HYBM;
    if (PS)
        delete[] PS;
    if (PHIS)
        delete[] PHIS;
    if (TV)
        delete[] TV;
}

int NetCDFCFCollection::DerivedVar_AHSPC::Open(size_t ts) {

    if (_is_open)
        return (0); // Only open first time step
    if (!_ok) {
        SetErrMsg("Missing forumla terms");
        return (-1);
    }

    _slice_num = 0;

    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    int rc;
    double mv;

    // If Z3 exists, populate it, otherwise we derive Z2.
    if (_ncdfc->VariableExists("Z3")) {
        int fd = _ncdfc->OpenRead(ts, "Z3");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(_Z3, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("Z3", mv)) {
            for (size_t i = 0; i < nx * ny * nz; i++) {
                if (_Z3[i] == mv)
                    _Z3[i] = 0.0;
            }
        }
    } else {
        // PS - Surface Pressure
        int fd = _ncdfc->OpenRead(ts, "PS");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(PS, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("PS", mv)) { // zero out any mv
            for (size_t i = 0; i < nx * ny; i++) {
                if (PS[i] == mv)
                    PS[i] = 0.0;
            }
        }

        // PHIS - Surface Geopotential Height
        if (_ncdfc->VariableExists("PHIS")) {
            fd = _ncdfc->OpenRead(ts, "PHIS");
            if (fd < 0) {
                for (size_t i = 0; i < nx * ny; i++) {
                    PHIS[i] = 0.0;
                }
            }

            rc = _ncdfc->Read(PHIS, fd);
            if (rc < 0)
                return (-1);
            rc = _ncdfc->Close(fd);
            if (rc < 0)
                return (-1);
            if (_ncdfc->GetMissingValue("PHIS", mv)) { // zero out any mv
                for (size_t i = 0; i < nx * ny; i++) {
                    if (PHIS[i] == mv)
                        PHIS[i] = 0.0;
                }
            }
        } else {
            for (size_t i = 0; i < nx * ny; i++) {
                PHIS[i] = 0.0;
            }
        }

        // T - Temperature (Virtual)
        fd = _ncdfc->OpenRead(ts, "T");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(TV, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("T", mv)) { // zero out any mv
            for (size_t i = 0; i < nx * ny; i++) {
                if (TV[i] == mv)
                    TV[i] = 0.0;
            }
        }

        // P0 - CAM pressure constant
        fd = _ncdfc->OpenRead(ts, "P0");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(&P0, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);

        // HYAM - Hybrid A midpoint coefficients
        fd = _ncdfc->OpenRead(ts, "hyam");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(HYAM, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("hyam", mv)) {
            for (size_t i = 0; i < nz; i++) {
                if (HYAM[i] == mv)
                    HYAM[i] = 0.0;
            }
        }

        // HYBM - Hybrid B midpoint coefficients
        fd = _ncdfc->OpenRead(ts, "hybm");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(HYBM, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("hybm", mv)) {
            for (size_t i = 0; i < nz; i++) {
                if (HYBM[i] == mv)
                    HYBM[i] = 0.0;
            }
        }

        // HYAI - Hybrid A interface coefficients
        fd = _ncdfc->OpenRead(ts, "hyai");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(HYAI, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("hyai", mv)) {
            for (size_t i = 0; i < nz; i++) {
                if (HYAI[i] == mv)
                    HYAI[i] = 0.0;
            }
        }

        // HYBI - Hybrid B interface coefficients
        fd = _ncdfc->OpenRead(ts, "hybi");
        if (fd < 0)
            return (-1);
        rc = _ncdfc->Read(HYBI, fd);
        if (rc < 0)
            return (-1);
        rc = _ncdfc->Close(fd);
        if (rc < 0)
            return (-1);
        if (_ncdfc->GetMissingValue("hybi", mv)) {
            for (size_t i = 0; i < nz; i++) {
                if (HYBI[i] == mv)
                    HYBI[i] = 0.0;
            }
        }

        int rc = NetCDFCFCollection::DerivedVar_AHSPC::CalculateElevation();

        if (rc > 0) {
            SetErrMsg("Unable to calculate vertical elevation slice");
            return (-1);
        }

        _is_open = true;
    }
    return (0);
}

int NetCDFCFCollection::DerivedVar_AHSPC::Read(float *buf, int) {
    float *ptr = buf;

    int rc;
    while ((rc = NetCDFCFCollection::DerivedVar_AHSPC::ReadSlice(ptr, 0)) > 0) {
        ptr += _dims[0] * _dims[1];
    }
    return (rc);
}

int NetCDFCFCollection::DerivedVar_AHSPC::CalculateElevation() {
    // The NCL program cz2ccm_dp.f processes vertical
    // slices of latitude when calculating geopotential
    // height.  This reimplementation does the same.
    // Variables have been named the same for reference.

    size_t MLON = _dims[2];
    size_t NLAT = _dims[1];
    size_t KLEV = _dims[0];

    PS1 = new float[MLON];
    PHIS1 = new float[MLON];

    // Initialize 2D scratch arrays
    float **HYBA = new float *[2];
    float **HYBB = new float *[2];
    for (size_t i = 0; i < 2; i++) {
        HYBA[i] = new float[_dims[0] + 1];
        HYBB[i] = new float[_dims[0] + 1];
    }

    float **HYPDLN = new float *[_dims[2]];
    float **HYALPH = new float *[_dims[2]];
    float **PTERM = new float *[_dims[2]];
    float **TV2 = new float *[_dims[2]];
    float **ZSLICE = new float *[_dims[2]];
    for (size_t i = 0; i < _dims[2]; i++) {
        HYPDLN[i] = new float[_dims[0] + 1];
        HYALPH[i] = new float[_dims[0]];
        PTERM[i] = new float[_dims[0]];
        TV2[i] = new float[_dims[0]];
        ZSLICE[i] = new float[_dims[0]];
    }

    // Feed hyai, hyam, hybi, and hybm into two arrays,
    // HYBA and HYBB, as required by the cz conversion
    // algorithm (cz2ccm_dp.f:61)
    for (size_t KL = 0; KL < KLEV + 1; KL++) {
        HYBA[0][KL] = HYAI[KLEV - KL];
        HYBB[0][KL] = HYBI[KLEV - KL];
    }
    // (cz2ccm_dp.f:71)
    for (size_t KL = 0; KL < KLEV; KL++) {
        HYBA[1][KL + 1] = HYAM[KLEV - KL - 1];
        HYBB[1][KL + 1] = HYBM[KLEV - KL - 1];
    }
    HYBA[1][0] = 0;
    HYBB[1][0] = 0;

    // Calculate elevation, one vertical slice at a time
    // (cz2ccm_dp.f:78)
    for (size_t NL = 0; NL < NLAT; NL++) {
        for (size_t J = 0; J < KLEV; J++) {
            for (size_t I = 0; I < MLON; I++) {
                //Volume              //2D Slice    //Point
                size_t tIndex = (NLAT * MLON * J) + (MLON * NL) + (I);
                size_t pIndex = (MLON * NL) + (I);

                // Create a 2D slice of temperature,
                // and 1D slices of PS, and PHIS
                // Slices are divided along the latitude (Y) domain
                TV2[I][J] = TV[tIndex];
                PS1[I] = PS[pIndex];
                PHIS1[I] = PHIS[pIndex];
            }
        }

        int rc = NetCDFCFCollection::DerivedVar_AHSPC::DCZ2(
            PS1,     // surface pressure
            PHIS1,   // surface height
            TV2,     // latitudinal temperature slice
            NL,      // latitude index
            P0,      // pressure constant (aka HPRB)
            HYBA,    // composite array of interface/midpoint constants
            HYBB,    // composite array of interface/midpoint constants
            KLEV,    // number of vertical layers, same as KMAX
            MLON,    // number of longitudes
            MLON,    // number of longitudes (redundant at cz2ccm_dp.f:86)
            HYPDLN,  // scratch array
            HYALPH,  // scratch array
            PTERM,   // scratch array
            ZSLICE); // calculated geopotential height

        if (rc > 0) {
            SetErrMsg("Unable to calculate vertical elevation slice");
            return (-1);
        }

        //float min;// = _Z3[0];
        //float max;// = _Z3[0];
        for (int J = 0; J < KLEV; J++) {
            for (int I = 0; I < MLON; I++) {
                int zIndex = (NLAT * MLON * J) + (MLON * NL) + (I);
                //if (zIndex == 0){
                //	min = ZSLICE[I][J];
                //	max = ZSLICE[I][J];
                //}
                _Z3[zIndex] = ZSLICE[I][J];
                //if (_Z3[zIndex] > max) max = _Z3[zIndex];
                //if (_Z3[zIndex] < min) min = _Z3[zIndex];
            }
        }
    }

    for (int i = 0; i < _dims[2]; i++) {
        if (HYPDLN[i])
            delete[] HYPDLN[i];
        if (HYALPH[i])
            delete[] HYALPH[i];
        if (PTERM[i])
            delete[] PTERM[i];
        if (TV2[i])
            delete[] TV2[i];
        if (ZSLICE[i])
            delete[] ZSLICE[i];
    }
    if (HYPDLN)
        delete[] HYPDLN;
    if (HYALPH)
        delete[] HYALPH;
    if (PTERM)
        delete[] PTERM;
    if (TV2)
        delete[] TV2;
    if (ZSLICE)
        delete[] ZSLICE;
    if (PS1)
        delete[] PS1;
    if (PHIS1)
        delete[] PHIS1;
    if (HYBA)
        delete[] HYBA;
    if (HYBB)
        delete[] HYBB;
    return (0);
}

int NetCDFCFCollection::DerivedVar_AHSPC::DCZ2(const float *PS1,
                                               const float *PHIS1,
                                               float **TV2, // cannot make const...
                                               const int NL,
                                               const float P0,
                                               float **HYBA,   // cannot make const...
                                               float **HYBB,   // cannot make const...
                                               const int KMAX, // same as KLEV
                                               const int IDIM, // same as MLON
                                               const int IMAX, // same as MLON
                                               float **HYPDLN, // cannot make const...
                                               float **HYALPH, // cannot make const...
                                               float **PTERM,
                                               float **ZSLICE) {
    // compute midpoint pressure levels (pmln)
    // cz2ccm_dp.f::222
    //float PMLN[IDIM][KMAX+1];
    float **PMLN = new float *[IDIM];
    for (size_t i = 0; i < IDIM; i++) {
        PMLN[i] = new float[KMAX + 1];
    }
    for (size_t I = 0; I < IMAX; I++) {
        PMLN[I][0] = log(P0 * HYBA[1][KMAX - 1] + PS1[I] * HYBB[0][KMAX - 1]);
        PMLN[I][KMAX] = log(P0 * HYBA[1][0] + PS1[I] * HYBB[0][0]);
    }

    for (size_t I = 0; I < IMAX; I++) {
        for (size_t K = 1; K < KMAX + 1; K++) {
            //for (size_t I=0; I<IMAX; I++){
            float arg = P0 * HYBA[1][K] + PS1[I] * HYBB[0][K]; //?
            if (arg > 0)
                PMLN[I][KMAX - K] = log(arg);
            else
                PMLN[I][KMAX - K] = 0;
        }
    }

    float R, G0, RBYG;
    R = 287.04;
    G0 = 9.80616;
    RBYG = R / G0;

    // Eq 3.a.109.2, cz2ccm_dp.f:256
    for (size_t K = 1; K < KMAX - 1; K++) {
        for (size_t I = 0; I < IMAX; I++) {
            float vpd = PMLN[I][K + 1] - PMLN[I][K - 1]; // vertical pressure difference between layers
            PTERM[I][K] = RBYG * TV2[I][K] * .5 * vpd;
        }
    }

    //cz2ccm_dp.f:265
    for (size_t K = 0; K < KMAX - 1; K++) {
        for (size_t I = 0; I < IMAX; I++) {
            float vpd = PMLN[I][K + 1] - PMLN[I][K]; // vertical pressure difference between layers
            ZSLICE[I][K] = PHIS1[I] / G0 + RBYG * TV2[I][K] * .5 * vpd;
        }
    }

    // Eq 3.a.109.5, cz2ccm_dp.f:272
    size_t K = KMAX - 1;
    for (size_t I = 0; I < IMAX; I++) {
        float a = PHIS1[I] / G0;
        float b = RBYG * TV2[I][K] * (log(PS1[I] * HYBB[0][0]) - PMLN[I][K]); //-1]);
        ZSLICE[I][KMAX - 1] = a + b;
    }

    // Eq 3.a.109.4, cz2ccm_dp.f:282
    for (size_t K = 0; K < KMAX - 1; K++) {
        size_t L = KMAX - 1;
        for (size_t I = 0; I < IMAX; I++) {
            float a = ZSLICE[I][K];
            float b = RBYG * TV2[I][L] * (log(PS1[I] * HYBB[0][0]) - .5 * (PMLN[I][L - 1] + PMLN[I][L]));

            ZSLICE[I][K] = a + b;
        }
    }
    // Eq 3.a.109.3, cz2ccm_dp.f:297
    for (size_t K = 0; K < KMAX - 2; K++) {
        for (size_t L = K + 1; L < KMAX - 1; L++) {
            for (size_t I = 0; I < IMAX; I++) {
                ZSLICE[I][K] = ZSLICE[I][K] + PTERM[I][L];
            }
        }
    }

    if (PMLN) {
        for (size_t i = 0; i < IDIM; i++) {
            delete[] PMLN[i];
        }
        delete[] PMLN;
    }
    return (0);
}

int NetCDFCFCollection::DerivedVar_AHSPC::ReadSlice(
    float *slice, int) {
    size_t nx = _dims[2];
    size_t ny = _dims[1];
    size_t nz = _dims[0];

    if (_slice_num >= nz)
        return (0);

    size_t z = nz - _slice_num - 1;

    float summation = 0;

    for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
            slice[y * nx + x] =
                _Z3[(z * nx * ny) + (nx * y) + (x)];
            summation += slice[y * nx + x];
        }
    }

    _slice_num++;
    return (1);
}

int NetCDFCFCollection::DerivedVar_AHSPC::SeekSlice(
    int offset, int whence, int) {
    size_t nz = _dims[0];

    int slice = 0;
    if (whence == 0) {
        slice = offset;
    } else if (whence == 1) {
        slice = _slice_num + offset;
    } else if (whence == 2) {
        slice = offset + nz - 1;
    }
    if (slice < 0)
        slice = 0;
    if (slice > nz - 1)
        slice = nz - 1;

    _slice_num = slice;

    return (0);
}

NetCDFCFCollection::DerivedVar_noop::DerivedVar_noop(
    NetCDFCFCollection *ncdfcf,
    const std::map<string, string> &formula_map, string units) : DerivedVar(ncdfcf) {

    _zvar.clear();
    _native_units.clear();
    _derived_units = units;

    map<string, string>::const_iterator itr;
    itr = formula_map.find("z");
    if (itr != formula_map.end())
        _zvar = itr->second;
}

int NetCDFCFCollection::DerivedVar_noop::Open(size_t ts) {

    if (_zvar.empty() || (GetSpatialDims().size() != 1)) {
        SetErrMsg("Invalid state");
        return (-1);
    }

    NetCDFCFCollection *ncdfc = (NetCDFCFCollection *)_ncdfc;

    int rc = ncdfc->GetVarUnits(_zvar, _native_units);
    if (rc < 0)
        return (-1);

    int fd = _ncdfc->OpenRead(ts, _zvar);
    if (fd < 0)
        return (-1);

    return (fd);
}

int NetCDFCFCollection::DerivedVar_noop::Read(
    float *buf, int fd) {
    int rc = _ncdfc->Read(buf, fd);

    size_t n = GetSpatialDims()[0];

    NetCDFCFCollection *ncdfc = (NetCDFCFCollection *)_ncdfc;

    rc = ncdfc->Convert(
        _native_units, _derived_units, buf, buf, n);
    if (rc < 0)
        return (-1);
    return (0);
}

int NetCDFCFCollection::DerivedVar_noop::ReadSlice(
    float *slice, int fd) {
    return (Read(slice, fd));
}

int NetCDFCFCollection::DerivedVar_noop::SeekSlice(
    int offset, int whence, int fd) {
    return (0);
}

NetCDFCFCollection::DerivedVarTime::DerivedVarTime(
    NetCDFCFCollection *ncdfcf,
    string native_var, string native_units, string derived_units) : DerivedVar(ncdfcf) {

    _native_var = native_var;
    _native_units = native_units;
    _derived_units = derived_units;
    _dims.clear();
    _dimnames.clear();
    _first = true;
    _ts = 0;

    _timedim = _ncdfc->GetTimeDim(native_var);
    _timedimname = _ncdfc->GetTimeDim(native_var);

    _timecoords = new double[_timedim];
}

NetCDFCFCollection::DerivedVarTime::~DerivedVarTime() {
    if (_timecoords)
        delete[] _timecoords;
}

int NetCDFCFCollection::DerivedVarTime::Open(size_t ts) {

    _ts = ts;
    if (!_first)
        return (0); // Only open once

    vector<double> timesvec;
    int rc = _ncdfc->GetTimes(_native_var, timesvec);
    if (rc < 0)
        return (-1);

    assert(timesvec.size() == _timedim);
    for (int i = 0; i < timesvec.size(); i++) {
        _timecoords[i] = timesvec[i];
    }

    // Convert to desired units
    //
    NetCDFCFCollection *ncdfc = (NetCDFCFCollection *)_ncdfc;
    rc = ncdfc->Convert(
        _native_units, _derived_units, _timecoords,
        _timecoords, _timedim);
    if (rc < 0)
        return (-1);

    _first = false;
    return (0);
}

int NetCDFCFCollection::DerivedVarTime::Read(
    float *buf, int) {
    if (_first)
        return (-1);
    *buf = _timecoords[_ts];
    return (0);
}

int NetCDFCFCollection::DerivedVarTime::ReadSlice(
    float *slice, int) {
    if (_first)
        return (-1);
    *slice = _timecoords[_ts];
    return (1);
}

int NetCDFCFCollection::DerivedVarTime::SeekSlice(
    int offset, int whence, int) {
    return (0);
}
