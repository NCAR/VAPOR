#include <iostream>
#include <algorithm>
#include <utility>
#include "vapor/VAssert.h"
#include <netcdf.h>
#include <vapor/NetCDFCollection.h>

using namespace VAPoR;
using namespace Wasp;
using namespace std;

namespace {

const string derivedTimeDimName = "derivedTimeDim";

bool readSliceOK(vector<size_t> dims, size_t start[], size_t count[])
{
    if (dims.size() == 3) {
        if (count[0] != 1) return (false);
        for (int i = 1; i < dims.size(); i++) {
            if (count[i] != dims[i]) return (false);
            if (start[i] != 0) return (false);
        }
        return (true);
    } else if (dims.size() == 2) {
        for (int i = 0; i < dims.size(); i++) {
            if (count[i] != dims[i]) return (false);
            if (start[i] != 0) return (false);
        }
        return (true);
    }

    return (false);
}

bool readOK(vector<size_t> dims, size_t start[], size_t count[])
{
    if (dims.size() == 0) return (true);
    if (dims.size() != 1) return (false);
    if (count[0] != dims[0]) return (false);
    if (start[0] != 0) return (false);

    return (true);
}

};    // namespace

NetCDFCollection::NetCDFCollection()
{
    _variableList.clear();
    _staggeredDims.clear();
    _dimNames.clear();
    _dimLens.clear();
    _missingValAttName.clear();
    _times.clear();
    _timesMap.clear();
    _ovr_table.clear();
    _ncdfmap.clear();
    _failedVars.clear();
}

NetCDFCollection::~NetCDFCollection() { ReInitialize(); }

void NetCDFCollection::ReInitialize()
{
    map<string, NetCDFSimple *>::iterator itr;
    for (itr = _ncdfmap.begin(); itr != _ncdfmap.end(); ++itr) { delete itr->second; }

    std::map<int, fileHandle>::iterator itr1;
    for (itr1 = _ovr_table.begin(); itr1 != _ovr_table.end(); ++itr1) {
        int fd = itr1->first;
        (void)NetCDFCollection::Close(fd);
    }

    _variableList.clear();
    _staggeredDims.clear();
    _dimNames.clear();
    _dimLens.clear();
    _missingValAttName.clear();
    _times.clear();
    _timesMap.clear();
    _ovr_table.clear();
    _ncdfmap.clear();
    _failedVars.clear();
}

int NetCDFCollection::Initialize(const vector<string> &files, const vector<string> &time_dimnames, const vector<string> &time_coordvars)
{
    vector<string> l_time_dimnames = time_dimnames;

    ReInitialize();

    //
    // Build a hash table to map a variable's time dimension
    // to its time coordinates
    //
    int file_org;    // case 1, 2, 3 (3a or 3b)
    int rc = NetCDFCollection::_InitializeTimesMap(files, l_time_dimnames, time_coordvars, _timesMap, _times, file_org);
    if (rc < 0) return (-1);

    //
    // If no time dimension specified we create one. Really only need
    // to do this if there are multiple files with the same variable(s)
    // appearing in multiple files.
    //
    if (l_time_dimnames.empty() && _times.size() > 1) {
        l_time_dimnames.push_back(derivedTimeDimName);
        _dimNames.push_back(derivedTimeDimName);
        _dimLens.push_back(_times.size());
    }

    for (int i = 0; i < files.size(); i++) {
        NetCDFSimple *netcdf = new NetCDFSimple();
        _ncdfmap[files[i]] = netcdf;

        rc = netcdf->Initialize(files[i]);
        if (rc < 0) {
            SetErrMsg("NetCDFSimple::Initialize(%s)", files[i].c_str());
            return (-1);
        }
        //        printf("INIT %i = %s\n", i, files[i].c_str());

        //
        // Get dimension names and lengths
        //
        vector<string> dimnames;
        vector<size_t> dims;
        netcdf->GetDimensions(dimnames, dims);
        for (int j = 0; j < dimnames.size(); j++) {
            // No duplicates
            //
            vector<string>::iterator itr;
            itr = find(_dimNames.begin(), _dimNames.end(), dimnames[j]);
            if (itr == _dimNames.end()) {
                _dimNames.push_back(dimnames[j]);
                _dimLens.push_back(dims[j]);
                _dimIsTimeVarying.push_back(false);
            }
            //
            // if this is a time dimension and it hasn't shown up in another
            // file we increment the dimension length
            //
            else if (find(l_time_dimnames.begin(), l_time_dimnames.end(), dimnames[j]) != l_time_dimnames.end()) {
                _dimLens[itr - _dimNames.begin()] += dims[j];
            } else if (_dimLens[itr - _dimNames.begin()] != dims[j]) {
                _dimIsTimeVarying[itr - _dimNames.begin()] = true;
                //                SetErrMsg("Spatial dimension %s changed size", dimnames[j].c_str());
                //                return (-1);
            }
        }

        //
        // Get variable info for all variables in current file
        //
        const vector<NetCDFSimple::Variable> &variables = netcdf->GetVariables();

        //
        // For each variable in the file add it to _variablesList
        //
        for (int j = 0; j < variables.size(); j++) {
            string name = variables[j].GetName();

            map<string, TimeVaryingVar>::iterator p = _variableList.find(name);

            //
            // If this is the first time we've seen this variable
            // add it to _variablesList
            //
            if (p == _variableList.end()) {
                TimeVaryingVar tvv;
                _variableList[name] = tvv;
                p = _variableList.find(name);
            }
            TimeVaryingVar &tvvref = p->second;

            bool enable = EnableErrMsg(false);
            int  rc = tvvref.Insert(netcdf, variables[j], files[i], l_time_dimnames, _timesMap, file_org);
            (void)EnableErrMsg(enable);
            if (rc < 0) {
                SetErrCode(0);
                _failedVars.push_back(files[i] + ": " + variables[j].GetName());
                continue;
            }
        }
    }

    for (auto itr = _variableList.begin(); itr != _variableList.end(); ++itr) {
        TimeVaryingVar &tvvref = itr->second;
        tvvref.Sort();
    }

    return (0);
}

#include <vapor/STLUtils.h>

long NetCDFCollection::GetDimLengthAtTime(string name, long ts)
{
    double realTime = _times[ts];

    const auto end = _timesMap.cend();
    string     filePath;
    for (auto it = _timesMap.cbegin(); it != end; ++it) {
        for (int i = 0; i < it->second.size(); i++) {
            if (it->second[i] == realTime) {
                filePath = it->first;
                goto SEARCH_FINISHED;
            }
        }
    }
SEARCH_FINISHED:

    if (filePath.empty()) {
        MyBase::SetErrMsg("Time %li (%f) not found", ts, realTime);
        assert(0);
        return -1;
    }

    NetCDFSimple *nc = nullptr;
    for (auto it = _ncdfmap.cbegin(); it != _ncdfmap.cend(); ++it) {
        if (STLUtils::BeginsWith(filePath, it->first)) {
            nc = it->second;
            break;
        }
    }

    if (!nc) {
        MyBase::SetErrMsg("NC for file not found");
        assert(0);
        return -1;
    }

    vector<string> names;
    vector<size_t> lengths;
    nc->GetDimensions(names, lengths);

    for (int i = 0; i < names.size(); i++) {
        if (names[i] == name) { return lengths[i]; }
    }

    MyBase::SetErrMsg("Dimension not found at timestep %li", ts);
    return -1;
}

bool NetCDFCollection::VariableExists(string varname) const
{
    //
    // Should be checking dependencies for derived variable!
    //
    if (NetCDFCollection::IsDerivedVar(varname)) return (true);

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (false); }
    return (true);
}

bool NetCDFCollection::VariableExists(size_t ts, string varname) const
{
    if (ts >= _times.size()) return (false);

    //
    // Should be checking dependencies for derived variable!
    //
    if (NetCDFCollection::IsDerivedVar(varname)) return (true);

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (false); }
    const TimeVaryingVar &tvvar = p->second;

    if (!tvvar.GetTimeVarying()) return (true);    // CV variables exist for all times

    double mytime = _times[ts];

    vector<double> times = tvvar.GetTimes();
    for (int i = 0; i < times.size(); i++) {
        if (times[i] == mytime) return (true);
    }
    return (false);
}

bool NetCDFCollection::IsStaggeredVar(string varname) const
{
    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (false); }
    const TimeVaryingVar &var = p->second;

    vector<string> dimnames = var.GetSpatialDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        if (IsStaggeredDim(dimnames[i])) return (true);
    }
    return (false);
}

bool NetCDFCollection::IsStaggeredDim(string dimname) const
{
    for (int i = 0; i < _staggeredDims.size(); i++) {
        if (dimname.compare(_staggeredDims[i]) == 0) return (true);
    }
    return (false);
}

vector<string> NetCDFCollection::GetVariableNames(int ndims, bool spatial) const
{
    map<string, TimeVaryingVar>::const_iterator p = _variableList.begin();

    vector<string> names;

    for (; p != _variableList.end(); ++p) {
        const TimeVaryingVar &tvvars = p->second;
        int                   myndims = tvvars.GetSpatialDims().size();
        if (!spatial && tvvars.GetTimeVarying()) { myndims++; }
        if (myndims == ndims) { names.push_back(p->first); }
    }

    //
    // Add any derived variables
    //
    map<string, DerivedVar *>::const_iterator itr;
    for (itr = _derivedVarsMap.begin(); itr != _derivedVarsMap.end(); ++itr) {
        DerivedVar *derivedVar = itr->second;

        int myndim = derivedVar->GetSpatialDims().size();

        if (!spatial && derivedVar->TimeVarying()) { myndim++; }

        if (myndim == ndims) { names.push_back(itr->first); }
    }

    return (names);
}

vector<size_t> NetCDFCollection::GetSpatialDims(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetSpatialDims());
    }

    vector<size_t> dims;
    dims.clear();

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (dims); }
    const TimeVaryingVar &tvvars = p->second;

    vector<size_t> mydims = tvvars.GetSpatialDims();
    vector<string> dimnames = tvvars.GetSpatialDimNames();

    //
    // Deal with any staggered dimensions
    //
    for (int i = 0; i < mydims.size(); i++) {
        dims.push_back(mydims[i]);
        for (int j = 0; j < _staggeredDims.size(); j++) {
            if (dimnames[i].compare(_staggeredDims[j]) == 0) { dims[i] = mydims[i] - 1; }
        }
    }
    return (dims);
}

vector<string> NetCDFCollection::GetSpatialDimNames(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetSpatialDimNames());
    }

    vector<string> dimnames;

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (dimnames); }
    const TimeVaryingVar &tvvars = p->second;

    dimnames = tvvars.GetSpatialDimNames();
    return (dimnames);
}

size_t NetCDFCollection::GetTimeDim(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetTimeDim());
    }

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (0); }
    const TimeVaryingVar &tvvars = p->second;

    return (tvvars.GetNumTimeSteps());
}

string NetCDFCollection::GetTimeDimName(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetTimeDimName());
    }

    string dimname;

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (dimname); }
    const TimeVaryingVar &tvvars = p->second;

    dimname = tvvars.GetTimeDimName();
    return (dimname);
}

vector<size_t> NetCDFCollection::GetDims(string varname) const
{
    std::vector<size_t> dims = NetCDFCollection::GetSpatialDims(varname);
    if (NetCDFCollection::IsTimeVarying(varname)) { dims.insert(dims.begin(), NetCDFCollection::GetTimeDim(varname)); }
    return (dims);
}

vector<string> NetCDFCollection::GetDimNames(string varname) const
{
    std::vector<string> dimnames = NetCDFCollection::GetSpatialDimNames(varname);
    if (NetCDFCollection::IsTimeVarying(varname)) { dimnames.insert(dimnames.begin(), NetCDFCollection::GetTimeDimName(varname)); }
    return (dimnames);
}

bool NetCDFCollection::IsTimeVarying(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->TimeVarying());
    }

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (false); }
    const TimeVaryingVar &tvvars = p->second;
    return (tvvars.GetTimeVarying());
}

int NetCDFCollection::GetXType(string varname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) { return (NC_FLOAT); }

    NetCDFSimple::Variable varinfo;
    int                    rc = NetCDFCollection::GetVariableInfo(varname, varinfo);
    if (rc < 0) return (-1);

    return (varinfo.GetXType());
}

std::vector<string> NetCDFCollection::GetAttNames(string varname) const
{
    std::vector<string> attnames;

    if (NetCDFCollection::IsDerivedVar(varname)) { return (attnames); }

    //
    // See if global attribute
    //
    if (varname.empty()) {
        if (_ncdfmap.empty()) { return (attnames); }
        NetCDFSimple *netcdf = _ncdfmap.begin()->second;
        return (netcdf->GetAttNames());
    }

    NetCDFSimple::Variable varinfo;
    bool                   ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) { return (attnames); }

    return (varinfo.GetAttNames());
}

int NetCDFCollection::GetAttType(string varname, string attname) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) { return (-1); }

    //
    // See if global attribute
    //
    if (varname.empty()) {
        if (_ncdfmap.empty()) { return (-1); }
        NetCDFSimple *netcdf = _ncdfmap.begin()->second;
        return (netcdf->GetAttType(attname));
    }

    NetCDFSimple::Variable varinfo;
    bool                   ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) { return (-1); }

    return (varinfo.GetAttType(attname));
}

void NetCDFCollection::GetAtt(string varname, string attname, std::vector<double> &values) const
{
    values.clear();

    if (NetCDFCollection::IsDerivedVar(varname)) return;

    //
    // See if global attribute
    //
    if (varname.empty()) {
        if (_ncdfmap.empty()) { return; }
        NetCDFSimple *netcdf = _ncdfmap.begin()->second;
        netcdf->GetAtt(attname, values);
        return;
    }

    NetCDFSimple::Variable varinfo;
    bool                   ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) { return; }
    varinfo.GetAtt(attname, values);
}

void NetCDFCollection::GetAtt(string varname, string attname, std::vector<long> &values) const
{
    values.clear();

    if (NetCDFCollection::IsDerivedVar(varname)) return;

    //
    // See if global attribute
    //
    if (varname.empty()) {
        if (_ncdfmap.empty()) { return; }
        NetCDFSimple *netcdf = _ncdfmap.begin()->second;
        netcdf->GetAtt(attname, values);
        return;
    }

    NetCDFSimple::Variable varinfo;
    bool                   ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) { return; }
    varinfo.GetAtt(attname, values);
}

void NetCDFCollection::GetAtt(string varname, string attname, string &values) const
{
    values.clear();

    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar;
        derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetAtt(attname, values));
    }

    //
    // See if global attribute
    //
    if (varname.empty()) {
        if (_ncdfmap.empty()) { return; }
        NetCDFSimple *netcdf = _ncdfmap.begin()->second;
        netcdf->GetAtt(attname, values);
        return;
    }

    NetCDFSimple::Variable varinfo;
    bool                   ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) { return; }
    varinfo.GetAtt(attname, values);
}

int NetCDFCollection::GetTime(size_t ts, double &time) const
{
    time = 0.0;
    if (ts >= _times.size()) {
        SetErrMsg("Invalid time step: %d", ts);
        return (-1);
    }
    time = _times[ts];
    return (0);
}

int NetCDFCollection::GetTimes(string varname, vector<double> &times) const
{
    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) {
        SetErrMsg("Invalid variable \"%s\"", varname.c_str());
        return (-1);
    }
    const TimeVaryingVar &tvvars = p->second;
    if (!tvvars.GetTimeVarying()) {
        times = _times;    // CV variables defined for all times
    } else {
        times = tvvars.GetTimes();
    }
    return (0);
}

int NetCDFCollection::GetFile(size_t ts, string varname, string &file, size_t &local_ts) const
{
    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) {
        SetErrMsg("Invalid variable \"%s\"", varname.c_str());
        return (-1);
    }
    const TimeVaryingVar &tvvars = p->second;

    double time;
    int    rc = GetTime(ts, time);
    if (rc < 0) return (-1);

    size_t var_ts;
    rc = tvvars.GetTimeStep(time, var_ts);
    if (rc < 0) return (-1);

    local_ts = tvvars.GetLocalTimeStep(var_ts);

    return (tvvars.GetFile(var_ts, file));
}

bool NetCDFCollection::_GetVariableInfo(string varname, NetCDFSimple::Variable &varinfo) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar;
        derivedVar = _derivedVarsMap.find(varname)->second;

        vector<string> dimnames = derivedVar->GetSpatialDimNames();
        if (derivedVar->TimeVarying()) { dimnames.insert(dimnames.begin(), derivedVar->GetTimeDimName()); }

        varinfo = NetCDFSimple::Variable(varname, dimnames, NC_FLOAT);
        vector<string> attnames = derivedVar->GetAttNames();
        for (int i = 0; i < attnames.size(); i++) {
            string s = attnames[i];
            if (derivedVar->GetAttType(s) == NC_CHAR) {
                string values;
                derivedVar->GetAtt(s, values);
                varinfo.SetAtt(s, values);
            } else if (derivedVar->GetAttType(s) == NC_INT64) {
                vector<long> values;
                derivedVar->GetAtt(s, values);
                varinfo.SetAtt(s, values);
            } else if (derivedVar->GetAttType(s) == NC_DOUBLE) {
                vector<double> values;
                derivedVar->GetAtt(s, values);
                varinfo.SetAtt(s, values);
            }
        }
        return (true);
    }

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) { return (false); }
    const TimeVaryingVar &tvvars = p->second;
    tvvars.GetVariableInfo(varinfo);

    vector<string> dimnames = tvvars.GetSpatialDimNames();
    if (!(tvvars.GetTimeDimName().empty())) { dimnames.insert(dimnames.begin(), tvvars.GetTimeDimName()); }
    varinfo.SetDimNames(dimnames);

    return (true);
}

int NetCDFCollection::GetVariableInfo(string varname, NetCDFSimple::Variable &varinfo) const
{
    bool ok = NetCDFCollection::_GetVariableInfo(varname, varinfo);
    if (!ok) {
        SetErrMsg("Invalid variable \"%s\"", varname.c_str());
        return (-1);
    }

    return (0);
}

bool NetCDFCollection::GetMissingValue(string varname, double &mv) const
{
    if (NetCDFCollection::IsDerivedVar(varname)) {
        NetCDFCollection::DerivedVar *derivedVar = _derivedVarsMap.find(varname)->second;
        return (derivedVar->GetMissingValue(mv));
    }

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) {
        SetErrMsg("Invalid variable \"%s\"", varname.c_str());
        return (false);
    }
    const TimeVaryingVar &tvvars = p->second;

    return (tvvars.GetMissingValue(_missingValAttName, mv));
}

int NetCDFCollection::OpenRead(size_t ts, string varname)
{
    //
    // Find a file descriptor. Use lowest available, starting with zero
    //
    int fd;
    for (fd = 0; fd < _ovr_table.size(); fd++) {
        if (_ovr_table.find(fd) == _ovr_table.end()) { break; }
    }
    fileHandle fh;

    // Ugh. Reserve the descriptor in _ovr_table before calling
    // fh._derived_var->Open(ts), which may in turn recursively call
    // NetCDFCollection::OpenRead()
    //
    _ovr_table[fd] = fh;

    if (NetCDFCollection::IsDerivedVar(varname)) {
        fh._derived_var = _derivedVarsMap[varname];
        fh._fd = fh._derived_var->Open(ts);
        if (fh._fd < 0) return (-1);
        _ovr_table[fd] = fh;
        return (fd);
    }

    map<string, TimeVaryingVar>::const_iterator p = _variableList.find(varname);
    if (p == _variableList.end()) {
        SetErrMsg("Invalid variable \"%s\"", varname.c_str());
        return (-1);
    }
    const TimeVaryingVar &tvvars = p->second;

    double time;
    int    rc = GetTime(ts, time);
    if (rc < 0) return (-1);

    size_t var_ts;
    rc = tvvars.GetTimeStep(time, var_ts);
    if (rc < 0) return (-1);

    fh._tvvars = tvvars;
    fh._local_ts = fh._tvvars.GetLocalTimeStep(var_ts);
    fh._slice = 0;
    fh._first_slice = true;

    string                 path;
    NetCDFSimple::Variable varinfo;
    fh._tvvars.GetFile(var_ts, path);
    fh._tvvars.GetVariableInfo(varinfo);

    fh._has_missing = fh._tvvars.GetMissingValue(_missingValAttName, fh._missing_value);

    fh._ncdfptr = _ncdfmap[path];
    fh._fd = fh._ncdfptr->OpenRead(varinfo);
    if (fh._fd < 0) {
        SetErrMsg("NetCDFCollection::OpenRead(%d, %s) : failed", var_ts, varname.c_str());
        return (-1);
    }

    _ovr_table[fd] = fh;
    return (fd);
}

int NetCDFCollection::ReadNative(size_t start[], size_t count[], float *data, int fd)
{
    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];

    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        vector<size_t> dims = fh._derived_var->GetSpatialDims();

        if (readSliceOK(dims, start, count)) {
            return (fh._derived_var->ReadSlice(data, fh._fd));
        } else if (readOK(dims, start, count)) {
            return (fh._derived_var->Read(data, fh._fd));
        } else {
            SetErrMsg("Not implemented");
            return (-1);
        }
    }

    int idx = 0;
    if (fh._tvvars.GetTimeVarying() && !(fh._tvvars.GetTimeDimName().empty() || fh._tvvars.GetTimeDimName() == derivedTimeDimName)) {
        mystart[idx] = fh._local_ts;
        mycount[idx] = 1;
        idx++;
    }
    for (int i = 0; i < fh._tvvars.GetSpatialDims().size(); i++) {
        mystart[idx] = start[i];
        mycount[idx] = count[i];
        idx++;
    }

    return (fh._ncdfptr->Read(mystart, mycount, data, fh._fd));
}

int NetCDFCollection::ReadNative(size_t start[], size_t count[], int *data, int fd)
{
    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];

    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    int idx = 0;
    if (fh._tvvars.GetTimeVarying() && !(fh._tvvars.GetTimeDimName().empty() || fh._tvvars.GetTimeDimName() == derivedTimeDimName)) {
        mystart[idx] = fh._local_ts;
        mycount[idx] = 1;
        idx++;
    }
    for (int i = 0; i < fh._tvvars.GetSpatialDims().size(); i++) {
        mystart[idx] = start[i];
        mycount[idx] = count[i];
        idx++;
    }

    return (fh._ncdfptr->Read(mystart, mycount, data, fh._fd));
}

int NetCDFCollection::ReadNative(size_t start[], size_t count[], char *data, int fd)
{
    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];

    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    int idx = 0;
    if (fh._tvvars.GetTimeVarying() && !(fh._tvvars.GetTimeDimName().empty() || fh._tvvars.GetTimeDimName() == derivedTimeDimName)) {
        mystart[idx] = fh._local_ts;
        mycount[idx] = 1;
        idx++;
    }
    for (int i = 0; i < fh._tvvars.GetSpatialDims().size(); i++) {
        mystart[idx] = start[i];
        mycount[idx] = count[i];
        idx++;
    }

    return (fh._ncdfptr->Read(mystart, mycount, data, fh._fd));
}

int NetCDFCollection::ReadNative(float *data, int fd)
{
    size_t start[NC_MAX_VAR_DIMS];
    size_t count[NC_MAX_VAR_DIMS];

    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    vector<size_t> dims = fh._tvvars.GetSpatialDims();
    for (int i = 0; i < dims.size(); i++) {
        start[i] = 0;
        count[i] = dims[i];
    }

    return (NetCDFCollection::ReadNative(start, count, data, fd));
}

int NetCDFCollection::ReadNative(int *data, int fd)
{
    size_t start[NC_MAX_VAR_DIMS];
    size_t count[NC_MAX_VAR_DIMS];

    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    vector<size_t> dims = fh._tvvars.GetSpatialDims();
    for (int i = 0; i < dims.size(); i++) {
        start[i] = 0;
        count[i] = dims[i];
    }

    return (NetCDFCollection::ReadNative(start, count, data, fd));
}

int NetCDFCollection::_InitializeTimesMap(const vector<string> &files, const vector<string> &time_dimnames, const vector<string> &time_coordvars, map<string, vector<double>> &timesMap,
                                          vector<double> &times, int &file_org) const
{
    timesMap.clear();
    if (time_coordvars.size() && (time_coordvars.size() != time_dimnames.size())) {
        SetErrMsg("NetCDFCollection::Initialize() : number of time coordinate variables and time dimensions must match when time coordinate variables specified");
        return (-1);
    }

    int rc;
    if (time_dimnames.size() == 0) {
        file_org = 1;
        rc = _InitializeTimesMapCase1(files, timesMap);
    } else if ((time_dimnames.size() != 0) && (time_coordvars.size() == 0)) {
        file_org = 2;
        rc = _InitializeTimesMapCase2(files, time_dimnames, timesMap);
    } else {
        file_org = 3;
        rc = _InitializeTimesMapCase3(files, time_dimnames, time_coordvars, timesMap);
    }
    if (rc < 0) return (rc);

    //
    // Generate times: a single vector of all the time coordinates
    //
    map<string, vector<double>>::const_iterator itr1;
    for (itr1 = timesMap.begin(); itr1 != timesMap.end(); ++itr1) {
        const vector<double> &timesref = itr1->second;
        times.insert(times.end(), timesref.begin(), timesref.end());
    }

    //
    // sort and remove duplicates
    //
    sort(times.begin(), times.end());
    vector<double>::iterator lastd;
    lastd = unique(times.begin(), times.end());
    times.erase(lastd, times.end());

    if (times.empty()) { times.push_back(0.0); }

    //
    // Create an entry for constant variables, which are defined at all times
    //
    timesMap["constant"] = times;
    return (0);
}

int NetCDFCollection::_InitializeTimesMapCase1(const vector<string> &files, map<string, vector<double>> &timesMap) const
{
    timesMap.clear();

    // If only on file and no time dimensions than there is no
    //
    if (files.size() < 2) return (0);

    map<string, double> currentTime;    // current time for each variable

    // Case 1: No TDs or TCVs => synthesize TCV
    // A variable's time coordinate is determined by the ordering
    // of the file that it occurs in. The timesMap key is the
    // concatentation of file name (where the variable occurs) and
    // variable name. The only type of variables present are ITVV
    //

    for (int i = 0; i < files.size(); i++) {
        NetCDFSimple *netcdf = new NetCDFSimple();

        int rc = netcdf->Initialize(files[i]);
        if (rc < 0) {
            SetErrMsg("NetCDFSimple::Initialize(%s)", files[i].c_str());
            return (-1);
        }

        const vector<NetCDFSimple::Variable> &variables = netcdf->GetVariables();

        for (int j = 0; j < variables.size(); j++) {
            //
            // Skip 0D variables
            //
            if (variables[j].GetDimNames().size() == 0) continue;

            string varname = variables[j].GetName();

            // If first time this variable has been seen
            // initialize the currentTime
            //
            if (currentTime.find(varname) == currentTime.end()) { currentTime[varname] = 0.0; }

            string         key = files[i] + varname;
            vector<double> times(1, currentTime[varname]);
            timesMap[key] = times;

            currentTime[varname] += 1.0;
        }
        delete netcdf;
    }
    return (0);
}

int NetCDFCollection::_InitializeTimesMapCase2(const vector<string> &files, const vector<string> &time_dimnames, map<string, vector<double>> &timesMap) const
{
    timesMap.clear();
    map<string, double> currentTime;    // current time for each variable

    // Case 2: TD specified, but no TCV.
    // A variable's time coordinate is determined by the ordering
    // of the file that it occurs in, offset by its time dimesion.
    // The timesMap key is the
    // concatentation of file name (where the variable occurs) and
    // variable name.
    // Both TVV and CV variables may be present.
    //

    for (int i = 0; i < files.size(); i++) {
        NetCDFSimple *netcdf = new NetCDFSimple();

        int rc = netcdf->Initialize(files[i]);
        if (rc < 0) {
            SetErrMsg("NetCDFSimple::Initialize(%s)", files[i].c_str());
            return (-1);
        }

        const vector<NetCDFSimple::Variable> &variables = netcdf->GetVariables();

        for (int j = 0; j < variables.size(); j++) {
            //
            // Skip 0D variables
            //
            if (variables[j].GetDimNames().size() == 0) continue;

            string varname = variables[j].GetName();
            string key = files[i] + varname;
            string timedim = variables[j].GetDimNames()[0];

            // If this is a CV variable (no time dimension) we skip it
            //
            if (find(time_dimnames.begin(), time_dimnames.end(), timedim) == time_dimnames.end()) continue;    // CV variable

            // Number of time steps for this variable
            //
            size_t timedimlen = netcdf->DimLen(timedim);

            // If first time this varname has been seen
            // initialize the currentTime
            //
            if (currentTime.find(varname) == currentTime.end()) { currentTime[varname] = 0.0; }

            vector<double> times;
            for (int t = 0; t < timedimlen; t++) {
                times.push_back(currentTime[varname]);
                currentTime[varname] += 1.0;
            }

            timesMap[key] = times;
        }
        delete netcdf;
    }
    return (0);
}

int NetCDFCollection::_InitializeTimesMapCase3(const vector<string> &files, const vector<string> &time_dimnames, const vector<string> &time_coordvars, map<string, vector<double>> &timesMap) const
{
    timesMap.clear();

    //
    // tcvcount counts occurrences of each TCV. tcvfile and tcvdim record
    // the last file name and TD pair used to generate hash key for each TCV
    //
    map<string, int>    tcvcount;    // # of files of TCV appears in
    map<string, string> tcvfile;
    map<string, string> tcvdim;
    for (int i = 0; i < time_coordvars.size(); i++) { tcvcount[time_coordvars[i]] = 0; }

    for (int i = 0; i < files.size(); i++) {
        NetCDFSimple *netcdf = new NetCDFSimple();

        int rc = netcdf->Initialize(files[i]);
        if (rc < 0) return (-1);

        const vector<NetCDFSimple::Variable> &variables = netcdf->GetVariables();

        //
        // For each TCV see if it exists in current file, if so
        // read it and add times to timesMap
        //
        for (int j = 0; j < time_coordvars.size(); j++) {
            int index = _get_var_index(variables, time_coordvars[j]);
            if (index < 0) continue;    // TCV doesn't exist

            // Increment count
            //

            tcvcount[time_coordvars[j]] += 1;

            // Read TCV
            float *buf = _Get1DVar(netcdf, variables[index]);
            if (!buf) {
                SetErrMsg("Failed to read time coordinate variable \"%s\"", time_coordvars[j].c_str());
                return (-1);
            }

            string timedim = variables[index].GetDimNames()[0];
            size_t timedimlen = netcdf->DimLen(timedim);

            vector<double> times;
            for (int t = 0; t < timedimlen; t++) { times.push_back(buf[t]); }
            delete[] buf;

            //
            // The hash key for timesMap is the file plus the
            // time dimension name
            //
            string key = files[i] + timedim;

            // record file and timedim used to generate the hash key
            //
            tcvfile[time_coordvars[j]] = files[i];
            tcvdim[time_coordvars[j]] = timedim;

            if (timesMap.find(key) == timesMap.end()) {
                timesMap[key] = times;
            } else {
                vector<double> &timesref = timesMap[key];
                for (int t = 0; t < times.size(); t++) { timesref.push_back(times[t]); }
            }
        }

        delete netcdf;
    }

    //
    // Finally, if see if this is case 3a (only one file contains the TCV),
    // or case 3b (a TCV is present in any file containing a TVV).
    // We're only checking for case 3a here. If case 1, replicate the
    // times for each file & time dimension pair
    //
    //

    for (int i = 0; i < time_coordvars.size(); i++) {
        if (tcvcount[time_coordvars[i]] == 1) {
            string                                      key1 = tcvfile[time_coordvars[i]] + tcvdim[time_coordvars[i]];
            map<string, vector<double>>::const_iterator itr;
            for (int j = 0; j < files.size(); j++) {
                string key = files[j] + tcvdim[time_coordvars[i]];
                timesMap[key] = timesMap[key1];
            }
        }
    }

    return (0);
}

void NetCDFCollection::_InterpolateLine(const float *src, size_t n, size_t stride, bool has_missing, float mv, float *dst) const
{
    if (!has_missing) {
        for (size_t i = 0; i < n - 1; i++) { dst[i * stride] = 0.5 * (src[i * stride] + src[(i + 1) * stride]); }
    } else {
        for (size_t i = 0; i < n - 1; i++) {
            if (src[i * stride] == mv || src[(i + 1) * stride] == mv) {
                dst[i * stride] = mv;
            } else {
                dst[i * stride] = 0.5 * (src[i * stride] + src[(i + 1) * stride]);
            }
        }
    }
}

void NetCDFCollection::_InterpolateSlice(size_t nx, size_t ny, bool xstag, bool ystag, bool has_missing, float mv, float *slice) const
{
    if (xstag) {
        for (int i = 0; i < ny; i++) {
            float *src = slice + (i * nx);
            float *dst = slice + (i * (nx - 1));
            _InterpolateLine(src, nx, 1, has_missing, mv, dst);
        }
        nx--;
    }

    if (ystag) {
        for (int i = 0; i < nx; i++) {
            float *src = slice + i;
            float *dst = slice + i;
            _InterpolateLine(src, ny, nx, has_missing, mv, dst);
        }
        ny--;
    }
}

float *NetCDFCollection::_Get1DVar(NetCDFSimple *netcdf, const NetCDFSimple::Variable &variable) const
{
    if (variable.GetDimNames().size() != 1) return (NULL);
    int fd = netcdf->OpenRead(variable);
    if (fd < 0) {
        SetErrMsg("Time coordinate variable \"%s\" is invalid", variable.GetName().c_str());
        return (NULL);
    }
    string dimname = variable.GetDimNames()[0];
    size_t dimlen = netcdf->DimLen(dimname);
    size_t start[] = {0};
    size_t count[] = {dimlen};
    float *buf = new float[dimlen];
    int    rc = netcdf->Read(start, count, buf, fd);
    if (rc < 0) { return (NULL); }
    netcdf->Close(fd);
    return (buf);
}

int NetCDFCollection::_get_var_index(const vector<NetCDFSimple::Variable> variables, string varname) const
{
    for (int i = 0; i < variables.size(); i++) {
        if (varname.compare(variables[i].GetName()) == 0) return (i);
    }
    return (-1);
}

int NetCDFCollection::ReadSliceNative(float *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    const TimeVaryingVar &var = fh._tvvars;
    vector<size_t>        dims = var.GetSpatialDims();

    size_t nx = dims[dims.size() - 1];
    size_t ny = dims[dims.size() - 2];
    size_t nz = dims.size() > 2 ? dims[dims.size() - 3] : 1;

    if (fh._slice >= nz) return (0);

    size_t start[] = {0, 0, 0};
    size_t count[] = {1, 1, 1};

    if (dims.size() > 2) {
        start[0] = fh._slice;
        count[1] = ny;
        count[2] = nx;
    } else {
        count[0] = ny;
        count[1] = nx;
    }

    int rc = NetCDFCollection::ReadNative(start, count, data, fd);
    fh._slice++;
    if (rc < 0) return (rc);
    return (1);
}

int NetCDFCollection::ReadSlice(float *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) { return (fh._derived_var->ReadSlice(data, fh._fd)); }

    const TimeVaryingVar &var = fh._tvvars;
    vector<size_t>        dims = var.GetSpatialDims();
    vector<string>        dimnames = var.GetSpatialDimNames();

    if (dims.size() < 2 || dims.size() > 3) {
        SetErrMsg("Only 2D and 3D variables supported");
        return (-1);
    }
    if (!IsStaggeredVar(var.GetName())) { return (NetCDFCollection::ReadSliceNative(data, fd)); }

    bool xstag = IsStaggeredDim(dimnames[dimnames.size() - 1]);
    bool ystag = IsStaggeredDim(dimnames[dimnames.size() - 2]);
    bool zstag = dims.size() == 3 ? IsStaggeredDim(dimnames[dimnames.size() - 3]) : false;

    size_t nx = dims[dims.size() - 1];
    size_t ny = dims[dims.size() - 2];

    size_t nxus = xstag ? nx - 1 : nx;
    size_t nyus = ystag ? ny - 1 : ny;

    if (fh._slicebufsz < (2 * nx * ny * sizeof(*data))) {
        if (fh._slicebuf) delete[] fh._slicebuf;
        fh._slicebuf = (unsigned char *)new float[2 * nx * ny];
        fh._slicebufsz = 2 * nx * ny * sizeof(*data);
    }

    float *buffer = (float *)fh._slicebuf;    // cast to float*
    float *slice1 = buffer;
    float *slice2 = NULL;

    int firstSlice = 0;    // index of first and second slice read
    int secondSlice = 1;
    if (fh._first_slice) {
        firstSlice = fh._slice;
        secondSlice = firstSlice + 1;
        fh._first_slice = false;
    }

    if (zstag) {
        //
        // If this is the first read we read the first slice into
        // the front (first half) of buffer, and the second into
        // the back (second half) of
        // buffer. For all other reads the previous slice is in
        // the front of buffer, so we read the current slice into
        // the back of buffer
        //
        if (fh._slice == firstSlice) {
            slice1 = buffer;
            slice2 = buffer + (nxus * nyus);
        } else {
            slice1 = buffer + (nxus * nyus);
            slice2 = buffer;
        }
    }

    int rc = NetCDFCollection::ReadSliceNative(slice1, fd);
    if (rc < 1) return (rc);    // eof or error

    _InterpolateSlice(nx, ny, xstag, ystag, fh._has_missing, fh._missing_value, slice1);

    if (zstag) {
        // N.B. ReadSliceNative increments fh._slice
        //
        if (fh._slice == secondSlice) {
            rc = NetCDFCollection::ReadSliceNative(slice2, fd);
            if (rc < 1)
                return (rc);    // eof or error
                                //			fh._slice--;

            _InterpolateSlice(nx, ny, xstag, ystag, fh._has_missing, fh._missing_value, slice2);
        }

        //
        // At this point the ith slice is in the front of buffer
        // and the ith+1 slice is in the back of buffer. Moreover,
        // any horizontal staggering has be taken care of. Hence, the
        // horizontal dimensions are given by nxus x nyus.
        //
        //
        for (int i = 0; i < nxus * nyus; i++) {
            float *src = buffer + i;
            float *dst = buffer + i;
            _InterpolateLine(src, 2, nxus * nyus, fh._has_missing, fh._missing_value, dst);
        }
    }

    memcpy(data, buffer, sizeof(*data) * nxus * nyus);

    if (zstag) {
        //
        // move the ith+1 slice to the front of the buffer
        //
        memcpy(buffer, buffer + (nxus * nyus), sizeof(*data) * nxus * nyus);
    }

    return (1);
}

int NetCDFCollection::SeekSlice(int offset, int whence, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) { return (fh._derived_var->SeekSlice(offset, whence, fh._fd)); }

    if (whence < 0 || whence > 2) {
        SetErrMsg("Invalid whence specification : %d", whence);
        return (-1);
    }

    vector<size_t> dims = fh._tvvars.GetSpatialDims();
    vector<string> dimnames = fh._tvvars.GetSpatialDimNames();

    bool   zstag = dims.size() == 3 ? IsStaggeredDim(dimnames[dimnames.size() - 3]) : false;
    size_t nz = dims.size() == 3 ? dims[dims.size() - 3] : 1;
    long   nzus = zstag ? nz - 1 : nz;

    int slice = 0;
    if (whence == 0) {
        slice = offset;
    } else if (whence == 1) {
        slice = fh._slice + offset;
    } else if (whence == 2) {
        slice = offset + nzus - 1;
    }
    if (slice < 0) slice = 0;
    if (slice > nzus - 1) slice = nzus - 1;

    fh._slice = slice;
    fh._first_slice = true;
    return (0);
}

int NetCDFCollection::Read(size_t start[], size_t count[], float *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    fileHandle &fh = itr->second;
    if (fh._derived_var) {
        vector<size_t> dims = fh._derived_var->GetSpatialDims();

        if (readSliceOK(dims, start, count)) {
            return (fh._derived_var->ReadSlice(data, fh._fd));
        } else if (readOK(dims, start, count)) {
            return (fh._derived_var->Read(data, fh._fd));
        } else {
            SetErrMsg("Not implemented");
            return (-1);
        }
    }

    const TimeVaryingVar &var = fh._tvvars;
    if (!IsStaggeredVar(var.GetName())) { return (NetCDFCollection::ReadNative(start, count, data, fd)); }

    SetErrMsg("Not implemented");
    return (-1);
}

int NetCDFCollection::Read(vector<size_t> start, vector<size_t> count, float *data, int fd)
{
    VAssert(start.size() == count.size());

    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];
    for (int i = 0; i < start.size(); i++) {
        mystart[i] = start[i];
        mycount[i] = count[i];
    }
    return (NetCDFCollection::Read(mystart, mycount, data, fd));
}

int NetCDFCollection::Read(size_t start[], size_t count[], int *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    const TimeVaryingVar &var = fh._tvvars;
    if (!IsStaggeredVar(var.GetName())) { return (NetCDFCollection::ReadNative(start, count, data, fd)); }

    SetErrMsg("Not implemented");
    return (-1);
}

int NetCDFCollection::Read(vector<size_t> start, vector<size_t> count, int *data, int fd)
{
    VAssert(start.size() == count.size());

    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];
    for (int i = 0; i < start.size(); i++) {
        mystart[i] = start[i];
        mycount[i] = count[i];
    }
    return (NetCDFCollection::Read(mystart, mycount, data, fd));
}

int NetCDFCollection::Read(float *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) { return (fh._derived_var->Read(data, fh._fd)); }

    const TimeVaryingVar &var = fh._tvvars;
    vector<size_t>        dims = var.GetSpatialDims();
    vector<string>        dimnames = var.GetSpatialDimNames();

    //
    // Handle different dimenion cases
    //
    if (dims.size() > 3) {
        SetErrMsg("Only 0D, 1D, 2D and 3D variables supported");
        return (-1);
    } else if (dims.size() == 0) {
        size_t start[] = {0};
        size_t count[] = {1};
        return (NetCDFCollection::ReadNative(start, count, data, fd));
    } else if (dims.size() == 1) {
        size_t nx = dims[dims.size() - 1];
        size_t start[] = {0};
        size_t count[] = {nx};
        bool   xstag = IsStaggeredDim(dimnames[dimnames.size() - 1]);

        if (xstag) {    // Deal with staggered data

            if (fh._linebufsz < (nx * sizeof(*data))) {
                if (fh._linebuf) delete[] fh._linebuf;
                fh._linebuf = (unsigned char *)new float[nx];
                fh._linebufsz = nx * sizeof(*data);
            }
            int rc = NetCDFCollection::ReadNative(start, count, (float *)fh._linebuf, fd);
            if (rc < 0) return (-1);
            _InterpolateLine((const float *)fh._linebuf, nx, 1, fh._has_missing, fh._missing_value, data);
            return (0);
        } else {
            return (NetCDFCollection::ReadNative(start, count, data, fd));
        }
    }

    bool xstag = IsStaggeredDim(dimnames[dimnames.size() - 1]);
    bool ystag = IsStaggeredDim(dimnames[dimnames.size() - 2]);
    bool zstag = dims.size() == 3 ? IsStaggeredDim(dimnames[dimnames.size() - 3]) : false;

    size_t nx = dims[dims.size() - 1];
    size_t ny = dims[dims.size() - 2];
    size_t nz = dims.size() == 3 ? dims[dims.size() - 3] : 1;

    size_t nxus = xstag ? nx - 1 : nx;
    size_t nyus = ystag ? ny - 1 : ny;
    size_t nzus = zstag ? nz - 1 : nz;

    float *buf = data;
    for (int i = 0; i < nzus; i++) {
        int rc = NetCDFCollection::ReadSlice(buf, fd);
        if (rc < 0) return (rc);
        buf += nxus * nyus;
    }
    return (0);
}

template<typename T> int NetCDFCollection::_read_template(T *data, int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    const TimeVaryingVar &var = fh._tvvars;
    vector<size_t>        dims = var.GetSpatialDims();
    vector<string>        dimnames = var.GetSpatialDimNames();

    bool xstag = IsStaggeredDim(dimnames[dimnames.size() - 1]);
    bool ystag = dims.size() > 1 ? IsStaggeredDim(dimnames[dimnames.size() - 2]) : false;
    bool zstag = dims.size() > 2 ? IsStaggeredDim(dimnames[dimnames.size() - 3]) : false;

    if (xstag || ystag || zstag) {
        SetErrMsg("Not implemented");
        return (-1);
    }

    //
    // Handle different dimenion cases
    //
    if (dims.size() > 3) {
        SetErrMsg("Only 0D, 1D, 2D and 3D variables supported");
        return (-1);
    }

    size_t start[3] = {0, 0, 0};
    size_t count[3];
    if (dims.size() == 0) {
        count[0] = 1;
        return (NetCDFCollection::ReadNative(start, count, data, fd));
    } else if (dims.size() == 1) {
        size_t nx = dims[dims.size() - 1];
        count[0] = nx;
    } else if (dims.size() == 2) {
        size_t nx = dims[dims.size() - 1];
        size_t ny = dims[dims.size() - 2];
        count[0] = ny;
        count[1] = nx;
    } else if (dims.size() == 3) {
        size_t nx = dims[dims.size() - 1];
        size_t ny = dims[dims.size() - 2];
        size_t nz = dims[dims.size() - 3];
        count[0] = nz;
        count[1] = ny;
        count[2] = nx;
    }

    return (NetCDFCollection::ReadNative(start, count, data, fd));
}

int NetCDFCollection::Read(char *data, int fd) { return (_read_template(data, fd)); }

int NetCDFCollection::Read(int *data, int fd) { return (_read_template(data, fd)); }

int NetCDFCollection::Close(int fd)
{
    std::map<int, fileHandle>::iterator itr;
    if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    fileHandle &fh = itr->second;

    if (fh._derived_var) {
        fh._derived_var->Close(fh._fd);
        fh._derived_var = NULL;
        _ovr_table.erase(itr);
        return (0);
    }

    if (!fh._ncdfptr) return (0);

    int rc = fh._ncdfptr->Close(fh._fd);
    if (fh._slicebuf) delete[] fh._slicebuf;
    if (fh._linebuf) delete[] fh._linebuf;

    _ovr_table.erase(itr);
    return (rc);
}

void NetCDFCollection::InstallDerivedVar(string varname, DerivedVar *derivedVar)
{
    vector<string> sdimnames = derivedVar->GetSpatialDimNames();
    vector<size_t> sdimlens = derivedVar->GetSpatialDims();
    VAssert(sdimlens.size() == sdimnames.size());

    // Add any new dimensions to the dimensions map. Should be
    // checking for re-definition of exisiting dimensions, but
    // being lazy.
    //
    for (int i = 0; i < sdimnames.size(); i++) {
        vector<string>::iterator itr;
        itr = find(_dimNames.begin(), _dimNames.end(), sdimnames[i]);
        if (itr == _dimNames.end()) {
            _dimNames.push_back(sdimnames[i]);
            _dimLens.push_back(sdimlens[i]);
        }
    }

    _derivedVarsMap[varname] = derivedVar;
};

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const NetCDFCollection &ncdfc)
{
    o << "NetCDFCollection" << endl;
    o << " _staggeredDims : ";
    for (int i = 0; i < ncdfc._staggeredDims.size(); i++) { o << ncdfc._staggeredDims[i] << " "; }
    o << endl;
    o << " _times : ";
    for (int i = 0; i < ncdfc._times.size(); i++) { o << ncdfc._times[i] << " "; }
    o << endl;
    o << " _missingValAttName : " << ncdfc._missingValAttName << endl;

    o << " _variableList : " << endl;
    map<string, NetCDFCollection::TimeVaryingVar>::const_iterator itr;
    for (itr = ncdfc._variableList.begin(); itr != ncdfc._variableList.end(); ++itr) {
        o << itr->second;
        o << endl;
    }

    return (o);
}
};    // namespace VAPoR

NetCDFCollection::TimeVaryingVar::TimeVaryingVar()
{
    _files.clear();
    _tvmaps.clear();
    _spatial_dims.clear();
    _spatial_dim_names.clear();
    _name.clear();
    _time_name.clear();
    _time_varying = false;
}

int NetCDFCollection::TimeVaryingVar::Insert(const NetCDFSimple *netcdf, const NetCDFSimple::Variable &variable, string file, const vector<string> &time_dimnames,
                                             const map<string, vector<double>> &timesmap, int file_org)
{
    bool first = (_tvmaps.size() == 0);    // first insertion?

    vector<string> space_dim_names = variable.GetDimNames();
    vector<size_t> space_dims;
    for (int i = 0; i < space_dim_names.size(); i++) { space_dims.push_back(netcdf->DimLen(space_dim_names[i])); }
    string time_name;

    //
    // Check if variable is time varying. I.e. if its slowest varying
    // dimension name matches a dimension name specified in time_dimnames
    //
    bool   time_varying = false;
    string key;    // hash key for timesmap

    if (variable.GetDimNames().size()) {
        string s = variable.GetDimNames()[0];

        // Handle ITVV case
        //
        if (time_dimnames.size() == 1 && time_dimnames[0] == derivedTimeDimName) {
            time_varying = true;
            time_name = derivedTimeDimName;
        } else if (find(time_dimnames.begin(), time_dimnames.end(), s) != time_dimnames.end()) {
            time_varying = true;
            time_name = s;
            space_dims.erase(space_dims.begin());
            space_dim_names.erase(space_dim_names.begin());
        }
    }
    if (!time_varying) {
        key = "constant";
    } else if (file_org == 1 || file_org == 2) {
        key = file + variable.GetName();
    } else {
        key = file + variable.GetDimNames()[0];
    }

    if (first) {
        _spatial_dims = space_dims;
        _spatial_dim_names = space_dim_names;
        _time_varying = time_varying;
        _name = variable.GetName();
        _time_name = time_name;
        _variable = variable;
    } else {
        //
        // If this isn't the first variable to be inserted the new variable
        // must match the existing ones
        //
        if (!((variable.GetDimNames() == _variable.GetDimNames()) && variable.GetXType() == _variable.GetXType())) {
            SetErrMsg("Multiple definitions of variable \"%s\"", variable.GetName().c_str());
            return (-1);
        }
    }
    _files.push_back(file);

    map<string, vector<double>>::const_iterator itr;
    itr = timesmap.find(key);
    if (itr == timesmap.end()) {
        SetErrMsg("Time coordinates not available for variable");
        return (-1);
    }
    const vector<double> &timesref = itr->second;

    size_t local_ts = 0;
    for (int i = 0; i < timesref.size(); i++) {
        tvmap_t tvmap;
        tvmap._fileidx = _files.size() - 1;
        tvmap._time = timesref[i];
        tvmap._local_ts = local_ts;
        _tvmaps.push_back(tvmap);
        local_ts++;
    }

    return (0);
}

int NetCDFCollection::TimeVaryingVar::GetTime(size_t ts, double &time) const
{
    if (ts >= _tvmaps.size()) return (-1);

    time = _tvmaps[ts]._time;

    return (0);
}

vector<double> NetCDFCollection::TimeVaryingVar::GetTimes() const
{
    vector<double> times;

    for (int i = 0; i < _tvmaps.size(); i++) times.push_back(_tvmaps[i]._time);

    return (times);
}

int NetCDFCollection::TimeVaryingVar::GetTimeStep(double time, size_t &ts) const
{
    if (!_time_varying) {
        ts = 0;
        return (0);
    }

    for (size_t i = 0; i < _tvmaps.size(); i++) {
        if (_tvmaps[i]._time == time) {
            ts = i;
            return (0);
        }
    }
    SetErrMsg("Invalid time %f", time);
    return (-1);
}

size_t NetCDFCollection::TimeVaryingVar::GetLocalTimeStep(size_t ts) const
{
    if (ts >= _tvmaps.size()) return (0);

    return (_tvmaps[ts]._local_ts);
}

int NetCDFCollection::TimeVaryingVar::GetFile(size_t ts, string &file) const
{
    if (ts >= _tvmaps.size()) return (-1);

    int fileidx = _tvmaps[ts]._fileidx;
    file = _files[fileidx];
    return (0);
}

bool NetCDFCollection::TimeVaryingVar::GetMissingValue(string attname, double &mv) const
{
    mv = 0.0;

    if (!attname.length()) return (false);

    vector<double> vec;
    _variable.GetAtt(attname, vec);
    if (!vec.size()) return (false);

    mv = vec[0];
    return (true);
}

void NetCDFCollection::TimeVaryingVar::Sort()
{
    //
    // Sort variable by time
    //

    auto lambda = [](const NetCDFCollection::TimeVaryingVar::tvmap_t &s1, const NetCDFCollection::TimeVaryingVar::tvmap_t &s2) -> bool { return (s1._time < s2._time); };

    std::sort(_tvmaps.begin(), _tvmaps.end(), lambda);
}

NetCDFCollection::fileHandle::fileHandle()
{
    _derived_var = NULL;
    _ncdfptr = NULL;
    _fd = -1;
    _local_ts = 0;
    _slice = 0;
    _slicebuf = NULL;
    _slicebufsz = 0;
    _linebuf = NULL;
    _linebufsz = 0;
    _has_missing = false;
    _missing_value = 0.0;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const NetCDFCollection::TimeVaryingVar &var)
{
    o << " TimeVaryingVar" << endl;
    o << " Variable : " << var._name << endl;
    o << "  Files : " << endl;
    for (int i = 0; i < var._files.size(); i++) { o << "   " << var._files[i] << endl; }
    o << "  Dims : ";
    for (int i = 0; i < var._spatial_dims.size(); i++) { o << var._spatial_dims[i] << " "; }
    o << endl;
    o << "  Dim Names : ";
    for (int i = 0; i < var._spatial_dim_names.size(); i++) { o << var._spatial_dim_names[i] << " "; }
    o << endl;

    o << "  Time Varying : " << var._time_varying << endl;

    o << "  Time Varying Map : " << endl;
    for (int i = 0; i < var._tvmaps.size(); i++) {
        o << "   _fileidx : " << var._tvmaps[i]._fileidx << endl;
        o << "   _time : " << var._tvmaps[i]._time << endl;
        o << "   _local_ts : " << var._tvmaps[i]._local_ts << endl;
        o << endl;
    }

    return (o);
}
};    // namespace VAPoR
