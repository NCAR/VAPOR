#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cstring>
#include "vapor/VAssert.h"
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

// Again, stupid gcc-4.8 on CentOS7 requires this alias.
template<bool B, class T = void> using enable_if_t = typename std::enable_if<B, T>::type;

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

#ifdef UNUSED_FUNCTION

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
    VAssert(src_bs.size() == 1);

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

        VAssert(w <= 1.0);

        dst[ii] += w * src[i];
        if (i % 2) ii++;
    }
}

template<typename T> void decimate2d(const vector<size_t> &src_bs, const T *src, T *dst)
{
    VAssert(src_bs.size() == 2);

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

            VAssert(w <= 1.0);

            dst[jj * ni1 + ii] += w * src[j * ni0 + i];
            if (i % 2) ii++;
        }
        if (j % 2) jj++;
    }
}

template<typename T> void decimate3d(const vector<size_t> &src_bs, const T *src, T *dst)
{
    VAssert(src_bs.size() == 3);

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

                VAssert(w <= 1.0);

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
    VAssert(bmin.size() == bmax.size());
    VAssert(bmin.size() == src_bs.size());
    VAssert(src_bs.size() >= 1 && src_bs.size() <= 3);

    vector<size_t> dst_bs;
    for (int i = 0; i < src_bs.size(); i++) { dst_bs.push_back(decimate_length(src_bs[i])); }

    size_t src_block_size = VProduct(src_bs);
    size_t dst_block_size = VProduct(dst_bs);

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

#endif

void downsample_compute_weights(size_t nIn, size_t nOut, vector<float> &wgts)
{
    VAssert(nOut <= nIn);
    wgts.resize(nOut, 0.0);

    float deltax = (float)nIn / (float)nOut;
    float shift = ((nIn - 1) - (deltax * (nOut - 1))) / 2.0;
    for (int i = 0; i < nOut; i++) { wgts[i] = (i * deltax) + shift; }
}

template<typename T> void downsample1d(const T *signalIn, size_t nIn, size_t strideIn, T *signalOut, size_t nOut, size_t strideOut, const vector<float> &wgts)
{
    VAssert(nOut <= nIn);
    VAssert(nOut == wgts.size());

    for (size_t i = 0; i < nOut; i++) {
        size_t i0 = wgts[i];
        float  w = wgts[i] - i0;
        signalOut[i * strideOut] = (signalIn[i0 * strideIn] * (1.0 - w)) + (signalIn[(i0 + 1) * strideIn] * w);
    }
}

template<typename T> void downsample2d(const T *signalIn, vector<size_t> inDims, T *signalOut, vector<size_t> outDims)
{
    VAssert(inDims.size() == 2);
    VAssert(inDims.size() == outDims.size());

    // Sample along first dimension
    //
    vector<float> wgts;
    downsample_compute_weights(inDims[0], outDims[0], wgts);

    T *buf = new T[inDims[1] * outDims[0]];

    size_t nIn = inDims[0];
    size_t nOut = outDims[0];
    size_t strideIn = 1;
    size_t strideOut = 1;
    size_t n = inDims[1];
    for (int i = 0; i < n; i++) {
        const T *inPtr = signalIn + (i * nIn);
        T *      outPtr = buf + (i * nOut);
        downsample1d(inPtr, nIn, strideIn, outPtr, nOut, strideOut, wgts);
    }

    // Sample along second dimension
    //
    downsample_compute_weights(inDims[1], outDims[1], wgts);

    nIn = inDims[1];
    nOut = outDims[1];
    strideIn = outDims[0];
    strideOut = outDims[0];
    n = outDims[0];
    for (int i = 0; i < n; i++) {
        const T *inPtr = buf + i;
        T *      outPtr = signalOut + i;
        downsample1d(inPtr, nIn, strideIn, outPtr, nOut, strideOut, wgts);
    }

    delete[] buf;
}

template<typename T> void downsample3d(const T *signalIn, vector<size_t> inDims, T *signalOut, vector<size_t> outDims)
{
    VAssert(inDims.size() == 3);
    VAssert(inDims.size() == outDims.size());

    // Sample along XY planes first
    //

    T *buf = new T[inDims[2] * outDims[1] * outDims[0]];

    vector<size_t> inDims2d = {inDims[0], inDims[1]};
    vector<size_t> outDims2d = {outDims[0], outDims[1]};

    size_t nIn = inDims[0] * inDims[1];
    size_t nOut = outDims[0] * outDims[1];
    size_t n = inDims[2];
    for (int i = 0; i < n; i++) {
        const T *inPtr = signalIn + (i * nIn);
        T *      outPtr = buf + (i * nOut);
        downsample2d(inPtr, inDims2d, outPtr, outDims2d);
    }

    // Sample along Z dimension
    //
    vector<float> wgts;
    downsample_compute_weights(inDims[2], outDims[2], wgts);

    nIn = inDims[2];
    nOut = outDims[2];
    size_t strideIn = outDims[0] * outDims[1];
    size_t strideOut = outDims[0] * outDims[1];
    n = outDims[0] * outDims[1];
    for (int i = 0; i < n; i++) {
        const T *inPtr = buf + i;
        T *      outPtr = signalOut + i;
        downsample1d(inPtr, nIn, strideIn, outPtr, nOut, strideOut, wgts);
    }

    delete[] buf;
}

template<typename T> void downsample(const T *signalIn, vector<size_t> inDims, T *signalOut, vector<size_t> outDims)
{
    VAssert(inDims.size() >= 1 && inDims.size() <= 3);
    VAssert(inDims.size() == outDims.size());

    if (inDims.size() == 1) {
        vector<float> wgts;
        downsample_compute_weights(inDims[0], outDims[0], wgts);

        downsample1d(signalIn, inDims[0], 1, signalOut, outDims[0], 1, wgts);
    } else if (inDims.size() == 2) {
        downsample2d(signalIn, inDims, signalOut, outDims);
    } else if (inDims.size() == 3) {
        downsample3d(signalIn, inDims, signalOut, outDims);
    }
}

// Map voxel to block coordinates
//
void map_vox_to_blk(vector<size_t> bs, const vector<size_t> &vcoord, vector<size_t> &bcoord)
{
    VAssert(bs.size() == vcoord.size());
    bcoord.clear();

    for (int i = 0; i < bs.size(); i++) { bcoord.push_back(vcoord[i] / bs[i]); }
}

void map_blk_to_vox(vector<size_t> bs, const vector<size_t> &bmin, const vector<size_t> &bmax, vector<size_t> &vmin, vector<size_t> &vmax)
{
    VAssert(bs.size() == bmin.size());
    VAssert(bs.size() == bmax.size());
    vmin.clear();
    vmax.clear();

    for (int i = 0; i < bs.size(); i++) {
        vmin.push_back(bmin[i] * bs[i]);
        vmax.push_back(bmax[i] * bs[i] + bs[i] - 1);
    }
}

// Map block to vox coordinates, and clamp to dimension boundaries
//
void map_blk_to_vox(const vector<size_t> &bs, const vector<size_t> &dims, const vector<size_t> &bmin, const vector<size_t> &bmax, vector<size_t> &vmin, vector<size_t> &vmax)
{
    VAssert(bs.size() == bmin.size());
    VAssert(bs.size() == bmax.size());
    VAssert(bs.size() == dims.size());
    vmin.clear();
    vmax.clear();

    for (int i = 0; i < bs.size(); i++) {
        vmin.push_back(bmin[i] * bs[i]);
        vmax.push_back(bmax[i] * bs[i] + bs[i] - 1);
        if (vmin[i] >= dims[i]) vmin[i] = dims[i] - 1;
        if (vmax[i] >= dims[i]) vmax[i] = dims[i] - 1;
    }
}

// Copy a contiguous region to a blocked grid
//
// src : pointer to contiguous region
// dst : pointer to blocked grid
// min : min region coordinates within destination grid (in voxels)
// max : max region coordinates within destination grid (in voxels)
// bs : block size of destination grid (in voxels)
// dims : dimensions of destination grid (in voxels)
//
template<class T> void copy_block(const T *src, T *dst, const vector<size_t> &min, const vector<size_t> &max, const vector<size_t> &bs, const vector<size_t> &grid_min, const vector<size_t> &grid_max)
{
    const int ndim = 3;
    VAssert(min.size() <= ndim);
    VAssert(min.size() == max.size());
    VAssert(min.size() == bs.size());
    VAssert(min.size() == grid_min.size());
    VAssert(min.size() == grid_max.size());

    // 3D versions of input parameters
    //
    vector<long>   min3(ndim, 0);
    vector<long>   max3(ndim, 0);
    vector<size_t> bs3(ndim, 1);
    vector<long>   grid_min3(ndim, 0);
    vector<long>   grid_max3(ndim, 0);
    for (int i = 0; i < min.size(); i++) {
        min3[i] = min[i];
        max3[i] = max[i];
        bs3[i] = bs[i];
        grid_min3[i] = grid_min[i];
        grid_max3[i] = grid_max[i];
    }

    vector<size_t> dims3;        // dimensions of destination region in voxels
    vector<size_t> bdims3;       // dimensions of destination grid in blocks
    vector<size_t> src_dims3;    // dimensions of source region in voxels
    for (int i = 0; i < ndim; i++) {
        dims3.push_back(grid_max3[i] - grid_min3[i] + 1);
        bdims3.push_back(((dims3[i] - 1) / bs3[i]) + 1);
        src_dims3.push_back(max3[i] - min3[i] + 1);
    }

    // Input coordinates are specified relative to origin of entire
    // domain, but 'dst' region origin is at 'grid_min'
    //
    for (int i = 0; i < min3.size(); i++) {
        min3[i] -= grid_min3[i];
        max3[i] -= grid_min3[i];
    }

    size_t block_size = VProduct(bs3);

    for (long k = min3[2], kk = 0; k <= (long)max3[2] && k < (long)dims3[2]; k++, kk++) {
        if (k < 0) continue;

        // Coordinates of destination block (block coordinates)
        //
        size_t dst_k_b = k / bs3[2];

        // Coordinates within destination block (voxel coordinates)
        //
        size_t dst_k = k % bs3[2];

        for (long j = min3[1], jj = 0; j <= (long)max3[1] && j < (long)dims3[1]; j++, jj++) {
            if (j < 0) continue;
            size_t dst_j_b = j / bs3[1];
            size_t dst_j = j % bs3[1];

            for (long i = min3[0], ii = 0; i <= (long)max3[0] && i < (long)dims3[0]; i++, ii++) {
                if (i < 0) continue;

                size_t dst_i_b = i / bs3[0];
                size_t dst_i = i % bs3[0];

                size_t dst_block_offset = (dst_k_b * bdims3[0] * bdims3[1] + dst_j_b * bdims3[0] + dst_i_b) * block_size;

                size_t dst_offset = dst_k * bs3[0] * bs3[1] + dst_j * bs3[0] + dst_i;

                size_t src_offset = kk * src_dims3[0] * src_dims3[1] + jj * src_dims3[0] + ii;

                dst[dst_block_offset + dst_offset] = src[src_offset];
            }
        }
    }
}

bool is_blocked(const vector<size_t> &bs)
{
    return (!std::all_of(bs.cbegin(), bs.cend(), [](size_t i) { return i == 1; }));
}

// Is string a number?
//
bool is_int(std::string str)
{
    if (!str.empty() && str[0] == '-') str.erase(0, 1);
    return str.find_first_not_of("0123456789") == std::string::npos;
}

template<typename T, enable_if_t<std::is_floating_point<T>::value, int> = 0> void _sanitizeFloats(T *buffer, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (std::isnan(buffer[i])) buffer[i] = std::numeric_limits<T>::infinity();
    }
}

// MSVC has a bug where isnan() is not overloaded for integral types.
// This function specializes the template to bypass this bug.
template<typename T, enable_if_t<std::is_integral<T>::value, int> = 0> void _sanitizeFloats(T *buffer, size_t n) {}

template<typename T> bool contains(const vector<T> &v, T element) { return (find(v.begin(), v.end(), element) != v.end()); }

};    // namespace

DataMgr::DataMgr(string format, size_t mem_size, int nthreads)
{
    SetDiagMsg("DataMgr::DataMgr(%s,%d,%d)", format.c_str(), nthreads, mem_size);

    _format = format;
    _nthreads = nthreads;
    _mem_size = mem_size;

    if (!_mem_size) _mem_size = std::numeric_limits<size_t>::max() / 1048576;

    _dc = NULL;

    _blk_mem_mgr = NULL;

    _PipeLines.clear();

    _regionsList.clear();

    _varInfoCacheSize_T.Clear();
    _varInfoCacheDouble.Clear();
    _varInfoCacheVoidPtr.Clear();

    _doTransformHorizontal = false;
    _doTransformVertical = false;
    _openVarName.clear();
    _proj4String.clear();
    _proj4StringDefault.clear();
    _bs = {64, 64, 64};
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
    VAssert(_dc);

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
    VAssert(_dc);

    vector<string> validvars;
    for (int ndim = 2; ndim <= 3; ndim++) {
        vector<string> vars = GetDataVarNames(ndim);
        validvars.insert(validvars.end(), vars.begin(), vars.end());
    }
    return (validvars);
}

vector<string> DataMgr::GetDataVarNames(int ndim) const
{
    VAssert(_dc);

    if (_dataVarNamesCache[ndim].size()) { return (_dataVarNamesCache[ndim]); }

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

    _dataVarNamesCache[ndim] = validVars;
    return (validVars);
}

vector<string> DataMgr::GetCoordVarNames() const
{
    VAssert(_dc);

    vector<string> vars = _dc->GetCoordVarNames();
    vector<string> derived_vars = _dvm.GetCoordVarNames();
    vars.insert(vars.end(), derived_vars.begin(), derived_vars.end());

    return (vars);
}

string DataMgr::GetTimeCoordVarName() const
{
    VAssert(_dc);

    // There can be only one time coordinate variable. If a
    // derived one exists, use it.
    //
    vector<string> cvars = _dvm.GetTimeCoordVarNames();
    if (!cvars.empty()) return (cvars[0]);

    cvars = _dc->GetTimeCoordVarNames();
    if (!cvars.empty()) return (cvars[0]);

    return ("");
}

bool DataMgr::GetVarCoordVars(string varname, bool spatial, std::vector<string> &coord_vars) const
{
    VAssert(_dc);

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
    VAssert(_dc);

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
    VAssert(_dc);

    bool ok = _dvm.GetCoordVarInfo(varname, var);
    if (!ok) { ok = _dc->GetCoordVarInfo(varname, var); }
    return (ok);
}

bool DataMgr::GetBaseVarInfo(string varname, VAPoR::DC::BaseVar &var) const
{
    VAssert(_dc);

    bool ok = _dvm.GetBaseVarInfo(varname, var);
    if (!ok) { ok = _dc->GetBaseVarInfo(varname, var); }
    return (ok);
}

bool DataMgr::IsTimeVarying(string varname) const
{
    VAssert(_dc);

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
    VAssert(_dc);

    DC::BaseVar var;

    // No error checking here!!!!!
    //
    bool status = GetBaseVarInfo(varname, var);
    if (!status) return (status);

    return (var.IsCompressed());
}

int DataMgr::GetNumTimeSteps(string varname) const
{
    VAssert(_dc);

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
    VAssert(_dc);

    if (varname == "") return 1;

    DerivedVar *dvar = _getDerivedVar(varname);
    if (dvar) { return (dvar->GetNumRefLevels()); }

    return (_dc->GetNumRefLevels(varname));
}

vector<size_t> DataMgr::GetCRatios(string varname) const
{
    VAssert(_dc);

    if (varname == "") return vector<size_t>(1, 1);

    DerivedVar *dvar = _getDerivedVar(varname);
    if (dvar) { return (dvar->GetCRatios()); }

    return (_dc->GetCRatios(varname));
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
    VAssert(min.size() == max.size());

    SetDiagMsg("DataMgr::GetVariable(%d, %s, %d, %d, %s, %s, %d)", ts, varname.c_str(), level, lod, vector_to_string(min).c_str(), vector_to_string(max).c_str(), lock);

    int rc = _level_correction(varname, level);
    if (rc < 0) return (NULL);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (NULL);

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

    return (DataMgr::GetVariable(ts, varname, level, lod, min_ui, max_ui, lock));
}

Grid *DataMgr::_getVariable(size_t ts, string varname, int level, int lod, bool lock, bool dataless)
{
    if (!VariableExists(ts, varname, level, lod)) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (NULL);
    }

    vector<size_t> dims_at_level;
    int            rc = GetDimLensAtLevel(varname, level, dims_at_level);
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
void DataMgr::_setupCoordVecsHelper(string data_varname, const vector<size_t> &data_dimlens, const vector<size_t> &data_bmin, const vector<size_t> &data_bmax, string coord_varname, int order,
                                    vector<size_t> &coord_dimlens, vector<size_t> &coord_bmin, vector<size_t> &coord_bmax, bool structured) const
{
    VAssert(data_bmin.size() == data_bmax.size());
    coord_dimlens.clear();
    coord_bmin.clear();
    coord_bmax.clear();

    vector<DC::Dimension> data_dims;
    bool                  ok = _getVarDimensions(data_varname, data_dims);
    VAssert(ok);
    VAssert(data_dims.size() == data_bmin.size());

    vector<DC::Dimension> coord_dims;
    ok = _getVarDimensions(coord_varname, coord_dims);
    VAssert(ok);

    if (structured) {
        // For structured data
        // Here we try to match dimensions of coordinate variables
        // with dimensions of data variables. Ideally, this would be done
        // by matching the dimension names. However, some CF data sets (e.g. MOM)
        // have data and coordinate variables that use different dimension
        // names (but have the same length)
        //
        VAssert(data_dims.size() >= 1 && data_dims.size() <= 3);
        if (data_dims.size() == 1) {
            VAssert(coord_dims.size() == 1);
            VAssert(order == 0);
            VAssert(data_dims[0].GetLength() == coord_dims[0].GetLength());

            coord_dimlens.push_back(data_dimlens[0]);
            coord_bmin.push_back(data_bmin[0]);
            coord_bmax.push_back(data_bmax[0]);
        } else if (data_dims.size() == 2) {
            VAssert(coord_dims.size() >= 1 && coord_dims.size() <= 2);
            VAssert(order >= 0 && order <= 1);

            if (coord_dims.size() == 1) {
                VAssert(data_dims[order].GetLength() == coord_dims[0].GetLength());

                coord_dimlens.push_back(data_dimlens[order]);
                coord_bmin.push_back(data_bmin[order]);
                coord_bmax.push_back(data_bmax[order]);
            } else {
                VAssert(data_dims[0].GetLength() == coord_dims[0].GetLength());
                VAssert(data_dims[1].GetLength() == coord_dims[1].GetLength());

                coord_dimlens.push_back(data_dimlens[0]);
                coord_bmin.push_back(data_bmin[0]);
                coord_bmax.push_back(data_bmax[0]);
                coord_dimlens.push_back(data_dimlens[1]);
                coord_bmin.push_back(data_bmin[1]);
                coord_bmax.push_back(data_bmax[1]);
            }
        } else if (data_dims.size() == 3) {
            VAssert(coord_dims.size() >= 1 && coord_dims.size() <= 3);
            VAssert(order >= 0 && order <= 2);

            if (coord_dims.size() == 1) {
                VAssert(data_dims[order].GetLength() == coord_dims[0].GetLength());

                coord_dimlens.push_back(data_dimlens[order]);
                coord_bmin.push_back(data_bmin[order]);
                coord_bmax.push_back(data_bmax[order]);
            } else if (coord_dims.size() == 2) {
                // We assume 2D coordinates are horizontal (in XY plane).
                // No way currently to distinguish XZ and YZ plane 2D coordinates
                // for 3D data :-(
                //
                VAssert(order >= 0 && order <= 1);
                coord_dimlens.push_back(data_dimlens[0]);
                coord_bmin.push_back(data_bmin[0]);
                coord_bmax.push_back(data_bmax[0]);
                coord_dimlens.push_back(data_dimlens[1]);
                coord_bmin.push_back(data_bmin[1]);
                coord_bmax.push_back(data_bmax[1]);

            } else if (coord_dims.size() == 3) {
                coord_dimlens.push_back(data_dimlens[0]);
                coord_bmin.push_back(data_bmin[0]);
                coord_bmax.push_back(data_bmax[0]);
                coord_dimlens.push_back(data_dimlens[1]);
                coord_bmin.push_back(data_bmin[1]);
                coord_bmax.push_back(data_bmax[1]);
                coord_dimlens.push_back(data_dimlens[2]);
                coord_bmin.push_back(data_bmin[2]);
                coord_bmax.push_back(data_bmax[2]);
            }
        } else {
            VAssert(0 && "Only 1, 2, or 3D data supported");
        }

    } else {
        // For unstructured data we can just match data dimension names
        // with coord dimension names. Yay!
        //
        int i = 0;
        for (int j = 0; j < coord_dims.size(); j++) {
            while (data_dims[i].GetLength() != coord_dims[j].GetLength() && i < data_dims.size()) { i++; }
            VAssert(i < data_dims.size());
            coord_dimlens.push_back(data_dimlens[i]);
            coord_bmin.push_back(data_bmin[i]);
            coord_bmax.push_back(data_bmax[i]);
        }
    }
}

int DataMgr::_setupCoordVecs(size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, vector<string> &varnames, vector<size_t> &roi_dims,
                             vector<vector<size_t>> &dimsvec, vector<vector<size_t>> &bsvec, vector<vector<size_t>> &bminvec, vector<vector<size_t>> &bmaxvec, bool structured) const
{
    varnames.clear();
    roi_dims.clear();
    dimsvec.clear();
    bsvec.clear();
    bminvec.clear();
    bmaxvec.clear();

    // Compute dimenions of ROI
    //
    for (int i = 0; i < min.size(); i++) { roi_dims.push_back(max[i] - min[i] + 1); }

    // Grid and block dimensions at requested refinement
    //
    vector<size_t> dims;
    int            rc = GetDimLensAtLevel(varname, level, dims);
    VAssert(rc >= 0);
    dimsvec.push_back(dims);

    vector<size_t> bs(_bs.begin(), _bs.begin() + dims.size());
    bsvec.push_back(bs);

    // Map voxel coordinates into block coordinates
    //
    vector<size_t> bmin, bmax;
    map_vox_to_blk(bs, min, bmin);
    map_vox_to_blk(bs, max, bmax);
    bminvec.push_back(bmin);
    bmaxvec.push_back(bmax);

    vector<string> cvarnames;
    bool           ok = GetVarCoordVars(varname, true, cvarnames);
    VAssert(ok);

    for (int i = 0; i < cvarnames.size(); i++) {
        // Map data indices to coordinate indices. Coordinate indices
        // are a subset of the data indices.
        //
        vector<size_t> coord_dims, coord_bmin, coord_bmax;
        _setupCoordVecsHelper(varname, dims, bmin, bmax, cvarnames[i], i, coord_dims, coord_bmin, coord_bmax, structured);

        vector<size_t> bs(_bs.begin(), _bs.begin() + coord_bmin.size());

        dimsvec.push_back(coord_dims);
        bsvec.push_back(bs);
        bminvec.push_back(coord_bmin);
        bmaxvec.push_back(coord_bmax);
    }

    varnames.push_back(varname);
    varnames.insert(varnames.end(), cvarnames.begin(), cvarnames.end());

    return (0);
}

int DataMgr::_setupConnVecs(size_t ts, string varname, int level, int lod, vector<string> &varnames, vector<vector<size_t>> &dimsvec, vector<vector<size_t>> &bsvec, vector<vector<size_t>> &bminvec,
                            vector<vector<size_t>> &bmaxvec) const
{
    varnames.clear();
    dimsvec.clear();
    bsvec.clear();
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
        int            rc = GetDimLensAtLevel(name, level, dims);
        if (rc < 0) {
            SetErrMsg("Invalid variable reference : %s", name.c_str());
            return (-1);
        }

        // Ugh. Connection data are not blocked.
        //
        vector<size_t> bs = dims;

        // Always read the entire connection variable
        //
        vector<size_t> conn_min = vector<size_t>(dims.size(), 0);
        vector<size_t> conn_max = dims;
        for (int i = 0; i < conn_max.size(); i++) { conn_max[i]--; }

        // Map voxel coordinates into block coordinates
        //
        vector<size_t> bmin, bmax;
        map_vox_to_blk(bs, conn_min, bmin);
        map_vox_to_blk(bs, conn_max, bmax);

        dimsvec.push_back(dims);
        bsvec.push_back(bs);
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
    VAssert(status);

    vector<DC::CoordVar> cvarsinfo;
    DC::CoordVar         dummy;
    status = _get_coord_vars(varname, cvarsinfo, dummy);
    VAssert(status);

    vector<string>         varnames;
    vector<size_t>         roi_dims;
    vector<vector<size_t>> dimsvec;
    vector<vector<size_t>> bsvec;
    vector<vector<size_t>> bminvec;
    vector<vector<size_t>> bmaxvec;

    // Get dimensions for coordinate variables
    //
    int rc = _setupCoordVecs(ts, varname, level, lod, min, max, varnames, roi_dims, dimsvec, bsvec, bminvec, bmaxvec, !_gridHelper.IsUnstructured(gridType));
    if (rc < 0) return (NULL);

    //
    // if dataless we only load coordinate data
    //
    if (dataless) varnames[0].clear();

    vector<float *> blkvec;
    rc = DataMgr::_get_regions<float>(ts, varnames, level, lod, true, dimsvec, bsvec, bminvec, bmaxvec, blkvec);
    if (rc < 0) return (NULL);

    // Get dimensions for connectivity variables (if any)
    //
    vector<string>         conn_varnames;
    vector<vector<size_t>> conn_dimsvec;
    vector<vector<size_t>> conn_bsvec;
    vector<vector<size_t>> conn_bminvec;
    vector<vector<size_t>> conn_bmaxvec;

    vector<int *> conn_blkvec;
    if (_gridHelper.IsUnstructured(gridType)) {
        rc = _setupConnVecs(ts, varname, level, lod, conn_varnames, conn_dimsvec, conn_bsvec, conn_bminvec, conn_bmaxvec);
        if (rc < 0) return (NULL);

        rc = DataMgr::_get_regions<int>(ts, conn_varnames, level, lod, true, conn_dimsvec, conn_bsvec, conn_bminvec, conn_bmaxvec, conn_blkvec);
        if (rc < 0) return (NULL);
    }

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

        rg = _gridHelper.MakeGridUnstructured(gridType, ts, level, lod, dvar, cvarsinfo, roi_dims, dimsvec[0], blkvec, bsvec, bminvec, bmaxvec, conn_blkvec, conn_bsvec, conn_bminvec, conn_bmaxvec,
                                              vertexDims, faceDims, edgeDims, location, maxVertexPerFace, maxFacePerVertex, vertexOffset, faceOffset);
    } else {
        rg = _gridHelper.MakeGridStructured(gridType, ts, level, lod, dvar, cvarsinfo, roi_dims, dimsvec[0], blkvec, bsvec, bminvec, bmaxvec);
    }
    VAssert(rg);

    //
    // Inform the grid of the offsets from the larger mesh to the
    // mesh subset contained in g. In general, gmin<=min
    //
    vector<size_t> gmin, gmax;
    map_blk_to_vox(bsvec[0], dimsvec[0], bminvec[0], bmaxvec[0], gmin, gmax);
    rg->SetMinAbs(gmin);

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
    VAssert(min.size() == max.size());

    SetDiagMsg("DataMgr::GetVariable(%d, %s, %d, %d, %s, %s, %d)", ts, varname.c_str(), level, lod, vector_to_string(min).c_str(), vector_to_string(max).c_str(), lock);

    int rc = _level_correction(varname, level);
    if (rc < 0) return (NULL);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (NULL);

    // Make sure variable dimensions match extents specification
    //
    vector<string> coord_vars;
    bool           ok = GetVarCoordVars(varname, true, coord_vars);
    VAssert(ok);

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

int DataMgr::GetVariableExtents(size_t ts, string varname, int level, int lod, vector<double> &min, vector<double> &max)
{
    SetDiagMsg("DataMgr::GetVariableExtents(%d, %s, %d, %d)", ts, varname.c_str(), level, lod);

    min.clear();
    max.clear();

    int rc = _lod_correction(varname, lod);
    if (rc < 0) return (-1);

    rc = _level_correction(varname, level);
    if (rc < 0) return (-1);

    vector<string> cvars;
    string         dummy;
    bool           ok = _get_coord_vars(varname, cvars, dummy);
    if (!ok) return (-1);

    string         key = "VariableExtents";
    vector<double> values;
    if (_varInfoCacheDouble.Get(ts, cvars, level, lod, key, values)) {
        int n = values.size();
        for (int i = 0; i < n / 2; i++) {
            min.push_back(values[i]);
            max.push_back(values[i + (n / 2)]);
        }
        return (0);
    }

    Grid *rg = _getVariable(ts, varname, level, lod, false, true);
    if (!rg) return (-1);

    rg->GetUserExtents(min, max);

    // Cache results
    //
    values.clear();
    for (int i = 0; i < min.size(); i++) values.push_back(min[i]);
    for (int i = 0; i < max.size(); i++) values.push_back(max[i]);
    _varInfoCacheDouble.Set(ts, cvars, level, lod, key, values);

    return (0);
}

int DataMgr::GetDataRange(size_t ts, string varname, int level, int lod, vector<double> &range)
{
    SetDiagMsg("DataMgr::GetDataRange(%d,%s)", ts, varname.c_str());

    vector<double> min, max;
    int            rc = GetVariableExtents(ts, varname, level, lod, min, max);
    if (rc < 0) return (-1);

    return (GetDataRange(ts, varname, level, lod, min, max, range));
}

int DataMgr::GetDataRange(size_t ts, string varname, int level, int lod, vector<double> min, vector<double> max, vector<double> &range)
{
    SetDiagMsg("DataMgr::GetDataRange(%d,%s)", ts, varname.c_str());

    range = {0.0, 0.0};

    int rc = _level_correction(varname, level);
    if (rc < 0) return (-1);

    rc = _lod_correction(varname, lod);
    if (rc < 0) return (-1);

    //
    // Find the coordinates in voxels of the grid that contains
    // the axis aligned bounding box specified in user coordinates
    // by min and max
    //
    vector<size_t> min_ui, max_ui;
    rc = _find_bounding_grid(ts, varname, level, lod, min, max, min_ui, max_ui);
    if (rc < 0) return (-1);

    // See if we've already cache'd it.
    //
    ostringstream oss;
    oss << "VariableRange";
    oss << vector_to_string(min_ui);
    oss << vector_to_string(max_ui);
    string key = oss.str();

    if (_varInfoCacheDouble.Get(ts, varname, level, lod, key, range)) {
        VAssert(range.size() == 2);
        return (0);
    }

    const Grid *sg = DataMgr::GetVariable(ts, varname, level, lod, min_ui, max_ui, false);
    if (!sg) return (-1);

    float range_f[2];
    sg->GetRange(range_f);
    range = {range_f[0], range_f[1]};

    delete sg;

    _varInfoCacheDouble.Set(ts, varname, level, lod, key, range);

    return (0);
}

int DataMgr::GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    VAssert(_dc);
    dims_at_level.clear();
    bs_at_level.clear();

    int         rc = 0;
    DerivedVar *dvar = _getDerivedVar(varname);
    if (dvar) {
        rc = dvar->GetDimLensAtLevel(level, dims_at_level, bs_at_level);
    } else {
        rc = _dc->GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level);
    }
    if (rc < 0) return (-1);

#ifdef DEAD
    // If data are not blocked (i.e. if all elements of bs_at_level
    // are 1) set the block size to the dimension size. This forces
    // non-blocked data to be read as a single block, avoiding the
    // need to block the data into some arbitrary size, but at the same
    // time prevening non-blocked data from being subset :-(
    //
    if (std::all_of(bs_at_level.cbegin(), bs_at_level.cend(), [](size_t i) { return i == 1; })) { bs_at_level = dims_at_level; }
#endif

    return (0);
}

vector<string> DataMgr::_get_var_dependencies_1(string varname) const
{
    vector<string> varnames;

    DerivedVar *derivedVar = _getDerivedVar(varname);
    if (derivedVar) { varnames = derivedVar->GetInputs(); }

    // No more dependencies if not a data variable
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

    sort(varnames.begin(), varnames.end());
    varnames.erase(unique(varnames.begin(), varnames.end()), varnames.end());
    return (varnames);
}

vector<string> DataMgr::_get_var_dependencies_all(vector<string> varnames, vector<string> dependencies) const
{
    // Recursively look for all of a variable's dependencies, handing any
    // cycles in the dependency graph
    //
    vector<string> new_varnames;
    for (int i = 0; i < varnames.size(); i++) {
        vector<string> new_dependencies = _get_var_dependencies_1(varnames[i]);

        for (int j = 0; j < new_dependencies.size(); j++) {
            // Avoid cycles by not adding variable names that are already
            // in the dependencies vector
            //
            if (!contains(dependencies, new_dependencies[j])) {
                dependencies.push_back(new_dependencies[j]);
                new_varnames.push_back(new_dependencies[j]);
            }
        }
    }

    if (new_varnames.size()) { return (_get_var_dependencies_all(new_varnames, dependencies)); }
    return (dependencies);
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
    vector<size_t> found;
    if (_varInfoCacheSize_T.Get(ts, varname, level, lod, key, found)) { return (found[0]); }

    //
    // Recursively find all variable dependencies needed by this
    // varible. E.g. coordinate variables, input for derived variables,
    // auxillary variables
    //
    vector<string> varnames = _get_var_dependencies_all(vector<string>({varname}), vector<string>());
    varnames.insert(varnames.begin(), varname);

    // Now check for the existence of the variable and all of its
    // dependencies, updating variable existence cache in process
    //
    for (int i = 0; i < varnames.size(); i++) {
        if (_varInfoCacheSize_T.Get(ts, varnames[i], level, lod, key, found)) {
            if (found[0])
                continue;
            else
                return (false);
        }

        if (DataMgr::IsVariableNative(varnames[i])) {
            bool exists = _dc->VariableExists(ts, varnames[i], level, lod);
            if (!exists) {
                _varInfoCacheSize_T.Set(ts, varnames[i], level, lod, key, vector<size_t>({0}));
                return (false);
            }
        } else {
            DerivedVar *derivedVar = _getDerivedVar(varnames[i]);
            if (!derivedVar) {
                _varInfoCacheSize_T.Set(ts, varnames[i], level, lod, key, vector<size_t>({0}));
                return (false);
            }
        }
    }

    // Cache found results for later.
    //
    for (int i = 0; i < varnames.size(); i++) { _varInfoCacheSize_T.Set(ts, varnames[i], level, lod, key, vector<size_t>({1})); }
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

int DataMgr::AddDerivedVar(DerivedDataVar *derivedVar)
{
    string varname = derivedVar->GetName();

    if (_dvm.HasVar(varname)) {
        SetErrMsg("Variable named %s already defined", varname.c_str());
        return (-1);
    }
    _dvm.AddDataVar(derivedVar);

    //
    // Clear variable name cache
    //
    for (auto itr = _dataVarNamesCache.begin(); itr != _dataVarNamesCache.end(); ++itr) {
        vector<string> &ref = itr->second;
        ref.clear();
    }

    _varInfoCacheSize_T.Purge(vector<string>({varname}));

    return (0);
}

void DataMgr::RemoveDerivedVar(string varname)
{
    if (!_dvm.HasVar(varname)) return;

    _dvm.RemoveVar(_dvm.GetVar(varname));

    _free_var(varname);

    //
    // Clear variable name cache
    //
    for (auto itr = _dataVarNamesCache.begin(); itr != _dataVarNamesCache.end(); ++itr) {
        vector<string> &ref = itr->second;
        ref.clear();
    }

    _varInfoCacheSize_T.Purge(vector<string>({varname}));
}

void DataMgr::Clear()
{
    _PipeLines.clear();

    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        const region_t &region = *itr;

        if (region.blks) _blk_mem_mgr->FreeMem(region.blks);
    }
    _regionsList.clear();
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
    VAssert(_dc);

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
    VAssert(_dc);

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

template<typename T>
int DataMgr::_get_unblocked_region_from_fs(size_t ts, string varname, int level, int lod, const vector<size_t> &grid_dims, const vector<size_t> &grid_bs, const vector<size_t> &grid_min,
                                           const vector<size_t> &grid_max, T *blks)
{
    int fd = _openVariableRead(ts, varname, level, lod);
    if (fd < 0) return (fd);

    T *region = new T[VProduct(Dims(grid_min, grid_max))];

    int nlevels = DataMgr::GetNumRefLevels(varname);

    // Downsample the data if needed
    //
    if (level < -nlevels) {
        vector<size_t> dims;
        int            rc = GetDimLensAtLevel(varname, level, dims);
        VAssert(rc >= 0);
        VAssert(dims.size() == grid_dims.size());

        // grid_min and grid_max are specified in voxel coordinates
        // relative to the downsampled grid. Figure out coordinates for
        // region we need on the native grid
        //
        vector<size_t> file_min, file_max;
        for (int i = 0; i < dims.size(); i++) {
            vector<float> weights;
            downsample_compute_weights(dims[i], grid_dims[i], weights);
            int loffset = (int)weights[0];
            int roffset = (dims[i] - 1) - (int)(weights[weights.size() - 1] + 1.0);
            file_min.push_back((int)weights[grid_min[i]] - loffset);
            file_max.push_back((int)weights[grid_max[i]] + 1 + roffset);
        }

        T *buf = new T[VProduct(Dims(file_min, file_max))];

        rc = _readRegion(fd, file_min, file_max, buf);
        if (rc < 0) {
            delete[] buf;
            return (-1);
        }

        downsample(buf, Dims(file_min, file_max), region, Dims(grid_min, grid_max));

        if (buf) delete[] buf;
    } else {
        int rc = _readRegion(fd, grid_min, grid_max, region);
        if (rc < 0) {
            if (region) delete[] region;
            return (-1);
        }
    }

    copy_block(region, blks, grid_min, grid_max, grid_bs, grid_min, grid_max);

    (void)_closeVariable(fd);

    if (region) delete[] region;

    return (0);
}

template<typename T>
int DataMgr::_get_blocked_region_from_fs(size_t ts, string varname, int level, int lod, const vector<size_t> &file_bs, const vector<size_t> &file_dims, const vector<size_t> &grid_dims,
                                         const vector<size_t> &grid_bs, const vector<size_t> &grid_min, const vector<size_t> &grid_max, T *blks)
{
    // Map requested region voxel coordinates to disk block coordinates
    //
    vector<size_t> file_bmin, file_bmax;
    map_vox_to_blk(file_bs, grid_min, file_bmin);
    map_vox_to_blk(file_bs, grid_max, file_bmax);

    int fd = _openVariableRead(ts, varname, level, lod);
    if (fd < 0) return (fd);

    std::vector<size_t> bmin = file_bmin;
    std::vector<size_t> bmax = file_bmax;

    // For 3D data read 2D slabs one at a time. This just reduces
    // memory requirements for the temporary buffer we need
    //
    size_t nreads = 1;
    if (bmin.size() == 3 && bmax[2] > bmin[2]) {
        nreads = bmax[2] - bmin[2] + 1;
        bmax[2] = bmin[2];
    }

    vector<size_t> file_min, file_max;
    map_blk_to_vox(file_bs, bmin, bmax, file_min, file_max);
    T *file_block = new T[VProduct(Dims(file_min, file_max))];

    for (size_t i = 0; i < nreads; i++) {
        map_blk_to_vox(file_bs, file_dims, bmin, bmax, file_min, file_max);

        int rc = _readRegion(fd, file_min, file_max, file_block);
        if (rc < 0) {
            delete[] file_block;
            return (-1);
        }

        copy_block(file_block, blks, file_min, file_max, grid_bs, grid_min, grid_max);

        // Increment along slowest axis (2)
        // This is a no-op if less than 3 dimensions
        //
        bmin = IncrementCoords(file_bmin, file_bmax, bmin, 2);
        bmax = IncrementCoords(file_bmin, file_bmax, bmax, 2);
    }

    (void)_closeVariable(fd);

    if (file_block) delete[] file_block;

    return (0);
}

template<typename T>
T *DataMgr::_get_region_from_fs(size_t ts, string varname, int level, int lod, const vector<size_t> &grid_dims, const vector<size_t> &grid_bs, const vector<size_t> &grid_bmin,
                                const vector<size_t> &grid_bmax, bool lock)
{
    T *blks = (T *)_alloc_region(ts, varname, level, lod, grid_bmin, grid_bmax, grid_bs, sizeof(T), lock, false);
    if (!blks) return (NULL);

    vector<size_t> file_dims, file_bs;
    int            rc = GetDimLensAtLevel(varname, level, file_dims, file_bs);
    VAssert(rc >= 0);

    // Get voxel coordinates of requested region, clamped to grid
    // boundaries.
    //
    vector<size_t> grid_min, grid_max;
    map_blk_to_vox(grid_bs, grid_dims, grid_bmin, grid_bmax, grid_min, grid_max);

    int nlevels = DataMgr::GetNumRefLevels(varname);

    // If data aren't blocked on disk or if the requested level is not
    // available do a non-blocked read
    //
    if (!is_blocked(file_bs) || level < -nlevels) {
        rc = _get_unblocked_region_from_fs(ts, varname, level, lod, grid_dims, grid_bs, grid_min, grid_max, blks);
    } else {
        rc = _get_blocked_region_from_fs(ts, varname, level, lod, file_bs, file_dims, grid_dims, grid_bs, grid_min, grid_max, blks);
    }
    if (rc < 0) {
        _free_region(ts, varname, level, lod, grid_bmin, grid_bmax, true);
        return (NULL);
    }

    SetDiagMsg("DataMgr::GetGrid() - data read from fs\n");
    return (blks);
}

template<typename T>
T *DataMgr::_get_region(size_t ts, string varname, int level, int lod, int nlods, const vector<size_t> &dims, const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax,
                        bool lock)
{
    if (lod < -nlods) lod = -nlods;

    // See if region is already in cache. If not, read from the
    // file system.
    //
    T *blks = _get_region_from_cache<T>(ts, varname, level, lod, bmin, bmax, lock);
    if (!blks) { blks = (T *)_get_region_from_fs<T>(ts, varname, level, lod, dims, bs, bmin, bmax, lock); }
    if (!blks) {
        SetErrMsg("Failed to read region from variable/timestep/level/lod (%s, %d, %d, %d)", varname.c_str(), ts, level, lod);
        return (NULL);
    }
    return (blks);
}

template<typename T>
int DataMgr::_get_regions(size_t ts, const vector<string> &varnames, int level, int lod, bool lock, const vector<vector<size_t>> &dimsvec,
                          const vector<vector<size_t>> &bsvec,    // native coordinates
                          const vector<vector<size_t>> &bminvec, const vector<vector<size_t>> &bmaxvec, vector<T *> &blkvec)
{
    blkvec.clear();

    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i].empty()) {    // nothing to do
            blkvec.push_back(NULL);
            continue;
        }

        DC::BaseVar var;
        int         rc = GetBaseVarInfo(varnames[i], var);
        if (rc < 0) return (rc);

        int nlods = var.GetCRatios().size();

        size_t my_ts = ts;

        // If variable isn't time varying time step should always be 0
        //
        if (!DataMgr::IsTimeVarying(varnames[i])) my_ts = 0;

        T *blks = _get_region<T>(my_ts, varnames[i], level, lod, nlods, dimsvec[i], bsvec[i], bminvec[i], bmaxvec[i], true);
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
    VAssert(bmin.size() == bmax.size());
    VAssert(bmin.size() == bs.size());

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
    _free_region(ts, varname, level, lod, bmin, bmax, true);

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

void DataMgr::_free_region(size_t ts, string varname, int level, int lod, vector<size_t> bmin, vector<size_t> bmax, bool forceFlag)
{
    list<region_t>::iterator itr;
    for (itr = _regionsList.begin(); itr != _regionsList.end(); itr++) {
        const region_t &region = *itr;

        if (region.ts == ts && region.varname.compare(varname) == 0 && region.level == level && region.lod == lod && region.bmin == bmin && region.bmax == bmax) {
            if (region.lock_counter == 0 || forceFlag) {
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

    _varInfoCacheSize_T.Purge(vector<string>(1, varname));
    _varInfoCacheDouble.Purge(vector<string>(1, varname));
    _varInfoCacheVoidPtr.Purge(vector<string>(1, varname));
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

bool DataMgr::_hasHorizontalXForm() const
{
    VAssert(_dc);

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
        VAssert(ok);

        //
        if (varInfo.GetUnits().empty()) continue;

        // Version 1.0 of CF conventions allows "degrees" as units
        // for both lat and longitude. So we check IsLonUnit,
        // which looks for "degrees_east", "degrees_E", etc., and
        // IsLatOrLonUnit, which will return true for "degrees". Unforunately,
        // IsLatOrLonUnit will also return true for an empty string "", so
        // we explicitly test for that above.
        //
        if (varInfo.GetAxis() == 0 && (_udunits.IsLonUnit(varInfo.GetUnits()) || _udunits.IsLatOrLonUnit(varInfo.GetUnits()))) { return (true); }
        if (varInfo.GetAxis() == 1 && (_udunits.IsLatUnit(varInfo.GetUnits()) || _udunits.IsLatOrLonUnit(varInfo.GetUnits()))) { return (true); }
    }

    return (false);
}

bool DataMgr::_hasVerticalXForm() const
{
    VAssert(_dc);

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

    if (m.GetDimNames().size() != 3) return (false);

    vector<string> coordVars = m.GetCoordVars();

    bool         hasVertCoord = false;
    DC::CoordVar cvarInfo;
    for (int i = 0; i < coordVars.size(); i++) {
        bool ok = _dc->GetCoordVarInfo(coordVars[i], cvarInfo);
        VAssert(ok);

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

    // Make sure all of the dependent variables needed by the
    // formula actually exist
    //
    map<string, string> parsed_terms;
    ok = DerivedCFVertCoordVar::ParseFormula(formula_terms, parsed_terms);
    if (!ok) return (false);

    for (auto itr = parsed_terms.begin(); itr != parsed_terms.end(); ++itr) {
        const string &varname = itr->second;
        if (!_dc->VariableExists(0, varname, 0, 0)) return (false);
    }

    // Does a converter exist for this standard name?
    //
    vector<string> names = DerivedCFVertCoordVarFactory::Instance()->GetFactoryNames();

    for (int i = 0; i < names.size(); i++) {
        if (standard_name == names[i]) return (true);
    }

    return (false);
}

template<typename C> string DataMgr::VarInfoCache<C>::_make_hash(string key, size_t ts, vector<string> varnames, int level, int lod)
{
    ostringstream oss;

    oss << key << ":";
    oss << ts << ":";
    for (int i = 0; i < varnames.size(); i++) { oss << varnames[i] << ":"; }
    oss << level << ":";
    oss << lod;

    return (oss.str());
}

template<typename C> void DataMgr::VarInfoCache<C>::_decode_hash(const string &hash, string &key, size_t &ts, vector<string> &varnames, int &level, int &lod)
{
    varnames.clear();

    stringstream   ss(hash);
    vector<string> result;

    // parse hash into vector of strings
    //
    while (ss.good()) {
        string substr;
        getline(ss, substr, ':');
        result.push_back(substr);
    }
    VAssert(result.size() >= 5);

    int i = 0;
    key = result[i++];
    ts = (size_t)stoi(result[i++]);
    while (!is_int(result[i])) { varnames.push_back(result[i++]); }
    level = stoi(result[i++]);
    lod = stoi(result[i++]);
}

template<typename C> void DataMgr::VarInfoCache<C>::Set(size_t ts, vector<string> varnames, int level, int lod, string key, const vector<C> &values)
{
    string hash = _make_hash(key, ts, varnames, level, lod);
    _cache[hash] = values;
}

template<typename C> bool DataMgr::VarInfoCache<C>::Get(size_t ts, vector<string> varnames, int level, int lod, string key, vector<C> &values) const
{
    values.clear();

    string                                          hash = _make_hash(key, ts, varnames, level, lod);
    typename map<string, vector<C>>::const_iterator itr = _cache.find(hash);

    if (itr == _cache.end()) return (false);

    values = itr->second;
    return (true);
}

template<typename C> void DataMgr::VarInfoCache<C>::Purge(size_t ts, vector<string> varnames, int level, int lod, string key)
{
    string                                    hash = _make_hash(key, ts, varnames, level, lod);
    typename map<string, vector<C>>::iterator itr = _cache.find(hash);

    if (itr == _cache.end()) return;

    _cache.erase(itr);
}

template<typename C> void DataMgr::VarInfoCache<C>::Purge(vector<string> varnames)
{
    vector<string>                                 hashes;
    typename map<string, std::vector<C>>::iterator itr;
    for (itr = _cache.begin(); itr != _cache.end(); ++itr) { hashes.push_back(itr->first); }

    for (int i = 0; i < hashes.size(); i++) {
        string         hash = hashes[i];
        string         key;
        size_t         ts;
        vector<string> cvarnames;
        int            level;
        int            lod;

        _decode_hash(hash, key, ts, cvarnames, level, lod);

        if (varnames == cvarnames) { Purge(ts, varnames, level, lod, key); }
    }
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
    VAssert(bmin.size() == bmax.size());
    VAssert(bmin.size() >= 1 && bmax.size() <= 3);

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
    VAssert(_mins.size() >= 1);

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
        VAssert(scvars.size());

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
    VAssert(status);

    DC::Dimension dimension;

    size_t layers_dimlen = 0;
    if (m.GetMeshType() == DC::Mesh::UNSTRUC_LAYERED) {
        string dimname = m.GetLayersDimName();
        VAssert(!dimname.empty());
        status = _dc->GetDimension(dimname, dimension);
        VAssert(status);
        layers_dimlen = dimension.GetLength();
    }

    string dimname = m.GetNodeDimName();
    status = _dc->GetDimension(dimname, dimension);
    VAssert(status);
    vertexDims.push_back(dimension.GetLength());
    if (layers_dimlen) { vertexDims.push_back(layers_dimlen); }

    dimname = m.GetFaceDimName();
    status = _dc->GetDimension(dimname, dimension);
    VAssert(status);
    faceDims.push_back(dimension.GetLength());
    if (layers_dimlen) { faceDims.push_back(layers_dimlen - 1); }

    dimname = m.GetEdgeDimName();
    if (dimname.size()) {
        status = _dc->GetDimension(dimname, dimension);
        VAssert(status);
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
    VAssert(ok);

    DC::AuxVar auxvar;
    status = _dc->GetAuxVarInfo(face_node_var, auxvar);
    VAssert(status);
    vertexOffset = auxvar.GetOffset();

    status = _dc->GetAuxVarInfo(node_face_var, auxvar);
    VAssert(status);
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
    VAssert(ok);

    DC::Mesh m;
    ok = GetMesh(dvar.GetMeshName(), m);
    VAssert(ok);

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

    // Make sure variable dimensions match extents specification
    //
    while (min.size() > scvars.size()) {
        min.pop_back();
        max.pop_back();
    }

    size_t hash_ts = 0;
    for (int i = 0; i < scvars.size(); i++) {
        if (IsTimeVarying(scvars[i])) hash_ts = ts;
    }

    vector<size_t> dims_at_level;
    int            rc = GetDimLensAtLevel(varname, level, dims_at_level);
    if (rc < 0) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (-1);
    }

    // Currently unstructured grids can not be subset. We always need
    // to read the entire data set.
    //
    if (_gridHelper.IsUnstructured(_get_grid_type(varname))) {
        for (int i = 0; i < dims_at_level.size(); i++) {
            min_ui.push_back(0);
            max_ui.push_back(dims_at_level[i] - 1);
        }
        return (0);
    }

    vector<size_t> bs;
    for (int i = 0; i < dims_at_level.size(); i++) { bs.push_back(_bs[i]); }

    // hash tag for block coordinate cache
    //
    string hash = VarInfoCache<int>::_make_hash("BlkExts", hash_ts, scvars, level, lod);

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
        map_vox_to_blk(bs, vmin, bmin);
        map_vox_to_blk(bs, vmax, bmax);

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
                my_vmin[i] = bcoord[i] * bs[i];
                if (my_vmin[i] > 0) my_vmin[i] -= 1;    // not boundary face

                my_vmax[i] = bcoord[i] * bs[i] + bs[i] - 1;
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
        VAssert(itr != _blkExtsCache.end());

    } else {
        SetDiagMsg("DataMgr::_find_bounding_grid() - coordinates in cache");
    }

    const BlkExts &blkexts = itr->second;

    // Find block coordinates of region that contains the bounding volume
    //
    vector<size_t> bmin, bmax;
    ok = blkexts.Intersect(min, max, bmin, bmax);
    if (!ok) {
        for (int i = 0; i < dims_at_level.size(); i++) {
            min_ui.push_back(0);
            max_ui.push_back(dims_at_level[i] - 1);
        }
        return (0);
    }

    // Finally, map from block to voxel coordinates
    //
    map_blk_to_vox(bs, bmin, bmax, min_ui, max_ui);
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
    VAssert(axis >= 0 && axis <= 2);

    // For a structured mesh use the coresponding dimension name
    // as the coordinate variable name. For unstructured nothing
    // we can do
    //
    if (m.GetMeshType() == DC::Mesh::STRUCTURED) {
        VAssert(m.GetDimNames().size() >= axis);
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
        VAssert(ok);

        if (_udunits.IsLonUnit(varInfo.GetUnits())) { coord_vars[i] = coord_vars[i] + "X"; }
        if (_udunits.IsLatUnit(varInfo.GetUnits())) { coord_vars[i] = coord_vars[i] + "Y"; }
    }
}

void DataMgr::_assignTimeCoord(string &coord_var) const
{
    if (coord_var.empty()) return;

    DC::CoordVar varInfo;
    bool         ok = GetCoordVarInfo(coord_var, varInfo);
    VAssert(ok);

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
        case DC::Mesh::VOLUME: VAssert(0 && "VOLUME cells not supported"); break;
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
    int         rc = 0;
    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) {
        VAssert((std::is_same<T, float>::value) == true);
        rc = derivedVar->ReadRegionBlock(fd, min, max, (float *)region);
    } else {
        rc = _dc->ReadRegionBlock(fd, min, max, region);
    }

    _sanitizeFloats(region, Wasp::VProduct(Wasp::Dims(min, max)));
    return (rc);
}

template<class T> int DataMgr::_readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    int         rc = 0;
    DerivedVar *derivedVar = _getDerivedVar(_openVarName);
    if (derivedVar) {
        VAssert((std::is_same<T, float>::value) == true);
        rc = derivedVar->ReadRegion(fd, min, max, (float *)region);
    } else {
        rc = _dc->ReadRegion(fd, min, max, region);
    }

    _sanitizeFloats(region, Wasp::VProduct(Wasp::Dims(min, max)));
    return (rc);
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
    vector<size_t> dims_at_level, dummy;
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
    vector<size_t> dims_at_level, dummy;
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
    vector<size_t> dims, dummy;
    int            rc = _dc->GetDimLensAtLevel(varname, 0, dims, dummy);
    if (rc < 0) {
        SetErrMsg("Invalid variable reference : %s", varname.c_str());
        return (-1);
    }
    if (!(dims.size() == 1 || dims.size() == 2)) {
        SetErrMsg("Unsupported variable dimension for variable \"%s\"", varname.c_str());
        return (-1);
    }

    float *buf = new float[VProduct(dims)];

    rc = _getVar(0, varname, 0, 0, buf);
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
        if (!_getDerivedCoordVar(derivedCoordvars[0])) {
            DerivedCoordVar_PCSFromLatLon *derivedVar = new DerivedCoordVar_PCSFromLatLon(derivedCoordvars[0], _dc, coordvars, _proj4String, m.GetMeshType() != DC::Mesh::STRUCTURED, true);

            rc = derivedVar->Initialize();
            if (rc < 0) {
                SetErrMsg("Failed to initialize derived coord variable");
                return (-1);
            }

            _dvm.AddCoordVar(derivedVar);
        }

        if (!_getDerivedCoordVar(derivedCoordvars[1])) {
            DerivedCoordVar_PCSFromLatLon *derivedVar = new DerivedCoordVar_PCSFromLatLon(derivedCoordvars[1], _dc, coordvars, _proj4String, m.GetMeshType() != DC::Mesh::STRUCTURED, false);

            rc = derivedVar->Initialize();
            if (rc < 0) {
                SetErrMsg("Failed to initialize derived coord variable");
                return (-1);
            }

            _dvm.AddCoordVar(derivedVar);
        }
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

        VAssert(m.GetCoordVars().size() > 2);

        DerivedCoordVar *derivedVar = NULL;

        derivedVar = DerivedCFVertCoordVarFactory::Instance()->CreateInstance(standard_name, _dc, meshnames[i], formula_terms);
        if (!derivedVar) {
            SetErrMsg("Failed to initialize derived coord variable");
            return (-1);
        }

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
    VAssert(b._bmin.size() == b._bmax.size());
    VAssert(b._mins.size() == b._maxs.size());

    o << "Block dimensions" << endl;
    for (int i = 0; i < b._bmin.size(); i++) { o << "  " << b._bmin[i] << " " << b._bmax[i] << endl; }
    o << "Block coordinates" << endl;
    for (int i = 0; i < b._mins.size(); i++) {
        VAssert(b._mins[i].size() == b._maxs[i].size());
        o << "Block index " << i << endl;
        for (int j = 0; j < b._mins[i].size(); j++) { o << "  " << b._mins[i][j] << " " << b._maxs[i][j] << endl; }
        o << endl;
    }

    return (o);
}

};    // namespace VAPoR
