#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <cfloat>
#include <vector>
#include <map>
#include <type_traits>
#include <vapor/GeoUtil.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/DCCF.h>
#include <vapor/DCMPAS.h>
#include <vapor/DerivedVar.h>
#include <vapor/DataMgr.h>
#ifdef WIN32
    #include <float.h>
#endif
using namespace Wasp;
using namespace VAPoR;

namespace {

// Format a vector as a space-separated element string
//
template<class T> string vector_to_string(vector<T> v)
{
    ostringstream oss;

    oss << "[";
    for (int i = 0; i < v.size(); i++) { oss << v[i] << " "; }
    oss << "]";
    return (oss.str());
}

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

size_t decimate_length(size_t l)
{
    if (l % 2)
        return ((l + 1) / 2);
    else
        return (l / 2);
}

// Compute dimensions of a multidimensional array, specified by dims,
// after undergoing decimiation
//
vector<size_t> decimate_dims(const vector<size_t> &dims, int l)
{
    vector<size_t> new_dims = dims;
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < new_dims.size(); j++) { new_dims[j] = decimate_length(new_dims[j]); }
    }
    return (new_dims);
}

// Decimate a 1D array with simple averaging.
//
template<typename T> void decimate1d(const vector<size_t> &src_bs, const T *src, T *dst)
{
    assert(src_bs.size() == 1);

    vector<size_t> dst_bs;
    for (int i = 0; i < src_bs.size(); i++) { dst_bs.push_back(decimate_length(src_bs[i])); }

    size_t ni0 = src_bs[0];

    size_t ni1 = dst_bs[0];

    for (size_t i = 0; i < ni1; i++) { dst[i] = 0.0; }

    // Perform in-place 4-node averaging, handling odd boundary cases
    //
    for (size_t i = 0, ii = 0; i < ni0; i++) {
        float iwgt = 0.5;
        if (i == ni0 - 1 && ni0 % 2) iwgt = 1.0;    // odd i boundary
        float w = iwgt;

        assert(w <= 1.0);

        dst[ii] += w * src[i];
        if (i % 2) ii++;
    }
}

template<typename T> void decimate2d(const vector<size_t> &src_bs, const T *src, T *dst)
{
    assert(src_bs.size() == 2);

    vector<size_t> dst_bs;
    for (int i = 0; i < src_bs.size(); i++) { dst_bs.push_back(decimate_length(src_bs[i])); }

    size_t ni0 = src_bs[0];
    size_t nj0 = src_bs[1];

    size_t ni1 = dst_bs[0];
    size_t nj1 = dst_bs[1];

    for (size_t j = 0; j < nj1; j++) {
        for (size_t i = 0; i < ni1; i++) { dst[j * ni1 + i] = 0.0; }
    }

    // Perform in-place 4-node averaging, handling odd boundary cases
    //
    for (size_t j = 0, jj = 0; j < nj0; j++) {
        float jwgt = 0.0;
        if (j == nj0 - 1 && nj0 % 2) jwgt = 0.25;    // odd j boundary

        for (size_t i = 0, ii = 0; i < ni0; i++) {
            float iwgt = 0.25;
            if (i == ni0 - 1 && ni0 % 2) iwgt = 0.5;    // odd i boundary
            float w = iwgt + jwgt;

            assert(w <= 1.0);

            dst[jj * ni1 + ii] += w * src[j * ni0 + i];
            if (i % 2) ii++;
        }
        if (j % 2) jj++;
    }
}

template<typename T> void decimate3d(const vector<size_t> &src_bs, const T *src, T *dst)
{
    assert(src_bs.size() == 3);

    vector<size_t> dst_bs;
    for (int i = 0; i < src_bs.size(); i++) { dst_bs.push_back(decimate_length(src_bs[i])); }

    size_t ni0 = src_bs[0];
    size_t nj0 = src_bs[1];
    size_t nk0 = src_bs[2];

    size_t ni1 = dst_bs[0];
    size_t nj1 = dst_bs[1];
    size_t nk1 = dst_bs[2];

    for (size_t k = 0; k < nk1; k++) {
        for (size_t j = 0; j < nj1; j++) {
            for (size_t i = 0; i < ni1; i++) { dst[k * ni1 * nj1 + j * ni1 + i] = 0.0; }
        }
    }

    // Perform in-place 8-node averaging, handling odd boundary cases
    //
    for (size_t k = 0, kk = 0; k < nk0; k++) {
        float kwgt = 0.0;
        if (k == nk0 - 1 && nk0 % 2) kwgt = 0.125;    // odd k boundary

        for (size_t j = 0, jj = 0; j < nj0; j++) {
            float jwgt = 0.0;
            if (j == nj0 - 1 && nj0 % 2) jwgt = 0.125;    // odd j boundary

            for (size_t i = 0, ii = 0; i < ni0; i++) {
                float iwgt = 0.125;
                if (i == ni0 - 1 && ni0 % 2) iwgt = 0.25;    // odd i boundary
                float w = iwgt + jwgt + kwgt;

                assert(w <= 1.0);

                dst[kk * ni1 * nj1 + jj * ni1 + ii] += w * src[k * ni0 * nj0 + j * ni0 + i];
                if (i % 2) ii++;
            }
            if (j % 2) jj++;
        }
        if (k % 2) kk++;
    }
}

// Perform decimation of a 1D, 2D, or 3D blocked array
//
template<typename T> void decimate(const vector<size_t> &bmin, const vector<size_t> &bmax, const vector<size_t> &src_bs, const T *src, T *dst)
{
    assert(bmin.size() == bmax.size());
    assert(bmin.size() == src_bs.size());
    assert(src_bs.size() >= 1 && src_bs.size() <= 3);

    vector<size_t> dst_bs;
    for (int i = 0; i < src_bs.size(); i++) { dst_bs.push_back(decimate_length(src_bs[i])); }

    size_t src_block_size = vproduct(src_bs);
    size_t dst_block_size = vproduct(dst_bs);

    if (src_bs.size() == 1) {
        for (int i = bmin[0]; i <= bmax[0]; i++) {
            decimate1d(src_bs, src, dst);
            src += src_block_size;
            dst += dst_block_size;
        }
    } else if (src_bs.size() == 2) {
        for (int j = bmin[1]; j <= bmax[1]; j++) {
            for (int i = bmin[0]; i <= bmax[0]; i++) {
                decimate2d(src_bs, src, dst);
                src += src_block_size;
                dst += dst_block_size;
            }
        }
    } else {
        for (int k = bmin[2]; k <= bmax[2]; k++) {
            for (int j = bmin[1]; j <= bmax[1]; j++) {
                for (int i = bmin[0]; i <= bmax[0]; i++) {
                    decimate3d(src_bs, src, dst);
                    src += src_block_size;
                    dst += dst_block_size;
                }
            }
        }
    }
}

// Map voxel to block coordinates
//
void map_vox_to_blk(vector<size_t> bs, const vector<size_t> &vcoord, vector<size_t> &bcoord)
{
    assert(bs.size() == vcoord.size());
    bcoord.clear();

    for (int i = 0; i < bs.size(); i++) { bcoord.push_back(vcoord[i] / bs[i]); }
}

void map_blk_to_vox(vector<size_t> bs, const vector<size_t> &bmin, const vector<size_t> &bmax, vector<size_t> &vmin, vector<size_t> &vmax)
{
    assert(bs.size() == bmin.size());
    assert(bs.size() == bmax.size());
    vmin.clear();
    vmax.clear();

    for (int i = 0; i < bs.size(); i++) {
        vmin.push_back(bmin[i] * bs[i]);
        vmax.push_back(bmax[i] * bs[i] + bs[i] - 1);
    }
}

#ifdef UNUSED_FUNCTION
void coord_setup_helper(const vector<string> &dimnames, const vector<size_t> &dims, const vector<size_t> &dims_at_level, const vector<size_t> &bs, const vector<size_t> &bs_at_level,
                        const vector<size_t> &bmin, const vector<size_t> &bmax, const vector<string> &my_dimnames, vector<size_t> &my_dims, vector<size_t> &my_dims_at_level, vector<size_t> &my_bs,
                        vector<size_t> &my_bs_at_level, vector<size_t> &my_bmin, vector<size_t> &my_bmax)
{
    assert(dimnames.size() == dims.size());
    assert(dimnames.size() == dims_at_level.size());
    assert(dimnames.size() >= bs.size());
    assert(dimnames.size() >= bs_at_level.size());
    assert(dimnames.size() >= bmin.size());
    assert(dimnames.size() >= bmax.size());

    my_dims.clear();
    my_dims_at_level.clear();
    my_bs.clear();
    my_bs_at_level.clear();
    my_bmin.clear();
    my_bmax.clear();

    // Assumes mydimnames is an ordered subset of dimnames
    //
    for (int i = 0; i < dimnames.size(); i++) {
        for (int j = 0; j < my_dimnames.size(); j++) {
            if (dimnames[i] == my_dimnames[j]) {
                my_dims.push_back(dims[i]);
                my_dims_at_level.push_back(dims_at_level[i]);
                my_bs.push_back(bs[i]);
                my_bs_at_level.push_back(bs_at_level[i]);
                my_bmin.push_back(bmin[i]);
                my_bmax.push_back(bmax[i]);
            }
        }
    }
}
#endif

};    // namespace

DataMgr::DataMgr(string format, size_t mem_size, int nthreads)
{
    SetDiagMsg("DataMgr::DataMgr(%s,%d,%d)", format.c_str(), nthreads, mem_size);

    _format = format;
    _nthreads = nthreads;
    _mem_size = mem_size;

    if (!_mem_size) _mem_size = 100;

    _dc = NULL;

    _blk_mem_mgr = NULL;

    _PipeLines.clear();

    _regionsList.clear();

    _varInfoCache.Clear();

    _doTransformHorizontal = false;
    _doTransformVertical = false;
    _openVarName.clear();
    _proj4String.clear();
    _proj4StringDefault.clear();
}

DataMgr::~DataMgr()
{
    SetDiagMsg("DataMgr::~DataMgr()");

    if (_dc) delete _dc;
    _dc = NULL;

    Clear();
    if (_blk_mem_mgr) delete _blk_mem_mgr;

    _blk_mem_mgr = NULL;

    vector<string> names = _dvm.GetDataVarNames();
    for (int i = 0; i < names.size(); i++) {
        if (_dvm.GetVar(names[i])) delete _dvm.GetVar(names[i]);
    }

    names = _dvm.GetCoordVarNames();
    for (int i = 0; i < names.size(); i++) {
        if (_dvm.GetVar(names[i])) delete _dvm.GetVar(names[i]);
    }
}

int DataMgr::_parseOptions(vector<string> &options)
{
    vector<string> newOptions;
    bool           ok = true;
    int            i = 0;
    while (i < options.size() && ok) {
        if (options[i] == "-proj4") {
            i++;
            if (i >= options.size()) {
                ok = false;
            } else {
                _proj4String = options[i];
            }
        }
        if (options[i] == "-project_to_pcs") { _doTransformHorizontal = true; }
        if (options[i] == "-vertical_xform") {
            _doTransformVertical = true;
        } else {
            newOptions.push_back(options[i]);
        }
        i++;
    }

    options = newOptions;

    if (!ok) {
        SetErrMsg("Error parsing options");
        return (-1);
    }
    return (0);
}

int DataMgr::Initialize(const vector<string> &files, const std::vector<string> &options)
{
    vector<string> deviceOptions = options;
    int            rc = _parseOptions(deviceOptions);
    if (rc < 0) return (-1);

    Clear();
    if (_dc) delete _dc;

    _dc = NULL;
    if (files.empty()) {
        SetErrMsg("Empty file list");
        return (-1);
    }

    if (_format.compare("vdc") == 0) {
        _dc = new VDCNetCDF(_nthreads);
    } else if (_format.compare("wrf") == 0) {
        _dc = new DCWRF();
    } else if (_format.compare("cf") == 0) {
        _dc = new DCCF();
    } else if (_format.compare("mpas") == 0) {
        _dc = new DCMPAS();
    } else {
        SetErrMsg("Invalid data collection format : %s", _format.c_str());
        return (-1);
    }

    rc = _dc->Initialize(files, deviceOptions);
    if (rc < 0) {
        SetErrMsg("Failed to initialize data importer");
        return (-1);
    }

    // Use UDUnits for unit conversion
    //
    rc = _udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", _udunits.GetErrMsg().c_str());
        return (-1);
    }

    rc = _initHorizontalCoordVars();
    if (rc < 0) {
        SetErrMsg("Failed to initialize horizontal coordinates");
        return (-1);
    }

    rc = _initVerticalCoordVars();
    if (rc < 0) {
        SetErrMsg("Failed to initialize horizontal coordinates");
        return (-1);
    }

    rc = _initTimeCoord();
    if (rc < 0) {
        SetErrMsg("Failed to get time coordinates");
        return (-1);
    }
    return (0);
}

bool DataMgr::GetMesh(string meshname, DC::Mesh &m) const
{
    assert(_dc);

    bool ok = _dvm.GetMesh(meshname, m);
    if (!ok) { ok = _dc->GetMesh(meshname, m); }

    if (!ok) return (ok);

    // Make sure the number of coordinate variables is greater or
    // equal to the topological dimension. If not, add default coordinate
    // variables
    //
    vector<string> coord_vars = m.GetCoordVars();
    while (coord_vars.size() < m.GetTopologyDim()) {
        if (!_hasCoordForAxis(coord_vars, 0)) {
            coord_vars.insert(coord_vars.begin() + 0, _defaultCoordVar(m, 0));
            continue;
        } else if (!_hasCoordForAxis(coord_vars, 1)) {
            coord_vars.insert(coord_vars.begin() + 1, _defaultCoordVar(m, 1));
            continue;
        } else {
            coord_vars.insert(coord_vars.begin() + 2, _defaultCoordVar(m, 2));
            continue;
        }
    }

    // if requested, replace native horizontal geographic coordiate variables
    // with derived PCS coordinate variables
    //
    if (_doTransformHorizontal) { _assignHorizontalCoords(coord_vars); }

    m.SetCoordVars(coord_vars);

    return (true);
}

vector<string> DataMgr::GetDataVarNames() const
{
    assert(_dc);

    vector<string> validvars;
    for (int ndim = 2; ndim <= 3; ndim++) {
        vector<string> vars = GetDataVarNames(ndim);
        validvars.insert(validvars.end(), vars.begin(), vars.end());
    }
    return (validvars);
}

vector<string> DataMgr::GetDataVarNames(int ndim) const
{
    assert(_dc);

    vector<string> vars = _dc->GetDataVarNames(ndim);
    vector<string> derived_vars = _getDataVarNamesDerived(ndim);
    vars.insert(vars.end(), derived_vars.begin(), derived_vars.end());

    vector<string> validVars;
    for (int i = 0; i < vars.size(); i++) {
        // If we don't have a grid class to support this variable reject it
        //
        if (_get_grid_type(vars[i]).empty()) continue;

        vector<string> coordvars;
        GetVarCoordVars(vars[i], true, coordvars);
        if (coordvars.size() < ndim) continue;

        validVars.push_back(vars[i]);
    }
    return (validVars);
}

vector<string> DataMgr::GetCoordVarNames() const
{
    assert(_dc);

    vector<string> vars = _dc->GetCoordVarNames();
    vector<string> derived_vars = _dvm.GetCoordVarNames();
    vars.insert(vars.end(), derived_vars.begin(), derived_vars.end());

    return (vars);
}

string DataMgr::GetTimeCoordVarName() const
{
    assert(_dc);

    // There can be only one time coordinate variable. If a
    // derived one exists, use it.
    //
    vector<string> cvars = _dvm.GetTimeCoordVarNames();
    if (!cvars.empty()) return (cvars[0]);

    cvars = _dc->GetTimeCoordVarNames();
    assert(cvars.size());

    return (cvars[0]);
}

bool DataMgr::GetVarCoordVars(string varname, bool spatial, std::vector<string> &coord_vars) const
{
    assert(_dc);

    coord_vars.clear();

    DC::DataVar dvar;
    bool        status = GetDataVarInfo(varname, dvar);
    if (!status) return (false);

    DC::Mesh m;
    status = GetMesh(dvar.GetMeshName(), m);
    if (!status) return (false);

    coord_vars = m.GetCoordVars();

    if (spatial) return (true);

    if (!dvar.GetTimeCoordVar().empty()) { coord_vars.push_back(dvar.GetTimeCoordVar()); }

    return (true);
}

bool DataMgr::GetDataVarInfo(string varname, VAPoR::DC::DataVar &var) const
{
    assert(_dc);

    bool ok = _dvm.GetDataVarInfo(varname, var);
    if (!ok) { ok = _dc->GetDataVarInfo(varname, var); }
    if (!ok) return (ok);

    // Replace native time coordinate variables that are not expressed
    // in units of seconds with derived variables having units of seconds
    //
    string time_coord_var = var.GetTimeCoordVar();

    _assignTimeCoord(time_coord_var);

    var.SetTimeCoordVar(time_coord_var);

    return (true);
}

bool DataMgr::GetCoordVarInfo(string varname, VAPoR::DC::CoordVar &var) const
{
    assert(_dc);

    bool ok = _dvm.GetCoordVarInfo(varname, var);
    if (!ok) { ok = _dc->GetCoordVarInfo(varname, var); }
    return (ok);
}

bool DataMgr::GetBaseVarInfo(string varname, VAPoR::DC::BaseVar &var) const
{
    assert(_dc);

    bool ok = _dvm.GetBaseVarInfo(varname, var);
    if (!ok) { ok = _dc->GetBaseVarInfo(varname, var); }
    return (ok);
}

bool DataMgr::IsTimeVarying(string varname) const
{
    assert(_dc);

    // If var is a data variable and has a time coordinate variable defined
    //
    DC::DataVar dvar;
    bool        ok = GetDataVarInfo(varname, dvar);
    if (ok) return (!dvar.GetTimeCoordVar().empty());

    // If var is a coordinate variable and it has a time dimension
    //
    DC::CoordVar cvar;
    ok = GetCoordVarInfo(varname, cvar);
    if (ok) return (!cvar.GetTimeDimName().empty());

    return (false);
}

bool DataMgr::IsCompressed(string varname) const
{
    assert(_dc);

    DC::BaseVar var;

    // No error checking here!!!!!
    //
    bool status = GetBaseVarInfo(varname, var);
    if (!status) return (status);

    return (var.IsCompressed());
}

int DataMgr::GetNumTimeSteps(string varname) const
{
    assert(_dc);

    // If data variable get it's time coordinate variable if it exists
    //
    if (_isDataVar(varname)) {
        DC::DataVar var;
        bool        ok = GetDataVarInfo(varname, var);
        if (!ok) return (0);

        string time_coord_var = var.GetTimeCoordVar();
        if (time_coord_var.empty()) return (1);
        varname = time_coord_var;
    }

    DC::CoordVar var;
    bool         ok = GetCoordVarInfo(varname, var);
    if (!ok) return (0);

    string time_dim_name = var.GetTimeDimName();
    if (time_dim_name.empty()) return (1);

    DC::Dimension dim;
    ok = GetDimension(time_dim_name, dim);
    if (!ok) return (0);

    return (dim.GetLength());
}

int DataMgr::GetNumTimeSteps() const { return (_timeCoordinates.size()); }

size_t DataMgr::GetNumRefLevels(string varname) const
{
    assert(_dc);

    if (varname == "") return 1;

    if (IsVariableDerived(varname)) return (1);

    return (_dc->GetNumRefLevels(varname));
}

vector<size_t> DataMgr::GetCRatios(string varname) const
{
    assert(_dc);

    DC::BaseVar var;
    int         rc = GetBaseVarInfo(varname, var);
    if (rc < 0) return (vector<size_t>(1, 1));

    return (var.GetCRatios());
}

Grid *DataMgr::GetVariable(size_t ts, string varname, int level, int lod, bool lock)
{
    SetDiagMsg("DataMgr::GetVariable(%d,%s,%d,%d,%d, %d)", ts, varname.c_str(), level, lod, lock);

    int rc = _level_correction(varname, level);
    if (rc < 0) return (NULL);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (NULL);

    Grid *rg = _getVariable(ts, varname, level, lod, lock, false);
    if (!rg) {
        SetErrMsg("Failed to read variable \"%s\" at time step (%d), and\n"
                  "refinement level (%d) and level-of-detail (%d)",
                  varname.c_str(), ts, level, lod);
    }
    return (rg);
}

Grid *DataMgr::GetVariable(size_t ts, string varname, int level, int lod, vector<double> min, vector<double> max, bool lock)
{
    assert(min.size() == max.size());

    SetDiagMsg("DataMgr::GetVariable(%d, %s, %d, %d, %s, %s, %d)", ts, varname.c_str(), level, lod, vector_to_string(min).c_str(), vector_to_string(max).c_str(), lock);

    int rc = _level_correction(varname, level);
    if (rc < 0) return (NULL);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (NULL);

    // Make sure variable dimensions match extents specification
    //
    vector<string> coord_vars;
    bool           ok = GetVarCoordVars(varname, true, coord_vars);
    assert(ok);

    while (min.size() > coord_vars.size()) {
        min.pop_back();
        max.pop_back();
    }

    //
    // Find the coordinates in voxels of the grid that contains
    // the axis aligned bounding box specified in user coordinates
    // by min and max
    //
    vector<size_t> min_ui, max_ui;
    rc = _find_bounding_grid(ts, varname, level, lod, min, max, min_ui, max_ui);
    if (rc < 0) return (NULL);

    if (!min_ui.size()) {
        // Why not return NULL?
        //
        return (new RegularGrid());
    }

    return (DataMgr::GetVariable(ts, varname, level, lod, min_ui, max_ui));
}

Grid *DataMgr::_getVariable(size_t ts, string varname, int level, int lod, bool lock, bool dataless)
{
    if (!VariableExists(ts, varname, level, lod)) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (NULL);
    }

    vector<size_t> dims_at_level;
    vector<size_t> dummy;
    int            rc = DataMgr::GetDimLensAtLevel(varname, level, dims_at_level, dummy);
    if (rc < 0) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (NULL);
    }

    vector<size_t> min;
    vector<size_t> max;
    for (int i = 0; i < dims_at_level.size(); i++) {
        min.push_back(0);
        max.push_back(dims_at_level[i] - 1);
    }

    return (DataMgr::_getVariable(ts, varname, level, lod, min, max, lock, dataless));
}

// Find the subset of the data dimension that are the coord dimensions
//
void DataMgr::_setupCoordVecsHelper(string data_varname, const vector<size_t> &data_bmin, const vector<size_t> &data_bmax, string coord_varname, vector<size_t> &coord_bmin,
                                    vector<size_t> &coord_bmax) const
{
    assert(data_bmin.size() == data_bmax.size());
    coord_bmin.clear();
    coord_bmax.clear();

    vector<DC::Dimension> data_dims;
    bool                  ok = _getVarDimensions(data_varname, data_dims);
    assert(ok);
    assert(data_dims.size() == data_bmin.size());

    vector<DC::Dimension> coord_dims;
    ok = _getVarDimensions(coord_varname, coord_dims);
    assert(ok);

    int i = 0;
    for (int j = 0; j < coord_dims.size(); j++) {
        while (data_dims[i].GetLength() != coord_dims[j].GetLength() && i < data_dims.size()) { i++; }
        assert(i < data_dims.size());
        coord_bmin.push_back(data_bmin[i]);
        coord_bmax.push_back(data_bmax[i]);
    }
}

int DataMgr::_setupCoordVecs(size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, vector<string> &varnames, vector<size_t> &roi_dims,
                             vector<vector<size_t>> &dims_at_levelvec, vector<vector<size_t>> &bsvec, vector<vector<size_t>> &bs_at_levelvec, vector<vector<size_t>> &bminvec,
                             vector<vector<size_t>> &bmaxvec) const
{
    varnames.clear();
    roi_dims.clear();
    dims_at_levelvec.clear();
    bsvec.clear();
    bs_at_levelvec.clear();
    bminvec.clear();
    bmaxvec.clear();

    // Compute dimenions of ROI
    //
    for (int i = 0; i < min.size(); i++) { roi_dims.push_back(max[i] - min[i] + 1); }

    vector<size_t> dims;
    vector<size_t> bs;
    int            rc = DataMgr::GetDimLensAtLevel(varname, -1, dims, bs);
    assert(rc >= 0);
    bsvec.push_back(bs);

    // Grid and block dimensions at requested refinement
    //
    vector<size_t> bs_at_level;
    vector<size_t> dims_at_level;
    rc = DataMgr::GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level);
    assert(rc >= 0);
    dims_at_levelvec.push_back(dims_at_level);
    bs_at_levelvec.push_back(bs_at_level);

    // Map voxel coordinates into block coordinates
    //
    vector<size_t> bmin, bmax;
    map_vox_to_blk(bs_at_level, min, bmin);
    map_vox_to_blk(bs_at_level, max, bmax);
    bminvec.push_back(bmin);
    bmaxvec.push_back(bmax);

    vector<string> cvarnames;
    bool           ok = GetVarCoordVars(varname, true, cvarnames);
    assert(ok);

    for (int i = 0; i < cvarnames.size(); i++) {
        vector<size_t> dims;
        vector<size_t> bs;
        int            rc = DataMgr::GetDimLensAtLevel(cvarnames[i], -1, dims, bs);
        assert(rc >= 0);

        // Grid and block dimensions at requested refinement
        //
        vector<size_t> bs_at_level;
        vector<size_t> dims_at_level;
        rc = DataMgr::GetDimLensAtLevel(cvarnames[i], level, dims_at_level, bs_at_level);
        assert(rc >= 0);

        // Map data indices to coordinate indices. Coordinate indices
        // are a subset of the data indices.
        //
        vector<size_t> coord_bmin, coord_bmax;
        vector<size_t> coord_dims_at_level, coord_bs_at_level, coord_bs;
        _setupCoordVecsHelper(varname, bmin, bmax, cvarnames[i], coord_bmin, coord_bmax);

        dims_at_levelvec.push_back(dims_at_level);
        bsvec.push_back(bs);
        bs_at_levelvec.push_back(bs_at_level);
        bminvec.push_back(coord_bmin);
        bmaxvec.push_back(coord_bmax);
    }

    varnames.push_back(varname);
    varnames.insert(varnames.end(), cvarnames.begin(), cvarnames.end());

    return (0);
}

int DataMgr::_setupConnVecs(size_t ts, string varname, int level, int lod, vector<string> &varnames, vector<vector<size_t>> &dims_at_levelvec, vector<vector<size_t>> &bsvec,
                            vector<vector<size_t>> &bs_at_levelvec, vector<vector<size_t>> &bminvec, vector<vector<size_t>> &bmaxvec) const
{
    varnames.clear();
    dims_at_levelvec.clear();
    bsvec.clear();
    bs_at_levelvec.clear();
    bminvec.clear();
    bmaxvec.clear();

    string face_node_var;
    string node_face_var;
    string face_edge_var;
    string face_face_var;
    string edge_node_var;
    string edge_face_var;

    bool ok = _getVarConnVars(varname, face_node_var, node_face_var, face_edge_var, face_face_var, edge_node_var, edge_face_var);
    if (!ok) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (-1);
    }

    if (!face_node_var.empty()) varnames.push_back(face_node_var);
    if (!node_face_var.empty()) varnames.push_back(node_face_var);
    if (!face_edge_var.empty()) varnames.push_back(face_edge_var);
    if (!face_face_var.empty()) varnames.push_back(face_face_var);
    if (!edge_node_var.empty()) varnames.push_back(edge_node_var);
    if (!edge_face_var.empty()) varnames.push_back(edge_face_var);

    for (int i = 0; i < varnames.size(); i++) {
        string name = varnames[i];

        vector<size_t> dims;
        vector<size_t> bs;
        int            rc = DataMgr::GetDimLensAtLevel(name, -1, dims, bs);
        if (rc < 0) {
            SetErrMsg("Invalid variable reference : %s", name.c_str());
            return (-1);
        }

        // Grid and block dimensions at requested refinement
        //
        vector<size_t> bs_at_level;
        vector<size_t> dims_at_level;
        rc = DataMgr::GetDimLensAtLevel(name, level, dims_at_level, bs_at_level);

        if (rc < 0) {
            SetErrMsg("Invalid variable reference : %s", name.c_str());
            return (-1);
        }

        vector<size_t> conn_min = vector<size_t>(dims_at_level.size(), 0);
        vector<size_t> conn_max = dims_at_level;
        for (int i = 0; i < conn_max.size(); i++) { conn_max[i]--; }

        // Map voxel coordinates into block coordinates
        //
        vector<size_t> bmin, bmax;
        map_vox_to_blk(bs_at_level, conn_min, bmin);
        map_vox_to_blk(bs_at_level, conn_max, bmax);

        bsvec.push_back(bs);
        dims_at_levelvec.push_back(dims_at_level);
        bs_at_levelvec.push_back(bs_at_level);
        bminvec.push_back(bmin);
        bmaxvec.push_back(bmax);
    }

    return (0);
}

Grid *DataMgr::_getVariable(size_t ts, string varname, int level, int lod, vector<size_t> min, vector<size_t> max, bool lock, bool dataless)
{
    Grid *rg = NULL;

    string gridType = _get_grid_type(varname);
    if (gridType.empty()) {
        SetErrMsg("Unrecognized grid type for variable %s", varname.c_str());
        return (NULL);
    }

    DC::DataVar dvar;
    bool        status = DataMgr::GetDataVarInfo(varname, dvar);
    assert(status);

    vector<DC::CoordVar> cvarsinfo;
    DC::CoordVar         dummy;
    status = _get_coord_vars(varname, cvarsinfo, dummy);
    assert(status);

    vector<string>         varnames;
    vector<size_t>         roi_dims;
    vector<vector<size_t>> dims_at_levelvec;
    vector<vector<size_t>> bsvec;
    vector<vector<size_t>> bs_at_levelvec;
    vector<vector<size_t>> bminvec;
    vector<vector<size_t>> bmaxvec;

    // Get dimensions for coordinate variables
    //
    int rc = _setupCoordVecs(ts, varname, level, lod, min, max, varnames, roi_dims, dims_at_levelvec, bsvec, bs_at_levelvec, bminvec, bmaxvec);
    if (rc < 0) return (NULL);

    //
    // if dataless we only load coordinate data
    //
    if (dataless) varnames[0].clear();

    vector<float *> blkvec;
    rc = DataMgr::_get_regions<float>(ts, varnames, level, lod, true, bsvec, bminvec, bmaxvec, blkvec);
    if (rc < 0) return (NULL);

    // Get dimensions for connectivity variables (if any)
    //
    vector<string>         conn_varnames;
    vector<vector<size_t>> conn_dims_at_levelvec;
    vector<vector<size_t>> conn_bsvec;
    vector<vector<size_t>> conn_bs_at_levelvec;
    vector<vector<size_t>> conn_bminvec;
    vector<vector<size_t>> conn_bmaxvec;

    vector<int *> conn_blkvec;
    if (_gridHelper.IsUnstructured(gridType)) {
        rc = _setupConnVecs(ts, varname, level, lod, conn_varnames, conn_dims_at_levelvec, conn_bsvec, conn_bs_at_levelvec, conn_bminvec, conn_bmaxvec);
        if (rc < 0) return (NULL);

        rc = DataMgr::_get_regions<int>(ts, conn_varnames, level, lod, true, conn_bsvec, conn_bminvec, conn_bmaxvec, conn_blkvec);
        if (rc < 0) return (NULL);
    }

    if (DataMgr::IsVariableDerived(varname) && !blkvec[0]) {
        //
        // Derived variable that is not in cache, so we need to
        // create it
        //
#ifdef VAPOR3_0_0_ALPHA
        rg = execute_pipeline(ts, varname, level, lod, min, max, lock, xcblks, ycblks, zcblks);

        if (!rg) {
            for (int i = 0; i < blkvec.size(); i++) {
                if (blkvec[i]) _unlock_blocks(blkvec[i]);
            }
            return (NULL);
        }
#endif
    } else {
        if (_gridHelper.IsUnstructured(gridType)) {
            vector<size_t>             vertexDims;
            vector<size_t>             faceDims;
            vector<size_t>             edgeDims;
            UnstructuredGrid::Location location;
            size_t                     maxVertexPerFace;
            size_t                     maxFacePerVertex;
            long                       vertexOffset;
            long                       faceOffset;

            _ugrid_setup(dvar, vertexDims, faceDims, edgeDims, location, maxVertexPerFace, maxFacePerVertex, vertexOffset, faceOffset);

            rg = _gridHelper.MakeGridUnstructured(gridType, ts, level, lod, dvar, cvarsinfo, roi_dims, dims_at_levelvec[0], blkvec, bs_at_levelvec, bminvec, bmaxvec, conn_blkvec, conn_bs_at_levelvec,
                                                  conn_bminvec, conn_bmaxvec, vertexDims, faceDims, edgeDims, location, maxVertexPerFace, maxFacePerVertex, vertexOffset, faceOffset);
        } else {
            rg = _gridHelper.MakeGridStructured(gridType, ts, level, lod, dvar, cvarsinfo, roi_dims, dims_at_levelvec[0], blkvec, bs_at_levelvec, bminvec, bmaxvec);
        }
        assert(rg);
    }

    //
    // Safe to remove locks now that were not explicitly requested
    //
    if (!lock) {
        for (int i = 0; i < blkvec.size(); i++) {
            if (blkvec[i]) _unlock_blocks(blkvec[i]);
        }
        for (int i = 0; i < conn_blkvec.size(); i++) {
            if (conn_blkvec[i]) _unlock_blocks(conn_blkvec[i]);
        }
    }

    return (rg);
}

Grid *DataMgr::GetVariable(size_t ts, string varname, int level, int lod, vector<size_t> min, vector<size_t> max, bool lock)
{
    assert(min.size() == max.size());

    SetDiagMsg("DataMgr::GetVariable(%d, %s, %d, %d, %s, %s, %d)", ts, varname.c_str(), level, lod, vector_to_string(min).c_str(), vector_to_string(max).c_str(), lock);

    int rc = _level_correction(varname, level);
    if (rc < 0) return (NULL);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (NULL);

    // Make sure variable dimensions match extents specification
    //
    vector<string> coord_vars;
    bool           ok = GetVarCoordVars(varname, true, coord_vars);
    assert(ok);

    while (min.size() > coord_vars.size()) {
        min.pop_back();
        max.pop_back();
    }

    Grid *rg = _getVariable(ts, varname, level, lod, min, max, lock, false);
    if (!rg) {
        SetErrMsg("Failed to read variable \"%s\" at time step (%d), and\n"
                  "refinement level (%d) and level-of-detail (%d)",
                  varname.c_str(), ts, level, lod);
    }
    return (rg);
}

int DataMgr::GetVariableExtents(size_t ts, string varname, int level, vector<double> &min, vector<double> &max)
{
    min.clear();
    max.clear();

    int rc = _level_correction(varname, level);
    if (rc < 0) return (-1);

    vector<string> cvars;
    string         dummy;
    bool           ok = _get_coord_vars(varname, cvars, dummy);
    if (!ok) return (-1);

    string         key = "VariableExtents";
    vector<double> values;
    if (_varInfoCache.Get(ts, cvars, level, 0, key, values)) {
        int n = values.size();
        for (int i = 0; i < n / 2; i++) {
            min.push_back(values[i]);
            max.push_back(values[i + (n / 2)]);
        }
        return (0);
    }

    Grid *rg = _getVariable(ts, varname, level, -1, false, true);
    if (!rg) return (-1);

    rg->GetUserExtents(min, max);

    // Cache results
    //
    values.clear();
    for (int i = 0; i < min.size(); i++) values.push_back(min[i]);
    for (int i = 0; i < max.size(); i++) values.push_back(max[i]);
    _varInfoCache.Set(ts, cvars, level, 0, key, values);

    return (0);
}

int DataMgr::GetDataRange(size_t ts, string varname, int level, int lod, vector<double> &range)
{
    SetDiagMsg("DataMgr::GetDataRange(%d,%s)", ts, varname.c_str());
    range.clear();

    int rc = _level_correction(varname, level);
    if (rc < 0) return (-1);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (-1);

    // See if we've already cache'd it.
    //
    string key = "VariableRange";
    if (_varInfoCache.Get(ts, varname, level, lod, key, range)) {
        assert(range.size() == 2);
        return (0);
    }

    const Grid *sg = DataMgr::GetVariable(ts, varname, level, lod, false);
    if (!sg) return (-1);

    //
    // Have to calculate range
    //

    range.clear();
    range.push_back(0.0);
    range.push_back(0.0);
    bool                first = true;
    float               mv = sg->GetMissingValue();
    Grid::ConstIterator itr;
    Grid::ConstIterator enditr = sg->cend();
    for (itr = sg->cbegin(); itr != enditr; ++itr) {
        float v = *itr;
        if (v != mv) {
            if (first) {
                range[0] = range[1] = v;
                first = false;
            }
            if (v < range[0]) range[0] = v;
            if (v > range[1]) range[1] = v;
        }
    }
    delete sg;

    _varInfoCache.Set(ts, varname, level, lod, key, range);

    return (0);
}

int DataMgr::GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    assert(_dc);

    DerivedVar *dvar = _getDerivedVar(varname);
    if (dvar) { return (dvar->GetDimLensAtLevel(level, dims_at_level, bs_at_level)); }

    return (_dc->GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level));

    return (0);
}

#ifdef VAPOR3_0_0_ALPHA

int DataMgr::NewPipeline(PipeLine *pipeline)
{
    //
    // Delete any pipeline stage with the same name as the new one. This
    // is a no-op if the stage doesn't exist.
    //
    RemovePipeline(pipeline->GetName());

    //
    // Make sure outputs don't collide with existing outputs
    //
    const vector<pair<string, VarType_T>> &my_outputs = pipeline->GetOutputs();

    for (int i = 0; i < _PipeLines.size(); i++) {
        const vector<pair<string, VarType_T>> &outputs = _PipeLines[i]->GetOutputs();
        for (int j = 0; j < my_outputs.size(); j++) {
            for (int k = 0; k < outputs.size(); k++) {
                if (my_outputs[j].first.compare(outputs[k].first) == 0) {
                    SetErrMsg("Pipeline output %s already in use", my_outputs[j].first.c_str());
                    return (-1);
                }
            }
        }
    }

    //
    // Now make sure outputs don't match any native variables
    //
    vector<string> native_vars = _get_native_variables();

    for (int i = 0; i < native_vars.size(); i++) {
        for (int j = 0; j < my_outputs.size(); j++) {
            if (native_vars[i].compare(my_outputs[j].first) == 0) {
                SetErrMsg("Pipeline output %s matches native variable name", my_outputs[i].first.c_str());
                return (-1);
            }
        }
    }

    //
    // Add the new stage to a temporary pipeline. Generate a hash
    // table with all the dependencies of the temporary pipeline. And
    // then check for cycles in the graph (e.g. a -> b -> c -> a).
    //
    map<string, vector<string>> graph;
    vector<PipeLine *>          tmp_pipe = _PipeLines;
    tmp_pipe.push_back(pipeline);

    for (int i = 0; i < tmp_pipe.size(); i++) {
        vector<string> depends;
        for (int j = 0; j < tmp_pipe.size(); j++) {
            //
            // See if inputs to tmp_pipe[i] match outputs
            // of tmp_pipe[j]
            //
            if (depends_on(tmp_pipe[i], tmp_pipe[j])) { depends.push_back(tmp_pipe[j]->GetName()); }
        }
        graph[tmp_pipe[i]->GetName()] = depends;
    }

    //
    // Finally check for cycles in the graph
    //
    if (cycle_check(graph, pipeline->GetName(), graph[pipeline->GetName()])) {
        SetErrMsg("Invalid pipeline : circular dependency detected");
        return (-1);
    }

    _PipeLines.push_back(pipeline);
    return (0);
}

void DataMgr::RemovePipeline(string name)
{
    vector<PipeLine *>::iterator itr;
    for (itr = _PipeLines.begin(); itr != _PipeLines.end(); itr++) {
        if (name.compare((*itr)->GetName()) == 0) {
            _PipeLines.erase(itr);

            const vector<pair<string, VarType_T>> &output_vars = (*itr)->GetOutputs();
            //
            // Remove any cached instances of variable
            //
            for (int i = 0; i < output_vars.size(); i++) {
                string var = output_vars[i].first;
                DataMgr::PurgeVariable(var);
            }
            break;
        }
    }
}

#endif

vector<string> DataMgr::_get_var_dependencies(string varname) const
{
    vector<string> varnames = {varname};

    // No dependencies
    //
    if (!_isDataVar(varname)) return (varnames);

    vector<string> cvars;
    bool           ok = GetVarCoordVars(varname, false, cvars);
    if (ok) {
        for (int i = 0; i < cvars.size(); i++) varnames.push_back(cvars[i]);
    }

    // Test for connectivity variables, if any
    //
    string face_node_var;
    string node_face_var;
    string face_edge_var;
    string face_face_var;
    string edge_node_var;
    string edge_face_var;
    ok = _getVarConnVars(varname, face_node_var, node_face_var, face_edge_var, face_face_var, edge_node_var, edge_face_var);
    if (ok) {
        if (!face_node_var.empty()) varnames.push_back(face_node_var);
        if (!node_face_var.empty()) varnames.push_back(node_face_var);
        if (!face_edge_var.empty()) varnames.push_back(face_edge_var);
        if (!face_face_var.empty()) varnames.push_back(face_face_var);
        if (!edge_node_var.empty()) varnames.push_back(edge_node_var);
        if (!edge_face_var.empty()) varnames.push_back(edge_face_var);
    }

    return (varnames);
}

bool DataMgr::VariableExists(size_t ts, string varname, int level, int lod) const
{
    if (varname.empty()) return (false);

    // disable error reporting
    //
    bool enabled = EnableErrMsg(false);
    int  rc = _level_correction(varname, level);
    if (rc < 0) {
        EnableErrMsg(enabled);
        return (false);
    }

    rc = _lod_correction(varname, lod);
    if (rc < 0) {
        EnableErrMsg(enabled);
        return (false);
    }
    EnableErrMsg(enabled);

    string         key = "VariableExists";
    vector<size_t> dummy;
    if (_varInfoCache.Get(ts, varname, level, lod, key, dummy)) { return (true); }

    // If a data variable need to test for existance of all coordinate
    // variables
    //
    vector<string> varnames = _get_var_dependencies(varname);

    // Separate native and derived variables. Derived variables are
    // recursively tested
    //
    vector<string> native_vars;
    vector<string> derived_vars;
    for (int i = 0; i < varnames.size(); i++) {
        if (DataMgr::IsVariableNative(varnames[i])) {
            native_vars.push_back(varnames[i]);
        } else {
            derived_vars.push_back(varnames[i]);
        }
    }

    // Check native variables
    //
    vector<size_t> exists_vec;
    for (int i = 0; i < native_vars.size(); i++) {
        if (_varInfoCache.Get(ts, native_vars[i], level, lod, key, exists_vec)) { continue; }
        bool exists = _dc->VariableExists(ts, varname, level, lod);
        if (exists) {
            _varInfoCache.Set(ts, native_vars[i], level, lod, key, exists_vec);
        } else {
            return (false);
        }
    }

    // Check derived variables
    //
    for (int i = 0; i < derived_vars.size(); i++) {
        DerivedVar *derivedVar = _getDerivedVar(derived_vars[i]);
        if (!derivedVar) return (false);

        vector<string> ivars = derivedVar->GetInputs();

        //
        // Recursively test existence of all dependencies
        //
        for (int i = 0; i < ivars.size(); i++) {
            if (!VariableExists(ts, ivars[i], level, lod)) return (false);
        }
    }

    _varInfoCache.Set(ts, varname, level, lod, key, exists_vec);
    return (true);
}

bool DataMgr::IsVariableNative(string name) const
{
    vector<string> svec = _get_native_variables();

    for (int i = 0; i < svec.size(); i++) {
        if (name.compare(svec[i]) == 0) return (true);
    }
    return (false);
}

bool DataMgr::IsVariableDerived(string name) const { return (_getDerivedVar(name) != NULL); }

void DataMgr::Clear()
{
    _PipeLines.clear();

    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        const region_t &region = *itr;

        if (region.blks) _blk_mem_mgr->FreeMem(region.blks);
    }
    _regionsList.clear();

    vector<string> hash = _varInfoCache.GetVoidPtrHash();
    for (int i = 0; i < hash.size(); i++) {
        vector<void *> vals;
        _varInfoCache.Get(hash[i], vals);
        for (int j = 0; j < vals.size(); j++) {
            if (vals[j]) delete (KDTreeRG *)vals[j];
        }
    }
    _varInfoCache.Clear();
}

void DataMgr::UnlockGrid(const Grid *rg)
{
    SetDiagMsg("DataMgr::UnlockGrid()");
    const vector<float *> &blks = rg->GetBlks();
    if (blks.size()) _unlock_blocks(blks[0]);

    const LayeredGrid *lg = dynamic_cast<const LayeredGrid *>(rg);
    if (lg) {
        const Grid &rg = lg->GetZRG();
        if (rg.GetBlks().size()) _unlock_blocks(rg.GetBlks()[0]);
    }
}

size_t DataMgr::GetNumDimensions(string varname) const
{
    assert(_dc);

    DC::DataVar dvar;
    bool        status = GetDataVarInfo(varname, dvar);

    if (status) {
        DC::Mesh m;
        status = GetMesh(dvar.GetMeshName(), m);
        if (!status) return (0);

        return (m.GetDimNames().size());
    }

    DC::CoordVar cvar;
    status = GetCoordVarInfo(varname, cvar);
    if (status) return (cvar.GetDimNames().size());

    return (0);
}

size_t DataMgr::GetVarTopologyDim(string varname) const
{
    assert(_dc);

    DC::DataVar var;
    bool        status = GetDataVarInfo(varname, var);
    if (!status) return (0);

    string mname = var.GetMeshName();

    DC::Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status) return (0);

    return (mesh.GetTopologyDim());
}

template<typename T> T *DataMgr::_get_region_from_cache(size_t ts, string varname, int level, int lod, const vector<size_t> &bmin, const vector<size_t> &bmax, bool lock)
{
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        region_t &region = *itr;

        if (region.ts == ts && region.varname.compare(varname) == 0 && region.level == level && region.lod == lod && region.bmin == bmin && region.bmax == bmax) {
            // Increment the lock counter
            region.lock_counter += lock ? 1 : 0;

            // Move region to front of list
            region_t tmp_region = region;
            _regionsList.erase(itr);
            _regionsList.push_back(tmp_region);

            SetDiagMsg("DataMgr::_get_region_from_cache() - data in cache %xll\n", tmp_region.blks);
            return ((T *)tmp_region.blks);
        }
    }

    return (NULL);
}

template<typename T> T *DataMgr::_get_region_from_fs(size_t ts, string varname, int level, int lod, const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax, bool lock)
{
    T *blks = (T *)_alloc_region(ts, varname, level, lod, bmin, bmax, bs, sizeof(T), lock, false);
    if (!blks) return (NULL);

    vector<size_t> min, max;
    map_blk_to_vox(bs, bmin, bmax, min, max);

    // Ugh. This stupid hack is needed because 1D variables aren't blocked
    // in the VDC. However, we try to fake it by setting the block
    // size to match that of the data variable. But the fake bs results
    // in missaligned coords when we go to read 1D variables.
    //
    if (bs.size() == 1) {
        vector<size_t> dims_at_level;
        vector<size_t> bs_at_level;
        (void)DataMgr::GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level);
        if (bs_at_level[0] == 1) {
            if (max[0] >= dims_at_level[0]) max[0] = dims_at_level[0] - 1;
        }
    }

    int fd = _openVariableRead(ts, varname, level, lod);
    if (fd < 0) return (NULL);

    int rc = _readRegionBlock(fd, min, max, blks);
    if (rc < 0) {
        _free_region(ts, varname, level, lod, bmin, bmax);
        _closeVariable(fd);
        return (NULL);
    }

    rc = _closeVariable(fd);
    if (rc < 0) return (NULL);

    SetDiagMsg("DataMgr::GetGrid() - data read from fs\n");
    return (blks);
}

template<typename T>
T *DataMgr::_get_region(size_t ts, string varname, int level, int nlevels, int lod, int nlods, const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax, bool lock)
{
    if (lod < -nlods) lod = -nlods;

    // See if region is already in cache. If not, read from the
    // file system.
    //
    T *blks = _get_region_from_cache<T>(ts, varname, level, lod, bmin, bmax, lock);
    if (!blks) {
        // If level not available we recursively decimate
        //
        if (level < -nlevels) {
            level++;

            blks = _get_region<T>(ts, varname, level, nlevels, lod, nlods, bs, bmin, bmax, false);
            if (blks) {
                vector<size_t> bs_at_level = decimate_dims(bs, -level - 1);
                vector<size_t> bs_at_level_m1 = decimate_dims(bs, -level);

                T *newblks = (T *)_alloc_region(ts, varname, level - 1, lod, bmin, bmax, bs_at_level_m1, sizeof(T), false, false);
                if (!newblks) return (NULL);

                decimate(bmin, bmax, bs_at_level, blks, newblks);
                return (newblks);
            }
        } else {
            vector<size_t> bs_at_level = decimate_dims(bs, -level - 1);

            blks = (T *)_get_region_from_fs<T>(ts, varname, level, lod, bs_at_level, bmin, bmax, lock);
        }
        if (!blks) {
            SetErrMsg("Failed to read region from variable/timestep/level/lod (%s, %d, %d, %d)", varname.c_str(), ts, level, lod);
            return (NULL);
        }
    }
    return (blks);
}

template<typename T>
int DataMgr::_get_regions(size_t ts, const vector<string> &varnames, int level, int lod, bool lock,
                          const vector<vector<size_t>> &bsvec,    // native coordinates
                          const vector<vector<size_t>> &bminvec, const vector<vector<size_t>> &bmaxvec, vector<T *> &blkvec)
{
    blkvec.clear();

    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty()) {    // nothing to do
            blkvec.push_back(NULL);
            continue;
        }

        int nlevels = DataMgr::GetNumRefLevels(varnames[i]);

        DC::BaseVar var;
        int         rc = GetBaseVarInfo(varnames[i], var);
        if (rc < 0) return (rc);

        int nlods = var.GetCRatios().size();

        size_t my_ts = ts;

        // If variable isn't time varying time step should always be 0
        //
        if (!DataMgr::IsTimeVarying(varnames[i])) my_ts = 0;

        T *blks = _get_region<T>(my_ts, varnames[i], level, nlevels, lod, nlods, bsvec[i], bminvec[i], bmaxvec[i], true);
        if (!blks) {
            for (int i = 0; i < blkvec.size(); i++) {
                if (blkvec[i]) _unlock_blocks(blkvec[i]);
            }
            return (-1);
        }
        blkvec.push_back(blks);
    }

    //
    // Safe to remove locks now that were not explicitly requested
    //
    if (!lock) {
        for (int i = 0; i < blkvec.size(); i++) {
            if (blkvec[i]) _unlock_blocks(blkvec[i]);
        }
    }
    return (0);
}

void *DataMgr::_alloc_region(size_t ts, string varname, int level, int lod, vector<size_t> bmin, vector<size_t> bmax, vector<size_t> bs, int element_sz, bool lock, bool fill)
{
    assert(bmin.size() == bmax.size());
    assert(bmin.size() == bs.size());

    size_t mem_block_size;
    if (!_blk_mem_mgr) {
        mem_block_size = 1024 * 1024;

        size_t num_blks = (_mem_size * 1024 * 1024) / mem_block_size;

        BlkMemMgr::RequestMemSize(mem_block_size, num_blks);
        _blk_mem_mgr = new BlkMemMgr();
    }
    mem_block_size = BlkMemMgr::GetBlkSize();

    // Free region already exists
    //
    _free_region(ts, varname, level, lod, bmin, bmax);

    size_t size = element_sz;
    for (int i = 0; i < bmin.size(); i++) { size *= (bmax[i] - bmin[i] + 1) * bs[i]; }

    size_t nblocks = (size_t)ceil((double)size / (double)mem_block_size);

    void *blks;
    while (!(blks = (void *)_blk_mem_mgr->Alloc(nblocks, fill))) {
        if (!_free_lru()) {
            SetErrMsg("Failed to allocate requested memory");
            return (NULL);
        }
    }

    region_t region;

    region.ts = ts;
    region.varname = varname;
    region.level = level;
    region.lod = lod;
    region.bmin = bmin;
    region.bmax = bmax;
    region.lock_counter = lock ? 1 : 0;
    region.blks = blks;

    _regionsList.push_back(region);

    return (region.blks);
}

void DataMgr::_free_region(size_t ts, string varname, int level, int lod, vector<size_t> bmin, vector<size_t> bmax)
{
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        const region_t &region = *itr;

        if (region.ts == ts && region.varname.compare(varname) == 0 && region.level == level && region.lod == lod && region.bmin == bmin && region.bmax == bmax) {
            if (region.lock_counter == 0) {
                if (region.blks) _blk_mem_mgr->FreeMem(region.blks);

                _regionsList.erase(itr);
                return;
            }
        }
    }

    return;
}

void DataMgr::_free_var(string varname)
{
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end();) {
        const region_t &region = *itr;

        if (region.varname.compare(varname) == 0) {
            if (region.blks) _blk_mem_mgr->FreeMem(region.blks);

            _regionsList.erase(itr);
            itr = _regionsList.begin();
        } else
            itr++;
    }
}

bool DataMgr::_free_lru()
{
    // The least recently used region is at the front of the list
    //
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        const region_t &region = *itr;

        if (region.lock_counter == 0) {
            if (region.blks) _blk_mem_mgr->FreeMem(region.blks);
            _regionsList.erase(itr);
            return (true);
        }
    }

    // nothing to free
    return (false);
}

#ifdef VAPOR3_0_0_ALPHA
PipeLine *DataMgr::get_pipeline_for_var(string varname) const
{
    for (int i = 0; i < _PipeLines.size(); i++) {
        const vector<pair<string, VarType_T>> &output_vars = _PipeLines[i]->GetOutputs();

        for (int j = 0; j < output_vars.size(); j++) {
            if (output_vars[j].first.compare(varname) == 0) { return (_PipeLines[i]); }
        }
    }
    return (NULL);
}

Grid *DataMgr::execute_pipeline(size_t ts, string varname, int level, int lod, const size_t min[3], const size_t max[3], bool lock, float *xcblks, float *ycblks, float *zcblks)
{
    if (level < 0) level = GetNumTransforms();
    if (lod < 0) lod = GetCRatios().size() - 1;

    _VarInfoCache.PurgeRange(ts, varname, level, lod);
    _VarInfoCache.PurgeRegion(ts, varname, level);
    _VarInfoCache.PurgeExist(ts, varname, level, lod);

    PipeLine *pipeline = get_pipeline_for_var(varname);

    assert(pipeline != NULL);

    const vector<string> &                 input_varnames = pipeline->GetInputs();
    const vector<pair<string, VarType_T>> &output_vars = pipeline->GetOutputs();

    VarType_T vtype = DataMgr::GetVarType(varname);

    //
    // Ptrs to space for input and output variables
    //
    vector<const Grid *> in_grids;
    vector<Grid *>       out_grids;

    //
    // Get input variables, and lock them into memory
    //
    for (int i = 0; i < input_varnames.size(); i++) {
        size_t    min_in[] = {min[0], min[1], min[2]};
        size_t    max_in[] = {max[0], max[1], max[2]};
        VarType_T vtype_in = DataMgr::GetVarType(input_varnames[i]);

        //
        // If the requested output variable is 2D and an input variable
        // is 3D we need to make sure that the 3rd dimension of the
        // 3D input variable covers the full domain
        //
        if (vtype != VAR3D && vtype_in == VAR3D) {
            size_t dims[3];
            DataMgr::GetDim(dims, level);

            switch (vtype) {
            case VAR2D_XY:
                min_in[2] = 0;
                max_in[2] = dims[2] - 1;
                break;
            case VAR2D_XZ:
                min_in[1] = 0;
                max_in[1] = dims[1] - 1;
                break;
            case VAR2D_YZ:
                min_in[0] = 0;
                max_in[0] = dims[0] - 1;
                break;
            default: break;
            }
        }

        Grid *rg = GetGrid(ts, input_varnames[i], level, lod, min_in, max_in, true);
        if (!rg) {
            // Unlock any locked variables and abort
            //
            for (int j = 0; j < in_grids.size(); j++) UnlockGrid(in_grids[j]);
            return (NULL);
        }
        in_grids.push_back(rg);
    }

    //
    // Get space for all output variables generated by the pipeline,
    // including the single variable that we will return.
    //
    int output_index = -1;
    for (int i = 0; i < output_vars.size(); i++) {
        string    v = output_vars[i].first;
        VarType_T vtype_out = output_vars[i].second;

        //
        // if output variable i is the one we are interested in record
        // the index and use the lock value passed in to this method
        //
        if (v.compare(varname) == 0) { output_index = i; }

        float *blks = _alloc_region(ts, v.c_str(), vtype_out, level, lod, min, max, true, true);
        if (!blks) {
            // Unlock any locked variables and abort
            //
            for (int j = 0; j < in_grids.size(); j++) UnlockGrid(in_grids[j]);
            for (int j = 0; j < out_grids.size(); j++) UnlockGrid(out_grids[j]);
            return (NULL);
        }
        Grid *rg = _make_grid(ts, v, level, lod, min, max, blks, xcblks, ycblks, zcblks);
        if (!rg) {
            for (int j = 0; j < in_grids.size(); j++) UnlockGrid(in_grids[j]);
            for (int j = 0; j < out_grids.size(); j++) UnlockGrid(out_grids[j]);
            return (NULL);
        }
        out_grids.push_back(rg);
    }
    assert(output_index >= 0);

    int rc = pipeline->Calculate(in_grids, out_grids, ts, level, lod);

    //
    // Unlock input variables and output variables that are not
    // being returned.
    //
    // N.B. unlocking a variable doesn't necessarily free it, but
    // makes the space available if needed later
    //

    //
    // Always unlock/free all input variables
    //
    for (int i = 0; i < in_grids.size(); i++) {
        UnlockGrid(in_grids[i]);
        delete in_grids[i];
    }

    //
    // Unlock/free all outputs on error
    //
    if (rc < 0) {
        for (int i = 0; i < out_grids.size(); i++) {
            UnlockGrid(out_grids[i]);
            delete out_grids[i];
        }
        return (NULL);
    }

    //
    // Unlock/free outputs not being returned
    //
    for (int i = 0; i < out_grids.size(); i++) {
        if (i != output_index) {
            UnlockGrid(out_grids[i]);
            delete out_grids[i];
        } else if (!lock)
            UnlockGrid(out_grids[i]);
    }

    return (out_grids[output_index]);
}

bool DataMgr::cycle_check(const map<string, vector<string>> &graph, const string &node, const vector<string> &depends) const
{
    if (depends.size() == 0) return (false);

    for (int i = 0; i < depends.size(); i++) {
        if (node.compare(depends[i]) == 0) return (true);
    }

    for (int i = 0; i < depends.size(); i++) {
        const map<string, vector<string>>::const_iterator itr = graph.find(depends[i]);
        assert(itr != graph.end());

        if (cycle_check(graph, node, itr->second)) return (true);
    }

    return (false);
}

//
// return true iff 'a' depends on 'b' - true if a has inputs that
// match b's outputs.
//
bool DataMgr::depends_on(const PipeLine *a, const PipeLine *b) const
{
    const vector<string> &                 input_varnames = a->GetInputs();
    const vector<pair<string, VarType_T>> &output_vars = b->GetOutputs();

    for (int i = 0; i < input_varnames.size(); i++) {
        for (int j = 0; j < output_vars.size(); j++) {
            if (input_varnames[i].compare(output_vars[j].first) == 0) { return (true); }
        }
    }
    return (false);
}

#endif

//
// return complete list of native variables
//
vector<string> DataMgr::_get_native_variables() const
{
    vector<string> v1 = _dc->GetDataVarNames();
    vector<string> v2 = _dc->GetCoordVarNames();
    vector<string> v3 = _dc->GetAuxVarNames();

    v1.insert(v1.end(), v2.begin(), v2.end());
    v1.insert(v1.end(), v3.begin(), v3.end());
    return (v1);
}

#ifdef VAPOR3_0_0_ALPHA

void DataMgr::PurgeVariable(string varname)
{
    _free_var(varname);
    _VarInfoCache.PurgeVariable(varname);
}

#endif

bool DataMgr::_hasHorizontalXForm() const
{
    assert(_dc);

    vector<string> meshnames = _dc->GetMeshNames();

    for (int i = 0; i < meshnames.size(); i++) {
        if (_hasHorizontalXForm(meshnames[i])) return (true);
    }

    return (false);
}

bool DataMgr::_hasHorizontalXForm(string meshname) const
{
    DC::Mesh m;
    bool     status = _dc->GetMesh(meshname, m);
    if (!status) return (false);

    vector<string> coordVars = m.GetCoordVars();

    for (int i = 0; i < coordVars.size(); i++) {
        DC::CoordVar varInfo;

        bool ok = _dc->GetCoordVarInfo(coordVars[i], varInfo);
        assert(ok);

        if (varInfo.GetAxis() == 0 && _udunits.IsLonUnit(varInfo.GetUnits())) { return (true); }
        if (varInfo.GetAxis() == 1 && _udunits.IsLatUnit(varInfo.GetUnits())) { return (true); }
    }

    return (false);
}

bool DataMgr::_hasVerticalXForm() const
{
    assert(_dc);

    // Only 3D variables can have vertical coordinates?
    //
    vector<string> meshnames = _dc->GetMeshNames();

    for (int i = 0; i < meshnames.size(); i++) {
        if (_hasVerticalXForm(meshnames[i])) return (true);
    }

    return (false);
}

bool DataMgr::_hasVerticalXForm(string meshname, string &standard_name, string &formula_terms) const
{
    standard_name.clear();
    formula_terms.clear();

    DC::Mesh m;
    bool     ok = _dc->GetMesh(meshname, m);
    if (!ok) return (false);

    vector<string> coordVars = m.GetCoordVars();

    bool         hasVertCoord = false;
    DC::CoordVar cvarInfo;
    for (int i = 0; i < coordVars.size(); i++) {
        bool ok = _dc->GetCoordVarInfo(coordVars[i], cvarInfo);
        assert(ok);

        if (cvarInfo.GetAxis() == 2) {
            hasVertCoord = true;
            break;
        }
    }
    if (!hasVertCoord) return (false);

    DC::Attribute attr_name;
    if (!cvarInfo.GetAttribute("standard_name", attr_name)) return (false);

    attr_name.GetValues(standard_name);

    if (standard_name.empty()) return (false);

    DC::Attribute attr_formula;
    if (!cvarInfo.GetAttribute("formula_terms", attr_formula)) return (false);

    attr_formula.GetValues(formula_terms);

    if (formula_terms.empty()) return (false);

    // Currently only support one vertical transform!!!
    //
    if (!DerivedCoordVarStandardWRF_Terrain::ValidFormula(formula_terms)) { return (false); }

    return (true);
}

string DataMgr::VarInfoCache::_make_hash(string key, size_t ts, vector<string> varnames, int level, int lod)
{
    ostringstream oss;

    oss << key << ":";
    oss << ts << ":";
    for (int i = 0; i < varnames.size(); i++) { oss << varnames[i] << ":"; }
    oss << level << ":";
    oss << lod;

    return (oss.str());
}

void DataMgr::VarInfoCache::Set(size_t ts, vector<string> varnames, int level, int lod, string key, const vector<size_t> &values)
{
    string hash = _make_hash(key, ts, varnames, level, lod);
    _cacheSize_t[hash] = values;
}

bool DataMgr::VarInfoCache::Get(size_t ts, vector<string> varnames, int level, int lod, string key, vector<size_t> &values) const
{
    values.clear();

    string                                      hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<size_t>>::const_iterator itr = _cacheSize_t.find(hash);

    if (itr == _cacheSize_t.end()) return (false);

    values = itr->second;
    return (true);
}

void DataMgr::VarInfoCache::PurgeSize_t(size_t ts, vector<string> varnames, int level, int lod, string key)
{
    string                                hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<size_t>>::iterator itr = _cacheSize_t.find(hash);

    if (itr == _cacheSize_t.end()) return;

    _cacheSize_t.erase(itr);
}

void DataMgr::VarInfoCache::Set(size_t ts, vector<string> varnames, int level, int lod, string key, const vector<double> &values)
{
    string hash = _make_hash(key, ts, varnames, level, lod);
    _cacheDouble[hash] = values;
}

bool DataMgr::VarInfoCache::Get(size_t ts, vector<string> varnames, int level, int lod, string key, vector<double> &values) const
{
    values.clear();

    string                                      hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<double>>::const_iterator itr = _cacheDouble.find(hash);

    if (itr == _cacheDouble.end()) return (false);

    values = itr->second;
    return (true);
}

void DataMgr::VarInfoCache::PurgeDouble(size_t ts, vector<string> varnames, int level, int lod, string key)
{
    string                                hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<double>>::iterator itr = _cacheDouble.find(hash);

    if (itr == _cacheDouble.end()) return;

    _cacheDouble.erase(itr);
}

void DataMgr::VarInfoCache::Set(size_t ts, vector<string> varnames, int level, int lod, string key, const vector<void *> &values)
{
    string hash = _make_hash(key, ts, varnames, level, lod);
    _cacheVoidPtr[hash] = values;
}

bool DataMgr::VarInfoCache::Get(size_t ts, vector<string> varnames, int level, int lod, string key, vector<void *> &values) const
{
    values.clear();

    string                                      hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<void *>>::const_iterator itr = _cacheVoidPtr.find(hash);

    if (itr == _cacheVoidPtr.end()) return (false);

    values = itr->second;
    return (true);
}

void DataMgr::VarInfoCache::PurgeVoidPtr(size_t ts, vector<string> varnames, int level, int lod, string key)
{
    string                                hash = _make_hash(key, ts, varnames, level, lod);
    map<string, vector<void *>>::iterator itr = _cacheVoidPtr.find(hash);

    if (itr == _cacheVoidPtr.end()) return;

    _cacheVoidPtr.erase(itr);
}

DataMgr::BlkExts::BlkExts()
{
    _bmin.clear();
    _bmax.clear();
    _mins.clear();
    _maxs.clear();
}

DataMgr::BlkExts::BlkExts(const std::vector<size_t> &bmin, const std::vector<size_t> &bmax)
{
    assert(bmin.size() == bmax.size());
    assert(bmin.size() >= 1 && bmax.size() <= 3);

    _bmin = bmin;
    _bmax = bmax;

    size_t nelements = Wasp::LinearizeCoords(bmax, bmin, bmax) + 1;

    _mins.resize(nelements);
    _maxs.resize(nelements);
}

void DataMgr::BlkExts::Insert(const std::vector<size_t> &bcoord, const std::vector<double> &min, const std::vector<double> &max)
{
    size_t offset = Wasp::LinearizeCoords(bcoord, _bmin, _bmax);

    _mins[offset] = min;
    _maxs[offset] = max;
}

bool DataMgr::BlkExts::Intersect(const std::vector<double> &min, const std::vector<double> &max, std::vector<size_t> &bmin, std::vector<size_t> &bmax) const
{
    assert(_mins.size() >= 1);

    bmin = _bmax;
    bmax = _bmin;

    bool intersection = false;

    // Test for intersection with the axis aligned bounding box of each
    // block.
    //
    for (size_t offset = 0; offset < _mins.size(); offset++) {
        bool overlap = true;
        for (int j = 0; j < min.size(); j++) {
            if (_maxs[offset][j] < min[j] || _mins[offset][j] > max[j]) {
                overlap = false;
                continue;
            }
        }

        // If the current block intersects the specified bounding volume
        // compute the block coordinates of the first and last block
        // that intersect the volume
        //
        if (overlap) {
            intersection = true;    // at least one block intersects

            vector<size_t> coord = Wasp::VectorizeCoords(offset, _bmin, _bmax);

            for (int i = 0; i < coord.size(); i++) {
                if (coord[i] < bmin[i]) bmin[i] = coord[i];
                if (coord[i] > bmax[i]) bmax[i] = coord[i];
            }
        }
    }

    return (intersection);
}

int DataMgr::_level_correction(string varname, int &level) const
{
    int nlevels = DataMgr::GetNumRefLevels(varname);

    if (level >= nlevels) level = nlevels - 1;
    if (level >= 0) level = -(nlevels - level);
    if (level < -nlevels) level = -nlevels;

    return (0);
}

int DataMgr::_lod_correction(string varname, int &lod) const
{
    DC::BaseVar var;
    int         rc = GetBaseVarInfo(varname, var);
    if (rc < 0) return (rc);

    int nlod = var.GetCRatios().size();

    if (lod >= nlod) lod = nlod - 1;
    if (lod >= 0) lod = -(nlod - lod);
    if (lod < -nlod) lod = -nlod;

    return (0);
}

//
// Get coordiante variable names for a data variable, return as
// a list of spatial coordinate variables, and a single (if it exists)
// time coordinate variable
//
bool DataMgr::_get_coord_vars(string varname, vector<string> &scvars, string &tcvar) const
{
    scvars.clear();
    tcvar.clear();

    // Get space and time coord vars
    //
    bool ok = GetVarCoordVars(varname, false, scvars);
    if (!ok) return (ok);

    // Split out time and space coord vars
    //
    if (IsTimeVarying(varname)) {
        assert(scvars.size());

        tcvar = scvars.back();
        scvars.pop_back();
    }

    return (true);
}

bool DataMgr::_get_coord_vars(string varname, vector<DC::CoordVar> &scvarsinfo, DC::CoordVar &tcvarinfo) const
{
    scvarsinfo.clear();

    vector<string> scvarnames;
    string         tcvarname;
    bool           ok = _get_coord_vars(varname, scvarnames, tcvarname);
    if (!ok) return (ok);

    for (int i = 0; i < scvarnames.size(); i++) {
        DC::CoordVar cvarinfo;

        bool ok = GetCoordVarInfo(scvarnames[i], cvarinfo);
        if (!ok) return (ok);

        scvarsinfo.push_back(cvarinfo);
    }

    if (!tcvarname.empty()) {
        bool ok = GetCoordVarInfo(tcvarname, tcvarinfo);
        if (!ok) return (ok);
    }

    return (true);
}

int DataMgr::_initTimeCoord()
{
    _timeCoordinates.clear();

    vector<string> vars = _dc->GetTimeCoordVarNames();
    if (vars.size() > 1) {
        SetErrMsg("Data set contains more than one time coordinate");
        return (-1);
    }

    if (vars.size() == 0) {
        // No time coordinates present
        //
        _timeCoordinates.push_back(0.0);
        return (0);
    }

    string nativeTimeCoordName = vars[0];

    size_t numTS = _dc->GetNumTimeSteps(nativeTimeCoordName);

    // If we have a time unit try to convert to seconds from EPOCH
    //
    VAPoR::DC::CoordVar cvar;
    _dc->GetCoordVarInfo(nativeTimeCoordName, cvar);
    if (_udunits.IsTimeUnit(cvar.GetUnits())) {
        string derivedTimeCoordName = nativeTimeCoordName;

        _assignTimeCoord(derivedTimeCoordName);

        DerivedCoordVar_TimeInSeconds *derivedVar = new DerivedCoordVar_TimeInSeconds(derivedTimeCoordName, _dc, nativeTimeCoordName, cvar.GetTimeDimName());

        int rc = derivedVar->Initialize();
        if (rc < 0) {
            SetErrMsg("Failed to initialize derived coord variable");
            return (-1);
        }

        _dvm.AddCoordVar(derivedVar);

        _timeCoordinates = derivedVar->GetTimes();
    } else {
        float *buf = new float[numTS];
        int    rc = _getVar(nativeTimeCoordName, -1, -1, buf);
        if (rc < 0) { return (-1); }

        for (int j = 0; j < numTS; j++) { _timeCoordinates.push_back(buf[j]); }
        delete[] buf;
    }

    return (0);
}

void DataMgr::_ugrid_setup(const DC::DataVar &var, std::vector<size_t> &vertexDims, std::vector<size_t> &faceDims, std::vector<size_t> &edgeDims,
                           UnstructuredGrid::Location &location,    // node,face, edge
                           size_t &maxVertexPerFace, size_t &maxFacePerVertex, long &vertexOffset, long &faceOffset) const
{
    vertexDims.clear();
    faceDims.clear();
    edgeDims.clear();

    DC::Mesh m;
    bool     status = GetMesh(var.GetMeshName(), m);
    assert(status);

    DC::Dimension dimension;

    size_t layers_dimlen = 0;
    if (m.GetMeshType() == DC::Mesh::UNSTRUC_LAYERED) {
        string dimname = m.GetLayersDimName();
        assert(!dimname.empty());
        status = _dc->GetDimension(dimname, dimension);
        assert(status);
        layers_dimlen = dimension.GetLength();
    }

    string dimname = m.GetNodeDimName();
    status = _dc->GetDimension(dimname, dimension);
    assert(status);
    vertexDims.push_back(dimension.GetLength());
    if (layers_dimlen) { vertexDims.push_back(layers_dimlen); }

    dimname = m.GetFaceDimName();
    status = _dc->GetDimension(dimname, dimension);
    assert(status);
    faceDims.push_back(dimension.GetLength());
    if (layers_dimlen) { faceDims.push_back(layers_dimlen - 1); }

    dimname = m.GetEdgeDimName();
    if (dimname.size()) {
        status = _dc->GetDimension(dimname, dimension);
        assert(status);
        edgeDims.push_back(dimension.GetLength());
        if (layers_dimlen) { edgeDims.push_back(layers_dimlen - 1); }
    }

    DC::Mesh::Location l = var.GetSamplingLocation();
    if (l == DC::Mesh::NODE) {
        location = UnstructuredGrid::NODE;
    } else if (l == DC::Mesh::EDGE) {
        location = UnstructuredGrid::EDGE;
    } else if (l == DC::Mesh::FACE) {
        location = UnstructuredGrid::CELL;
    } else if (l == DC::Mesh::VOLUME) {
        location = UnstructuredGrid::CELL;
    }

    maxVertexPerFace = m.GetMaxNodesPerFace();
    maxFacePerVertex = m.GetMaxFacesPerNode();

    string face_node_var;
    string node_face_var;
    string dummy;

    bool ok = _getVarConnVars(var.GetName(), face_node_var, node_face_var, dummy, dummy, dummy, dummy);
    assert(ok);

    DC::AuxVar auxvar;
    status = _dc->GetAuxVarInfo(face_node_var, auxvar);
    assert(status);
    vertexOffset = auxvar.GetOffset();

    status = _dc->GetAuxVarInfo(node_face_var, auxvar);
    assert(status);
    faceOffset = auxvar.GetOffset();
}

string DataMgr::_get_grid_type(string varname) const
{
    vector<DC::CoordVar> cvarsinfo;
    DC::CoordVar         dummy;
    bool                 ok = _get_coord_vars(varname, cvarsinfo, dummy);
    if (!ok) return ("");

    vector<vector<string>> cdimnames;
    for (int i = 0; i < cvarsinfo.size(); i++) {
        vector<string> v;
        bool           ok = _getVarDimNames(cvarsinfo[i].GetName(), v);
        if (!ok) return ("");

        cdimnames.push_back(v);
    }

    DC::DataVar dvar;
    ok = GetDataVarInfo(varname, dvar);
    assert(ok);

    DC::Mesh m;
    ok = GetMesh(dvar.GetMeshName(), m);
    assert(ok);

    return (_gridHelper.GetGridType(m, cvarsinfo, cdimnames));
}

// Find the grid coordinates, in voxels, for the region containing
// the axis aligned bounding box specified by min and max
//
int DataMgr::_find_bounding_grid(size_t ts, string varname, int level, int lod, vector<double> min, vector<double> max, vector<size_t> &min_ui, vector<size_t> &max_ui)
{
    min_ui.clear();
    max_ui.clear();

    vector<string> scvars;
    string         tcvar;

    bool ok = _get_coord_vars(varname, scvars, tcvar);
    if (!ok) return (-1);

    size_t hash_ts = 0;
    for (int i = 0; i < scvars.size(); i++) {
        if (IsTimeVarying(scvars[i])) hash_ts = ts;
    }

    vector<size_t> dims_at_level;
    vector<size_t> bs_at_level;
    int            rc = DataMgr::GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level);
    if (rc < 0) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (-1);
    }

    // hash tag for block coordinate cache
    //
    string hash = VarInfoCache::_make_hash("BlkExts", hash_ts, scvars, level, lod);

    // See if bounding volumes for individual blocks are already
    // cached for this grid
    //
    map<string, BlkExts>::iterator itr = _blkExtsCache.find(hash);

    if (itr == _blkExtsCache.end()) {
        SetDiagMsg("DataMgr::_find_bounding_grid() - coordinates not in cache");

        // Get a "dataless" Grid - a Grid class the contains
        // coordiante information, but not data
        //
        Grid *rg = _getVariable(ts, varname, level, lod, false, true);
        if (!rg) return (-1);

        // Voxel and block min and max coordinates of entire grid
        //
        vector<size_t> vmin, vmax;
        vector<size_t> bmin, bmax;

        for (int i = 0; i < dims_at_level.size(); i++) {
            vmin.push_back(0);
            vmax.push_back(dims_at_level[i] - 1);
        }
        map_vox_to_blk(bs_at_level, vmin, bmin);
        map_vox_to_blk(bs_at_level, vmax, bmax);

        BlkExts blkexts(bmin, bmax);

        // For each block in the grid compute the block's bounding
        // box. Include a one-voxel halo region on all non-boundary
        // faces
        //
        size_t nblocks = Wasp::LinearizeCoords(bmax, bmin, bmax) + 1;
        for (size_t offset = 0; offset < nblocks; offset++) {
            vector<double> my_min, my_max;
            vector<size_t> my_vmin(vmin.size()), my_vmax(vmin.size());

            // Get coordinates for current block
            //
            vector<size_t> bcoord = Wasp::VectorizeCoords(offset, bmin, bmax);

            for (int i = 0; i < bcoord.size(); i++) {
                my_vmin[i] = bcoord[i] * bs_at_level[i];
                if (my_vmin[i] > 0) my_vmin[i] -= 1;    // not boundary face

                my_vmax[i] = bcoord[i] * bs_at_level[i] + bs_at_level[i] - 1;
                if (my_vmax[i] > vmax[i]) my_vmax[i] = vmax[i];
                if (my_vmax[i] < vmax[i]) my_vmax[i] += 1;
            }

            // Use the regular grid class to compute the user-coordinate
            // axis aligned bounding volume for the block+halo
            //
            rg->GetBoundingBox(my_vmin, my_vmax, my_min, my_max);

            // Insert the bounding volume into blkexts
            //
            blkexts.Insert(bcoord, my_min, my_max);
        }

        // Add to the hash table
        //
        _blkExtsCache[hash] = blkexts;
        itr = _blkExtsCache.find(hash);
        assert(itr != _blkExtsCache.end());

    } else {
        SetDiagMsg("DataMgr::_find_bounding_grid() - coordinates in cache");
    }

    const BlkExts &blkexts = itr->second;

    // Find block coordinates of region that contains the bounding volume
    //
    vector<size_t> bmin, bmax;
    ok = blkexts.Intersect(min, max, bmin, bmax);
    if (!ok) {
        return (0);    // No intersection
    }

    // Finally, map from block to voxel coordinates
    //
    map_blk_to_vox(bs_at_level, bmin, bmax, min_ui, max_ui);
    for (int i = 0; i < max_ui.size(); i++) {
        if (max_ui[i] >= dims_at_level[i]) { max_ui[i] = dims_at_level[i] - 1; }
    }

    return (0);
}

void DataMgr::_unlock_blocks(const void *blks)
{
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        region_t &region = *itr;

        if (region.blks == blks && region.lock_counter > 0) {
            region.lock_counter--;
            return;
        }
    }
    return;
}

vector<string> DataMgr::_getDataVarNamesDerived(int ndim) const
{
    vector<string> names;

    vector<string> allnames = _dvm.GetDataVarNames();
    ;
    for (int i = 0; i < allnames.size(); i++) {
        string name = allnames[i];

        DC::DataVar dvar;
        bool        ok = GetDataVarInfo(name, dvar);
        if (!ok) continue;

        string mesh_name;
        mesh_name = dvar.GetMeshName();

        DC::Mesh mesh;
        ok = GetMesh(mesh_name, mesh);
        if (!ok) continue;

        size_t d = mesh.GetTopologyDim();

        if (d == ndim) { names.push_back(name); }
    }

    return (names);
}

bool DataMgr::_hasCoordForAxis(vector<string> coord_vars, int axis) const
{
    for (int i = 0; i < coord_vars.size(); i++) {
        DC::CoordVar varInfo;

        bool ok = GetCoordVarInfo(coord_vars[i], varInfo);
        if (!ok) continue;

        if (varInfo.GetAxis() == axis) return (true);
    }
    return (false);
}

string DataMgr::_defaultCoordVar(const DC::Mesh &m, int axis) const
{
    assert(axis >= 0 && axis <= 2);

    // For a structured mesh use the coresponding dimension name
    // as the coordinate variable name. For unstructured nothing
    // we can do
    //
    if (m.GetMeshType() == DC::Mesh::STRUCTURED) {
        assert(m.GetDimNames().size() >= axis);
        return (m.GetDimNames()[axis]);
    } else {
        return ("");
    }
}

void DataMgr::_assignHorizontalCoords(vector<string> &coord_vars) const
{
    for (int i = 0; i < coord_vars.size(); i++) {
        DC::CoordVar varInfo;
        bool         ok = GetCoordVarInfo(coord_vars[i], varInfo);
        assert(ok);

        if (_udunits.IsLonUnit(varInfo.GetUnits())) { coord_vars[i] = coord_vars[i] + "X"; }
        if (_udunits.IsLatUnit(varInfo.GetUnits())) { coord_vars[i] = coord_vars[i] + "Y"; }
    }
}

void DataMgr::_assignTimeCoord(string &coord_var) const
{
    if (coord_var.empty()) return;

    DC::CoordVar varInfo;
    bool         ok = GetCoordVarInfo(coord_var, varInfo);
    assert(ok);

    if (varInfo.GetAxis() == 3 && !_udunits.IsTimeUnit(varInfo.GetUnits())) { coord_var = coord_var + "T"; }
}

bool DataMgr::_getVarDimensions(string varname, vector<DC::Dimension> &dimensions) const
{
    dimensions.clear();

    if (!IsVariableDerived(varname)) { return (_dc->GetVarDimensions(varname, true, dimensions)); }

    if (_getDerivedDataVar(varname)) {
        return (_getDataVarDimensions(varname, dimensions));
    } else if (_getDerivedCoordVar(varname)) {
        return (_getCoordVarDimensions(varname, dimensions));
    } else {
        return (false);
    }
}

bool DataMgr::_getDataVarDimensions(string varname, vector<DC::Dimension> &dimensions) const
{
    dimensions.clear();

    DC::DataVar var;
    bool        status = GetDataVarInfo(varname, var);
    if (!status) return (false);

    string mname = var.GetMeshName();

    DC::Mesh mesh;
    status = GetMesh(mname, mesh);
    if (!status) return (false);

    vector<string> dimnames;
    if (mesh.GetMeshType() == DC::Mesh::STRUCTURED) {
        dimnames = mesh.GetDimNames();
    } else {
        switch (var.GetSamplingLocation()) {
        case DC::Mesh::NODE: dimnames.push_back(mesh.GetNodeDimName()); break;
        case DC::Mesh::EDGE: dimnames.push_back(mesh.GetEdgeDimName()); break;
        case DC::Mesh::FACE: dimnames.push_back(mesh.GetFaceDimName()); break;
        case DC::Mesh::VOLUME: assert(0 && "VOLUME cells not supported"); break;
        }
        if (mesh.GetMeshType() == DC::Mesh::UNSTRUC_LAYERED) { dimnames.push_back(mesh.GetLayersDimName()); }
    }

    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dim;

        status = _dc->GetDimension(dimnames[i], dim);
        if (!status) return (false);

        dimensions.push_back(dim);
    }

    return (true);
}

bool DataMgr::_getCoordVarDimensions(string varname, vector<DC::Dimension> &dimensions) const
{
    dimensions.clear();

    DC::CoordVar var;
    bool         status = GetCoordVarInfo(varname, var);
    if (!status) return (false);

    vector<string> dimnames = var.GetDimNames();

    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dim;
        status = _dc->GetDimension(dimnames[i], dim);
        if (!status) return (false);

        dimensions.push_back(dim);
    }
    return (true);
}

bool DataMgr::_getVarDimNames(string varname, vector<string> &dimnames) const
{
    dimnames.clear();

    vector<DC::Dimension> dims;

    bool status = _getVarDimensions(varname, dims);
    if (!status) return (status);

    for (int i = 0; i < dims.size(); i++) { dimnames.push_back(dims[i].GetName()); }

    return (true);
}

bool DataMgr::_getVarConnVars(string varname, string &face_node_var, string &node_face_var, string &face_edge_var, string &face_face_var, string &edge_node_var, string &edge_face_var) const
{
    face_node_var.clear();
    node_face_var.clear();
    face_edge_var.clear();
    face_face_var.clear();
    edge_node_var.clear();
    edge_face_var.clear();

    DC::DataVar dvar;
    bool        status = GetDataVarInfo(varname, dvar);
    if (!status) return (false);

    DC::Mesh m;
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

DerivedVar *DataMgr::_getDerivedVar(string varname) const
{
    DerivedVar *dvar;

    dvar = _getDerivedDataVar(varname);
    if (dvar) return (dvar);

    dvar = _getDerivedCoordVar(varname);
    if (dvar) return (dvar);

    return (NULL);
}

DerivedDataVar *DataMgr::_getDerivedDataVar(string varname) const
{
    vector<string> varnames = _dvm.GetDataVarNames();
    if (find(varnames.begin(), varnames.end(), varname) != varnames.end()) { return (dynamic_cast<DerivedDataVar *>(_dvm.GetVar(varname))); }

    return (NULL);
}

DerivedCoordVar *DataMgr::_getDerivedCoordVar(string varname) const
{
    vector<string> varnames = _dvm.GetCoordVarNames();
    if (find(varnames.begin(), varnames.end(), varname) != varnames.end()) { return (dynamic_cast<DerivedCoordVar *>(_dvm.GetVar(varname))); }

    return (NULL);
}

int DataMgr::_openVariableRead(size_t ts, string varname, int level, int lod)
{
    _openVarName = varname;

    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) { return (derivedVar->OpenVariableRead(ts, level, lod)); }

    return (_dc->OpenVariableRead(ts, _openVarName, level, lod));
}

template<class T> int DataMgr::_readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) {
        assert((std::is_same<T, float>::value) == true);
        return (derivedVar->ReadRegionBlock(fd, min, max, (float *)region));
    }

    return (_dc->ReadRegionBlock(fd, min, max, region));
}

int DataMgr::_readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) { return (derivedVar->ReadRegion(fd, min, max, region)); }

    return (_dc->ReadRegion(fd, min, max, region));
}

int DataMgr::_closeVariable(int fd)
{
    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) { return (derivedVar->CloseVariable(fd)); }

    _openVarName.clear();

    return (_dc->CloseVariable(fd));
}

int DataMgr::_getVar(string varname, int level, int lod, float *data)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;
    int            rc = _dc->GetDimLensAtLevel(varname, level, dims_at_level, dummy);
    if (rc < 0) return (-1);

    // Number of per time step
    //
    size_t var_size = 1;
    for (int i = 0; i < dims_at_level.size(); i++) { var_size *= dims_at_level[i]; }

    size_t numts = _dc->GetNumTimeSteps(varname);

    float *ptr = data;
    for (size_t ts = 0; ts < numts; ts++) {
        rc = _getVar(ts, varname, level, lod, ptr);
        if (rc < 0) return (-1);

        ptr += var_size;
    }

    return (0);
}

int DataMgr::_getVar(size_t ts, string varname, int level, int lod, float *data)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;
    int            rc = _dc->GetDimLensAtLevel(varname, level, dims_at_level, dummy);
    if (rc < 0) return (-1);
    vector<size_t> min, max;
    for (int i = 0; i < dims_at_level.size(); i++) {
        min.push_back(0);
        max.push_back(dims_at_level[i] - 1);
    }

    int fd = _dc->OpenVariableRead(ts, varname, level, lod);
    if (fd < 0) return (-1);

    rc = _dc->ReadRegion(fd, min, max, data);
    if (rc < 0) return (-1);

    rc = _dc->CloseVariable(fd);
    if (rc < 0) return (-1);

    return (0);
}

int DataMgr::_getLatlonExtents(string varname, bool lonflag, float &min, float &max)
{
    vector<size_t> dims;
    vector<size_t> dummy;
    int            rc = _dc->GetDimLensAtLevel(varname, 0, dims, dummy);
    if (rc < 0) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (-1);
    }
    assert(dims.size() >= 1 && dims.size() <= 2);

    float *buf = new float[vproduct(dims)];

    rc = _getVar(varname, 0, 0, buf);
    if (rc < 0) return (-1);

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

    delete[] buf;

    return (0);
}

int DataMgr::_getCoordPairExtents(string lon, string lat, float &lonmin, float &lonmax, float &latmin, float &latmax)
{
    lonmin = lonmax = latmin = latmax = 0.0;

    int rc = _getLatlonExtents(lon, true, lonmin, lonmax);
    if (rc < 0) return (-1);

    rc = _getLatlonExtents(lat, false, latmin, latmax);
    if (rc < 0) return (-1);

    return (0);
}

int DataMgr::_initProj4StringDefault()
{
    // If data set has a map projection use it
    //
    _proj4StringDefault = _dc->GetMapProjection();
    if (!_proj4StringDefault.empty()) { return (0); }

    // Generate our own proj4 string
    //

    vector<string> meshnames = _dc->GetMeshNames();
    if (meshnames.empty()) return (0);

    vector<string> coordvars;
    for (int i = 0; i < meshnames.size() && coordvars.size() < 2; i++) {
        if (!_hasHorizontalXForm(meshnames[i])) continue;

        DC::Mesh m;
        bool     ok = _dc->GetMesh(meshnames[i], m);
        if (!ok) continue;

        if (m.GetCoordVars().size() < 2) continue;

        coordvars = m.GetCoordVars();
    }
    if (coordvars.empty()) return (0);

    float lonmin, lonmax, latmin, latmax;
    int   rc = _getCoordPairExtents(coordvars[0], coordvars[1], lonmin, lonmax, latmin, latmax);
    if (rc < 0) return (-1);

    float         lon_0 = (lonmin + lonmax) / 2.0;
    float         lat_0 = (latmin + latmax) / 2.0;
    ostringstream oss;
    oss << " +lon_0=" << lon_0 << " +lat_0=" << lat_0;
    _proj4StringDefault = "+proj=eqc +ellps=WGS84" + oss.str();

    return (0);
}

int DataMgr::_initHorizontalCoordVars()
{
    if (!_doTransformHorizontal) return (0);

    if (!_hasHorizontalXForm()) return (0);

    int rc = _initProj4StringDefault();
    if (rc < 0) return (-1);

    // Already initialized via Initialize() options
    //
    if (_proj4String.empty()) { _proj4String = _proj4StringDefault; }

    vector<string> meshnames = _dc->GetMeshNames();

    vector<string> coordvars;
    for (int i = 0; i < meshnames.size(); i++) {
        if (!_hasHorizontalXForm(meshnames[i])) continue;

        DC::Mesh m;
        bool     ok = _dc->GetMesh(meshnames[i], m);
        if (!ok) continue;

        if (m.GetCoordVars().size() < 2) continue;

        coordvars = m.GetCoordVars();
        while (coordvars.size() > 2) { coordvars.pop_back(); }

        vector<string> derivedCoordvars = coordvars;
        _assignHorizontalCoords(derivedCoordvars);

        // no duplicates
        //
        if (_getDerivedCoordVar(derivedCoordvars[0])) continue;
        if (_getDerivedCoordVar(derivedCoordvars[1])) continue;

        DerivedCoordVar_PCSFromLatLon *derivedVar = new DerivedCoordVar_PCSFromLatLon(derivedCoordvars[0], _dc, coordvars, _proj4String, m.GetMeshType() != DC::Mesh::STRUCTURED, true);

        rc = derivedVar->Initialize();
        if (rc < 0) {
            SetErrMsg("Failed to initialize derived coord variable");
            return (-1);
        }
        _dvm.AddCoordVar(derivedVar);

        derivedVar = new DerivedCoordVar_PCSFromLatLon(derivedCoordvars[1], _dc, coordvars, _proj4String, m.GetMeshType() != DC::Mesh::STRUCTURED, false);

        rc = derivedVar->Initialize();
        if (rc < 0) {
            SetErrMsg("Failed to initialize derived coord variable");
            return (-1);
        }

        _dvm.AddCoordVar(derivedVar);
    }

    return (0);
}

int DataMgr::_initVerticalCoordVars()
{
    if (!_doTransformVertical) return (0);

    if (!_hasVerticalXForm()) return (0);

    vector<string> meshnames = _dc->GetMeshNames();

    vector<string> coordvars;
    for (int i = 0; i < meshnames.size(); i++) {
        string standard_name, formula_terms;
        if (!_hasVerticalXForm(meshnames[i], standard_name, formula_terms)) { continue; }

        DC::Mesh m;
        bool     ok = _dc->GetMesh(meshnames[i], m);
        if (!ok) continue;

        assert(m.GetCoordVars().size() > 2);

        DerivedCoordVarStandardWRF_Terrain *derivedVar = new DerivedCoordVarStandardWRF_Terrain(_dc, meshnames[i], formula_terms);

        int rc = derivedVar->Initialize();
        if (rc < 0) {
            SetErrMsg("Failed to initialize derived coord variable");
            return (-1);
        }

        _dvm.AddCoordVar(derivedVar);

        vector<string> coord_vars = m.GetCoordVars();
        coord_vars[2] = derivedVar->GetName();
        m.SetCoordVars(coord_vars);

        _dvm.AddMesh(m);
    }

    return (0);
}

namespace VAPoR {

std::ostream &operator<<(std::ostream &o, const DataMgr::BlkExts &b)
{
    assert(b._bmin.size() == b._bmax.size());
    assert(b._mins.size() == b._maxs.size());

    o << "Block dimensions" << endl;
    for (int i = 0; i < b._bmin.size(); i++) { o << "  " << b._bmin[i] << " " << b._bmax[i] << endl; }
    o << "Block coordinates" << endl;
    for (int i = 0; i < b._mins.size(); i++) {
        assert(b._mins[i].size() == b._maxs[i].size());
        o << "Block index " << i << endl;
        for (int j = 0; j < b._mins[i].size(); j++) { o << "  " << b._mins[i][j] << " " << b._maxs[i][j] << endl; }
        o << endl;
    }

    return (o);
}

};    // namespace VAPoR
