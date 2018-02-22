#include <cassert>
#include <sstream>
#include <algorithm>
#include <vapor/UDUnitsClass.h>
#include <vapor/NetCDFCollection.h>
#include <vapor/utils.h>
#include <vapor/DerivedVar.h>

using namespace VAPoR;
using namespace Wasp;

namespace {

size_t numBlocks(const vector<size_t> &min, const vector<size_t> &max, const vector<size_t> &bs)
{
    assert(min.size() == max.size());
    assert(min.size() == bs.size());

    size_t nblocks = 1;
    for (int i = 0; i < bs.size(); i++) {
        int b0 = min[i] / bs[i];
        int b1 = max[i] / bs[i];
        nblocks *= (b1 - b0 + 1);
    }
    return (nblocks);
}

size_t blockSize(const vector<size_t> &bs)
{
    size_t sz = 1;
    for (int i = 0; i < bs.size(); i++) { sz *= bs[i]; }
    return (sz);
}

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

// make 2D lat and lon arrays from 1D arrays by replication, in place
//
void make2D(float *lonBuf, float *latBuf, vector<size_t> dims)
{
    assert(dims.size() == 2);
    size_t nx = dims[0];
    size_t ny = dims[1];

    // longitude
    //
    for (int j = 1; j < ny; j++) {
        for (int i = 0; i < nx; i++) { lonBuf[j * nx + i] = lonBuf[i]; }
    }

    // latitude requires a transpose first
    //
    for (int i = 0; i < ny; i++) { latBuf[i * nx] = latBuf[i]; }

    for (int j = 0; j < ny; j++) {
        for (int i = 1; i < nx; i++) { latBuf[j * nx + i] = latBuf[j * nx]; }
    }
}

};    // namespace

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_PCSFromLatLon
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_PCSFromLatLon::DerivedCoordVar_PCSFromLatLon(const vector<string> &derivedVarNames, DC *dc, vector<string> inNames, string proj4String, bool uGridFlag)
: DerivedCoordVar(derivedVarNames)
{
    assert(inNames.size() == 2);
    assert(derivedVarNames.size() == 2);

    _dc = dc;
    _proj4String = proj4String;
    _lonName = inNames[0];
    _latName = inNames[1];
    _xCoordName = _derivedVarNames[0];
    _yCoordName = _derivedVarNames[1];
    _make2DFlag = false;
    _uGridFlag = uGridFlag;
    _dimLens.clear();
}

int DerivedCoordVar_PCSFromLatLon::Initialize()
{
    int rc = _proj4API.Initialize("", _proj4String);
    if (rc < 0) {
        SetErrMsg("Invalid map projection : %s", _proj4String.c_str());
        return (-1);
    }

    rc = _setupVar();
    if (rc < 0) return (-1);

    return (0);
}

bool DerivedCoordVar_PCSFromLatLon::GetBaseVarInfo(string varname, DC::BaseVar &var) const
{
    if (varname == _xCoordName) {
        var = _xCoordVarInfo;
        return (true);
    } else if (varname == _yCoordName) {
        var = _yCoordVarInfo;
        return (true);
    }
    return (false);
}

bool DerivedCoordVar_PCSFromLatLon::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    if (varname == _xCoordName) {
        cvar = _xCoordVarInfo;
        return (true);
    } else if (varname == _yCoordName) {
        cvar = _yCoordVarInfo;
        return (true);
    }
    return (false);
}

int DerivedCoordVar_PCSFromLatLon::GetDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    if (!((varname == _xCoordName) || (varname == _yCoordName))) {
        SetErrMsg("Invalid variable name: %s", varname.c_str());
        return (-1);
    }

    dims_at_level = _dimLens;
    bs_at_level = _bs;

    return (0);
}

int DerivedCoordVar_PCSFromLatLon::OpenVariableRead(size_t ts, string varname, int, int)
{
    if (!((varname == _xCoordName) || (varname == _yCoordName))) {
        SetErrMsg("Invalid variable name: %s", varname.c_str());
        return (-1);
    }

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, varname, -1, -1);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_PCSFromLatLon::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;
    return (0);
}

int DerivedCoordVar_PCSFromLatLon::ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    size_t ts = f->GetTS();
    string varname = f->GetVarname();
    int    level = f->GetLevel();
    int    lod = f->GetLOD();
    int    sliceNum = f->GetSlice();

    // Dimensions are same for X & Y coord vars
    //
    vector<size_t> dims, bs;
    GetDimLensAtLevel(_xCoordName, level, dims, bs);

    // Need temporary buffer space for the X or Y coordinate
    // NOT being returned (we still need to calculate it)
    //
    size_t nElements = numBlocks(min, max, bs) * blockSize(bs);
    float *buf = new float[nElements];

    // Assign temporary buffer 'buf' as appropriate
    //
    float *lonBufPtr;
    float *latBufPtr;
    if (varname == _xCoordName) {
        lonBufPtr = region;
        latBufPtr = buf;
    } else {
        lonBufPtr = buf;
        latBufPtr = region;
    }

    int rc = _getVarBlock(ts, _lonName, level, lod, min, max, lonBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    rc = _getVarBlock(ts, _latName, level, lod, min, max, latBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    if (_make2DFlag) { make2D(lonBufPtr, latBufPtr, dims); }

    rc = _proj4API.Transform(lonBufPtr, latBufPtr, nElements);

    delete[] buf;

    return (rc);
}

bool DerivedCoordVar_PCSFromLatLon::VariableExists(size_t ts, string varname, int, int) const
{
    if (!(varname == _xCoordName || varname == _yCoordName)) return (false);

    return (_dc->VariableExists(ts, _lonName, -1, -1) && _dc->VariableExists(ts, _latName, -1, -1));
}

int DerivedCoordVar_PCSFromLatLon::_setupVar()
{
    DC::CoordVar lonVar;
    bool         ok = _dc->GetCoordVarInfo(_lonName, lonVar);
    if (!ok) return (-1);

    DC::CoordVar latVar;
    ok = _dc->GetCoordVarInfo(_latName, latVar);
    if (!ok) return (-1);

    vector<size_t> lonDims;
    ok = _dc->GetVarDimLens(_lonName, true, lonDims);
    if (!ok) {
        SetErrMsg("GetVarDimLens(%s) failed", _lonName.c_str());
        return (-1);
    }

    vector<size_t> latDims;
    ok = _dc->GetVarDimLens(_latName, true, latDims);
    if (!ok) {
        SetErrMsg("GetVarDimLens(%s) failed", _lonName.c_str());
        return (-1);
    }

    if (lonDims.size() != latDims.size()) {
        SetErrMsg("Incompatible block size");
        return (-1);
    }

    _bs = _dc->GetBlockSize();
    while (_bs.size() > 2) _bs.pop_back();

    vector<string> dimNames;
    if (lonVar.GetDimNames().size() == 1 && !_uGridFlag) {
        dimNames.push_back(lonVar.GetDimNames()[0]);
        dimNames.push_back(latVar.GetDimNames()[0]);
        _dimLens.push_back(lonDims[0]);
        _dimLens.push_back(latDims[0]);
        _make2DFlag = true;
    } else if (lonVar.GetDimNames().size() == 2 && !_uGridFlag) {
        if (lonDims[0] != latDims[0] && lonDims[1] != latDims[1]) {
            SetErrMsg("Incompatible dimensions ");
            return (-1);
        }
        dimNames.push_back(lonVar.GetDimNames()[0]);
        dimNames.push_back(lonVar.GetDimNames()[1]);
        _dimLens.push_back(lonDims[0]);
        _dimLens.push_back(lonDims[1]);
        _make2DFlag = false;
    } else {
        assert(lonVar.GetDimNames().size() == 1 && _uGridFlag);
        dimNames = lonVar.GetDimNames();
        _dimLens = lonDims;
    }

    if (lonVar.GetTimeDimName() != latVar.GetTimeDimName()) {
        SetErrMsg("Incompatible time dimensions");
        return (-1);
    }
    string timeDimName = lonVar.GetTimeDimName();

    DC::XType    xtype = lonVar.GetXType();
    vector<bool> periodic = lonVar.GetPeriodic();

    // X coordinate variable
    //
    _xCoordVarInfo.SetName(_xCoordName);
    _xCoordVarInfo.SetUnits("meters");
    _xCoordVarInfo.SetXType(xtype);
    _xCoordVarInfo.SetWName("");
    _xCoordVarInfo.SetCRatios(vector<size_t>());
    _xCoordVarInfo.SetPeriodic(periodic);
    _xCoordVarInfo.SetUniform(false);

    _xCoordVarInfo.SetDimNames(dimNames);
    _xCoordVarInfo.SetTimeDimName(timeDimName);
    _xCoordVarInfo.SetAxis(0);

    // Y coordinate variable
    //
    _yCoordVarInfo.SetName(_yCoordName);
    _yCoordVarInfo.SetUnits("meters");
    _yCoordVarInfo.SetXType(xtype);
    _yCoordVarInfo.SetWName("");
    _yCoordVarInfo.SetCRatios(vector<size_t>());
    //_yCoordVarInfo.SetPeriodic(periodic);
    _yCoordVarInfo.SetUniform(false);

    _yCoordVarInfo.SetDimNames(dimNames);
    _yCoordVarInfo.SetTimeDimName(timeDimName);
    _yCoordVarInfo.SetAxis(1);

    return (0);
}

int DerivedCoordVar_PCSFromLatLon::_getVarBlock(size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    assert((varname == _lonName) || (varname == _latName));

    // Set up min & max for read of lat and lon variables that may be 1D
    //
    vector<size_t> myMin;
    vector<size_t> myMax;
    if (_make2DFlag && varname == _lonName) {
        myMin.push_back(min[0]);
        myMax.push_back(max[0]);
    } else if (_make2DFlag && varname == _latName) {
        myMin.push_back(min[1]);
        myMax.push_back(max[1]);
    } else {
        myMin = min;
        myMax = max;
    }

    int fd = _dc->OpenVariableRead(ts, varname, level, lod);
    if (fd < 0) return (-1);

    // Unblocked read!
    //
    int rc = _dc->ReadRegionBlock(fd, myMin, myMax, region);
    if (rc < 0) {
        _dc->CloseVariable(fd);
        return (-1);
    }

    return (_dc->CloseVariable(fd));
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_CF1D
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_CF1D::DerivedCoordVar_CF1D(const vector<string> &derivedVarNames, DC *dc, string dimName, int axis, string units) : DerivedCoordVar(derivedVarNames)
{
    assert(derivedVarNames.size() == 1);

    _dc = dc;
    _dimName = dimName;
    _coordName = _derivedVarNames[0];
    _dimLen = 0;

    _coordVarInfo = DC::CoordVar(_coordName, units, DC::XType::FLOAT, vector<bool>(1, false), axis, false, vector<string>(1, dimName), "");
}

int DerivedCoordVar_CF1D::Initialize()
{
    DC::Dimension dimension;
    int           rc = _dc->GetDimension(_dimName, dimension);
    if (rc < 0) return (-1);

    _dimLen = dimension.GetLength();

    return (0);
}

bool DerivedCoordVar_CF1D::GetBaseVarInfo(string varname, DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_CF1D::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_CF1D::GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    dims_at_level.push_back(_dimLen);
    bs_at_level.push_back(_dc->GetBlockSize()[0]);

    return (0);
}

int DerivedCoordVar_CF1D::OpenVariableRead(size_t ts, string varname, int level, int lod)
{
    if (varname != _coordName) {
        SetErrMsg("Invalid variable name: %s", varname.c_str());
        return (-1);
    }

    if (level != 0) {
        SetErrMsg("Invalid parameter");
        return (-1);
    }

    if (lod != 0) {
        SetErrMsg("Invalid parameter");
        return (-1);
    }

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, varname, level, lod);

    return (_fileTable.AddEntry(f));

    return (0);
}

int DerivedCoordVar_CF1D::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;

    return (0);
}

int DerivedCoordVar_CF1D::ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    assert(min.size() == 1);
    assert(max.size() == 1);

    float *regptr = region;
    for (size_t i = min[0]; i <= max[0]; i++) { *regptr++ = (float)i; }

    return (0);
}

bool DerivedCoordVar_CF1D::VariableExists(size_t ts, string varname, int reflevel, int lod) const
{
    if (varname != _coordName) return (false);
    if (reflevel != 0) return (false);
    if (lod != 0) return (false);

    return (true);
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_WRFTime
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_WRFTime::DerivedCoordVar_WRFTime(string derivedVarName, NetCDFCollection *ncdfc, string wrfTimeVar, string dimName, float p2si) : DerivedCoordVar(derivedVarName)
{
    _ncdfc = ncdfc;
    _times.clear();
    _timePerm.clear();
    _coordName = _derivedVarNames[0];
    _wrfTimeVar = wrfTimeVar;
    _p2si = p2si;
    _ovr_ts = 0;

    string units = "seconds";
    int    axis = 3;
    _coordVarInfo = DC::CoordVar(_coordName, units, DC::XType::FLOAT, vector<bool>(), axis, false, vector<string>(), dimName);
}

int DerivedCoordVar_WRFTime::Initialize()
{
    // Use UDUnits for unit conversion
    //
    UDUnits udunits;
    int     rc = udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", udunits.GetErrMsg().c_str());
        return (-1);
    }

    size_t numTS = _ncdfc->GetNumTimeSteps();

    vector<size_t> dims = _ncdfc->GetSpatialDims(_wrfTimeVar);

    char *buf = new char[dims[0] + 1];
    buf[dims[0]] = '\0';    // Null terminate

    const char *format = "%4d-%2d-%2d_%2d:%2d:%2d";

    // Read all of the formatted time strings up front - it's a 1D array
    // so we can simply store the results in memory - and convert from
    // a formatted time string to seconds since the EPOCH
    //
    _times.clear();
    for (size_t ts = 0; ts < numTS; ts++) {
        int fd = _ncdfc->OpenRead(ts, _wrfTimeVar);
        if (fd < 0) {
            SetErrMsg("Can't read time variable");
            return (-1);
        }

        int rc = _ncdfc->Read(buf, fd);
        if (rc < 0) {
            SetErrMsg("Can't read time variable");
            _ncdfc->Close(fd);
            return (-1);
        }
        _ncdfc->Close(fd);

        int year, mon, mday, hour, min, sec;
        rc = sscanf(buf, format, &year, &mon, &mday, &hour, &min, &sec);
        if (rc != 6) {
            rc = sscanf(buf, "%4d-%5d_%2d:%2d:%2d", &year, &mday, &hour, &min, &sec);
            mon = 1;
            if (rc != 5) {
                SetErrMsg("Unrecognized time stamp: %s", buf);
                return (-1);
            }
        }

        _times.push_back(udunits.EncodeTime(year, mon, mday, hour, min, sec) * _p2si);
    }
    delete[] buf;

    // The NetCDFCollection class doesn't handle the WRF time
    // variable. Hence, the time steps aren't sorted. Sort them now and
    // create a lookup table to map a time index to the correct time step
    // in the WRF data collection. N.B. this is only necessary if multiple
    // WRF files are present and they're not passed to Initialize() in
    // the correct order.
    //

    _timePerm.clear();
    for (int i = 0; i != _times.size(); i++) { _timePerm.push_back(i); }
    sort(_timePerm.begin(), _timePerm.end(), [&](const int &a, const int &b) { return (_times[a] < _times[b]); });

    return (0);
}

bool DerivedCoordVar_WRFTime::GetBaseVarInfo(string varname, DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_WRFTime::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_WRFTime::GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    return (0);
}

int DerivedCoordVar_WRFTime::OpenVariableRead(size_t ts, string varname, int level, int lod)
{
    if (varname != _coordName) {
        SetErrMsg("Invalid variable name: %s", varname.c_str());
        return (-1);
    }

    ts = ts < _times.size() ? ts : _times.size() - 1;

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, varname, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_WRFTime::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    _fileTable.RemoveEntry(fd);
    delete f;

    return (0);
}

int DerivedCoordVar_WRFTime::ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    assert(min.size() == 0);
    assert(max.size() == 0);

    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    size_t ts = f->GetTS();

    *region = _times[ts];

    return (0);
}

bool DerivedCoordVar_WRFTime::VariableExists(size_t ts, string varname, int reflevel, int lod) const
{
    if (varname != _coordName) return (false);

    return (ts < _times.size());
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_Staggered
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_Staggered::DerivedCoordVar_Staggered(string derivedVarName, string stagDimName, DC *dc, string inName, string dimName) : DerivedCoordVar(derivedVarName)
{
    _derivedVarName = derivedVarName;
    _stagDimName = stagDimName;
    _inName = inName;
    _dimName = dimName;
    _dc = dc;
}

int DerivedCoordVar_Staggered::Initialize()
{
    bool ok = _dc->GetCoordVarInfo(_inName, _coordVarInfo);
    if (!ok) return (-1);

    vector<size_t> dims, bs;
    int            rc = _dc->GetDimLensAtLevel(_inName, 0, dims, bs);
    if (rc < 0) return (-1);

    if (dims != bs) {
        SetErrMsg("Blocked data not supported");
        return (-1);
    }

    vector<string> dimNames = _coordVarInfo.GetDimNames();
    _stagDim = -1;
    for (int i = 0; i < dimNames.size(); i++) {
        if (dimNames[i] == _dimName) {
            _stagDim = i;
            dimNames[i] = _stagDimName;
            break;
        }
    }
    if (_stagDim < 0) {
        SetErrMsg("Dimension %s not found", _dimName.c_str());
        return (-1);
    }

    // Change the name of the staggered dimension
    //
    _coordVarInfo.SetDimNames(dimNames);

    return (0);
}

bool DerivedCoordVar_Staggered::GetBaseVarInfo(string varname, DC::BaseVar &var) const
{
    if (varname == _derivedVarName) {
        var = _coordVarInfo;
        return (true);
    }
    return (false);
}

bool DerivedCoordVar_Staggered::GetCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    if (varname == _derivedVarName) {
        cvar = _coordVarInfo;
        return (true);
    }
    return (false);
}

int DerivedCoordVar_Staggered::GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    int rc = _dc->GetDimLensAtLevel(_inName, level, dims_at_level, bs_at_level);
    if (rc < 0) return (-1);

    dims_at_level[_stagDim] += 1;

    return (0);
}

int DerivedCoordVar_Staggered::OpenVariableRead(size_t ts, string varname, int level, int lod)
{
    if (varname != _derivedVarName) {
        SetErrMsg("Invalid variable name: %s", varname.c_str());
        return (-1);
    }

    int fd = _dc->OpenVariableRead(ts, _inName, level, lod);
    if (fd < 0) return (fd);

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, varname, level, lod, fd);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_Staggered::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int rc = _dc->CloseVariable(f->GetAux());

    _fileTable.RemoveEntry(f->GetAux());
    delete f;

    return (rc);
}

void DerivedCoordVar_Staggered::_transpose(const float *a, float *b, vector<size_t> inDims, int axis) const
{
    assert(inDims.size() < 4);
    assert(axis >= 0 && axis < inDims.size());

    size_t sz = vproduct(inDims);

    // No-op if axis is 0
    //
    if (axis == 0) {    // 1D, 2D, and 3D case
        for (size_t i = 0; i < sz; i++) { b[i] = a[i]; }
        return;
    }

    if (inDims.size() == 2) {
        assert(axis == 1);

        Transpose(a, b, inDims[0], inDims[1]);
    } else if (inDims.size() == 3) {
        assert(axis == 1 || axis == 2);

        if (axis == 1) {
            size_t stride = inDims[0] * inDims[1];
            ;
            const float *aptr = a;
            float *      bptr = b;
            for (size_t i = 0; i < inDims[2]; i++) {
                Transpose(aptr, bptr, inDims[0], inDims[1]);
                aptr += stride;
                bptr += stride;
            }
        } else if (axis == 2) {
            // We can treat 3D array as 2D in this case, linearizing X and Y
            //
            Transpose(a, b, inDims[0] * inDims[1], inDims[2]);
        }
    }
}

void DerivedCoordVar_Staggered::_transpose(vector<size_t> inDims, int axis, vector<size_t> &outDims) const
{
    outDims = inDims;

    if (axis == 1) {
        size_t tmp = outDims[0];
        outDims[0] = outDims[1];
        outDims[1] = tmp;
    } else if (axis == 2) {
        size_t tmp = outDims[0];
        outDims[0] = outDims[2];
        outDims[2] = tmp;
    }
}

int DerivedCoordVar_Staggered::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    string varname = f->GetVarname();
    int    level = f->GetLevel();

    vector<size_t> dims, bs;
    int            rc = GetDimLensAtLevel(varname, level, dims, bs);
    if (rc < 0) return (-1);

    vector<size_t> inMin = min;
    vector<size_t> inMax = max;

    // adjust coords for native data so that we have what we
    // need for interpolation or extrapolation.
    //

    // Lower bound on boundary => extrapolation
    //
    if (min[_stagDim] == 0) {
        if (max[_stagDim] == 0) { inMax[_stagDim] += 1; }
    } else {
        inMin[_stagDim] -= 1;
    }

    // Upper bound boundary => extrapolation
    //
    if (max[_stagDim] == (dims[_stagDim] - 1)) {
        if (min[_stagDim] == (dims[_stagDim] - 2)) { inMin[_stagDim] -= 1; }
        inMax[_stagDim] -= 1;
    }

    vector<size_t> inDims, outDims;
    for (size_t i = 0; i < min.size(); i++) {
        inDims.push_back(inMax[i] - inMin[i] + 1);
        outDims.push_back(max[i] - min[i] + 1);
    }
    size_t sz = std::max(vproduct(outDims), vproduct(inDims));

    float *buf1 = new float[sz];
    float *buf2 = new float[sz];

    // Read unstaggered data
    //
    rc = _dc->ReadRegion(f->GetAux(), inMin, inMax, buf1);
    if (rc < 0) return (-1);

    // Tranpose the dimensions and array so that we always interpolate
    // with unit stride
    //
    vector<size_t> inDimsT;     // transposed input dimensions
    vector<size_t> outDimsT;    // transposed output dimensions
    _transpose(inDims, _stagDim, inDimsT);
    _transpose(outDims, _stagDim, outDimsT);

    _transpose(buf1, buf2, inDims, _stagDim);

    size_t nz = inDimsT.size() == 3 ? inDimsT[2] : 1;
    size_t ny = inDimsT.size() == 2 ? inDimsT[1] : 1;
    size_t nx = inDimsT.size() == 1 ? inDimsT[0] : 1;

    // Interpolate interior
    //
    size_t nxs = outDimsT[0];    // staggered dimension
    for (size_t k = 0; k < nz; k++) {
        for (size_t j = 0; j < ny; j++) {
            for (size_t i = 1; i < nxs - 1; i++) { buf1[k * nxs * ny + j * nxs + k] = 0.5 * (buf2[k * nx * ny + j * nx + i - 1] + buf2[k * nx * ny + j * nx + i]); }
        }
    }

    // Next extrapolate boundary points
    //
    for (size_t k = 0; k < nz; k++) {
        for (size_t j = 0; j < ny; j++) {
            // left boundary
            //
            buf1[k * nxs * ny + j * nxs] = buf2[k * nx * ny + j * nx + 0] + (-0.5 * (buf2[k * nx * ny + j * nx + 1] - buf2[k * nx * ny + j * nx + 0]));
        }
    }

    for (size_t k = 0; k < nz; k++) {
        for (size_t j = 0; j < ny; j++) {
            // right boundary
            //
            buf1[k * nxs * ny + j * nxs + nxs - 1] = buf2[k * nx * ny + j * nx + nx - 2] + (0.5 * (buf2[k * nx * ny + j * nx + nx - 1] - buf2[k * nx * ny + j * nx + nx - 2]));
        }
    }

    // Undo tranpose
    //
    _transpose(buf1, region, outDimsT, _stagDim);

    delete[] buf1;
    delete[] buf2;

    return (0);
}

bool DerivedCoordVar_Staggered::VariableExists(size_t ts, string varname, int reflevel, int lod) const
{
    if (varname != _derivedVarName) return (false);

    return (_dc->VariableExists(ts, _inName, reflevel, lod));
}
