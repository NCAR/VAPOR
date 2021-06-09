#include "vapor/VAssert.h"
#include <sstream>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <netcdf.h>
#include "vapor/VDCNetCDF.h"
#include "vapor/CFuncs.h"
#include "vapor/Version.h"
#include "vapor/FileUtils.h"

using namespace VAPoR;
using namespace Wasp;

namespace {

// Map a level specification for a given number of levels into
// clevel (coarsest level is 0 increasing values corespond to finer
// levels), and flevel (finest level is -1 and decreasing values
// correspond to coarser levels)
//
void levels(int level, int nlevels, int &clevel, int &flevel)
{
    if (level > nlevels - 1) level = nlevels - 1;
    if (level < -nlevels) level = -nlevels;

    if (level >= 0) {
        clevel = level;
        flevel = -nlevels + level;
    } else {
        clevel = level + nlevels;
        flevel = level;
    }
}

int vdc_xtype2ncdf_xtype(VDC::XType v_xtype)
{
    int n_xtype;
    switch (v_xtype) {
    case VDC::FLOAT: n_xtype = NC_FLOAT; break;
    case VDC::DOUBLE: n_xtype = NC_DOUBLE; break;
    case VDC::INT8: n_xtype = NC_BYTE; break;
    case VDC::INT32: n_xtype = NC_INT; break;
    case VDC::INT64: n_xtype = NC_INT64; break;
    case VDC::UINT8: n_xtype = NC_UBYTE; break;
    case VDC::TEXT: n_xtype = NC_CHAR; break;
    default: n_xtype = NC_NAT; break;
    }
    return (n_xtype);
}

#ifdef UNUSED_FUNCTION
VDC::XType ncdf_xtype2vdc_xtype(int n_xtype)
{
    VDC::XType v_xtype;
    switch (n_xtype) {
    case NC_FLOAT: v_xtype = VDC::FLOAT; break;
    case NC_DOUBLE: v_xtype = VDC::DOUBLE; break;
    case NC_BYTE: v_xtype = VDC::INT8; break;
    case NC_INT: v_xtype = VDC::INT32; break;
    case NC_INT64: v_xtype = VDC::INT64; break;
    case NC_UBYTE: v_xtype = VDC::UINT8; break;
    case NC_CHAR:
    case NC_STRING: v_xtype = VDC::TEXT; break;
    default: v_xtype = VDC::INVALID; break;
    }
    return (v_xtype);
}
#endif

void vdc_2_ncdfcoords(size_t ts0, size_t ts1, bool time_varying, const vector<size_t> &min, const vector<size_t> &max, vector<size_t> &start, vector<size_t> &count)
{
    start.clear();
    count.clear();

    VAssert(min.size() == max.size());
    ;
    VAssert(max.size() <= 3);
    VAssert(ts1 >= ts0);

    if (time_varying) {
        start.push_back(ts0);
        count.push_back(ts1 - ts0 + 1);
    }

    for (int i = min.size() - 1; i >= 0; i--) {
        VAssert(max[i] >= min[i]);
        start.push_back(min[i]);
        count.push_back(max[i] - min[i] + 1);
    }
}

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

size_t gcd(size_t n1, size_t n2)
{
    size_t tmp;
    while (n2 != 0) {
        tmp = n1;
        n1 = n2;
        n2 = tmp % n2;
    }
    return n1;
}

size_t lcm(size_t n1, size_t n2) { return ((n1 * n2) / gcd(n1, n2)); }

bool isblocked(vector<size_t> bs)
{
    for (int i = 0; i < bs.size(); i++) {
        if (bs[i] != 1) return (true);
    }
    return (false);
}

};    // namespace

VDCNetCDF::VDCNetCDF(int nthreads, size_t master_threshold, size_t variable_threshold) : VDC()
{
    _nthreads = nthreads;
    _master_threshold = master_threshold;
    _variable_threshold = variable_threshold;
    _chunksizehint = 0;
    _master = new WASP(nthreads);
    _version = 1;
}

VDCNetCDF::~VDCNetCDF()
{
    vector<int> fds = _fileTable.GetEntries();
    for (int i = 0; i < fds.size(); i++) { (void)closeVariable(i); }

    if (_master) {
        _master->Close();
        delete _master;
    }
}

int VDCNetCDF::GetHyperSliceInfo(string varname, int level, std::vector<size_t> &hslice_dims, size_t &nslice)
{
    hslice_dims.clear();
    nslice = 0;

    vector<size_t> dims_at_level;
    vector<size_t> bs_at_level;

    int rc = GetDimLensAtLevel(varname, level, dims_at_level, bs_at_level, -1);
    if (rc < 0) return (-1);

    if (dims_at_level.size() == 0) return (0);

    if (!isblocked(bs_at_level)) { return (DC::GetHyperSliceInfo(varname, level, hslice_dims, nslice)); }

    hslice_dims = dims_at_level;

    if (dims_at_level.size() == 1) {
        nslice = 1;
        return (0);
    }

    int dim = hslice_dims.size() - 1;

    // This is where we override the DC::GetHyperSliceInfo() method.
    // Need to preserve block alignment.
    //
    hslice_dims[dim] = bs_at_level[dim];
    nslice = (dims_at_level[dim] - 1) / hslice_dims[dim] + 1;

    return (0);
}

int VDCNetCDF::Initialize(const vector<string> &paths, const vector<string> &options, AccessMode mode, vector<size_t> bs, size_t chunksizehint)
{
    _chunksizehint = chunksizehint;

    int rc = VDC::initialize(paths, options, mode, bs);
    if (rc < 0) return (-1);

    if (mode == VDC::W) {
        size_t chsz = _chunksizehint;
        rc = _master->Create(_master_path, NC_64BIT_OFFSET | NC_WRITE, 0, chsz, 1);
    } else if (mode == VDC::A) {
        rc = _master->Open(_master_path, NC_WRITE);
    } else {
        rc = _master->Open(_master_path, NC_NOWRITE);
    }
    if (rc < 0) return (-1);
    return (0);
}

string VDCNetCDF::GetDataDir(string master)
{
    string path = master;
    string extension = FileUtils::Extension(path);
    if (!extension.empty()) path.erase(path.rfind("." + extension));
    path += "_data";
    return (path);
}

//
// Figure out where variable lives. This algorithm will most likely
// change.
//
int VDCNetCDF::GetPath(string varname, size_t ts, string &path, size_t &file_ts, size_t &max_ts) const
{
    path.clear();
    file_ts = 0;
    max_ts = 0;

    bool time_varying = IsTimeVarying(varname);
    if (!time_varying) {
        ts = 0;    // Could be anything if data aren't time varying;
    }

    VDC::BaseVar var;
    if (!VDC::GetBaseVarInfo(varname, var)) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (false);
    }

    vector<Dimension> dimensions;
    bool              ok = GetVarDimensions(varname, false, dimensions, -1);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (false);
    }

    // Does this variable live in the master file?
    //
    if (_var_in_master(var)) {
        path = _master_path;
        file_ts = ts;
        return (0);
    }

    path = VDCNetCDF::GetDataDir(_master_path);

    path += "/";

    if (VDC::IsDataVar(varname)) {
        path += "data";
        path += "/";
    } else {
        path += "coordinates";
        path += "/";
    }

    path += varname;
    path += "/";
    path += varname;

    if (time_varying) {
        size_t nelements = 1;
        size_t ngridpoints = 1;
        for (int i = 0; i < dimensions.size() - 1; i++) {
            nelements *= dimensions[i].GetLength();
            ngridpoints *= dimensions[i].GetLength();
        }

        int           idx;
        ostringstream oss;
        size_t        numts = dimensions[dimensions.size() - 1].GetLength();
        VAssert(numts > 0);
        max_ts = _variable_threshold / ngridpoints;
        if (max_ts > numts) max_ts = numts;
        if (max_ts == 0) {
            idx = ts;
            file_ts = 0;
            max_ts = 1;
        } else {
            idx = ts / max_ts;
            ;
            file_ts = ts % max_ts;
        }
        int width = (int)log10((double)numts - 1) + 1;
        if (width < 4) width = 4;
        oss.width(width);
        oss.fill('0');
        oss << idx;

        path += ".";
        path += oss.str();
    }

    path += ".";
    path += "nc";

    return (0);
}

int VDCNetCDF::getDimLensAtLevel(string varname, int level, vector<size_t> &dims_at_level, vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    int nlevels = VDC::GetNumRefLevels(varname);

    int clevel, dummy;
    levels(level, nlevels, clevel, dummy);

    vector<size_t> dimlens;
    bool           ok = GetVarDimLens(varname, true, dimlens, -1);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    DC::BaseVar varinfo;
    ok = GetBaseVarInfo(varname, varinfo);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    vector<size_t> bs = _bs;
    while (bs.size() > dimlens.size()) { bs.pop_back(); }
    VAssert(bs.size() == dimlens.size());

    if (varinfo.IsCompressed()) {
        string wname = varinfo.GetWName();

        reverse(bs.begin(), bs.end());
        reverse(dimlens.begin(), dimlens.end());
        WASP::InqDimsAtLevel(wname, clevel, dimlens, bs, dims_at_level, bs_at_level);
        reverse(bs_at_level.begin(), bs_at_level.end());
        reverse(dims_at_level.begin(), dims_at_level.end());
    } else {
        bs_at_level = bs;
        dims_at_level = dimlens;
    }

    return (0);
}

bool VDCNetCDF::DataDirExists(string master)
{
    string path = VDCNetCDF::GetDataDir(master);

    struct stat statbuf;

    if (stat(path.c_str(), &statbuf) < 0) return (false);

    return (true);
}

WASP *VDCNetCDF::_OpenVariableRead(size_t ts, string varname, int clevel, int lod, size_t &file_ts)
{
    file_ts = 0;

    DC::BaseVar var;
    if (!VDC::GetBaseVarInfo(varname, var)) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (NULL);
    }

    string path;
    size_t max_ts;
    int    rc = GetPath(varname, ts, path, file_ts, max_ts);
    if (rc < 0) return (NULL);

    int ncratios = var.GetCRatios().size();

    if (lod > ncratios - 1) lod = ncratios - 1;
    if (lod < 0) lod = lod + ncratios;
    if (lod < 0) lod = 0;

    WASP *wasp = NULL;

    if (path.compare(_master_path) == 0) {
        wasp = _master;
    } else {
        wasp = new WASP(_nthreads);
        rc = wasp->Open(path, NC_NOWRITE);
        if (rc < 0) return (NULL);
    }

    rc = wasp->OpenVarRead(varname, clevel, lod);
    if (rc < 0) return (NULL);

    return (wasp);
}

string VDCNetCDF::_get_mask_varname(string varname, double &mv) const
{
    VDC::DataVar dvar;
    mv = 0.0;

    string mask_varname;

    if (VDC::getDataVarInfo(varname, dvar)) {
        mask_varname = dvar.GetMaskvar();
        if (!mask_varname.empty()) { mv = dvar.GetMissingValue(); }
    }
    return (mask_varname);
}

int VDCNetCDF::openVariableRead(size_t ts, string varname, int level, int lod)
{
    int nlevels = VDC::GetNumRefLevels(varname);

    int clevel, flevel;
    levels(level, nlevels, clevel, flevel);

    size_t file_ts;
    WASP * wasp = _OpenVariableRead(ts, varname, clevel, lod, file_ts);
    if (!wasp) return (-1);

    double mv;
    string maskvar = _get_mask_varname(varname, mv);

    //
    // If there is a mask variable we need to open it.
    //

    WASP * wasp_mask = NULL;
    int    clevel_mask = -1;
    size_t file_ts_mask = 0;

    if (!maskvar.empty()) {
        //
        // the level specification can be tricky because the data variable
        // and the mask variable can have different numbers of levels (if
        // different wavelets are used). Convert the flevel - which indexes
        // from finest to coarsest and hence will be the same for both
        // variables - to a clevel, which is required by WASP API
        //
        nlevels = VDC::GetNumRefLevels(maskvar);

        levels(flevel, nlevels, clevel_mask, flevel);

        wasp_mask = _OpenVariableRead(ts, maskvar, clevel_mask, lod, file_ts_mask);
        if (!wasp_mask) return (-1);
    }

    VDCFileObject *o = new VDCFileObject(ts, varname, clevel, lod, file_ts, wasp, wasp_mask, maskvar, clevel_mask, file_ts_mask, mv);

    return (_fileTable.AddEntry(o));
}

int VDCNetCDF::OpenVariableWrite(size_t ts, string varname, int lod)
{
    VDC::BaseVar *varptr;

    VDC::DataVar  dvar;
    VDC::CoordVar cvar;
    bool          isdvar;
    if (VDC::getDataVarInfo(varname, dvar)) {
        varptr = &dvar;
        isdvar = true;
    } else if (VDC::getCoordVarInfo(varname, cvar)) {
        varptr = &cvar;
        isdvar = false;
    } else {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (false);
    }

    string path;
    size_t file_ts;
    size_t max_ts;
    int    rc = GetPath(varname, ts, path, file_ts, max_ts);
    if (rc < 0) return (-1);

    WASP *wasp = NULL;

    if (path.compare(_master_path) == 0) {
        wasp = _master;
    } else if (_master->ValidFile(path)) {
        wasp = new WASP(_nthreads);
        rc = wasp->Open(path, NC_WRITE);
    } else {
        wasp = new WASP(_nthreads);
        string dir;
        dir = FileUtils::Dirname(path);
        rc = MkDirHier(dir);
        if (rc < 0) return (-1);

        size_t chsz = _chunksizehint;
        rc = wasp->Create(path, NC_WRITE | NC_64BIT_OFFSET, 0, chsz, varptr->GetCRatios().size());
        if (rc < 0) return (-1);

        // Make a copy of attributes contained in master file
        //
        rc = _WriteAttributes(wasp, "", _atts);
        if (rc < 0) return (-1);

        if (isdvar) {
            rc = _DefDataVar(wasp, dvar, max_ts);
        } else {
            rc = _DefCoordVar(wasp, cvar, max_ts);
        }
        if (rc < 0) return (-1);

        rc = wasp->EndDef();
        if (rc < 0) return (-1);
    }
    rc = wasp->OpenVarWrite(varname, lod);
    if (rc < 0) return (-1);

    int nlevels = VDC::GetNumRefLevels(varname);

    //
    // If there is a mask variable we need to open it for **reading**
    //

    double mv;
    string maskvar = _get_mask_varname(varname, mv);
    WASP * wasp_mask = NULL;
    size_t file_ts_mask = 0;
    if (!maskvar.empty()) {
        nlevels = VDC::GetNumRefLevels(maskvar);

        wasp_mask = _OpenVariableRead(ts, maskvar, nlevels - 1, lod, file_ts_mask);
        if (!wasp_mask) return (-1);
    }

    VDCFileObject *o = new VDCFileObject(ts, varname, nlevels - 1, lod, file_ts, wasp, wasp_mask, maskvar, nlevels - 1, file_ts_mask, mv);

    return (_fileTable.AddEntry(o));
}

int VDCNetCDF::closeVariable(int fd)
{
    VDCFileObject *o = (VDCFileObject *)_fileTable.GetEntry(fd);

    if (!o) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    WASP *wasp = o->GetWaspData();

    if (wasp) { wasp->CloseVar(); }
    if (wasp && wasp != _master) {
        wasp->Close();
        delete wasp;
    }

    WASP *wasp_mask = o->GetWaspMask();
    if (wasp_mask) { wasp_mask->CloseVar(); }
    if (wasp_mask && wasp_mask != _master) {
        wasp_mask->Close();
        delete wasp_mask;
    }

    _fileTable.RemoveEntry(fd);
    delete o;

    return (0);
}

unsigned char *VDCNetCDF::_read_mask_var(WASP *wasp, string varname, string varname_mask, vector<size_t> start, vector<size_t> count)
{
    // data variable may be time varying, while mask variable is not.
    // If so remove the time dimension from start and count
    //
    if (VDC::IsTimeVarying(varname) && !VDC::IsTimeVarying(varname_mask)) {
        start.erase(start.begin());
        count.erase(count.begin());
    }

    size_t size = vproduct(count) * sizeof(unsigned char);

    unsigned char *mask = (unsigned char *)_mask_buffer.Alloc(size);

    int rc = wasp->GetVara(start, count, mask);
    if (rc < 0) return (NULL);

    return (mask);
}

template<class T> int VDCNetCDF::_writeTemplate(int fd, const T *data)
{
    VDCFileObject *o = (VDCFileObject *)_fileTable.GetEntry(fd);

    if (!o) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    WASP * wasp = o->GetWaspData();
    string varname = o->GetVarname();
    int    level = o->GetLevel();

    vector<size_t> dims, dummy;
    int            rc = GetDimLensAtLevel(varname, level, dims, dummy, -1);
    if (rc < 0) return (rc);

    bool time_varying = VDC::IsTimeVarying(varname);

    vector<size_t> start;
    vector<size_t> count;

    vector<size_t> mins(dims.size(), 0);
    vector<size_t> maxs = dims;
    for (int i = 0; i < maxs.size(); i++) maxs[i] -= 1;

    size_t file_ts = o->GetFileTS();
    vdc_2_ncdfcoords(file_ts, file_ts, time_varying, mins, maxs, start, count);

    double mv;
    string maskvar = _get_mask_varname(varname, mv);
    if (maskvar.empty()) { return (wasp->PutVara(start, count, data)); }

    unsigned char *mask = _read_mask_var(o->GetWaspMask(), varname, maskvar, start, count);
    if (!mask) return (-1);

    return (wasp->PutVara(start, count, data, mask));
}

template<class T> int VDCNetCDF::_writeSliceTemplate(int fd, const T *slice)
{
    VDCFileObject *o = (VDCFileObject *)_fileTable.GetEntry(fd);

    if (!o) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    WASP * wasp = o->GetWaspData();
    string varname = o->GetVarname();
    int    level = o->GetLevel();

    vector<size_t> dims_at_level;
    vector<size_t> dummy;

    int rc = GetDimLensAtLevel(varname, level, dims_at_level, dummy, -1);
    if (rc < 0) return (rc);

    vector<size_t> hslice_dims;
    size_t         nslice;
    rc = GetHyperSliceInfo(varname, level, hslice_dims, nslice);
    if (rc < 0) return (rc);
    VAssert(hslice_dims.size() == dims_at_level.size());

    int slice_num = o->GetSlice();
    if (slice_num >= nslice) return (0);    // Done writing;

    vector<size_t> min;
    vector<size_t> max;
    int            dim = 0;
    for (; dim < hslice_dims.size() - 1; dim++) {
        min.push_back(0);
        max.push_back(hslice_dims[dim] - 1);
    };
    min.push_back(slice_num * hslice_dims[dim]);
    max.push_back(min[dim] + hslice_dims[dim] - 1);

    // Last slice is a partial read if not block-aligned
    //
    if (max[dim] >= dims_at_level[dim]) { max[dim] = dims_at_level[dim] - 1; }

    //
    // Map from VDC to NetCDF coordinates
    //
    vector<size_t> start;
    vector<size_t> count;
    size_t         file_ts = o->GetFileTS();
    vdc_2_ncdfcoords(file_ts, file_ts, IsTimeVarying(varname), min, max, start, count);

    double mv;
    string maskvar = _get_mask_varname(varname, mv);
    if (maskvar.empty()) {
        rc = wasp->PutVara(start, count, slice);
    } else {
        unsigned char *mask = _read_mask_var(o->GetWaspMask(), varname, maskvar, start, count);
        if (!mask) return (-1);

        rc = wasp->PutVara(start, count, slice, mask);
    }
    if (rc < 0) return (rc);

    slice_num++;
    o->SetSlice(slice_num);

    return (0);
}

template int VDCNetCDF::_writeSliceTemplate<float>(int fd, const float *slice);

template<class T> int VDCNetCDF::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    VDCFileObject *o = (VDCFileObject *)_fileTable.GetEntry(fd);
    if (!o) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    WASP * wasp = o->GetWaspData();
    string varname = o->GetVarname();
    size_t file_ts = o->GetFileTS();
    WASP * wasp_mask = o->GetWaspMask();

    bool time_varying = VDC::IsTimeVarying(varname);

    vector<size_t> start;
    vector<size_t> count;
    vdc_2_ncdfcoords(file_ts, file_ts, time_varying, min, max, start, count);

    int rc = wasp->GetVara(start, count, region);
    if (rc < 0) return (rc);

    // if no mask we're done
    //
    if (!wasp_mask) return (0);

    size_t file_ts_mask = o->GetFileTSMask();
    double mv = o->GetMissingValue();

    // If there is a mask associated with this variable we need to
    // restore the missing value
    //
    string mask_varname = o->GetVarnameMask();
    time_varying = VDC::IsTimeVarying(mask_varname);
    vdc_2_ncdfcoords(file_ts_mask, file_ts_mask, time_varying, min, max, start, count);

    size_t         size = vproduct(count);
    unsigned char *mask = (unsigned char *)_mask_buffer.Alloc(size);
    rc = wasp_mask->GetVara(start, count, mask);
    if (rc < 0) return (rc);

    for (size_t i = 0; i < size; i++) {
        if (!mask[i]) { region[i] = mv; }
    }
    return (0);
}

int VDCNetCDF::readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionTemplate(fd, min, max, region)); }

int VDCNetCDF::readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionTemplate(fd, min, max, region)); }

template<class T> int VDCNetCDF::_readRegionBlockTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    VDCFileObject *o = (VDCFileObject *)_fileTable.GetEntry(fd);
    if (!o) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    WASP * wasp = o->GetWaspData();
    string varname = o->GetVarname();
    size_t file_ts = o->GetFileTS();
    WASP * wasp_mask = o->GetWaspMask();

    bool time_varying = VDC::IsTimeVarying(varname);

    vector<size_t> start;
    vector<size_t> count;
    vdc_2_ncdfcoords(file_ts, file_ts, time_varying, min, max, start, count);

    int rc = wasp->GetVaraBlock(start, count, region);
    if (rc < 0) return (rc);

    // if no mask we're done
    //
    if (!wasp_mask) return (0);

    size_t file_ts_mask = o->GetFileTSMask();
    double mv = o->GetMissingValue();

    // If there is a mask associated with this variable we need to
    // restore the missing value
    //
    string mask_varname = o->GetVarnameMask();
    time_varying = VDC::IsTimeVarying(mask_varname);
    vdc_2_ncdfcoords(file_ts_mask, file_ts_mask, time_varying, min, max, start, count);

    size_t         size = vproduct(count);
    unsigned char *mask = (unsigned char *)_mask_buffer.Alloc(size);
    rc = wasp_mask->GetVaraBlock(start, count, mask);
    if (rc < 0) return (rc);

    for (size_t i = 0; i < size; i++) {
        if (!mask[i]) { region[i] = mv; }
    }
    return (0);
}

int VDCNetCDF::readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (_readRegionBlockTemplate(fd, min, max, region)); }

int VDCNetCDF::readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (_readRegionBlockTemplate(fd, min, max, region)); }

template<class T> int VDCNetCDF::_putVarTemplate(string varname, int lod, const T *data)
{
    vector<size_t> dims_at_level;
    vector<size_t> dummy;
    int            rc = VDCNetCDF::GetDimLensAtLevel(varname, -1, dims_at_level, dummy, -1);
    if (rc < 0) return (-1);

    // If not a 1D time-varying variable.
    //
    if (!(VDC::IsTimeVarying(varname) && dims_at_level.size() == 1)) {
        // Number of per time step
        //
        size_t var_size = 1;
        for (int i = 0; i < dims_at_level.size(); i++) var_size *= dims_at_level[i];

        int numts = VDC::GetNumTimeSteps(varname);

        const T *ptr = data;
        for (size_t ts = 0; ts < numts; ts++) {
            rc = VDCNetCDF::PutVar(ts, varname, lod, ptr);
            if (rc < 0) return (-1);

            ptr += var_size;
        }

        return (0);
    }

    // Write 1D time-varying variables directly with
    // NetCDFCpp class.
    //

    VDC::BaseVar var;
    if (!VDC::GetBaseVarInfo(varname, var)) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (false);
    }

    // Don't currently handle case where a variable is split across
    // multiple files.
    //
    if (!_var_in_master(var)) {
        SetErrMsg("Distributed variable reads not supported");
        return (-1);
    }

    // N.B. calling NetCDFCpp::PutVar
    //
    rc = ((NetCDFCpp *)_master)->PutVar(varname, data);
    if (rc < 0) return (-1);

    return (0);
}

template int VDCNetCDF::_putVarTemplate<float>(string varname, int lod, const float *data);
template int VDCNetCDF::_putVarTemplate<int>(string varname, int lod, const int *data);

template<class T> int VDCNetCDF::_putVarTemplate(size_t ts, string varname, int lod, const T *data)
{
    int fd = VDCNetCDF::OpenVariableWrite(ts, varname, lod);
    if (fd < 0) return (-1);

    int rc = VDCNetCDF::Write(fd, data);
    if (rc < 0) return (-1);

    rc = closeVariable(fd);
    if (rc < 0) return (-1);

    return (0);
}

template int VDCNetCDF::_putVarTemplate<float>(size_t ts, string varname, int lod, const float *data);
template int VDCNetCDF::_putVarTemplate<int>(size_t ts, string varname, int lod, const int *data);

int VDCNetCDF::_copyVar0d(DC &dc, size_t ts, const BaseVar &varInfo)
{
    if (varInfo.GetXType() == FLOAT || varInfo.GetXType() == DOUBLE) {
        float buf;

        int rc = dc.GetVar(ts, varInfo.GetName(), -1, -1, &buf);
        if (rc < 0) return (rc);

        rc = PutVar(ts, varInfo.GetName(), -1, &buf);
        if (rc < 0) return (rc);
    } else {
        int buf;

        int rc = dc.GetVar(ts, varInfo.GetName(), -1, -1, &buf);
        if (rc < 0) return (rc);

        rc = PutVar(ts, varInfo.GetName(), -1, &buf);
        if (rc < 0) return (rc);
    }
    return (0);
}

template<class T>
int VDCNetCDF::_copyVarHelper(DC &dc, int fdr, int fdw, vector<size_t> &buffer_dims, vector<size_t> &src_hslice_dims, vector<size_t> &dst_hslice_dims, size_t src_nslice, size_t dst_nslice, T *buffer)
{
    VAssert(buffer_dims.size() == src_hslice_dims.size());
    VAssert(buffer_dims.size() == dst_hslice_dims.size());

    size_t dim = buffer_dims.size() - 1;

    size_t src_slice_count = 0;
    size_t dst_slice_count = 0;
    while (src_slice_count < src_nslice) {
        T * bufptr = buffer;
        int n = buffer_dims[dim] / src_hslice_dims[dim];

        for (int i = 0; i < n && src_slice_count < src_nslice; i++) {
            int rc = dc.ReadSlice(fdr, bufptr);
            if (rc < 0) return (-1);
            bufptr += vproduct(src_hslice_dims);

            src_slice_count++;
        }

        bufptr = buffer;
        n = buffer_dims[dim] / dst_hslice_dims[dim];

        for (int i = 0; i < n && dst_slice_count < dst_nslice; i++) {
            int rc = WriteSlice(fdw, bufptr);
            if (rc < 0) return (-1);

            bufptr += vproduct(dst_hslice_dims);

            dst_slice_count++;
        }
    }
    return (0);
}

int VDCNetCDF::CopyVar(DC &dc, size_t ts, string varname, int srclod, int dstlod)
{
    BaseVar varInfo;
    bool    status = dc.GetBaseVarInfo(varname, varInfo);
    if (!status) {
        SetErrMsg("Invalid source variable name : %s", varname.c_str());
        return (-1);
    }

    // Get the dimensions of a hyper slice for the source and destination
    // varible
    //
    vector<size_t> src_hslice_dims;
    size_t         src_nslice;
    int            rc = dc.GetHyperSliceInfo(varname, -1, src_hslice_dims, src_nslice);
    if (rc < 0) return (rc);

    vector<size_t> dst_hslice_dims;
    size_t         dst_nslice;
    rc = GetHyperSliceInfo(varname, -1, dst_hslice_dims, dst_nslice);
    if (rc < 0) return (rc);

    if (src_hslice_dims.size() != dst_hslice_dims.size()) {
        SetErrMsg("Incompatible source and destination variable definitions");
        return (-1);
    }

    if (src_hslice_dims.size() == 0) { return (_copyVar0d(dc, ts, varInfo)); }

    // n-1 fastest varying dimensions must be the same for both hyper-slices.
    // Slowest dimension may be different.
    //
    int    dim = src_hslice_dims.size() - 1;
    size_t src_dimlen = src_hslice_dims[dim];
    size_t dst_dimlen = dst_hslice_dims[dim];

    for (int i = 0; i < src_hslice_dims.size() - 1; i++) {
        if (src_hslice_dims[i] != dst_hslice_dims[i]) {
            SetErrMsg("Incompatible source and destination variable definitions");
            return (-1);
        }
    }

    // Find the slice dimension for slowest varying dimension, the Least
    // Common Multiple for the source and destination
    //
    size_t slice_dim = lcm(src_dimlen, dst_dimlen);

    // Common (fastest-varying) dimensions for both variables, plus
    // the lcm of the slowest varying dimension for the source
    // and destination.
    //
    vector<size_t> buffer_dims = src_hslice_dims;
    buffer_dims.pop_back();    // Remove slowest varying dimension
    buffer_dims.push_back(slice_dim);

    int fdr = dc.OpenVariableRead(ts, varname, srclod);
    if (fdr < 0) return (fdr);

    int fdw = OpenVariableWrite(ts, varname, dstlod);
    if (fdw < 0) return (fdw);

    if (varInfo.GetXType() == FLOAT || varInfo.GetXType() == DOUBLE) {
        size_t bufsize = vproduct(buffer_dims);
        float *buffer = new float[bufsize];

        rc = _copyVarHelper(dc, fdr, fdw, buffer_dims, src_hslice_dims, dst_hslice_dims, src_nslice, dst_nslice, buffer);
        delete[] buffer;
    } else {
        size_t bufsize = vproduct(buffer_dims);
        int *  buffer = new int[bufsize];

        rc = _copyVarHelper(dc, fdr, fdw, buffer_dims, src_hslice_dims, dst_hslice_dims, src_nslice, dst_nslice, buffer);
        delete[] buffer;
    }

    dc.CloseVariable(fdr);
    closeVariable(fdw);

    return (rc);
}

int VDCNetCDF::CopyVar(DC &dc, string varname, int srclod, int dstlod)
{
    size_t numTS = dc.GetNumTimeSteps(varname);
    for (size_t ts = 0; ts < numTS; ts++) {
        int rc = CopyVar(dc, ts, varname, srclod, dstlod);
        if (rc < 0) return (rc);
    }
    return (0);
}

bool VDCNetCDF::CompressionInfo(std::vector<size_t> bs, string wname, size_t &nlevels, size_t &maxcratio) const
{
    nlevels = 1;
    maxcratio = 1;
    if (wname.empty()) return (true);

    std::reverse(bs.begin(), bs.end());    // NetCDF order
    return (WASP::InqCompressionInfo(bs, wname, nlevels, maxcratio));
}

bool VDCNetCDF::variableExists(size_t ts, string varname, int level, int lod) const
{
    VDC::BaseVar var;
    if (!VDC::GetBaseVarInfo(varname, var)) return (false);

    string path;
    size_t file_ts;
    size_t max_ts;
    int    rc = GetPath(varname, ts, path, file_ts, max_ts);
    if (rc < 0) return (-1);

    vector<string> paths;
    if (!var.IsCompressed()) {
        paths.push_back(path);
    } else {
        int numfiles = var.GetCRatios().size();
        paths = WASP::GetPaths(path, numfiles);
    }

    int ncratios = var.GetCRatios().size();

    if (lod > ncratios - 1) lod = ncratios - 1;
    if (lod < 0) lod = lod + ncratios;
    if (lod < 0) lod = 0;

    for (int i = 0; i <= lod; i++) {
        struct stat statbuf;
        if (stat(paths[i].c_str(), &statbuf) < 0) return (false);
    }
    return (true);
}

int VDCNetCDF::SetFill(int fillmode)
{
    int last;
    int ret = ((NetCDFCpp *)_master)->SetFill(fillmode, last);
    return ret;
}

int VDCNetCDF::_WriteMasterMeta()
{
    int                                    rc;
    map<string, Dimension>::const_iterator itr;
    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) {
        const Dimension &dimension = itr->second;

        rc = _master->DefDim(dimension.GetName(), dimension.GetLength());
        if (rc < 0) return (-1);
    }

    rc = _master->PutAtt("", "VDC.Version", Version::GetVersionString());
    if (rc < 0) return (rc);

    rc = _master->PutAtt("", "VDC.BlockSize", _bs);
    if (rc < 0) return (rc);

    rc = _master->PutAtt("", "VDC.WaveName", _wname);
    if (rc < 0) return (rc);

    rc = _master->PutAtt("", "VDC.CompressionRatios", _cratios);
    if (rc < 0) return (rc);

    rc = _master->PutAtt("", "VDC.MasterThreshold", _master_threshold);
    if (rc < 0) return (rc);

    rc = _master->PutAtt("", "VDC.VariableThreshold", _variable_threshold);
    if (rc < 0) return (rc);

    vector<int> periodic;
    for (int i = 0; i < _periodic.size(); i++) { periodic.push_back((int)_periodic[i]); }
    rc = _master->PutAtt("", "VDC.Periodic", periodic);
    if (rc < 0) return (rc);

    rc = _WriteMasterDimensions();
    if (rc < 0) return (rc);

    rc = _WriteMasterAttributes();
    if (rc < 0) return (rc);

    rc = _WriteMasterMeshDefs();
    if (rc < 0) return (rc);

    rc = _WriteMasterCoordVarsDefs();
    if (rc < 0) return (rc);

    rc = _WriteMasterDataVarsDefs();
    if (rc < 0) return (rc);

    rc = _master->EndDef();
    if (rc < 0) return (rc);

    return (0);
}

int VDCNetCDF::_ReadMasterMeta()
{
    int rc = _master->Open(_master_path, 0);
    if (rc < 0) return (-1);

    rc = _master->GetAtt("", "VDC.Version", _version);
    if (rc < 0) {
        SetErrMsg("VDC versions prior to 3.0.0 not supported");
        return (rc);
    }

    if (Version::Compare(_version, "3.0.0") < 0) {
        SetErrMsg("VDC versions prior to 3.0.0 not supported");
        return (-1);
    }

    rc = _master->GetAtt("", "VDC.BlockSize", _bs);
    if (rc < 0) return (rc);

    rc = _master->GetAtt("", "VDC.WaveName", _wname);
    if (rc < 0) return (rc);

    rc = _master->GetAtt("", "VDC.CompressionRatios", _cratios);
    if (rc < 0) return (rc);
    sort(_cratios.begin(), _cratios.end());
    reverse(_cratios.begin(), _cratios.end());

    rc = _master->GetAtt("", "VDC.MasterThreshold", _master_threshold);
    if (rc < 0) return (rc);

    rc = _master->GetAtt("", "VDC.VariableThreshold", _variable_threshold);
    if (rc < 0) return (rc);

    vector<int> periodic;
    rc = _master->GetAtt("", "VDC.Periodic", periodic);

    _periodic.clear();
    for (int i = 0; i < periodic.size(); i++) { _periodic.push_back((bool)periodic[i]); }
    if (rc < 0) return (rc);

    rc = _ReadMasterDimensions();
    if (rc < 0) return (rc);

    rc = _ReadMasterAttributes();
    if (rc < 0) return (rc);

    rc = _ReadMasterMeshDefs();
    if (rc < 0) return (rc);

    rc = _ReadMasterCoordVarsDefs();
    if (rc < 0) return (rc);

    rc = _ReadMasterDataVarsDefs();
    if (rc < 0) return (rc);

    return (0);
}

int VDCNetCDF::_ReadMasterDimensions()
{
    _dimsMap.clear();

    string         tag = "VDC.DimensionNames";
    vector<string> dimnames;
    int            rc = _master->GetAtt("", tag, dimnames);
    if (rc < 0) return (rc);

    for (int i = 0; i < dimnames.size(); i++) {
        tag = "VDC.Dimension." + dimnames[i] + ".Length";
        int length;
        rc = _master->GetAtt("", tag, length);
        if (rc < 0) return (rc);

        _dimsMap[dimnames[i]] = VDC::Dimension(dimnames[i], (size_t)length);
    }
    return (0);
}

int VDCNetCDF::_ReadMasterAttributes(string prefix, map<string, Attribute> &atts)
{
    atts.clear();

    string         tag = prefix + ".AttributeNames";
    vector<string> attnames;
    int            rc = _master->GetAtt("", tag, attnames);
    if (rc < 0) return (rc);

    for (int i = 0; i < attnames.size(); i++) {
        tag = prefix + ".Attribute." + attnames[i] + ".XType";
        int typetmp = -1;
        rc = _master->GetAtt("", tag, typetmp);
        VDC::XType xtype;
        if (rc < 0)
            return (rc);
        else
            xtype = (VAPoR::DC::XType)typetmp;

        tag = prefix + ".Attribute." + attnames[i] + ".Values";
        switch (xtype) {
        case FLOAT:
        case DOUBLE: {
            vector<double> values;
            rc = _master->GetAtt("", tag, values);
            if (rc < 0) return (rc);
            VDC::Attribute attr(attnames[i], xtype, values);
            atts[attnames[i]] = attr;

            break;
        }
        case UINT8:
        case INT8:
        case INT32:
        case INT64: {
            vector<int> values;
            rc = _master->GetAtt("", tag, values);
            if (rc < 0) return (rc);
            VDC::Attribute attr(attnames[i], xtype, values);
            atts[attnames[i]] = attr;
            break;
        }
        case TEXT: {
            string values;
            rc = _master->GetAtt("", tag, values);
            if (rc < 0) return (rc);
            VDC::Attribute attr(attnames[i], xtype, values);
            atts[attnames[i]] = attr;
            break;
        }
        default:
            SetErrMsg("Invalid attribute xtype : %d", xtype);
            return (-1);
            break;
        }
    }
    return (0);
}

int VDCNetCDF::_ReadMasterAttributes()
{
    string prefix = "VDC";
    return (_ReadMasterAttributes(prefix, _atts));
}

int VDCNetCDF::_ReadMasterMeshDefs()
{
    string         tag = "VDC.MeshNames";
    vector<string> mesh_names;
    int            rc = _master->GetAtt("", tag, mesh_names);
    if (rc < 0) return (rc);

    // Only support STRUCTURED meshes currently
    //
    string prefix = "VDC.Mesh";
    for (int i = 0; i < mesh_names.size(); i++) {
        tag = prefix + "." + mesh_names[i] + ".DimensionNames";
        vector<string> dim_names;
        int            rc = _master->GetAtt("", tag, dim_names);
        if (rc < 0) return (rc);

        tag = prefix + "." + mesh_names[i] + ".CoordVars";
        vector<string> coord_vars;
        rc = _master->GetAtt("", tag, coord_vars);
        if (rc < 0) return (rc);

        _meshes[mesh_names[i]] = Mesh(mesh_names[i], dim_names, coord_vars);
    }

    return (0);
}

int VDCNetCDF::_ReadMasterBaseVarDefs(string prefix, BaseVar &var)
{
    string tag;

    tag = prefix + "." + var.GetName() + ".Units";
    string units;
    int    rc = _master->GetAtt("", tag, units);
    if (rc < 0) return (rc);
    var.SetUnits(units);

    tag = prefix + "." + var.GetName() + ".XType";
    int xtype;
    rc = _master->GetAtt("", tag, xtype);
    if (rc < 0) return (rc);
    var.SetXType((VAPoR::VDC::XType)xtype);

    tag = prefix + "." + var.GetName() + ".Periodic";
    vector<int> iperiodic;
    rc = _master->GetAtt("", tag, iperiodic);
    vector<bool> periodic;
    for (int i = 0; i < iperiodic.size(); i++) periodic.push_back(iperiodic[i]);
    if (rc < 0) return (rc);
    var.SetPeriodic(periodic);

    tag = prefix + "." + var.GetName() + ".WaveName";
    string wname;
    rc = _master->GetAtt("", tag, wname);
    if (rc < 0) return (rc);
    var.SetWName(wname);

    tag = prefix + "." + var.GetName() + ".CompressionRatios";
    vector<size_t> cratios;
    rc = _master->GetAtt("", tag, cratios);
    if (rc < 0) return (rc);
    var.SetCRatios(cratios);

    prefix += "." + var.GetName();
    map<string, Attribute> atts;
    rc = _ReadMasterAttributes(prefix, atts);
    if (rc < 0) return (rc);

    var.SetAttributes(atts);
    return (0);
}

int VDCNetCDF::_ReadMasterCoordVarsDefs()
{
    _coordVars.clear();

    string         tag = "VDC.CoordVarNames";
    vector<string> varnames;
    int            rc = _master->GetAtt("", tag, varnames);
    if (rc < 0) return (rc);

    string prefix = "VDC.CoordVar";
    for (int i = 0; i < varnames.size(); i++) {
        CoordVar cvar;
        cvar.SetName(varnames[i]);

        tag = prefix + "." + cvar.GetName() + ".DimensionNames";
        vector<string> dim_names;
        int            rc = _master->GetAtt("", tag, dim_names);
        if (rc < 0) return (rc);
        cvar.SetDimNames(dim_names);

        tag = prefix + "." + cvar.GetName() + ".TimeDimName";
        string time_dim_name;
        rc = _master->GetAtt("", tag, time_dim_name);
        if (rc < 0) return (rc);
        cvar.SetTimeDimName(time_dim_name);

        tag = prefix + "." + cvar.GetName() + ".Axis";
        int axis;
        rc = _master->GetAtt("", tag, axis);
        if (rc < 0) return (rc);
        cvar.SetAxis(axis);

        tag = prefix + "." + cvar.GetName() + ".UniformHint";
        int uniform;
        rc = _master->GetAtt("", tag, uniform);
        if (rc < 0) return (rc);
        cvar.SetUniform(uniform);

        rc = _ReadMasterBaseVarDefs(prefix, cvar);
        if (rc < 0) return (rc);

        _coordVars[varnames[i]] = cvar;
    }
    return (0);
}

int VDCNetCDF::_ReadMasterDataVarsDefs()
{
    string         tag = "VDC.DataVarNames";
    vector<string> varnames;
    int            rc = _master->GetAtt("", tag, varnames);
    if (rc < 0) return (rc);

    string prefix = "VDC.DataVar";
    for (int i = 0; i < varnames.size(); i++) {
        DataVar var;
        var.SetName(varnames[i]);

        tag = prefix + "." + var.GetName() + ".Mesh";
        string mesh_name;
        rc = _master->GetAtt("", tag, mesh_name);
        if (rc < 0) return (rc);
        var.SetMeshName(mesh_name);

        tag = prefix + "." + var.GetName() + ".TimeCoordVar";
        string time_coord_var;
        int    rc = _master->GetAtt("", tag, time_coord_var);
        if (rc < 0) return (rc);
        var.SetTimeCoordVar(time_coord_var);

        tag = prefix + "." + var.GetName() + ".MaskVar";
        string maskvar;
        rc = _master->GetAtt("", tag, maskvar);
        if (rc < 0) return (rc);
        var.SetMaskvar(maskvar);

        tag = prefix + "." + var.GetName() + ".MissingValue";
        if (_master->InqAttDefined("", tag)) {
            double mv;
            rc = _master->GetAtt("", tag, mv);
            if (rc < 0) return (rc);
            var.SetHasMissing(true);
            var.SetMissingValue(mv);
        }

        rc = _ReadMasterBaseVarDefs(prefix, var);
        if (rc < 0) return (rc);

        _dataVars[varnames[i]] = var;
    }
    return (0);
}

int VDCNetCDF::_WriteMasterDimensions()
{
    map<string, Dimension>::const_iterator itr;
    string                                 s;
    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) {
        s += itr->first;
        s += " ";
    }
    string tag = "VDC.DimensionNames";
    int    rc = _master->PutAtt("", tag, s);
    if (rc < 0) return (rc);

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) {
        const Dimension &dimension = itr->second;

        tag = "VDC.Dimension." + dimension.GetName() + ".Length";
        rc = _master->PutAtt("", tag, dimension.GetLength());
        if (rc < 0) return (rc);
    }
    return (0);
}

int VDCNetCDF::_WriteMasterAttributes(string prefix, const map<string, Attribute> &atts)
{
    map<string, Attribute>::const_iterator itr;
    string                                 s;
    for (itr = atts.begin(); itr != atts.end(); ++itr) {
        s += itr->first;
        s += " ";
    }
    string tag = prefix + ".AttributeNames";
    int    rc = _master->PutAtt("", tag, s);
    if (rc < 0) return (rc);

    for (itr = atts.begin(); itr != atts.end(); ++itr) {
        const Attribute &attr = itr->second;

        tag = prefix + ".Attribute." + attr.GetName() + ".XType";
        rc = _master->PutAtt("", tag, attr.GetXType());
        if (rc < 0) return (rc);

        tag = prefix + ".Attribute." + attr.GetName() + ".Values";

        rc = _PutAtt(_master, "", tag, attr);
        if (rc < 0) return (rc);
    }
    return (0);
}

int VDCNetCDF::_WriteMasterAttributes()
{
    string prefix = "VDC";
    return (_WriteMasterAttributes(prefix, _atts));
}

int VDCNetCDF::_WriteMasterBaseVarDefs(string prefix, const BaseVar &var)
{
    string tag;

    tag = prefix + "." + var.GetName() + ".Units";
    int rc = _master->PutAtt("", tag, var.GetUnits());
    if (rc < 0) return (rc);

    tag = prefix + "." + var.GetName() + ".XType";
    rc = _master->PutAtt("", tag, (int)var.GetXType());
    if (rc < 0) return (rc);

    tag = prefix + "." + var.GetName() + ".Periodic";
    vector<bool> periodic = var.GetPeriodic();
    vector<int>  iperiodic;
    for (int i = 0; i < periodic.size(); i++) iperiodic.push_back(periodic[i]);
    rc = _master->PutAtt("", tag, iperiodic);
    if (rc < 0) return (rc);

    tag = prefix + "." + var.GetName() + ".WaveName";
    rc = _master->PutAtt("", tag, var.GetWName());
    if (rc < 0) return (rc);

    tag = prefix + "." + var.GetName() + ".CompressionRatios";
    rc = _master->PutAtt("", tag, var.GetCRatios());
    if (rc < 0) return (rc);

    prefix += "." + var.GetName();
    return (_WriteMasterAttributes(prefix, var.GetAttributes()));
}

int VDCNetCDF::_WriteMasterMeshDefs()
{
    map<string, Mesh>::const_iterator itr;
    string                            s;
    for (itr = _meshes.begin(); itr != _meshes.end(); ++itr) {
        s += itr->first;
        s += " ";
    }

    string tag = "VDC.MeshNames";
    int    rc = _master->PutAtt("", tag, s);
    if (rc < 0) return (rc);

    string prefix = "VDC.Mesh";

    for (itr = _meshes.begin(); itr != _meshes.end(); ++itr) {
        const Mesh &m = itr->second;

        tag = prefix + "." + m.GetName() + ".DimensionNames";
        vector<string> dim_names = m.GetDimNames();
        string         s;
        for (int i = 0; i < dim_names.size(); i++) {
            s += dim_names[i];
            s += " ";
        }

        int rc = _master->PutAtt("", tag, s);
        if (rc < 0) return (rc);

        tag = prefix + "." + m.GetName() + ".CoordVars";
        vector<string> coord_vars = m.GetCoordVars();
        s.clear();
        for (int i = 0; i < coord_vars.size(); i++) {
            s += coord_vars[i];
            s += " ";
        }

        rc = _master->PutAtt("", tag, s);
        if (rc < 0) return (rc);
    }

    return (0);
}

int VDCNetCDF::_WriteMasterCoordVarsDefs()
{
    map<string, CoordVar>::const_iterator itr;
    string                                s;
    for (itr = _coordVars.begin(); itr != _coordVars.end(); ++itr) {
        s += itr->first;
        s += " ";
    }

    string tag = "VDC.CoordVarNames";
    int    rc = _master->PutAtt("", tag, s);
    if (rc < 0) return (rc);

    string prefix = "VDC.CoordVar";
    for (itr = _coordVars.begin(); itr != _coordVars.end(); ++itr) {
        const CoordVar &cvar = itr->second;

        int rc;
        if (_var_in_master(cvar)) {
            size_t numts = VDC::GetNumTimeSteps(cvar.GetName());
            rc = _DefCoordVar(_master, cvar, numts);
            if (rc < 0) return (-1);
        }

        tag = prefix + "." + cvar.GetName() + ".DimensionNames";
        vector<string> dim_names = cvar.GetDimNames();
        string         s;
        for (int i = 0; i < dim_names.size(); i++) {
            s += dim_names[i];
            s += " ";
        }

        rc = _master->PutAtt("", tag, s);
        if (rc < 0) return (rc);

        tag = prefix + "." + cvar.GetName() + ".TimeDimName";
        rc = _master->PutAtt("", tag, cvar.GetTimeDimName());
        if (rc < 0) return (rc);

        tag = prefix + "." + cvar.GetName() + ".Axis";
        rc = _master->PutAtt("", tag, cvar.GetAxis());
        if (rc < 0) return (rc);

        tag = prefix + "." + cvar.GetName() + ".UniformHint";
        rc = _master->PutAtt("", tag, (int)cvar.GetUniform());
        if (rc < 0) return (rc);

        rc = _WriteMasterBaseVarDefs(prefix, cvar);
        if (rc < 0) return (rc);
    }
    return (0);
}

int VDCNetCDF::_WriteMasterDataVarsDefs()
{
    map<string, DataVar>::const_iterator itr;
    string                               s;
    for (itr = _dataVars.begin(); itr != _dataVars.end(); ++itr) {
        s += itr->first;
        s += " ";
    }

    string tag = "VDC.DataVarNames";
    int    rc = _master->PutAtt("", tag, s);
    if (rc < 0) return (rc);

    string prefix = "VDC.DataVar";
    for (itr = _dataVars.begin(); itr != _dataVars.end(); ++itr) {
        const DataVar &var = itr->second;

        if (_var_in_master(var)) {
            size_t numts = VDC::GetNumTimeSteps(var.GetName());
            rc = _DefDataVar(_master, var, numts);
            if (rc < 0) return (-1);
        }

        tag = prefix + "." + var.GetName() + ".Mesh";
        int rc = _master->PutAtt("", tag, var.GetMeshName());
        if (rc < 0) return (rc);

        tag = prefix + "." + var.GetName() + ".TimeCoordVar";
        rc = _master->PutAtt("", tag, var.GetTimeCoordVar());
        if (rc < 0) return (rc);

        tag = prefix + "." + var.GetName() + ".MaskVar";
        rc = _master->PutAtt("", tag, var.GetMaskvar());
        if (rc < 0) return (rc);

        if (var.GetHasMissing()) {
            tag = prefix + "." + var.GetName() + ".MissingValue";
            rc = _master->PutAtt("", tag, var.GetMissingValue());
            if (rc < 0) return (rc);
        }

        rc = _WriteMasterBaseVarDefs(prefix, var);
        if (rc < 0) return (rc);
    }
    return (0);
}

int VDCNetCDF::_DefBaseVar(WASP *wasp, const VDC::BaseVar &var, size_t max_ts)
{
    vector<VDC::Dimension> dims;
    bool                   status = GetVarDimensions(var.GetName(), false, dims, -1);
    VAssert(status);

    bool time_varying = IsTimeVarying(var.GetName());

    vector<string> dimnames;
    for (int i = 0; i < dims.size(); i++) {
        size_t len = dims[i].GetLength();

        // If data are split across files need to set the time dimension
        // to the number of time steps in the file
        //
        if (i == dims.size() - 1 && time_varying) { len = max_ts; }

        // Don't define same dimension twice
        //
        if (!wasp->InqDimDefined(dims[i].GetName())) {
            int rc = wasp->DefDim(dims[i].GetName(), len);
            if (rc < 0) return (-1);
        }

        dimnames.push_back(dims[i].GetName());
    }
    reverse(dimnames.begin(), dimnames.end());    // NetCDF order

    size_t nspatial = time_varying ? dimnames.size() - 1 : dimnames.size();

    // Only spatial dimensions are blocked
    //
    vector<size_t> bs = _bs;
    while (bs.size() > nspatial) { bs.pop_back(); }
    reverse(bs.begin(), bs.end());    // NetCDF order
    int rc = wasp->DefVar(var.GetName(), vdc_xtype2ncdf_xtype(var.GetXType()), dimnames, var.GetWName(), bs, var.GetCRatios());
    if (rc < 0) return (-1);

    //
    // Attributes
    //

    rc = wasp->PutAtt(var.GetName(), "Units", var.GetUnits());
    if (rc < 0) return (rc);

    rc = wasp->PutAtt(var.GetName(), "BlockSize", bs);
    if (rc < 0) return (rc);

    vector<bool> periodic = var.GetPeriodic();
    vector<int>  iperiodic;
    for (int i = 0; i < periodic.size(); i++) iperiodic.push_back(periodic[i]);
    rc = wasp->PutAtt(var.GetName(), "Periodic", iperiodic);
    if (rc < 0) return (rc);

    const map<string, Attribute> &         atts = var.GetAttributes();
    map<string, Attribute>::const_iterator itr;
    for (itr = atts.begin(); itr != atts.end(); ++itr) {
        const Attribute &attr = itr->second;

        rc = _PutAtt(wasp, var.GetName(), "", attr);
        if (rc < 0) return (-1);
    }

    return (0);
}

int VDCNetCDF::_WriteAttributes(WASP *wasp, string varname, const map<string, Attribute> &atts)
{
    map<string, Attribute>::const_iterator itr;
    for (itr = _atts.begin(); itr != _atts.end(); ++itr) {
        const Attribute &attr = itr->second;

        int rc = _PutAtt(wasp, varname, attr.GetName(), attr);
        if (rc < 0) return (rc);
    }
    return (0);
}

int VDCNetCDF::_DefDataVar(WASP *wasp, const VDC::DataVar &var, size_t max_ts)
{
    int rc = _DefBaseVar(wasp, var, max_ts);
    if (rc < 0) return (-1);

    vector<string> coord_vars;
    bool           status = GetVarCoordVars(var.GetName(), false, coord_vars);
    VAssert(status);

    rc = wasp->PutAtt(var.GetName(), "CoordVars", coord_vars);
    if (rc < 0) return (-1);

    rc = wasp->PutAtt(var.GetName(), "MaskVar", var.GetMaskvar());

    if (var.GetHasMissing()) {
        rc = wasp->PutAtt(var.GetName(), "MissingValue", var.GetMissingValue());
        if (rc < 0) return (rc);
    }

    return (rc);
}

int VDCNetCDF::_DefCoordVar(WASP *wasp, const VDC::CoordVar &var, size_t max_ts)
{
    int rc = _DefBaseVar(wasp, var, max_ts);
    if (rc < 0) return (-1);

    rc = wasp->PutAtt(var.GetName(), "Axis", var.GetAxis());
    if (rc < 0) return (-1);

    rc = wasp->PutAtt(var.GetName(), "UniformHint", var.GetUniform());

    return (rc);
}

int VDCNetCDF::_PutAtt(WASP *wasp, string varname, string tag, const Attribute &attr)
{
    if (tag.empty()) tag = attr.GetName();

    DC::XType xtype = attr.GetXType();

    // Ugh. For the special attributes missing_value and _FillValue
    // the type must match that of the data. Since currently the only
    // output format we support is float we must force the type of
    // these attributes to float.
    //
    if (tag == "missing_value" || tag == "_FillValue") { xtype = FLOAT; }

    int rc;
    switch (xtype) {
    case FLOAT:
    case DOUBLE: {
        vector<double> values;
        attr.GetValues(values);
        rc = wasp->PutAtt(varname, tag, values);
        if (rc < 0) return (rc);
        break;
    }
    case UINT8:
    case INT8:
    case INT32:
    case INT64: {
        vector<int> values;
        attr.GetValues(values);
        rc = wasp->PutAtt(varname, tag, values);
        if (rc < 0) return (rc);
        break;
    }
    case TEXT: {
        string values;
        attr.GetValues(values);
        rc = wasp->PutAtt(varname, tag, values);
        if (rc < 0) return (rc);
        break;
    }
    default:
        SetErrMsg("Invalid value, Attribute::GetXType() : %d", attr.GetXType());
        return (-1);
        break;
    }

    return (0);
}

bool VDCNetCDF::_var_in_master(const VDC::BaseVar &var) const
{
    vector<DC::Dimension> dims;
    bool                  ok = GetVarDimensions(var.GetName(), false, dims, -1);
    VAssert(ok);

    bool time_varying = IsTimeVarying(var.GetName());

    if (time_varying && dims.size() > 3) return (false);

    size_t nelements = 1;
    size_t ngridpoints = 1;
    for (int i = 0; i < dims.size(); i++) {
        nelements *= dims[i].GetLength();

        if (!(i == dims.size() - 1 && time_varying)) { ngridpoints *= dims[i].GetLength(); }
    }
    if (nelements < _master_threshold && !var.IsCompressed()) { return (true); }

    return (false);
}
