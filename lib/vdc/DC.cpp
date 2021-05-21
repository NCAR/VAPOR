#include "vapor/VAssert.h"
#include <sstream>
#include "vapor/DC.h"

using namespace VAPoR;

namespace {

string join(const vector<string> &v, string separator)
{
    string s;

    for (int i = 0; i < (int)v.size() - 1; i++) {
        s += v[i];
        s += separator;
    }

    if (v.size()) { s += v[v.size() - 1]; }

    return (s);
}

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

DC::Attribute::Attribute(string name, XType type, const vector<float> &values)
{
    _name = name;
    _type = type;
    _values.clear();
    Attribute::SetValues(values);
}

void DC::Attribute::SetValues(const vector<float> &values)
{
    _values.clear();
    vector<double> dvec;
    for (int i = 0; i < values.size(); i++) { dvec.push_back((double)values[i]); }
    DC::Attribute::SetValues(dvec);
}

DC::Attribute::Attribute(string name, XType type, const vector<double> &values)
{
    _name = name;
    _type = type;
    _values.clear();
    Attribute::SetValues(values);
}

void DC::Attribute::SetValues(const vector<double> &values)
{
    _values.clear();
    for (int i = 0; i < values.size(); i++) {
        podunion pod;
        if (_type == FLOAT) {
            pod.f = (float)values[i];
        } else if (_type == DOUBLE) {
            pod.d = (double)values[i];
        } else if (_type == INT32) {
            pod.i = (int)values[i];
        } else if (_type == INT64) {
            pod.l = (int)values[i];
        } else if (_type == TEXT) {
            pod.c = (char)values[i];
        }
        _values.push_back(pod);
    }
}

DC::Attribute::Attribute(string name, XType type, const vector<int> &values)
{
    _name = name;
    _type = type;
    _values.clear();
    Attribute::SetValues(values);
}

void DC::Attribute::SetValues(const vector<int> &values)
{
    _values.clear();
    vector<long> lvec;
    for (int i = 0; i < values.size(); i++) { lvec.push_back((long)values[i]); }
    DC::Attribute::SetValues(lvec);
}

DC::Attribute::Attribute(string name, XType type, const vector<long> &values)
{
    _name = name;
    _type = type;
    _values.clear();
    Attribute::SetValues(values);
}

void DC::Attribute::SetValues(const vector<long> &values)
{
    _values.clear();

    for (int i = 0; i < values.size(); i++) {
        podunion pod;
        if (_type == FLOAT) {
            pod.f = (float)values[i];
        } else if (_type == DOUBLE) {
            pod.d = (double)values[i];
        } else if (_type == INT32) {
            pod.i = (int)values[i];
        } else if (_type == INT64) {
            pod.l = (long)values[i];
        } else if (_type == TEXT) {
            pod.c = (char)values[i];
        }
        _values.push_back(pod);
    }
}

DC::Attribute::Attribute(string name, XType type, const string &values)
{
    _name = name;
    _type = type;
    _values.clear();
    Attribute::SetValues(values);
}

void DC::Attribute::SetValues(const string &values)
{
    _values.clear();
    for (int i = 0; i < values.size(); i++) {
        podunion pod;
        if (_type == FLOAT) {
            pod.f = (float)values[i];
        } else if (_type == DOUBLE) {
            pod.d = (double)values[i];
        } else if (_type == INT32) {
            pod.i = (int)values[i];
        } else if (_type == INT64) {
            pod.l = (long)values[i];
        } else if (_type == TEXT) {
            pod.c = (char)values[i];
        }
        _values.push_back(pod);
    }
}

void DC::Attribute::GetValues(vector<float> &values) const
{
    values.clear();

    vector<double> dvec;
    DC::Attribute::GetValues(dvec);
    for (int i = 0; i < dvec.size(); i++) { values.push_back((float)dvec[i]); }
}

void DC::Attribute::GetValues(vector<double> &values) const
{
    values.clear();

    for (int i = 0; i < _values.size(); i++) {
        podunion pod = _values[i];
        if (_type == FLOAT) {
            values.push_back((double)pod.f);
        } else if (_type == DOUBLE) {
            values.push_back((double)pod.d);
        } else if (_type == INT32) {
            values.push_back((double)pod.i);
        } else if (_type == INT64) {
            values.push_back((double)pod.l);
        } else if (_type == TEXT) {
            values.push_back((double)pod.c);
        }
    }
}

void DC::Attribute::GetValues(vector<int> &values) const
{
    values.clear();

    vector<long> lvec;
    DC::Attribute::GetValues(lvec);
    for (int i = 0; i < lvec.size(); i++) { values.push_back((int)lvec[i]); }
}

void DC::Attribute::GetValues(vector<long> &values) const
{
    values.clear();

    for (int i = 0; i < _values.size(); i++) {
        podunion pod = _values[i];
        if (_type == FLOAT) {
            values.push_back((long)pod.f);
        } else if (_type == DOUBLE) {
            values.push_back((long)pod.d);
        } else if (_type == INT32) {
            values.push_back((long)pod.i);
        } else if (_type == INT64) {
            values.push_back((long)pod.l);
        } else if (_type == TEXT) {
            values.push_back((long)pod.c);
        }
    }
}

void DC::Attribute::GetValues(string &values) const
{
    values.clear();

    for (int i = 0; i < _values.size(); i++) {
        podunion pod = _values[i];
        if (_type == FLOAT) {
            values += (char)pod.f;
        } else if (_type == DOUBLE) {
            values += (char)pod.d;
        } else if (_type == INT32) {
            values += (char)pod.i;
        } else if (_type == INT64) {
            values += (char)pod.l;
        } else if (_type == TEXT) {
            values += (char)pod.c;
        }
    }
}

DC::BaseVar::BaseVar(string name, string units, XType type, std::vector<bool> periodic) : _name(name), _units(units), _type(type), _periodic(periodic)
{
    _wname = "";
    _cratios.push_back(1);
}

void DC::Mesh::_Mesh(string name, std::vector<string> coord_vars, int max_nodes_per_face, int max_faces_per_node, Mesh::Type mtype)
{
    _name.clear();
    _dim_names.clear();
    _coord_vars.clear();
    _max_nodes_per_face = 1;
    _max_faces_per_node = 1;
    _node_dim_name.clear();
    _face_dim_name.clear();
    _layers_dim_name.clear();
    _face_node_var.clear();
    _node_face_var.clear();
    _node_face_var.clear();
    _face_edge_var.clear();
    _face_face_var.clear();
    _edge_dim_name.clear();
    _edge_node_var.clear();
    _edge_face_var.clear();
    _mtype = STRUCTURED;

    _name = name;
    _coord_vars = coord_vars;
    _max_nodes_per_face = max_nodes_per_face;
    _max_faces_per_node = max_faces_per_node;
    _mtype = mtype;
}

DC::Mesh::Mesh(std::string name, std::vector<string> dim_names, std::vector<string> coord_vars)
{
    VAssert(coord_vars.size() >= dim_names.size());

    _Mesh(name, coord_vars, 4, 4, STRUCTURED);

    if (_name.empty()) { _name = join(dim_names, "x"); }

    _dim_names = dim_names;
}

DC::Mesh::Mesh(std::string name, size_t max_nodes_per_face, size_t max_faces_per_node, std::string node_dim_name, std::string face_dim_name, std::vector<std::string> coord_vars,
               std::string face_node_var, std::string node_face_var)
{
    VAssert(coord_vars.size() >= 2);

    _Mesh(name, coord_vars, max_nodes_per_face, max_faces_per_node, UNSTRUC_2D);

    _node_dim_name = node_dim_name;
    _face_dim_name = face_dim_name;
    _face_node_var = face_node_var;
    _node_face_var = node_face_var;

    _dim_names.push_back(node_dim_name);
}

DC::Mesh::Mesh(std::string name, size_t max_nodes_per_face, size_t max_faces_per_node, std::string node_dim_name, std::string face_dim_name, std::string layers_dim_name,
               std::vector<std::string> coord_vars, std::string face_node_var, std::string node_face_var)
{
    VAssert(coord_vars.size() == 3);

    _Mesh(name, coord_vars, max_nodes_per_face, max_faces_per_node, UNSTRUC_LAYERED);

    _node_dim_name = node_dim_name;
    _face_dim_name = face_dim_name;
    _layers_dim_name = layers_dim_name;
    _face_node_var = face_node_var;
    _node_face_var = node_face_var;

    _dim_names.push_back(node_dim_name);
    _dim_names.push_back(layers_dim_name);
}

DC::Mesh::Mesh(std::string name, int max_nodes_per_face, int max_faces_per_node, std::string node_dim_name, std::string face_dim_name, std::vector<string> coord_vars)
{
    _Mesh(name, coord_vars, max_nodes_per_face, max_faces_per_node, UNSTRUC_3D);

    _node_dim_name = node_dim_name;
    _face_dim_name = face_dim_name;

    _dim_names.push_back(node_dim_name);
}

size_t DC::Mesh::GetTopologyDim() const
{
    switch (_mtype) {
    case STRUCTURED: return (_dim_names.size()); break;
    case UNSTRUC_2D: return (2); break;
    case UNSTRUC_LAYERED: return (3); break;
    case UNSTRUC_3D: return (3); break;
    default:
        VAssert(false);
        return (0);
        break;
    }
}

DC::DC() {}

int DC::GetHyperSliceInfo(string varname, int level, std::vector<size_t> &hslice_dims, size_t &nslice, long ts)
{
    hslice_dims.clear();
    nslice = 0;

    vector<size_t> dims_at_level;
    vector<size_t> dummy;

    int rc = GetDimLensAtLevel(varname, level, dims_at_level, dummy, ts);
    if (rc < 0) return (-1);

    if (dims_at_level.size() == 0) return (0);

    hslice_dims = dims_at_level;

    if (dims_at_level.size() == 1) {
        nslice = 1;
        return (0);
    }

    int dim = hslice_dims.size() - 1;

    hslice_dims[dim] = 1;
    nslice = dims_at_level[dim] / hslice_dims[dim];

    return (0);
}

vector<string> DC::GetDataVarNames(int ndim) const
{
    vector<string> names, allnames;

    allnames = GetDataVarNames();

    for (int i = 0; i < allnames.size(); i++) {
        DataVar dvar;
        bool    ok = GetDataVarInfo(allnames[i], dvar);
        if (!ok) continue;

        string mesh_name;
        mesh_name = dvar.GetMeshName();

        Mesh mesh;
        ok = GetMesh(mesh_name, mesh);
        if (!ok) continue;

        size_t d = mesh.GetTopologyDim();

        if (d == ndim) { names.push_back(allnames[i]); }
    }
    return (names);
}

bool DC::_getDataVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions, long ts) const
{
    dimensions.clear();

    DataVar var;
    bool    status = GetDataVarInfo(varname, var);
    if (!status) return (false);

    string mname = var.GetMeshName();

    if (!mname.empty()) {
        ;    // 0-d variable
        Mesh mesh;
        status = GetMesh(mname, mesh);
        if (!status) return (false);

        vector<string> dimnames;
        if (mesh.GetMeshType() == Mesh::STRUCTURED) {
            dimnames = mesh.GetDimNames();
        } else {
            switch (var.GetSamplingLocation()) {
            case Mesh::NODE: dimnames.push_back(mesh.GetNodeDimName()); break;
            case Mesh::EDGE: dimnames.push_back(mesh.GetEdgeDimName()); break;
            case Mesh::FACE: dimnames.push_back(mesh.GetFaceDimName()); break;
            case Mesh::VOLUME: VAssert(0 && "VOLUME cells not supported"); break;
            }
            if (mesh.GetMeshType() == Mesh::UNSTRUC_LAYERED) { dimnames.push_back(mesh.GetLayersDimName()); }
        }

        for (int i = 0; i < dimnames.size(); i++) {
            Dimension dim;

            status = GetDimension(dimnames[i], dim, ts);
            if (!status) return (false);

            dimensions.push_back(dim);
        }
    }

    // we're done if time dim isn't wanted
    //
    if (spatial) return (true);

    string tvar = var.GetTimeCoordVar();
    if (!tvar.empty()) {
        vector<DC::Dimension> timedims;
        status = _getCoordVarDimensions(tvar, false, timedims, ts);
        if (!status) return (false);

        VAssert(timedims.size() == 1);
        dimensions.push_back(timedims[0]);
    }

    return (true);
}

bool DC::_getCoordVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions, long ts) const
{
    dimensions.clear();

    CoordVar var;
    bool     status = GetCoordVarInfo(varname, var);
    if (!status) return (false);

    vector<string> dimnames = var.GetDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim;
        status = GetDimension(dimnames[i], dim, ts);
        if (!status) return (false);

        dimensions.push_back(dim);
    }

    // we're done if time dim isn't wanted
    //
    if (spatial) return (true);

    string timedim = var.GetTimeDimName();
    if (!timedim.empty()) {
        Dimension dim;
        status = GetDimension(timedim, dim, ts);
        if (!status) return (false);

        dimensions.push_back(dim);
    }
    return (true);
}

bool DC::_getAuxVarDimensions(string varname, vector<DC::Dimension> &dimensions, long ts) const
{
    dimensions.clear();

    AuxVar var;
    bool   status = GetAuxVarInfo(varname, var);
    if (!status) return (false);

    vector<string> dimnames = var.GetDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim;
        status = GetDimension(dimnames[i], dim, ts);
        if (!status) return (false);

        dimensions.push_back(dim);
    }
    return (true);
}

vector<size_t> DC::_getBlockSize() const
{
    vector<size_t> bs = getBlockSize();
    while (bs.size() > 3) { bs.pop_back(); }
    return (bs);
}

int DC::_openVariableRead(size_t ts, string varname, int level, int lod) { return (openVariableRead(ts, varname, level, lod)); }

int DC::_closeVariable(int fd) { return (closeVariable(fd)); }

template<class T> int DC::_readSliceTemplate(int fd, T *slice)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;

    FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    string varname = f->GetVarname();
    int    level = f->GetLevel();
    int    sliceNum = f->GetSlice();

    int rc = GetDimLensAtLevel(varname, level, dims_at_level, dummy, f->GetTS());
    if (rc < 0) return (rc);

    vector<size_t> hslice_dims;
    size_t         nslice;
    rc = GetHyperSliceInfo(varname, level, hslice_dims, nslice);
    if (rc < 0) return (rc);
    VAssert(hslice_dims.size() == dims_at_level.size());

    if (sliceNum >= nslice) return (0);    // Done reading;

    vector<size_t> min;
    vector<size_t> max;
    int            dim = 0;
    for (; dim < hslice_dims.size() - 1; dim++) {
        min.push_back(0);
        max.push_back(hslice_dims[dim] - 1);
    };
    min.push_back(sliceNum * hslice_dims[dim]);
    max.push_back(min[dim] + hslice_dims[dim] - 1);

    // Last slice is a partial read if not block-aligned
    //
    if (max[dim] >= dims_at_level[dim]) { max[dim] = dims_at_level[dim] - 1; }

    rc = ReadRegion(fd, min, max, slice);
    if (rc < 0) return (rc);

    sliceNum++;
    f->SetSlice(sliceNum);

    return (rc);
}

template<class T> int DC::_readTemplate(int fd, T *data)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;

    FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }
    string varname = f->GetVarname();
    int    level = f->GetLevel();

    int rc = GetDimLensAtLevel(varname, level, dims_at_level, dummy, f->GetTS());
    if (rc < 0) return (rc);

    vector<size_t> min, max;
    for (int i = 0; i < dims_at_level.size(); i++) {
        min.push_back(0);
        max.push_back(dims_at_level[i] - 1);
    }

    return (ReadRegion(fd, min, max, data));
}

template<class T> int DC::_getVarTemplate(string varname, int level, int lod, T *data)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;

    size_t numts = GetNumTimeSteps(varname);

    T *ptr = data;
    for (size_t ts = 0; ts < numts; ts++) {
        int rc = GetDimLensAtLevel(varname, level, dims_at_level, dummy, ts);
        if (rc < 0) return (-1);
        size_t var_size = 1;
        for (int i = 0; i < dims_at_level.size(); i++) { var_size *= dims_at_level[i]; }

        rc = GetVar(ts, varname, level, lod, ptr);
        if (rc < 0) return (-1);

        ptr += var_size;
    }

    return (0);
}

template int DC::_getVarTemplate<float>(string varname, int level, int lod, float *data);
template int DC::_getVarTemplate<int>(string varname, int level, int lod, int *data);

template<class T> int DC::_getVarTemplate(size_t ts, string varname, int level, int lod, T *data)
{
    int fd = OpenVariableRead(ts, varname, level, lod);
    if (fd < 0) return (-1);

    int rc = Read(fd, data);
    if (rc < 0) {
        CloseVariable(fd);
        return (-1);
    }

    rc = CloseVariable(fd);

    return (rc);
}

template int DC::_getVarTemplate<float>(size_t ts, string varname, int level, int lod, float *data);
template int DC::_getVarTemplate<int>(size_t ts, string varname, int level, int lod, int *data);

bool DC::GetVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions, long ts) const
{
    dimensions.clear();

    if (IsDataVar(varname)) {
        return (_getDataVarDimensions(varname, spatial, dimensions, ts));
    } else if (IsCoordVar(varname)) {
        return (_getCoordVarDimensions(varname, spatial, dimensions, ts));
    } else if (IsAuxVar(varname)) {
        return (_getAuxVarDimensions(varname, dimensions, ts));
    } else {
        return (false);
    }
}

bool DC::GetVarDimLens(string varname, bool spatial, vector<size_t> &dimlens, long ts) const
{
    dimlens.clear();

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, spatial, dims, ts);
    if (!status) return (status);

    for (int i = 0; i < dims.size(); i++) { dimlens.push_back(dims[i].GetLength()); }

    return (true);
}

bool DC::GetVarDimLens(string varname, vector<size_t> &sdimlens, size_t &time_dimlen, long ts) const
{
    sdimlens.clear();
    time_dimlen = 0;

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, false, dims, ts);
    if (!status) return (status);

    if (DC::IsTimeVarying(varname)) {
        time_dimlen = dims[dims.size() - 1].GetLength();
        dims.pop_back();
    }

    for (int i = 0; i < dims.size(); i++) { sdimlens.push_back(dims[i].GetLength()); }

    return (true);
}

bool DC::GetVarDimNames(string varname, bool spatial, vector<string> &dimnames) const
{
    dimnames.clear();

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, spatial, dims, 0);
    if (!status) return (status);

    for (int i = 0; i < dims.size(); i++) { dimnames.push_back(dims[i].GetName()); }

    return (true);
}

bool DC::GetVarDimNames(string varname, vector<string> &sdimnames, string &time_dimname) const
{
    sdimnames.clear();
    time_dimname = "";

    vector<DC::Dimension> dims;

    bool status = DC::GetVarDimensions(varname, false, dims, 0);
    if (!status) return (status);

    if (DC::IsTimeVarying(varname)) {
        time_dimname = dims[dims.size() - 1].GetName();
        dims.pop_back();
    }

    for (int i = 0; i < dims.size(); i++) { sdimnames.push_back(dims[i].GetName()); }

    return (true);
}

size_t DC::GetVarTopologyDim(string varname) const
{
    DataVar var;
    bool    status = GetDataVarInfo(varname, var);
    if (!status) return (0);

    string mname = var.GetMeshName();

    Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status) return (0);

    return (mesh.GetTopologyDim());
}

size_t DC::GetVarGeometryDim(string varname) const
{
    DataVar var;
    bool    status = GetDataVarInfo(varname, var);
    if (!status) return (0);

    string mname = var.GetMeshName();

    Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status) return (0);

    return (mesh.GetGeometryDim());
}

bool DC::IsTimeVarying(string varname) const
{
    if (IsAuxVar(varname)) return (false);

    // If var is a data variable and has a time coordinate variable defined
    //
    if (IsDataVar(varname)) {
        DataVar var;
        bool    ok = GetDataVarInfo(varname, var);
        if (!ok) return (false);
        return (!var.GetTimeCoordVar().empty());
    }

    // If var is a coordinate variable and it has a time dimension
    //
    if (IsCoordVar(varname)) {
        CoordVar var;
        bool     ok = GetCoordVarInfo(varname, var);
        if (!ok) return (false);
        return (!var.GetTimeDimName().empty());
    }

    return (false);
}

bool DC::IsCompressed(string varname) const
{
    BaseVar var;

    bool ok = GetBaseVarInfo(varname, var);
    if (!ok) return (false);

    return (var.IsCompressed());
}

int DC::GetNumTimeSteps(string varname) const
{
    if (IsAuxVar(varname)) return (1);

    // If data variable get it's time coordinate variable if it exists
    //
    if (IsDataVar(varname)) {
        DataVar var;
        bool    ok = GetDataVarInfo(varname, var);
        if (!ok) return (0);

        string time_coord_var = var.GetTimeCoordVar();
        if (time_coord_var.empty()) return (1);
        varname = time_coord_var;
    }

    CoordVar var;
    bool     ok = GetCoordVarInfo(varname, var);
    if (!ok) return (0);

    string time_dim_name = var.GetTimeDimName();
    if (time_dim_name.empty()) return (1);

    DC::Dimension dim;
    ok = GetDimension(time_dim_name, dim, 0);
    if (!ok) return (0);

    return (dim.GetLength());
}

vector<size_t> DC::GetCRatios(string varname) const
{
    DC::BaseVar var;
    bool        status = GetBaseVarInfo(varname, var);
    if (!status) { return (vector<size_t>(1, 1)); }
    return (var.GetCRatios());
}

bool DC::GetVarCoordVars(string varname, bool spatial, std::vector<string> &coord_vars) const
{
    coord_vars.clear();

    DataVar dvar;
    bool    status = GetDataVarInfo(varname, dvar);
    if (!status) return (false);

    Mesh m;
    status = GetMesh(dvar.GetMeshName(), m);
    if (!status) return (false);

    coord_vars = m.GetCoordVars();

    if (spatial) return (true);

    if (!dvar.GetTimeCoordVar().empty()) { coord_vars.push_back(dvar.GetTimeCoordVar()); }

    return (true);
}

bool DC::GetVarConnVars(string varname, string &face_node_var, string &node_face_var, string &face_edge_var, string &face_face_var, string &edge_node_var, string &edge_face_var) const
{
    face_node_var.clear();
    node_face_var.clear();
    face_edge_var.clear();
    face_face_var.clear();
    edge_node_var.clear();
    edge_face_var.clear();

    DataVar dvar;
    bool    status = GetDataVarInfo(varname, dvar);
    if (!status) return (false);

    Mesh m;
    status = GetMesh(dvar.GetMeshName(), m);
    if (!status) return (false);

    face_node_var = m.GetFaceNodeVar();
    node_face_var = m.GetNodeFaceVar();
    face_edge_var = m.GetFaceEdgeVar();
    face_face_var = m.GetFaceFaceVar();
    edge_node_var = m.GetEdgeNodeVar();
    edge_face_var = m.GetEdgeFaceVar();

    return (true);
}

size_t DC::GetNumDimensions(string varname) const
{
    DataVar dvar;
    bool    status = GetDataVarInfo(varname, dvar);

    if (status) {
        Mesh m;
        status = GetMesh(dvar.GetMeshName(), m);
        if (!status) return (0);

        return (m.GetDimNames().size());
    }

    CoordVar cvar;
    status = GetCoordVarInfo(varname, cvar);
    if (status) return (cvar.GetDimNames().size());

    AuxVar avar;
    status = GetAuxVarInfo(varname, avar);
    if (status) return (avar.GetDimNames().size());

    return (0);
}

std::vector<string> DC::GetTimeCoordVarNames() const
{
    vector<string> cvars = GetCoordVarNames();

    vector<string> timeCoordVars;

    for (int i = 0; i < cvars.size(); i++) {
        CoordVar cvar;
        bool     status = GetCoordVarInfo(cvars[i], cvar);
        VAssert(status);

        if (cvar.GetAxis() == 3) { timeCoordVars.push_back(cvars[i]); }
    }
    return (timeCoordVars);
}

DC::FileTable::FileTable() { _table.clear(); }

DC::FileTable::~FileTable()
{
    for (int i = 0; i < _table.size(); i++) {
        if (_table[i]) delete _table[i];
        _table[i] = NULL;
    }
}

int DC::FileTable::AddEntry(DC::FileTable::FileObject *obj)
{
    // Try to re-use an existing entry;
    //
    for (int i = 0; i < _table.size(); i++) {
        if (!_table[i]) {
            _table[i] = obj;
            return (i);
        }
    }

    // Create new entry
    //
    _table.push_back(obj);
    return (_table.size() - 1);
}

DC::FileTable::FileObject *DC::FileTable::GetEntry(int fd) const
{
    if (fd < 0) return (NULL);

    if (fd < _table.size()) return (_table[fd]);

    return (NULL);
}

void DC::FileTable::RemoveEntry(int fd)
{
    if (fd < 0 || fd >= _table.size()) return;

    _table[fd] = NULL;
}

vector<int> DC::FileTable::GetEntries() const
{
    vector<int> fds;

    for (int i = 0; i < _table.size(); i++) {
        if (_table[i]) { fds.push_back(i); }
    }
    return (fds);
}

bool DC::GetMeshDimLens(const string &mesh_name, std::vector<size_t> &dims, long ts) const
{
    dims.clear();

    DC::Mesh mesh;
    bool     status = GetMesh(mesh_name, mesh);
    if (!status) return (false);

    vector<string> dimNames = mesh.GetDimNames();
    for (int i = 0; i < dimNames.size(); i++) {
        DC::Dimension dimension;

        status = GetDimension(dimNames[i], dimension, ts);
        if (!status) return (false);

        dims.push_back(dimension.GetLength());
    }
    return (true);
}

bool DC::GetMeshDimNames(const string &mesh_name, std::vector<string> &dimnames) const
{
    dimnames.clear();

    DC::Mesh mesh;
    bool     status = GetMesh(mesh_name, mesh);
    if (!status) return (false);

    dimnames = mesh.GetDimNames();

    return (true);
}

namespace VAPoR {

std::ostream &operator<<(std::ostream &o, const DC::Dimension &d)
{
    o << "  Dimension" << endl;
    o << "   Name: " << d._name << endl;
    o << "   Lengths:";
    for (int i = 0; i < d._lengths.size(); i++) { o << " " << d._lengths[i]; }
    o << endl;

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::Mesh &m)
{
    o << "  Mesh" << endl;
    o << "   Name: " << m._name << endl;

    o << "   DimNames:";
    for (int i = 0; i < m._dim_names.size(); i++) { o << " " << m._dim_names[i]; }
    o << endl;

    o << "   CoordVars:";
    for (int i = 0; i < m._coord_vars.size(); i++) { o << " " << m._coord_vars[i]; }
    o << endl;

    o << "   MaxNodesPerFace: " << m._max_nodes_per_face << endl;
    o << "   MaxFacesPerNode: " << m._max_faces_per_node << endl;
    o << "   NodeDimName: " << m._node_dim_name << endl;
    o << "   FaceDimName: " << m._face_dim_name << endl;
    o << "   LayersDimName: " << m._layers_dim_name << endl;
    o << "   FaceNodeVar: " << m._face_node_var << endl;
    o << "   NodeFaceVar: " << m._node_face_var << endl;
    o << "   FaceEdgeVar: " << m._face_edge_var << endl;
    o << "   FaceFaceVar: " << m._face_face_var << endl;
    o << "   EdgeDimName: " << m._edge_dim_name << endl;
    o << "   EdgeNodeVar: " << m._edge_node_var << endl;
    o << "   EdgeFaceVar: " << m._edge_face_var << endl;
    o << "   Mtype: " << m._mtype << endl;

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::Attribute &a)
{
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
        if (a._type == DC::TEXT) { o << p.c; }
        o << " ";
    }
    o << endl;
    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::BaseVar &var)
{
    o << "  BaseVar" << endl;
    o << "   Name: " << var._name << endl;
    o << "   Units: " << var._units << endl;
    o << "   XType: " << var._type << endl;
    o << "   Compressed: " << (var._cratios.size() > 0) << endl;
    o << "   WName: " << var._wname << endl;
    o << "   CRatios: ";
    for (int i = 0; i < var._cratios.size(); i++) { o << var._cratios[i] << " "; }
    o << endl;
    o << endl;
    o << "   Periodic: ";
    for (int i = 0; i < var._periodic.size(); i++) { o << var._periodic[i] << " "; }
    o << endl;

    std::map<string, DC::Attribute>::const_iterator itr;
    for (itr = var._atts.begin(); itr != var._atts.end(); ++itr) { o << itr->second; }

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::CoordVar &var)
{
    o << "  CoordVar" << endl;
    o << "   Axis: " << var._axis << endl;
    o << "   Uniform: " << var._uniform << endl;

    o << (DC::BaseVar)var;

    return (o);
}

std::ostream &operator<<(std::ostream &o, const DC::DataVar &var)
{
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
};    // namespace VAPoR
