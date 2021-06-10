#include "vapor/VAssert.h"
#include <sstream>
#include <cfloat>
#include "vapor/VDC.h"

using namespace VAPoR;

namespace {

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

void _compute_bs(size_t ndims, const vector<size_t> &default_bs, vector<size_t> &bs)
{
    bs.clear();

    // If the default block size exists for a dimension use it.
    // Otherwise set the bs to 1
    //
    for (int i = 0; i < ndims; i++) {
        if (i < default_bs.size()) {
            bs.push_back(default_bs[i]);
        } else {
            bs.push_back(1);    // slowest varying block size can be one
        }
    }

    VAssert(ndims == bs.size());
}

void _compute_periodic(const vector<string> &dim_names, const vector<bool> &default_periodic, vector<bool> &periodic)
{
    // If the default periodicity exists for a dimension use it.
    // Otherwise set the periodicity to false. No periodicity for
    // time dimensions
    //
    for (int i = 0; i < dim_names.size(); i++) {
        if (i < default_periodic.size()) {
            periodic.push_back(default_periodic[i]);
        } else {
            periodic.push_back(false);
        }
    }
}

string make_mesh_name(const vector<string> &dim_names)
{
    string name;
    for (int i = 0; i < dim_names.size(); i++) {
        name += dim_names[i];
        if (i < (dim_names.size() - 1)) name += "X";
    }
    return (name);
}

};    // namespace

VDC::VDC()
{
    _master_path.clear();
    _mode = R;
    _defineMode = false;

    _bs.clear();
    for (int i = 0; i < 3; i++) _bs.push_back(64);

    _wname = "bior4.4";

    _cratios.clear();
    _cratios.push_back(500);
    _cratios.push_back(100);
    _cratios.push_back(10);
    _cratios.push_back(1);

    _periodic.clear();
    for (int i = 0; i < 3; i++) _periodic.push_back(false);

    _coordVars.clear();
    _dataVars.clear();
    _dimsMap.clear();
    _atts.clear();
    _newUniformVars.clear();
    _proj4StringOption.clear();
}

int VDC::initialize(const vector<string> &paths, const vector<string> &options, AccessMode mode, vector<size_t> bs)
{
    _proj4StringOption.clear();
    if (options.size() >= 2) {
        if (options[0] == "-proj4") { _proj4StringOption = options[1]; }
    }

    if (!paths.size()) {
        SetErrMsg("Invalid argument");
        return (-1);
    }
    _master_path = paths[0];
    if (mode == R) {
        _defineMode = false;
    } else {
        _defineMode = true;
    }
    _mode = mode;

    if (mode == W) {
        _bs = bs;
        while (_bs.size() > 3) _bs.pop_back();
        while (_bs.size() < 3) _bs.push_back(1);
    }

    int rc = _udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", _udunits.GetErrMsg().c_str());
        return (-1);
    }

    if (mode == R || mode == A) { return (_ReadMasterMeta()); }
    return (0);
}

int VDC::SetCompressionBlock(string wname, vector<size_t> cratios)
{
    if (!cratios.size()) cratios.push_back(1);

    sort(cratios.begin(), cratios.end());
    reverse(cratios.begin(), cratios.end());

    if (!_ValidCompressionBlock(_bs, wname, cratios)) {
        SetErrMsg("Invalid compression settings");
        return (-1);
    }

    _wname = wname;
    _cratios = cratios;

    return (0);
}

void VDC::GetCompressionBlock(vector<size_t> &bs, string &wname, vector<size_t> &cratios) const
{
    bs = _bs;
    wname = _wname;
    cratios = _cratios;
}

int VDC::DefineDimension(string name, size_t length)
{
    if (!_defineMode) {
        SetErrMsg("Not in define mode");
        return (-1);
    }

    // Can't redefine existing dimension names in append mode
    //
    if (_mode == A) {
        if (_dimsMap.find(name) != _dimsMap.end()) {
            SetErrMsg("Dimension name %s already defined", name.c_str());
            return (-1);
        }
    }

    if (!_ValidDefineDimension(name, length)) {
        SetErrMsg("Invalid definition for variable %s", name.c_str());
        return (-1);
    }

    Dimension dimension(name, length);

    //
    // _dimsMap contains all defined dimensions for the VDC
    //
    _dimsMap[name] = dimension;

    return (0);
}

int VDC::DefineDimension(string name, size_t length, int axis)
{
    VAssert(axis >= 0 && axis <= 3);

    int rc = DefineDimension(name, length);

    if (rc < 0) return (rc);

    // Now define a 1D coordinate variable
    //
    vector<string> sdimnames;
    string         time_dim_name;
    if (axis == 3) {
        time_dim_name = name;
    } else {
        sdimnames.push_back(name);
    }

    return (DefineCoordVarUniform(name, sdimnames, time_dim_name, "", axis, XType::FLOAT, false));
}

bool VDC::getDimension(string name, Dimension &dimension) const
{
    map<string, Dimension>::const_iterator itr = _dimsMap.find(name);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

vector<string> VDC::getDimensionNames() const
{
    vector<string>                              dim_names;
    std::map<string, Dimension>::const_iterator itr = _dimsMap.begin();
    for (; itr != _dimsMap.end(); ++itr) { dim_names.push_back(itr->first); }
    return (dim_names);
}

vector<string> VDC::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshes.begin();
    for (; itr != _meshes.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool VDC::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshes.find(mesh_name);
    if (itr == _meshes.end()) return (false);

    mesh = itr->second;
    return (true);
}

vector<string> VDC::_GetCoordVarDimNames(const CoordVar &cvar, bool &time_varying) const
{
    time_varying = false;

    vector<string> dim_names;

    dim_names = cvar.GetDimNames();

    if (!cvar.GetTimeDimName().empty()) {
        dim_names.push_back(cvar.GetTimeDimName());
        time_varying = true;
    }

    return (dim_names);
}

vector<string> VDC::_GetDataVarDimNames(const DataVar &dvar, bool &time_varying) const
{
    time_varying = false;

    vector<string> dim_names;

    string mesh = dvar.GetMeshName();

    map<string, Mesh>::const_iterator itr = _meshes.find(mesh);
    VAssert(itr != _meshes.end());

    dim_names = itr->second.GetDimNames();

    // Get time dimension if exists
    //
    string time_coord_var = dvar.GetTimeCoordVar();
    if (time_coord_var.empty()) return (dim_names);

    std::map<string, DC::CoordVar>::const_iterator itr1;
    itr1 = _coordVars.find(time_coord_var);
    VAssert(itr1 != _coordVars.end());

    vector<string> dimvec = _GetCoordVarDimNames(itr1->second, time_varying);
    VAssert(dimvec.size() == 1);

    dim_names.push_back(dimvec[0]);

    return (dim_names);
}

int VDC::_DefineImplicitCoordVars(vector<string> dim_names, vector<string> coord_vars_in, vector<string> &coord_vars_out)
{
    coord_vars_out.clear();

    if (dim_names.size() <= coord_vars_in.size()) {
        // No need to do anything
        //
        coord_vars_out = coord_vars_in;
        return (0);
    }

    for (int i = 0; i < dim_names.size(); i++) coord_vars_out.push_back("");

    // figure out which coordinate variables are missing
    //
    for (int i = 0; i < coord_vars_in.size(); i++) {
        string name = coord_vars_in[i];

        map<string, CoordVar>::const_iterator itr = _coordVars.find(name);
        if (itr == _coordVars.end()) {
            SetErrMsg("Undefined coordinate variable \"%s\" ", name.c_str());
            return (-1);
        }

        int axis = itr->second.GetAxis();
        VAssert(axis >= 0 && axis <= 3);

        coord_vars_out[axis] = name;
    }

    // For every coordinate variable that is missing make up a new one
    //
    vector<string> new_coord_vars;
    vector<int>    new_coord_axes;
    for (int i = 0; i < coord_vars_out.size(); i++) {
        if (coord_vars_out[i].empty()) {
            coord_vars_out[i] = dim_names[i];

            // Keep track of the new coordinate variables that we will
            // create.
            //
            new_coord_vars.push_back(coord_vars_out[i]);
            new_coord_axes.push_back(i);
        }
    }

    // Create new, unitless coordinate variables if they don't
    // already exist
    //
    for (int i = 0; i < new_coord_vars.size(); i++) {
        string name = new_coord_vars[i];
        int    axis = new_coord_axes[i];

        vector<string> dim_names;
        string         time_dim_name;
        if (axis == 3) {
            time_dim_name = name;
        } else {
            dim_names.push_back(name);
        }

        map<string, CoordVar>::const_iterator itr = _coordVars.find(name);
        if (itr != _coordVars.end()) continue;

        vector<bool> periodic(1, false);
        _coordVars[name] = CoordVar(name, "", XType::FLOAT, "", vector<size_t>(), periodic, dim_names, time_dim_name, axis, false);

        _coordVars[name].SetUniform(true);

        // Keep track of any uniform variables that get defined
        //
        _newUniformVars.push_back(name);
    }
    return (0);
}

int VDC::DefineCoordVar(string varname, vector<string> dim_names, string time_dim_name, string units, int axis, XType type, bool compressed)
{
    if (!_defineMode) {
        SetErrMsg("Not in define mode");
        return (-1);
    }

    if (_mode == A) {
        if (_coordVars.find(varname) != _coordVars.end()) {
            SetErrMsg("Variable \"%s\" already defined", varname.c_str());
            return (-1);
        }
    }

    if (axis == 3 && units.empty()) units = "seconds";

    if (!_ValidDefineCoordVar(varname, dim_names, time_dim_name, units, axis, type, compressed)) { return (-1); }

    vector<bool> periodic;
    _compute_periodic(dim_names, _periodic, periodic);

    vector<size_t> cratios(1, 1);
    string         wname;
    if (compressed) {
        cratios = _cratios;
        wname = _wname;
    }

    // _coordVars contains a table of all the coordinate variables
    //
    _coordVars[varname] = CoordVar(varname, units, type, wname, cratios, periodic, dim_names, time_dim_name, axis, false);

    return (0);
}

int VDC::DefineCoordVarUniform(string varname, vector<string> dim_names, string time_dim_name, string units, int axis, XType type, bool compressed)
{
    int rc = VDC::DefineCoordVar(varname, dim_names, time_dim_name, units, axis, type, compressed);
    if (rc < 0) return (-1);

    VAssert(_coordVars.find(varname) != _coordVars.end());

    _coordVars[varname].SetUniform(true);

    // Keep track of any uniform variables that get defined
    //
    _newUniformVars.push_back(varname);

    return (0);
}

bool VDC::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, CoordVar>::const_iterator itr = _coordVars.find(varname);
    if (itr == _coordVars.end()) return (false);

    cvar = itr->second;
    return (true);
}

bool VDC::getBaseVarInfo(string varname, DC::BaseVar &var) const
{
    map<string, CoordVar>::const_iterator itr1 = _coordVars.find(varname);
    if (itr1 != _coordVars.end()) {
        var = itr1->second;
        return (true);
    }

    map<string, DataVar>::const_iterator itr2 = _dataVars.find(varname);
    if (itr2 != _dataVars.end()) {
        var = itr2->second;
        return (true);
    }
    return (false);
}

void VDC::_DefineMesh(string meshname, vector<string> dim_names, vector<string> coord_vars)
{
    VAssert(dim_names.size() == coord_vars.size());

    _meshes[meshname] = DC::Mesh(meshname, dim_names, coord_vars);
}

int VDC::_DefineDataVar(string varname, vector<string> dim_names, vector<string> coord_vars, string units, XType type, bool compressed, double mv, string maskvar)
{
    if (!_defineMode) {
        SetErrMsg("Not in define mode");
        return (-1);
    }

    if (_mode == A) {
        if (_dataVars.find(varname) != _dataVars.end()) {
            SetErrMsg("Variable \"%s\" already defined", varname.c_str());
            return (-1);
        }
    }

    int rc = _DefineImplicitCoordVars(dim_names, coord_vars, coord_vars);
    if (rc < 0) return (-1);

    if (!_ValidDefineDataVar(varname, dim_names, coord_vars, units, type, compressed, maskvar)) { return (-1); }

    //
    // Dimensions must have previosly been defined
    //
    vector<Dimension> dimensions;
    for (int i = 0; i < dim_names.size(); i++) {
        Dimension dimension;
        VDC::getDimension(dim_names[i], dimension);
        VAssert(!dimension.GetName().empty());
        dimensions.push_back(dimension);
    }

    // Check for a time coordinate
    //
    string time_coord_var;
    if (coord_vars.size() > 0) {
        map<string, CoordVar>::const_iterator itr;
        itr = _coordVars.find(coord_vars[coord_vars.size() - 1]);
        if (itr != _coordVars.end() && itr->second.GetAxis() == 3) {
            time_coord_var = coord_vars[coord_vars.size() - 1];
            dim_names.pop_back();
            coord_vars.pop_back();
        }
    }

    vector<bool> periodic;
    _compute_periodic(dim_names, _periodic, periodic);

    vector<size_t> cratios(1, 1);
    string         wname;
    if (compressed) {
        cratios = _cratios;
        wname = _wname;
    }

    string meshname = make_mesh_name(dim_names);
    _DefineMesh(meshname, dim_names, coord_vars);

    if (!maskvar.empty()) {
        _dataVars[varname] = DataVar(varname, units, type, wname, cratios, periodic, meshname, time_coord_var, DC::Mesh::NODE, mv, maskvar);
    } else {
        _dataVars[varname] = DataVar(varname, units, type, wname, cratios, periodic, meshname, time_coord_var, DC::Mesh::NODE);
    }

    return (0);
}

int VDC::DefineDataVar(string varname, vector<string> dim_names, vector<string> coord_vars, string units, XType type, bool compressed)
{
    return (VDC::_DefineDataVar(varname, dim_names, coord_vars, units, type, compressed, 0.0, ""));
}

int VDC::DefineDataVar(string varname, vector<string> dim_names, vector<string> coord_vars, string units, XType type, double mv, string maskvar

)
{
    return (VDC::_DefineDataVar(varname, dim_names, coord_vars, units, type, true, mv, maskvar));
}

bool VDC::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DataVar>::const_iterator itr = _dataVars.find(varname);

    if (itr == _dataVars.end()) return (false);

    datavar = itr->second;
    return (true);
}

vector<string> VDC::getDataVarNames() const
{
    vector<string> names;

    map<string, DataVar>::const_iterator itr;
    for (itr = _dataVars.begin(); itr != _dataVars.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

vector<string> VDC::getCoordVarNames() const
{
    vector<string> names;

    map<string, CoordVar>::const_iterator itr;
    for (itr = _coordVars.begin(); itr != _coordVars.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

size_t VDC::getNumRefLevels(string varname) const
{
    string wname;

    if (_coordVars.find(varname) != _coordVars.end()) {
        std::map<string, DC::CoordVar>::const_iterator itr = _coordVars.find(varname);
        ;
        if (!itr->second.IsCompressed()) return (1);
        wname = itr->second.GetWName();
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        std::map<string, DC::DataVar>::const_iterator itr = _dataVars.find(varname);
        if (!itr->second.IsCompressed()) return (1);
        ;
        wname = itr->second.GetWName();
    } else {
        // Var doesn't exist. Still return 1
        return (1);
    }

    vector<DC::Dimension> mdimensions;
    bool                  ok = GetVarDimensions(varname, true, mdimensions, -1);
    if (!ok) return (1);

    // Determine block size
    //
    vector<size_t> bs;
    _compute_bs(mdimensions.size(), _bs, bs);

    size_t nlevels, maxcratio;
    CompressionInfo(bs, wname, nlevels, maxcratio);

    return (nlevels);
}

int VDC::PutAtt(string varname, string attname, XType type, const vector<double> &values)
{
    Attribute attr(attname, type, values);

    if (varname.empty()) {
        _atts[attname] = attr;
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::iterator itr = _dataVars.find(varname);
        map<string, Attribute>         atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::iterator itr = _coordVars.find(varname);
        map<string, Attribute>          atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }
    return (0);
}

int VDC::PutAtt(string varname, string attname, XType type, const vector<long> &values)
{
    Attribute attr(attname, type, values);

    if (varname.empty()) {
        _atts[attname] = attr;
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::iterator itr = _dataVars.find(varname);
        map<string, Attribute>         atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::iterator itr = _coordVars.find(varname);
        map<string, Attribute>          atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }
    return (0);
}

int VDC::PutAtt(string varname, string attname, XType type, const string &values)
{
    Attribute attr(attname, type, values);

    if (varname.empty()) {
        _atts[attname] = attr;
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::iterator itr = _dataVars.find(varname);
        map<string, Attribute>         atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::iterator itr = _coordVars.find(varname);
        map<string, Attribute>          atts = itr->second.GetAttributes();
        atts[attname] = attr;
        itr->second.SetAttributes(atts);
    } else {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }
    return (0);
}

bool VDC::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    if (varname.empty() && (_atts.find(attname) != _atts.end())) {
        map<string, Attribute>::const_iterator itr = _atts.find(attname);
        VAssert(itr != _atts.end());
        const Attribute &attr = itr->second;
        attr.GetValues(values);
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::const_iterator   itr = _dataVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);

    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::const_iterator  itr = _coordVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);
    } else {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (false);
    }
    return (true);
}

bool VDC::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    if (varname.empty() && (_atts.find(attname) != _atts.end())) {
        map<string, Attribute>::const_iterator itr = _atts.find(attname);
        VAssert(itr != _atts.end());
        const Attribute &attr = itr->second;
        attr.GetValues(values);
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::const_iterator   itr = _dataVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);

    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::const_iterator  itr = _coordVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);
    } else {
        return (false);
    }
    return (true);
}

bool VDC::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    if (varname.empty() && (_atts.find(attname) != _atts.end())) {
        map<string, Attribute>::const_iterator itr = _atts.find(attname);
        VAssert(itr != _atts.end());
        const Attribute &attr = itr->second;
        attr.GetValues(values);
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::const_iterator   itr = _dataVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);

    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::const_iterator  itr = _coordVars.find(varname);
        map<string, Attribute>                 atts = itr->second.GetAttributes();
        map<string, Attribute>::const_iterator itr1 = atts.find(attname);
        if (itr1 == atts.end()) { return (false); }
        const Attribute &attr = itr1->second;
        attr.GetValues(values);
    } else {
        return (false);
    }
    return (true);
}

int VDC::CopyAtt(const DC &src, string varname, string attname)
{
    XType type = src.GetAttType(varname, attname);
    if (type == XType::INT32 || type == XType::INT64) {
        vector<long> values;
        if (!src.GetAtt(varname, attname, values)) return -1;
        return PutAtt(varname, attname, type, values);
    } else if (type == XType::FLOAT || type == XType::DOUBLE) {
        vector<double> values;
        if (!src.GetAtt(varname, attname, values)) return -1;
        return PutAtt(varname, attname, type, values);
    } else if (type == XType::TEXT) {
        string values;
        if (!src.GetAtt(varname, attname, values)) return -1;
        return PutAtt(varname, attname, type, values);
    } else if (type == XType::INVALID) {
        SetErrMsg("Invalid attribute : %s.%s", varname.c_str(), attname.c_str());
        return -1;
    }
    return 0;
}

int VDC::CopyAtt(const DC &src, string varname)
{
    vector<string> names = src.GetAttNames(varname);

    for (int i = 0; i < names.size(); i++) {
        int rc = CopyAtt(src, varname, names[i]);
        if (rc < 0) return (-1);
    }
    return (0);
}

vector<string> VDC::getAttNames(string varname) const
{
    vector<string> attnames;

    std::map<string, Attribute>::const_iterator itr;

    if (varname.empty()) {    // global attributes
        for (itr = _atts.begin(); itr != _atts.end(); ++itr) { attnames.push_back(itr->first); }
    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::const_iterator itr1 = _dataVars.find(varname);
        map<string, Attribute>               atts = itr1->second.GetAttributes();
        for (itr = atts.begin(); itr != atts.end(); ++itr) { attnames.push_back(itr->first); }
    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::const_iterator itr1 = _coordVars.find(varname);
        map<string, Attribute>                atts = itr1->second.GetAttributes();
        for (itr = atts.begin(); itr != atts.end(); ++itr) { attnames.push_back(itr->first); }
    }
    return (attnames);
}

DC::XType VDC::getAttType(string varname, string attname) const
{
    std::map<string, Attribute>::const_iterator itr;

    if (varname.empty()) {    // global attributes
        itr = _atts.find(attname);
        if (itr == _atts.end()) return (INVALID);

        return (itr->second.GetXType());

    } else if (_dataVars.find(varname) != _dataVars.end()) {
        map<string, DataVar>::const_iterator itr1 = _dataVars.find(varname);
        map<string, Attribute>               atts = itr1->second.GetAttributes();
        itr = atts.find(attname);
        if (itr == _atts.end()) return (INVALID);

        return (itr->second.GetXType());

    } else if (_coordVars.find(varname) != _coordVars.end()) {
        map<string, CoordVar>::const_iterator itr1 = _coordVars.find(varname);
        map<string, Attribute>                atts = itr1->second.GetAttributes();
        itr = atts.find(attname);
        if (itr == _atts.end()) return (INVALID);

        return (itr->second.GetXType());
    }
    return (INVALID);
}

string VDC::getMapProjection(string varname) const
{
    // Shoot. This doens't do anything. I.e. it doesn't force data to
    // be reprojected
    //
    if (!_proj4StringOption.empty()) { return (_proj4StringOption); }

    string attname = "MapProjection";

    vector<string> attnames = VDC::getAttNames(varname);
    if (find(attnames.begin(), attnames.end(), attname) == attnames.end()) {
        // Return default map projection.
        //
        // N.B. WE SHOULD BE VERIFYING THAT THIS IS A GEOREFERENCED
        // VARIABLE!!!
        //
        return (VDC::getMapProjection());
    }

    string proj4string;
    bool   status = VDC::getAtt(varname, attname, proj4string);
    if (status) return (proj4string);

    return ("");
}

string VDC::getMapProjection() const
{
    if (!_proj4StringOption.empty()) { return (_proj4StringOption); }

    string attname = "MapProjection";

    vector<string> attnames = VDC::getAttNames("");
    if (find(attnames.begin(), attnames.end(), attname) == attnames.end()) { return (""); }

    string proj4string;
    bool   status = VDC::getAtt("", attname, proj4string);
    if (status) return (proj4string);

    return ("");
}

int VDC::SetMapProjection(string varname, string projstring)
{
    string attname = "MapProjection";

    return (VDC::PutAtt(varname, attname, TEXT, projstring));
}

int VDC::SetMapProjection(string projstring)
{
    string attname = "MapProjection";

    return (VDC::PutAtt("", attname, TEXT, projstring));
}

int VDC::EndDefine()
{
    if (!_defineMode) return (0);

    if (_mode == R) return (0);

    if (_master_path.empty()) {
        // Initialized() not called
        //
        SetErrMsg("Master file not specified");
        return (-1);
    }

    if (_WriteMasterMeta() < 0) return (-1);

    _defineMode = false;

    // For any 1D Uniform coordinate variables that
    // were defined go ahead
    // and give them default values
    //
    for (int i = 0; i < _newUniformVars.size(); i++) {
        vector<DC::Dimension> dimensions;
        bool                  ok = GetVarDimensions(_newUniformVars[i], false, dimensions, -1);
        VAssert(ok);

        if (dimensions.size() != 1) continue;

        size_t l = dimensions[0].GetLength();

        float *buf = new float[l];
        for (size_t j = 0; j < l; j++) buf[j] = (float)j;

        int rc = PutVar(_newUniformVars[i], -1, buf);
        if (rc < 0) {
            delete[] buf;
            return (rc);
        }
        delete[] buf;
    }

    return (0);
}

bool VDC::_ValidDefineDimension(string name, size_t length) const
{
    if (length < 1) {
        SetErrMsg("Dimension must be of length 1 or more");
        return (false);
    }

    return (true);
}

bool VDC::_ValidDefineCoordVar(string varname, vector<string> sdim_names, string time_dim_name, string units, int axis, XType type, bool compressed) const
{
    if (sdim_names.size() > 3) {
        SetErrMsg("Invalid number of dimensions");
        return (false);
    }

    if (axis == 3 && (sdim_names.size() != 0 || time_dim_name.empty())) {
        SetErrMsg("Time coordinate variables must have exactly one dimension");
        return (false);
    }

    // All space + time dim names
    //
    vector<string> dim_names = sdim_names;
    if (!time_dim_name.empty()) dim_names.push_back(time_dim_name);

    for (int i = 0; i < dim_names.size(); i++) {
        if (_dimsMap.find(dim_names[i]) == _dimsMap.end()) {
            SetErrMsg("Dimension \"%s\" not defined", dim_names[i].c_str());
            return (false);
        }
    }

    if (axis < 0 || axis > 3) {
        SetErrMsg("Invalid axis specification : %d", axis);
        return (false);
    }

    if (!units.empty() && !_udunits.ValidUnit(units)) {
        SetErrMsg("Unrecognized units specification : %s ", units.c_str());
        return (false);
    }

#ifdef VAPOR3_0_0_ALPHA
    if (compressed && type != FLOAT) {
        SetErrMsg("Only FLOAT data supported with compressed variables");
        return (false);
    }
#endif

    //
    // IF this is a dimension coordinate variable (a variable with
    // the same name as any dimension) then the dimensions,
    // and compression cannot be changed.
    //
    map<string, Dimension>::const_iterator itr = _dimsMap.find(varname);
    if (itr != _dimsMap.end()) {
        if (dim_names.size() != 1 || varname.compare(dim_names[0]) != 0) {
            SetErrMsg("Invalid dimension coordinate variable definition");
            return (false);
        }
        if (compressed) {
            SetErrMsg("Invalid dimension coordinate variable definition");
            return (false);
        }
    }

    return (true);
}

bool VDC::_ValidCompressionBlock(vector<size_t> bs, string wname, vector<size_t> cratios) const
{
    for (int i = 0; i < cratios.size(); i++) {
        if (cratios[i] < 1) return (false);
    }

    // No compression if no blocking
    //
    if (vproduct(bs) == 1) {
        if (!wname.empty()) return (false);
        if (!(cratios.size() == 1 && cratios[0] == 1)) return (false);
    }

    size_t nlevels, maxcratio;
    bool   status = CompressionInfo(bs, wname, nlevels, maxcratio);
    if (!status) return (false);

    if (!wname.empty()) {
        for (int i = 0; i < cratios.size(); i++) {
            if (cratios[i] > maxcratio) return (false);
        }
    }
    return (true);
}

// Verify that dims0 is a subset of dims1, and that they have same block
// size
//
bool VDC::_valid_dims(const vector<DC::Dimension> &dims0, const vector<size_t> &bs0, const vector<DC::Dimension> &dims1, const vector<size_t> &bs1) const
{
    VAssert(dims0.size() == bs0.size());
    VAssert(dims1.size() == bs1.size());

    for (int i = 0; i < dims0.size(); i++) {
        bool match = false;
        for (int j = 0; j < dims1.size(); j++) {
            // Different names ok as long has have same length
            //
            // if (dims0[i].GetName() == dims1[j].GetName())

            if (dims0[i].GetLength() == dims1[j].GetLength()) {
                match = true;

                // Must have same blocking or no blocking
                //
                if (!(bs0[i] == bs1[j] || bs0[i] == 1 || bs1[j] == 1)) return (false);
            }
        }
        if (!match) return (false);
    }
    return (true);
}

bool VDC::_valid_mask_var(string varname, vector<DC::Dimension> dimensions, vector<size_t> bs, bool compressed, string maskvar) const
{
    map<string, DataVar>::const_iterator itr = _dataVars.find(maskvar);
    if (itr == _dataVars.end()) {
        SetErrMsg("Mask var \"%s\" not defined", maskvar.c_str());
        return (false);
    }
    const DataVar &mvar = itr->second;

    map<string, Mesh>::const_iterator mitr = _meshes.find(mvar.GetMeshName());
    if (mitr == _meshes.end()) return (false);

    // Fastest varying data variable dimensions must match mask
    // variable dimensions
    //
    vector<DC::Dimension> mdimensions;
    bool                  ok = GetVarDimensions(maskvar, false, mdimensions, -1);
    VAssert(ok);

    while (dimensions.size() > mdimensions.size()) dimensions.pop_back();
    while (bs.size() > mdimensions.size()) bs.pop_back();

    if (dimensions.size() != mdimensions.size()) {
        SetErrMsg("Data variable and mask variable dimensions must match");
        return (false);
    }

    if (compressed) {
        size_t nlevels, dummy;
        CompressionInfo(bs, _wname, nlevels, dummy);

        size_t nlevels_m;
        CompressionInfo(bs, mvar.GetWName(), nlevels_m, dummy);

        if (nlevels > nlevels_m) {
            SetErrMsg("Data variable and mask variable depth must match");
            return (false);
        }
    }

    return (true);
}

bool VDC::_ValidDefineDataVar(string varname, vector<string> dim_names, vector<string> coord_vars, string units, XType type, bool compressed, string maskvar) const
{
    if (dim_names.size() > 4) {
        SetErrMsg("Invalid number of dimensions");
        return (false);
    }

    if (dim_names.size() != coord_vars.size()) {
        SetErrMsg("Number of dimensions and coordinate vars don't match");
        return (false);
    }

    vector<DC::Dimension> dimensions;
    for (int i = 0; i < dim_names.size(); i++) {
        DC::Dimension dim;

        bool ok = VDC::GetDimension(dim_names[i], dim, -1);
        if (!ok) {
            SetErrMsg("Dimension \"%s\" not defined", dim_names[i].c_str());
            return (false);
        }
        dimensions.push_back(dim);
    }

    for (int i = 0; i < coord_vars.size(); i++) {
        if (_coordVars.find(coord_vars[i]) == _coordVars.end()) {
            SetErrMsg("Coordinate var \"%s\" not defined", coord_vars[i].c_str());
            return (false);
        }
    }

    if (!units.empty() && !_udunits.ValidUnit(units)) {
        SetErrMsg("Unrecognized units specification : %s", units.c_str());
        return (false);
    }

#ifdef VAPOR3_0_0_ALPHA
    if (compressed && type != FLOAT) {
        SetErrMsg("Only FLOAT data supported with compressed variables");
        return (false);
    }
#endif

    //
    // If multidimensional the dimensions and coord names must be
    // ordered X, Y, Z, T
    //

    if (coord_vars.size() > 1) {
        map<string, CoordVar>::const_iterator itr;
        int                                   axis = -1;
        for (int i = 0; i < coord_vars.size(); i++) {
            itr = _coordVars.find(coord_vars[i]);
            VAssert(itr != _coordVars.end());    // already checked for existance
            if (itr->second.GetAxis() <= axis) {
                SetErrMsg("Dimensions must be ordered X, Y, Z, T");
                return (false);
            }
            axis = itr->second.GetAxis();
        }
    }

    // Check for a time coordinate
    //
    if (coord_vars.size() > 0) {
        map<string, CoordVar>::const_iterator itr;
        itr = _coordVars.find(coord_vars[coord_vars.size() - 1]);
        if (itr != _coordVars.end() && itr->second.GetAxis() == 3) {
            dim_names.pop_back();
            dimensions.pop_back();
            coord_vars.pop_back();
        }
    }

    // Determine block size
    //
    vector<size_t> bs;
    _compute_bs(dim_names.size(), _bs, bs);

    // Validate mask variable if one exists
    //
    if (maskvar.empty()) return (true);

    return (_valid_mask_var(varname, dimensions, bs, compressed, maskvar));
}

namespace VAPoR {

std::ostream &operator<<(std::ostream &o, const VDC &vdc)
{
    o << "VDC" << endl;
    o << " MasterPath: " << vdc._master_path << endl;
    o << " AccessMode: " << vdc._mode << endl;
    o << " DefineMode: " << vdc._defineMode << endl;
    o << " Block Size: ";
    for (int i = 0; i < vdc._bs.size(); i++) { o << vdc._bs[i] << " "; }
    o << endl;
    o << " WName: " << vdc._wname << endl;
    o << " CRatios: ";
    for (int i = 0; i < vdc._cratios.size(); i++) { o << vdc._cratios[i] << " "; }
    o << endl;
    o << " Periodic: ";
    for (int i = 0; i < vdc._periodic.size(); i++) { o << vdc._periodic[i] << " "; }
    o << endl;

    {
        o << " Attributes" << endl;
        std::map<string, DC::Attribute>::const_iterator itr;
        for (itr = vdc._atts.begin(); itr != vdc._atts.end(); ++itr) { o << itr->second; }
        o << endl;
    }

    {
        o << " Dimensions" << endl;
        std::map<string, DC::Dimension>::const_iterator itr;
        for (itr = vdc._dimsMap.begin(); itr != vdc._dimsMap.end(); ++itr) { o << itr->second; }
        o << endl;
    }

    {
        o << " CoordVars" << endl;
        std::map<string, DC::CoordVar>::const_iterator itr;
        for (itr = vdc._coordVars.begin(); itr != vdc._coordVars.end(); ++itr) {
            o << itr->second;
            o << endl;
        }
        o << endl;
    }

    {
        o << " DataVars" << endl;
        std::map<string, DC::DataVar>::const_iterator itr;
        for (itr = vdc._dataVars.begin(); itr != vdc._dataVars.end(); ++itr) {
            o << itr->second;
            o << endl;
        }
        o << endl;
    }

    return (o);
}
};    // namespace VAPoR
