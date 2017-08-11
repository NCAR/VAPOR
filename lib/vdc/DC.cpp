#include <cassert>
#include <sstream>
#include "vapor/DC.h"

using namespace VAPoR;

namespace {

string join(const vector<string> &v, string separator) {
    string s;

    for (int i = 0; i < (int)v.size() - 1; i++) {
        s += v[i];
        s += separator;
    }

    if (v.size()) {
        s += v[v.size() - 1];
    }

    return (s);
}

}; // namespace

DC::BaseVar::BaseVar(
    string name,
    string units, XType type,
    std::vector<size_t> bs,
    std::vector<bool> periodic) : _name(name),
                                  _units(units),
                                  _type(type),
                                  _bs(bs),
                                  _periodic(periodic) {
    _wname = "";
    _cratios.push_back(1);
}

void DC::Mesh::_Mesh(
    string name,
    std::vector<string> coord_vars,
    int max_nodes_per_face,
    Mesh::Type mtype) {

    _name.clear();
    _dim_names.clear();
    _coord_vars.clear();
    _max_nodes_per_face = 1;
    _node_dim_name.clear();
    _face_dim_name.clear();
    _layers_dim_name.clear();
    _face_node_var.clear();
    _face_edge_var.clear();
    _face_face_var.clear();
    _edge_dim_name.clear();
    _edge_node_var.clear();
    _edge_face_var.clear();
    _mtype = STRUCTURED;

    _name = name;
    _coord_vars = coord_vars;
    _max_nodes_per_face = max_nodes_per_face;
    _mtype = mtype;
}

DC::Mesh::Mesh(
    std::string name,
    std::vector<string> dim_names,
    std::vector<string> coord_vars) {
    _Mesh(name, coord_vars, 4, STRUCTURED);

    if (_name.empty()) {
        _name = join(dim_names, "x");
    }

    _dim_names = dim_names;
}

DC::Mesh::Mesh(
    std::string name,
    size_t max_nodes_per_face,
    std::string node_dim_name,
    std::string face_dim_name,
    std::vector<std::string> coord_vars,
    std::string face_node_var) {
    _Mesh(name, coord_vars, max_nodes_per_face, UNSTRUC_2D);

    _node_dim_name = node_dim_name;
    _face_dim_name = face_dim_name;
    _face_node_var = face_node_var;
}

DC::Mesh::Mesh(
    std::string name,
    size_t max_nodes_per_face,
    std::string node_dim_name,
    std::string face_dim_name,
    std::string layers_dim_name,
    std::vector<std::string> coord_vars,
    std::string face_node_var) {
    _Mesh(name, coord_vars, max_nodes_per_face, UNSTRUC_LAYERED);

    _node_dim_name = node_dim_name;
    _face_dim_name = face_dim_name;
    _layers_dim_name = layers_dim_name;
    _face_node_var = face_node_var;
}

size_t DC::Mesh::GetTopologyDim() const {
    return (_coord_vars.size());
}

vector<string> DC::GetDataVarNames(int ndim, bool spatial) const {
    vector<string> names, allnames;

    allnames = GetDataVarNames();

    for (int i = 0; i < allnames.size(); i++) {
        DataVar dvar;
        bool ok = GetDataVarInfo(allnames[i], dvar);
        if (!ok)
            continue;

        string mesh_name;
        mesh_name = dvar.GetMeshName();

        Mesh mesh;
        ok = GetMesh(mesh_name, mesh);
        if (!ok)
            continue;

        size_t nsdim = mesh.GetTopologyDim();

        if (!spatial && IsTimeVarying(allnames[i])) {
            ndim--;
        }

        if (nsdim == ndim) {
            names.push_back(allnames[i]);
        }
    }
    return (names);
}

vector<string> DC::GetCoordVarNames(int ndim, bool spatial) const {
    vector<string> names, allnames;

    allnames = GetCoordVarNames();

    for (int i = 0; i < allnames.size(); i++) {
        CoordVar cvar;
        bool ok = GetCoordVarInfo(allnames[i], cvar);
        if (!ok)
            continue;

        vector<string> dim_names = cvar.GetDimNames();
        size_t myndim = dim_names.size();

        // if not spatial add time dimension if it exists.
        //
        if (!spatial && !cvar.GetTimeDimName().empty()) {
            myndim++;
        }

        if (myndim == ndim) {
            names.push_back(allnames[i]);
        }
    }
    return (names);
}

bool DC::_getDataVarDimensions(
    string varname, bool spatial,
    vector<DC::Dimension> &dimensions) const {
    dimensions.clear();

    DataVar var;
    bool status = GetDataVarInfo(varname, var);
    if (!status)
        return (false);

    string mname = var.GetMeshName();

    Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status)
        return (false);

    vector<string> dimnames = mesh.GetDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim;

        status = GetDimension(dimnames[i], dim);
        if (!status)
            return (false);

        dimensions.push_back(dim);
    }

    // we're done if time dim isn't wanted
    //
    if (spatial)
        return (true);

    string tvar = var.GetTimeCoordVar();
    if (!tvar.empty()) {

        vector<DC::Dimension> timedims;
        status = _getCoordVarDimensions(tvar, false, timedims);
        if (!status)
            return (false);

        assert(timedims.size() == 1);
        dimensions.push_back(timedims[0]);
    }

    return (true);
}

bool DC::_getCoordVarDimensions(
    string varname, bool spatial,
    vector<DC::Dimension> &dimensions) const {
    dimensions.clear();

    CoordVar var;
    bool status = GetCoordVarInfo(varname, var);
    if (!status)
        return (false);

    vector<string> dimnames = var.GetDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim;
        status = GetDimension(dimnames[i], dim);
        if (!status)
            return (false);

        dimensions.push_back(dim);
    }

    // we're done if time dim isn't wanted
    //
    if (spatial)
        return (true);

    string timedim = var.GetTimeDimName();
    if (!timedim.empty()) {

        Dimension dim;
        status = GetDimension(timedim, dim);
        if (!status)
            return (false);

        dimensions.push_back(dim);
    }
    return (true);
}

bool DC::GetVarDimensions(
    string varname, bool spatial,
    vector<DC::Dimension> &dimensions) const {
    dimensions.clear();

    if (IsDataVar(varname)) {
        return (_getDataVarDimensions(varname, spatial, dimensions));
    } else if (IsCoordVar(varname)) {
        return (_getCoordVarDimensions(varname, spatial, dimensions));
    } else {
        return (false);
    }
}

bool DC::GetVarDimLens(
    string varname, bool spatial,
    vector<size_t> &dimlens) const {
    dimlens.clear();

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, spatial, dims);
    if (!status)
        return (status);

    for (int i = 0; i < dims.size(); i++) {
        dimlens.push_back(dims[i].GetLength());
    }

    return (true);
}

bool DC::GetVarDimLens(
    string varname, vector<size_t> &sdimlens, size_t &time_dimlen) const {
    sdimlens.clear();
    time_dimlen = 0;

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, false, dims);
    if (!status)
        return (status);

    if (DC::IsTimeVarying(varname)) {
        time_dimlen = dims[dims.size() - 1].GetLength();
        dims.pop_back();
    }

    for (int i = 0; i < dims.size(); i++) {
        sdimlens.push_back(dims[i].GetLength());
    }

    return (true);
}

bool DC::GetVarDimNames(
    string varname, bool spatial,
    vector<string> &dimnames) const {
    dimnames.clear();

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, spatial, dims);
    if (!status)
        return (status);

    for (int i = 0; i < dims.size(); i++) {
        dimnames.push_back(dims[i].GetName());
    }

    return (true);
}

bool DC::GetVarDimNames(
    string varname, vector<string> &sdimnames, string &time_dimname) const {
    sdimnames.clear();
    time_dimname = "";

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, false, dims);
    if (!status)
        return (status);

    if (DC::IsTimeVarying(varname)) {
        time_dimname = dims[dims.size() - 1].GetName();
        dims.pop_back();
    }

    for (int i = 0; i < dims.size(); i++) {
        sdimnames.push_back(dims[i].GetName());
    }

    return (true);
}

size_t DC::GetVarTopologyDim(string varname) const {

    DataVar var;
    bool status = GetDataVarInfo(varname, var);
    if (!status)
        return (0);

    string mname = var.GetMeshName();

    Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status)
        return (0);

    return (mesh.GetTopologyDim());
}

bool DC::IsTimeVarying(string varname) const {

    if (IsAuxVar(varname))
        return (false);

    // If var is a data variable and has a time coordinate variable defined
    //
    if (IsDataVar(varname)) {
        DataVar var;
        bool ok = GetDataVarInfo(varname, var);
        if (!ok)
            return (false);
        return (!var.GetTimeCoordVar().empty());
    }

    // If var is a coordinate variable and it's axis is Time
    //
    if (IsCoordVar(varname)) {
        CoordVar var;
        bool ok = GetCoordVarInfo(varname, var);
        if (!ok)
            return (false);
        return (!var.GetTimeDimName().empty());
    }

    return (false);
}

bool DC::IsCompressed(string varname) const {

    BaseVar var;

    bool ok = GetBaseVarInfo(varname, var);
    if (!ok)
        return (false);

    return (var.IsCompressed());
}

int DC::GetNumTimeSteps(string varname) const {

    if (IsAuxVar(varname))
        return (1);

    // If data variable get it's time coordinate variable if it exists
    //
    if (IsDataVar(varname)) {
        DataVar var;
        bool ok = GetDataVarInfo(varname, var);
        if (!ok)
            return (0);

        string time_coord_var = var.GetTimeCoordVar();
        if (time_coord_var.empty())
            return (1);
        varname = time_coord_var;
    }

    CoordVar var;
    bool ok = GetCoordVarInfo(varname, var);
    if (!ok)
        return (0);

    string time_dim_name = var.GetTimeDimName();
    if (time_dim_name.empty())
        return (1);

    DC::Dimension dim;
    ok = GetDimension(time_dim_name, dim);
    if (!ok)
        return (0);

    return (dim.GetLength());
}

vector<size_t> DC::GetCRatios(string varname) const {

    DC::BaseVar var;
    bool status = GetBaseVarInfo(varname, var);
    if (!status) {
        return (vector<size_t>(1, 1));
    }
    return (var.GetCRatios());
}

bool DC::GetVarCoordVars(
    string varname, bool spatial, std::vector<string> &coord_vars) const {
    coord_vars.clear();

    DataVar dvar;
    bool status = GetDataVarInfo(varname, dvar);
    if (!status)
        return (false);

    Mesh m;
    status = GetMesh(dvar.GetMeshName(), m);
    if (!status)
        return (false);

    coord_vars = m.GetCoordVars();

    if (spatial)
        return (true);

    if (!dvar.GetTimeCoordVar().empty()) {
        coord_vars.push_back(dvar.GetTimeCoordVar());
    }

    return (true);
}

bool DC::GetNumDimensions(string varname, size_t &ndim) const {
    ndim = 0;

    DataVar dvar;
    bool status = GetDataVarInfo(varname, dvar);

    if (status) {

        Mesh m;
        status = GetMesh(dvar.GetMeshName(), m);
        if (!status)
            return (false);

        switch (m.GetMeshType()) {
        case Mesh::UNSTRUC_2D:
            ndim = 2;
            break;
        case Mesh::UNSTRUC_LAYERED:
            ndim = 3;
            break;
        case Mesh::UNSTRUC_3D:
            ndim = 3;
            break;
        case Mesh::STRUCTURED:
            ndim = m.GetDimNames().size();
            break;
        default:
            ndim = 0;
        }
        return (true);
    }

    CoordVar cvar;
    status = GetCoordVarInfo(varname, cvar);
    if (!status)
        return (0);

    ndim = cvar.GetDimNames().size();
    return (true);
}

namespace VAPoR {

std::ostream &operator<<(
    std::ostream &o, const DC::Dimension &d) {
    o << "  Dimension" << endl;
    o << "   Name: " << d._name << endl;
    o << "   Lengths:";
    for (int i = 0; i < d._lengths.size(); i++) {
        o << " " << d._lengths[i];
    }
    o << endl;

    return (o);
}

std::ostream &operator<<(
    std::ostream &o, const DC::Mesh &m) {
    o << "  Mesh" << endl;
    o << "   Name: " << m._name << endl;

    o << "   DimNames:";
    for (int i = 0; i < m._dim_names.size(); i++) {
        o << " " << m._dim_names[i];
    }
    o << endl;

    o << "   CoordVars:";
    for (int i = 0; i < m._coord_vars.size(); i++) {
        o << " " << m._coord_vars[i];
    }
    o << endl;

    o << "   MaxNodesPerFace: " << m._max_nodes_per_face << endl;
    o << "   NodeDimName: " << m._node_dim_name << endl;
    o << "   FaceDimName: " << m._face_dim_name << endl;
    o << "   LayersDimName: " << m._layers_dim_name << endl;
    o << "   FaceNodeVar: " << m._face_node_var << endl;
    o << "   FaceEdgeVar: " << m._face_edge_var << endl;
    o << "   FaceFaceVar: " << m._face_face_var << endl;
    o << "   EdgeDimName: " << m._edge_dim_name << endl;
    o << "   EdgeNodeVar: " << m._edge_node_var << endl;
    o << "   EdgeFaceVar: " << m._edge_face_var << endl;
    o << "   Mtype: " << m._mtype << endl;

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::Attribute &a) {
    o << "  Attribute:" << endl;
    o << "   Name: " << a._name << endl;
    o << "   Type: " << a._type << endl;

    o << "   Values: ";
    for (int i = 0; i < a._values.size(); i++) {
        DC::Attribute::podunion p = a._values[i];
        if (a._type == DC::FLOAT) {
            o << p.f;
        } else if (a._type == DC::DOUBLE) {
            o << p.d;
        } else if (a._type == DC::INT32) {
            o << p.i;
        } else if (a._type == DC::INT64) {
            o << p.l;
        }
        if (a._type == DC::TEXT) {
            o << p.c;
        }
        o << " ";
    }
    o << endl;
    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::BaseVar &var) {

    o << "  BaseVar" << endl;
    o << "   Name: " << var._name << endl;
    o << "   Units: " << var._units << endl;
    o << "   XType: " << var._type << endl;
    o << "   Compressed: " << (var._cratios.size() > 0) << endl;
    o << "   WName: " << var._wname << endl;
    o << "   CRatios: ";
    for (int i = 0; i < var._cratios.size(); i++) {
        o << var._cratios[i] << " ";
    }
    o << endl;
    o << "   BlockSize:";
    for (int i = 0; i < var._bs.size(); i++) {
        o << " " << var._bs[i];
    }
    o << endl;
    o << "   Periodic: ";
    for (int i = 0; i < var._periodic.size(); i++) {
        o << var._periodic[i] << " ";
    }
    o << endl;

    std::map<string, DC::Attribute>::const_iterator itr;
    for (itr = var._atts.begin(); itr != var._atts.end(); ++itr) {
        o << itr->second;
    }

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::CoordVar &var) {

    o << "  CoordVar" << endl;
    o << "   Axis: " << var._axis << endl;
    o << "   Uniform: " << var._uniform << endl;

    o << (DC::BaseVar)var;

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::DataVar &var) {

    o << "  DataVar" << endl;
    o << "   Mesh: " << var._mesh << endl;
    o << "   TimeCoordVar: " << var._time_coord_var << endl;
    o << "   Location: " << var._location << endl;
    o << "   MaskVar: " << var._maskvar << endl;
    o << "   HasMissing: " << var._has_missing << endl;
    o << "   MissingValue: " << var._missing_value << endl;

    o << (DC::BaseVar)var;

    return (o);
}
}; // namespace VAPoR
