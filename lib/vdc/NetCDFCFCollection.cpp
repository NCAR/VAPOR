#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include "vapor/VAssert.h"
#ifdef WIN32
    #include "vapor/udunits2.h"
#else
    #include <udunits2.h>
#endif
#include <vapor/NetCDFCFCollection.h>

using namespace VAPoR;
using namespace Wasp;
using namespace std;

namespace {
vector<string> make_unique(vector<string> v)
{
    sort(v.begin(), v.end());
    vector<string>::iterator last2;
    last2 = unique(v.begin(), v.end());
    v.erase(last2, v.end());
    return (v);
}
};    // namespace

NetCDFCFCollection::NetCDFCFCollection() : NetCDFCollection()
{
    _coordinateVars.clear();
    _auxCoordinateVars.clear();
    _lonCoordVars.clear();
    _latCoordVars.clear();
    _vertCoordVars.clear();
    _timeCoordVars.clear();
    _missingValueMap.clear();

    _udunit = NULL;
}
NetCDFCFCollection::~NetCDFCFCollection()
{
    if (_udunit) delete _udunit;

}

static bool IsNameCFCompliant(const string &name, bool strict=true) {
    if (!isalpha(name[0]) || (!strict && name[0] == '_')) return false;
    for (int i = 1; i < name.size(); i++)
        if (!(isalnum(name[i]) || name[i] == '_' || (!strict && (name[i] == '_' || name[i] == '.' || name[i] == '-'))))
            return false;
    return true;
}

int NetCDFCFCollection::_Initialize(const vector<string> &files)
{
    // Look for time coordinate variables. Must be 1D and have same
    // name as dimension. Need not be present in all files.
    //
    vector<string> tv;
    for (int j = 0; j < files.size(); j++) {
        vector<string> emptyvec;
        vector<string> file(1, files[j]);
        int            rc = NetCDFCollection::Initialize(file, emptyvec, emptyvec);
        if (rc < 0) return (-1);

        // Get all 1D variables
        //
        vector<string> vars = NetCDFCollection::GetVariableNames(1, false);
        for (int i = 0; i < vars.size(); i++) {
            NetCDFSimple::Variable varinfo;
            (void)NetCDFCollection::GetVariableInfo(vars[i], varinfo);

            if (_IsCoordinateVar(varinfo) && _IsTimeCoordVar(varinfo)) { tv.push_back(vars[i]); }
        }
        if (tv.size()) break;    // Should we process all files???
    }

    //
    // Reinitialize the base class now that we know the time coordinate
    // variable. We're assuming that the time coordinate variable and
    // the time dimension have the same name. Thus we're not handling
    // the case where time variable could be a CF "auxilliary
    // coordinate variable".
    //
    int rc = NetCDFCollection::Initialize(files, tv, tv);
    if (rc < 0) return (-1);

    for (const auto &v : GetVariableNames(-1, false)) {
        if (!IsNameCFCompliant(v, false)) {
            SetErrMsg("Variable name \"%s\" is not compliant with NetCDF-CF", v.c_str());
            return -1;
        }
    }

    return (0);
}

int NetCDFCFCollection::Initialize(const vector<string> &files)
{
    _coordinateVars.clear();
    _auxCoordinateVars.clear();
    _lonCoordVars.clear();
    _latCoordVars.clear();
    _vertCoordVars.clear();
    _timeCoordVars.clear();
    _missingValueMap.clear();

    if (_udunit) delete _udunit;
    _udunit = new UDUnits();
    int rc = _udunit->Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialized udunits2 library : %s", _udunit->GetErrMsg().c_str());
        return (0);
    }

    rc = _Initialize(files);
    if (rc < 0) return (-1);

    //
    // Identify all of the CF "coordinate" variables first,
    // which are 1D in space
    // or 1D in time and have
    // the same name as their one dimension
    //
    vector<string> vars = NetCDFCollection::GetVariableNames(1, true);
    vector<string> tcvars = NetCDFCollection::GetVariableNames(1, false);
    vars.insert(vars.end(), tcvars.begin(), tcvars.end());
    for (int i = 0; i < vars.size(); i++) {
        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(vars[i], varinfo);

        if (_IsCoordinateVar(varinfo)) { _coordinateVars.push_back(vars[i]); }
    }


    // Now find all coordinate variables that can be identified
    // by their units, standard name, etc.
    //
    vars.clear();
    for (int i = 1; i < 4; i++) {
        vector<string> v = NetCDFCollection::GetVariableNames(i, false);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    for (auto varName : vars) {
        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(varName, varinfo);

        if (_IsLonCoordVar(varinfo)) {
            _lonCoordVars.push_back(varName);
            _auxCoordinateVars.push_back(varName);
        } else if (_IsLatCoordVar(varinfo)) {
            _latCoordVars.push_back(varName);
            _auxCoordinateVars.push_back(varName);
        } else if (_IsVertCoordVar(varinfo)) {
            _vertCoordVars.push_back(varName);
            _auxCoordinateVars.push_back(varName);
            //
            // Only support CF "Coordinate" variables as time coord variables
            //
        } else if (_IsCoordinateVar(varinfo) && _IsTimeCoordVar(varinfo)) {
            _timeCoordVars.push_back(varName);
            _auxCoordinateVars.push_back(varName);
        }
    }

    // Finally, any variable that is identified by a "coordinate" variable
    // attribute is an "auxilliary" coordinate variable
    //
    vars.clear();
    for (int i = 0; i < 4; i++) {
        vector<string> v = NetCDFCollection::GetVariableNames(1, false);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    for (auto varName : vars) {
        string                 timeDimName;
        NetCDFSimple::Variable varinfo;
        (void)NetCDFCollection::GetVariableInfo(varName, varinfo);

        if (!GetTimeDimName(varName).empty()) timeDimName = GetTimeDimName(varName);

        vector<string> coordattr = _GetCoordAttrs(varinfo);
        if (!coordattr.size()) continue;

        for (int j = 0; j < coordattr.size(); j++) {
            //
            // Make sure the auxiliary coordinate variable
            // actually exists in the data collection
            //
            if (NetCDFCollection::VariableExists(coordattr[j])) { _auxCoordinateVars.push_back(coordattr[j]); }
        }
    }

    //
    // sort and remove duplicate coordinate variables
    //
    _coordinateVars = make_unique(_coordinateVars);
    _auxCoordinateVars = make_unique(_auxCoordinateVars);
    _lonCoordVars = make_unique(_lonCoordVars);
    _latCoordVars = make_unique(_latCoordVars);
    _vertCoordVars = make_unique(_vertCoordVars);
    _timeCoordVars = make_unique(_timeCoordVars);

    string mvattname;
    _GetMissingValueMap(_missingValueMap);

    return (0);
}

vector<string> NetCDFCFCollection::GetDataVariableNames(int ndim, bool spatial) const
{
    vector<string> tmp = NetCDFCollection::GetVariableNames(ndim, spatial);

    vector<string> varnames;
    for (int i = 0; i < tmp.size(); i++) {
        //
        // Strip off any coordinate variables
        //
        if (IsCoordVarCF(tmp[i])) continue;

        vector<string> cvars;
        bool           enable = EnableErrMsg(false);
        int            rc = NetCDFCFCollection::GetVarCoordVarNames(tmp[i], cvars);
        EnableErrMsg(enable);
        SetErrCode(0);

        if (rc < 0) continue;    // Doesn't have coordinate variables

        int myndim = cvars.size();
        if (spatial && IsTimeVarying(tmp[i])) {
            myndim--;    // don't count time dimension
        }
        if (myndim != ndim) continue;

        varnames.push_back(tmp[i]);
    }

    return (varnames);
}

int NetCDFCFCollection::GetVarCoordVarNames(string var, vector<string> &cvars) const
{
    cvars.clear();

    NetCDFSimple::Variable varinfo;

    int rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    if (rc < 0) return (rc);

    vector<string> dimnames = varinfo.GetDimNames();

    //
    // First look for auxiliary coordinate variables
    //
    bool           hasTimeCoord = false;
    bool           hasLatCoord = false;
    bool           hasLonCoord = false;
    bool           hasVertCoord = false;
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

    // Ugh. Stupid hack to make Z3 the vertical coordinate variable
    // for 3D CAM variables
    //
    if (!hasVertCoord) {
        vector<string> v, vars;
        v = NetCDFCollection::GetVariableNames(3, false);
        vars.insert(vars.end(), v.begin(), v.end());
        v = NetCDFCollection::GetVariableNames(4, false);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    //
    // Now see if any "coordinate variables" for which we haven't
    // already identified a coord var exist.
    //
    if (tmpcvars.size() != dimnames.size()) {
        for (int i = 0; i < dimnames.size(); i++) {
            if (NetCDFCFCollection::IsCoordVarCF(dimnames[i])) {    // is a CF "coordinate variable"?
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
    VAssert(cvars.size() == tmpcvars.size());

    return (0);
}

bool NetCDFCFCollection::IsVertCoordVarPressure(string var) const
{
    if (!IsVertCoordVar(var)) return (false);

    NetCDFSimple::Variable varinfo;

    bool enable = EnableErrMsg(false);
    int  rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    if (rc < 0) {
        EnableErrMsg(enable);
        return (false);
    }

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsPressureUnit(unit));
}

bool NetCDFCFCollection::IsVertCoordVarLength(string var) const
{
    if (!IsVertCoordVar(var)) return (false);

    NetCDFSimple::Variable varinfo;

    bool enable = EnableErrMsg(false);
    int  rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    EnableErrMsg(enable);
    if (rc < 0) { return (false); }

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsLengthUnit(unit));
}

bool NetCDFCFCollection::IsVertCoordVarUp(string cvar) const
{
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

int NetCDFCFCollection::GetVarUnits(string var, string &units) const
{
    units.clear();

    NetCDFSimple::Variable varinfo;

    int rc = NetCDFCollection::GetVariableInfo(var, varinfo);
    if (rc < 0) return (rc);

    varinfo.GetAtt("units", units);
    return (0);
}

int NetCDFCFCollection::Convert(const string from, const string to, const double *src, double *dst, size_t n) const
{
    bool status = _udunit->Convert(from, to, src, dst, n);

    if (!status) {
        SetErrMsg("NetCDFCFCollection::Convert(%s , %s,,) : failed", from.c_str(), to.c_str());
        return (-1);
    }
    return (0);
}
int NetCDFCFCollection::Convert(const string from, const string to, const float *src, float *dst, size_t n) const
{
    bool status = _udunit->Convert(from, to, src, dst, n);

    if (!status) {
        SetErrMsg("NetCDFCFCollection::Convert(%s , %s,,) : failed", from.c_str(), to.c_str());
        return (-1);
    }
    return (0);
}

bool NetCDFCFCollection::GetMissingValue(string varname, double &mv) const
{
    map<string, double>::const_iterator itr;

    itr = _missingValueMap.find(varname);
    if (itr != _missingValueMap.end()) {
        mv = itr->second;
        return (true);
    }
    return (false);
}

int NetCDFCFCollection::OpenRead(size_t ts, string varname)
{
    int fd = NetCDFCollection::OpenRead(ts, varname);

    return (fd);
}

bool NetCDFCFCollection::IsVertDimensionless(string cvar) const
{
    //
    // Return false if variable isn't a vertical coordinate variable at all
    //
    if (!IsVertCoordVar(cvar)) return (false);

    //
    // If we get to here the cvar is a valid coordinate variable
    // so GetVariableInfo() should return successfully
    //
    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(cvar, varinfo);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (!(_udunit->IsPressureUnit(unit) || _udunit->IsLengthUnit(unit)));
}

bool NetCDFCFCollection::GetMapProjectionProj4(string varname, string &proj4string) const
{
    proj4string.clear();

    bool                   enable = EnableErrMsg(false);
    NetCDFSimple::Variable varinfo;
    int                    rc = NetCDFCollection::GetVariableInfo(varname, varinfo);
    EnableErrMsg(enable);
    SetErrCode(0);
    if (rc < 0) return (false);

    // If variable has a map projection  a NetCDF variable named
    // after the projection will exist that contains map projection
    // parameter attributes
    //
    string projection;
    varinfo.GetAtt("grid_mapping", projection);
    if (projection.empty()) return (false);    // No map projection found

    // Currently only support rotated_latitude_longitude
    //
    if (projection.compare("rotated_latitude_longitude") != 0) return (false);

    enable = EnableErrMsg(false);
    rc = NetCDFCollection::GetVariableInfo(projection, varinfo);
    EnableErrMsg(enable);
    SetErrCode(0);
    if (rc < 0) return (false);

    vector<double> lon0, pole_lat, pole_lon;
    varinfo.GetAtt("longitude_of_prime_meridian", lon0);
    varinfo.GetAtt("grid_north_pole_longitude", pole_lon);
    varinfo.GetAtt("grid_north_pole_latitude", pole_lat);

    if (lon0.size() != 1 || pole_lon.size() != 1 || pole_lat.size() != 1) {
        return (false);    // Probably should return error
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
    proj4string += "d";    // degrees, not radians

    proj4string += " +o_lon_p=";
    oss.str("");
    //	oss << (double)(180. + pole_lon[0]);
    oss << (double)(-lon0[0]);
    proj4string += oss.str();
    proj4string += "d";    // degrees, not radians

    proj4string += " +lon_0=";
    oss.str("");
    //	oss << (double)(-lon0[0]);
    oss << (double)(180. + pole_lon[0]);
    proj4string += oss.str();
    proj4string += "d";    // degrees, not radians

    proj4string += " +no_defs";

    return (true);
}

void NetCDFCFCollection::FormatTimeStr(double seconds, string &str) const
{
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
std::ostream &operator<<(std::ostream &o, const NetCDFCFCollection &ncdfcfc)
{
    o << "NetCDFCFCollection" << endl;
    o << " _coordinateVars : ";
    for (int i = 0; i < ncdfcfc._coordinateVars.size(); i++) { o << ncdfcfc._coordinateVars[i] << " "; }
    o << endl;

    o << " _lonCoordVars : ";
    for (int i = 0; i < ncdfcfc._lonCoordVars.size(); i++) { o << ncdfcfc._lonCoordVars[i] << " "; }
    o << endl;

    o << " _latCoordVars : ";
    for (int i = 0; i < ncdfcfc._latCoordVars.size(); i++) { o << ncdfcfc._latCoordVars[i] << " "; }
    o << endl;

    o << " _vertCoordVars : ";
    for (int i = 0; i < ncdfcfc._vertCoordVars.size(); i++) { o << ncdfcfc._vertCoordVars[i] << " "; }
    o << endl;

    o << " _timeCoordVars : ";
    for (int i = 0; i < ncdfcfc._timeCoordVars.size(); i++) { o << ncdfcfc._timeCoordVars[i] << " "; }
    o << endl;

    o << " _missingValueMap : ";
    std::map<string, double>::const_iterator itr;
    for (itr = ncdfcfc._missingValueMap.begin(); itr != ncdfcfc._missingValueMap.end(); ++itr) { o << itr->first << " " << itr->second << ", "; }
    o << endl;

    o << " Data Variables and coordinates :" << endl;
    for (int dim = 1; dim < 4; dim++) {
        vector<string> vars = ncdfcfc.GetDataVariableNames(dim, true);
        for (int i = 0; i < vars.size(); i++) {
            o << "  " << vars[i] << " : ";
            vector<string> cvars;
            int            rc = ncdfcfc.GetVarCoordVarNames(vars[i], cvars);
            if (rc < 0) continue;
            for (int j = 0; j < cvars.size(); j++) { o << cvars[j] << " "; }
            o << endl;
        }
    }

    o << endl;
    o << endl;

    o << (const NetCDFCollection &)ncdfcfc;

    return (o);
}
};    // namespace VAPoR

bool NetCDFCFCollection::_IsCoordinateVar(const NetCDFSimple::Variable &varinfo) const
{
    string         varname = varinfo.GetName();
    vector<string> dimnames = varinfo.GetDimNames();

    // A CF "coordinate variable" is a 1D variable whose name matches
    // its dimension name. Here we also allow a variable with 1 spatial
    // dimension and 1 time dimension
    //
    if (dimnames.size() == 1 && varname == dimnames[0]) return (true);

    if (dimnames.size() == 2 && IsTimeVarying(varname) && varname == dimnames[1]) return (true);

    return (false);
}

vector<string> NetCDFCFCollection::_GetCoordAttrs(const NetCDFSimple::Variable &varinfo) const
{
    vector<string> coordattrs;

    string s;
    varinfo.GetAtt("coordinates", s);
    if (s.empty()) return (coordattrs);    // No coordinates attribute found

    //
    // split the string using white space as the delimiter
    //
    stringstream                  ss(s);
    istream_iterator<std::string> begin(ss);
    istream_iterator<std::string> end;
    coordattrs.insert(coordattrs.begin(), begin, end);

    return (coordattrs);
}

bool NetCDFCFCollection::_IsLonCoordVar(const NetCDFSimple::Variable &varinfo) const
{
    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "X") == 0) return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "longitude") == 0) return (true);

    if (StrCmpNoCase(s, "grid_longitude") == 0) return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsLonUnit(unit));
}

bool NetCDFCFCollection::_IsLatCoordVar(const NetCDFSimple::Variable &varinfo) const
{
    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "Y") == 0) return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "latitude") == 0) return (true);

    if (StrCmpNoCase(s, "grid_latitude") == 0) return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsLatUnit(unit));
}

bool NetCDFCFCollection::_IsVertCoordVar(const NetCDFSimple::Variable &varinfo) const
{
    if (varinfo.GetDimNames().size() < 1) return (false);

    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "Z") == 0) return (true);

    // The attribute "cartesian_axis" is not part of the CF spec, but
    // apparently the MOM6 ocean model uses it :-(
    //
    varinfo.GetAtt("cartesian_axis", s);
    if (StrCmpNoCase(s, "Z") == 0) return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);

    if (StrCmpNoCase(s, "atmosphere_ln_pressure_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "atmosphere_sigma_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "atmosphere_hybrid_sigma_pressure_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "atmosphere_hybrid_height_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "atmosphere_sleve_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_sigma_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_double_sigma_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_double_sigma_coordinate") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate_g1") == 0) return (true);
    if (StrCmpNoCase(s, "ocean_s_coordinate_g2") == 0) return (true);
    if (StrCmpNoCase(s, "altitude") == 0) return (true);

    s.clear();
    varinfo.GetAtt("long_name", s);

    if (StrCmpNoCase(s, "model_level_number") == 0) return (true);

    varinfo.GetAtt("positive", s);
    if ((StrCmpNoCase(s, "up") == 0) || (StrCmpNoCase(s, "down") == 0)) return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsPressureUnit(unit) || _udunit->IsLengthUnit(unit));
}

bool NetCDFCFCollection::_IsTimeCoordVar(const NetCDFSimple::Variable &varinfo) const
{
    string s;
    varinfo.GetAtt("axis", s);
    if (StrCmpNoCase(s, "T") == 0) return (true);

    s.clear();
    varinfo.GetAtt("standard_name", s);
    if (StrCmpNoCase(s, "time") == 0) return (true);

    string unit;
    varinfo.GetAtt("units", unit);
    if (unit.empty()) return (false);    // No coordinates attribute found

    return (_udunit->IsTimeUnit(unit));
}

bool NetCDFCFCollection::_GetMissingValue(string varname, string attname, double &mv) const
{
    mv = 0.0;

    NetCDFSimple::Variable varinfo;
    (void)NetCDFCollection::GetVariableInfo(varname, varinfo);

    vector<double> dvec;

    varinfo.GetAtt(attname, dvec);
    if (dvec.size()) {
        mv = dvec[0];
        return (true);
    }

    return (false);
}

void NetCDFCFCollection::_GetMissingValueMap(map<string, double> &missingValueMap) const
{
    missingValueMap.clear();

    //
    // Generate a map from all data variables with missing value
    // attributes to missing value values. The CF conventions allow
    // for two different attributes to specify missing values, "missing_value",
    // and "_FillValue". The latter is for historical reasaons. Here we look
    // for either, but give priority to "missing_value"
    //
    for (int d = 1; d < 5; d++) {
        vector<string> vars = NetCDFCFCollection::GetDataVariableNames(d, false);

        for (int i = 0; i < vars.size(); i++) {
            double mv;
            if (_GetMissingValue(vars[i], "_FillValue", mv)) { missingValueMap[vars[i]] = mv; }
            if (_GetMissingValue(vars[i], "missing_value", mv)) { missingValueMap[vars[i]] = mv; }
        }
    }
}
