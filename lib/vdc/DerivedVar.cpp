#include "vapor/VAssert.h"
#include <sstream>
#include <algorithm>
#include <set>
#include <vapor/UDUnitsClass.h>
#include <vapor/NetCDFCollection.h>
#include <vapor/utils.h>
#include <vapor/WASP.h>
#include <vapor/DerivedVar.h>

using namespace VAPoR;
using namespace Wasp;

namespace {

#ifdef UNUSED_FUNCTION
size_t numBlocks(size_t min, size_t max, size_t bs)
{
    size_t b0 = min / bs;
    size_t b1 = max / bs;
    return (b1 - b0 + 1);
}
#endif

#ifdef UNUSED_FUNCTION
size_t numBlocks(const vector<size_t> &min, const vector<size_t> &max, const vector<size_t> &bs)
{
    VAssert(min.size() == max.size());
    VAssert(min.size() == bs.size());

    size_t nblocks = 1;
    for (int i = 0; i < bs.size(); i++) { nblocks *= numBlocks(min[i], max[i], bs[i]); }
    return (nblocks);
}
#endif

#ifdef UNUSED_FUNCTION
size_t numBlocks(const vector<size_t> &dims, const vector<size_t> &bs)
{
    VAssert(dims.size() == bs.size());

    size_t nblocks = 1;
    for (int i = 0; i < bs.size(); i++) {
        VAssert(dims[i] != 0);
        nblocks *= (((dims[i] - 1) / bs[i]) + 1);
    }
    return (nblocks);
}
#endif

size_t numElements(const vector<size_t> &min, const vector<size_t> &max)
{
    VAssert(min.size() == max.size());

    size_t nElements = 1;
    for (int i = 0; i < min.size(); i++) { nElements *= (max[i] - min[i] + 1); }
    return (nElements);
}

#ifdef UNUSED_FUNCTION
size_t blockSize(const vector<size_t> &bs)
{
    size_t sz = 1;
    for (int i = 0; i < bs.size(); i++) { sz *= bs[i]; }
    return (sz);
}
#endif

#ifdef UNUSED_FUNCTION
vector<size_t> increment(vector<size_t> dims, vector<size_t> coord)
{
    VAssert(dims.size() == coord.size());

    for (int i = 0; i < coord.size(); i++) {
        coord[i] += 1;
        if (coord[i] < (dims[i])) { break; }
        coord[i] = 0;
    }
    return (coord);
}
#endif

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

#ifdef UNUSED_FUNCTION
void extractBlock(const float *data, const vector<size_t> &dims, const vector<size_t> &bcoords, const vector<size_t> &bs, float *block)
{
    VAssert(dims.size() == bcoords.size());
    VAssert(dims.size() == bs.size());

    // Block dimensions
    //
    size_t bz = bs.size() > 2 ? bs[2] : 1;
    size_t by = bs.size() > 1 ? bs[1] : 1;
    size_t bx = bs.size() > 0 ? bs[0] : 1;

    // Data dimensions
    //
    size_t nz = dims.size() > 2 ? dims[2] : 1;
    size_t ny = dims.size() > 1 ? dims[1] : 1;
    size_t nx = dims.size() > 0 ? dims[0] : 1;

    // Data dimensions
    //
    size_t bcz = bcoords.size() > 2 ? bcoords[2] : 0;
    size_t bcy = bcoords.size() > 1 ? bcoords[1] : 0;
    size_t bcx = bcoords.size() > 0 ? bcoords[0] : 0;

    size_t z = bcz * bz;
    for (size_t zb = 0; zb < bz && z < nz; zb++, z++) {
        size_t y = bcy * by;

        for (size_t yb = 0; yb < by && y < ny; yb++, y++) {
            size_t x = bcx * bx;

            for (size_t xb = 0; xb < bx && x < nx; xb++, x++) { block[bx * by * zb + bx * yb + xb] = data[nx * ny * z + nx * y + x]; }
        }
    }
}
#endif

#ifdef UNUSED_FUNCTION
void blockit(const float *data, const vector<size_t> &dims, const vector<size_t> &bs, float *blocks)
{
    VAssert(dims.size() == bs.size());

    size_t block_size = vproduct(bs);

    vector<size_t> bdims;
    for (int i = 0; i < bs.size(); i++) { bdims.push_back(((dims[i] - 1) / bs[i]) + 1); }

    size_t nbz = bdims.size() > 2 ? bdims[2] : 1;
    size_t nby = bdims.size() > 1 ? bdims[1] : 1;
    size_t nbx = bdims.size() > 0 ? bdims[0] : 1;

    float *        blockptr = blocks;
    vector<size_t> bcoord(bdims.size(), 0);

    for (size_t zb = 0; zb < nbz; zb++) {
        for (size_t yb = 0; yb < nby; yb++) {
            for (size_t xb = 0; xb < nbx; xb++) {
                extractBlock(data, dims, bcoord, bs, blockptr);
                blockptr += block_size;
                bcoord = increment(bdims, bcoord);
            }
        }
    }
}
#endif

// make 2D lat and lon arrays from 1D arrays by replication, in place
//
void make2D(float *lonBuf, float *latBuf, vector<size_t> dims)
{
    VAssert(dims.size() == 2);
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

// Transpose a 1D, 2D, or 3D array. For 1D 'a' is simply copied
// to 'b'. Otherwise 'b' contains a permuted version of 'a' as follows:
//
//    axis        1D        2D        3D
//    ----        --        --        --
//    0	          (0)       (0,1)     (0,1,2)
//    1	          N/A       (1,0)     (1,0,2)
//    2	          N/A       N/A       (2,0,1)
//
// where the numbers in parenthesis indicate the permutation of the
// axes.
//
// NOTE: The contents of 'a' are overwritten
//
void transpose(float *a, float *b, vector<size_t> inDims, int axis)
{
    VAssert(inDims.size() < 4);
    VAssert(axis >= 0 && axis < inDims.size());

    size_t sz = vproduct(inDims);

    // No-op if axis is 0
    //
    if (axis == 0) {    // 1D, 2D, and 3D case
        for (size_t i = 0; i < sz; i++) { b[i] = a[i]; }
        return;
    }

    if (inDims.size() == 2) {
        VAssert(axis == 1);

        Wasp::Transpose(a, b, inDims[0], inDims[1]);
    } else if (inDims.size() == 3) {
        VAssert(axis == 1 || axis == 2);

        size_t stride = inDims[0] * inDims[1];
        ;
        const float *aptr = a;
        float *      bptr = b;
        for (size_t i = 0; i < inDims[2]; i++) {
            Wasp::Transpose(aptr, bptr, inDims[0], inDims[1]);
            aptr += stride;
            bptr += stride;
        }

        // For (2,1,0) permutation we do (0,1,2) -> (1,0,2) -> (2,1,0)
        //
        if (axis == 2) {
            // We can treat 3D array as 2D in this case, linearizing X and Y
            //
            Wasp::Transpose(b, a, inDims[0] * inDims[1], inDims[2]);

            // Ugh need to copy data from a back to b
            //
            for (size_t i = 0; i < vproduct(inDims); i++) { b[i] = a[i]; }
        }
    }
}

void transpose(vector<size_t> inDims, int axis, vector<size_t> &outDims)
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

void resampleToStaggered(float *src, const vector<size_t> &inMin, const vector<size_t> &inMax, float *dst, const vector<size_t> &outMin, const vector<size_t> &outMax, int stagDim)
{
    VAssert(inMin.size() == inMax.size());
    VAssert(inMin.size() == outMax.size());
    VAssert(inMin.size() == outMax.size());

    vector<size_t> inDims, outDims;
    for (size_t i = 0; i < outMin.size(); i++) {
        inDims.push_back(inMax[i] - inMin[i] + 1);
        outDims.push_back(outMax[i] - outMin[i] + 1);
    }
    size_t sz = std::max(vproduct(outDims), vproduct(inDims));

    float *buf = new float[sz];

    // Tranpose the dimensions and array so that we always interpolate
    // with unit stride
    //
    vector<size_t> inDimsT;     // transposed input dimensions
    vector<size_t> outDimsT;    // transposed output dimensions
    transpose(inDims, stagDim, inDimsT);
    transpose(outDims, stagDim, outDimsT);

    transpose(src, buf, inDims, stagDim);

    size_t nz = inDimsT.size() >= 3 ? inDimsT[2] : 1;
    size_t ny = inDimsT.size() >= 2 ? inDimsT[1] : 1;
    size_t nx = inDimsT.size() >= 1 ? inDimsT[0] : 1;

    // Interpolate interior
    //
    size_t nxs = outDimsT[0];    // staggered dimension
    size_t i0 = outMin[stagDim] > inMin[stagDim] ? 0 : 1;

    for (size_t k = 0; k < nz; k++) {
        for (size_t j = 0; j < ny; j++) {
            for (size_t i = 0, ii = i0; i < nx - 1; i++, ii++) { src[k * nxs * ny + j * nxs + ii] = 0.5 * (buf[k * nx * ny + j * nx + i] + buf[k * nx * ny + j * nx + i + 1]); }
        }
    }

    // Next extrapolate boundary points if needed
    //
    // left boundary
    //
    if (outMin[stagDim] <= inMin[stagDim]) {
        if (inMin[stagDim] < inMax[stagDim]) {
            for (size_t k = 0; k < nz; k++) {
                for (size_t j = 0; j < ny; j++) { src[k * nxs * ny + j * nxs] = buf[k * nx * ny + j * nx + 0] + (-0.5 * (buf[k * nx * ny + j * nx + 1] - buf[k * nx * ny + j * nx + 0])); }
            }
        } else {
            for (size_t k = 0; k < nz; k++) {
                for (size_t j = 0; j < ny; j++) { src[k * nxs * ny + j * nxs] = buf[k * nx * ny + j * nx + 0]; }
            }
        }
    }

    // right boundary
    //
    if (outMax[stagDim] > inMax[stagDim]) {
        if (inMin[stagDim] < inMax[stagDim]) {
            for (size_t k = 0; k < nz; k++) {
                for (size_t j = 0; j < ny; j++) {
                    src[k * nxs * ny + j * nxs + nxs - 1] = buf[k * nx * ny + j * nx + nx - 1] + (0.5 * (buf[k * nx * ny + j * nx + nx - 1] - buf[k * nx * ny + j * nx + nx - 2]));
                }
            }
        } else {
            for (size_t k = 0; k < nz; k++) {
                for (size_t j = 0; j < ny; j++) { src[k * nxs * ny + j * nxs + nxs - 1] = buf[k * nx * ny + j * nx + nx - 1]; }
            }
        }
    }

    // Undo tranpose
    //
    transpose(src, dst, outDimsT, stagDim);

    delete[] buf;
}

void resampleToUnStaggered(float *src, const vector<size_t> &inMin, const vector<size_t> &inMax, float *dst, const vector<size_t> &outMin, const vector<size_t> &outMax, int stagDim)
{
    VAssert(inMin.size() == inMax.size());
    VAssert(inMin.size() == outMax.size());
    VAssert(inMin.size() == outMax.size());

    vector<size_t> myOutMax = outMax;
    vector<size_t> myOutMin = outMin;

    myOutMin[stagDim] += 1;
    myOutMax[stagDim] += 1;

    resampleToStaggered(src, inMin, inMax, dst, myOutMin, myOutMax, stagDim);
}

#ifdef UNIT_TEST

void print_matrix(const float *a, const vector<size_t> &dims)
{
    size_t nz = dims.size() >= 3 ? dims[2] : 1;
    size_t ny = dims.size() >= 2 ? dims[1] : 1;
    size_t nx = dims.size() >= 1 ? dims[0] : 1;

    for (int k = 0; k < nz; k++) {
        for (int j = 0; j < ny; j++) {
            for (int i = 0; i < nx; i++) { cout << a[k * nx * ny + j * nx + i] << " "; }
            cout << endl;
        }
        cout << endl;
    }
}

void test_resample(int stagDim)
{
    vector<size_t> inMin = {0, 0, 0};
    vector<size_t> inMax = {1, 2, 3};

    vector<size_t> outMin = inMin;
    vector<size_t> outMax = inMax;

    outMax[stagDim] += 1;

    vector<size_t> inDims, outDims;
    for (int i = 0; i < inMax.size(); i++) {
        inDims.push_back(inMax[i] - inMin[i] + 1);
        outDims.push_back(outMax[i] - outMin[i] + 1);
    }

    size_t nz = inDims.size() >= 3 ? inDims[2] : 1;
    size_t ny = inDims.size() >= 2 ? inDims[1] : 1;
    size_t nx = inDims.size() >= 1 ? inDims[0] : 1;

    size_t nzs = outDims.size() >= 3 ? outDims[2] : 1;
    size_t nys = outDims.size() >= 2 ? outDims[1] : 1;
    size_t nxs = outDims.size() >= 1 ? outDims[0] : 1;

    size_t sz = std::max(vproduct(outDims), vproduct(inDims));
    float *src = new float[sz];
    float *dst = new float[sz];

    for (int k = 0; k < nz; k++) {
        for (int j = 0; j < ny; j++) {
            for (int i = 0; i < nx; i++) { src[k * nx * ny + j * nx + i] = k * nx * ny + j * nx + i; }
        }
    }

    for (int k = 0; k < nzs; k++) {
        for (int j = 0; j < nys; j++) {
            for (int i = 0; i < nxs; i++) { dst[k * nxs * nys + j * nxs + i] = 99; }
        }
    }

    cout << "original array" << endl;
    print_matrix(src, inDims);

    resampleToStaggered(src, inMin, inMax, dst, outMin, outMax, stagDim);

    cout << endl << endl;

    cout << "staggered array" << endl;
    print_matrix(dst, outDims);

    resampleToUnStaggered(dst, outMin, outMax, src, inMin, inMax, stagDim);

    cout << "reconstructed unstaggered array" << endl;
    print_matrix(src, inDims);
}

int main(int argc, char **argv)
{
    VAssert(argc == 2);
    int stagDim = atoi(argv[1]);
    test_resample(stagDim);
}

#endif

};    // namespace

int DerivedVar::_getVar(DC *dc, size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, float *region) const
{
    int fd = dc->OpenVariableRead(ts, varname, level, lod);
    if (fd < 0) return (-1);

    int rc = dc->ReadRegion(fd, min, max, region);
    if (rc < 0) {
        dc->CloseVariable(fd);
        return (-1);
    }

    return (dc->CloseVariable(fd));
}

int DerivedVar::_getVarDestagger(DC *dc, size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, float *region, int stagDim) const
{
    VAssert(stagDim >= 0 && stagDim < max.size());
    VAssert(min.size() == max.size());

    vector<size_t> maxIn = max;
    maxIn[stagDim]++;

    vector<size_t> dimsIn;
    for (int i = 0; i < min.size(); i++) { dimsIn.push_back(max[i] - min[i] + 1); }
    vector<float> buf(vproduct(dimsIn));

    int rc = _getVar(dc, ts, varname, level, lod, min, maxIn, buf.data());
    if (rc < 0) return (rc);

    resampleToUnStaggered(buf.data(), min, maxIn, region, min, max, stagDim);

    return (0);
}

int DerivedVar::_getVarBlock(DC *dc, size_t ts, string varname, int level, int lod, const vector<size_t> &min, const vector<size_t> &max, float *region) const
{
    int fd = dc->OpenVariableRead(ts, varname, level, lod);
    if (fd < 0) return (-1);

    int rc = dc->ReadRegionBlock(fd, min, max, region);
    if (rc < 0) {
        dc->CloseVariable(fd);
        return (-1);
    }

    return (dc->CloseVariable(fd));
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_PCSFromLatLon
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_PCSFromLatLon::DerivedCoordVar_PCSFromLatLon(string derivedVarName, DC *dc, vector<string> inNames, string proj4String, bool uGridFlag, bool lonFlag) : DerivedCoordVar(derivedVarName)
{
    VAssert(inNames.size() == 2);

    _dc = dc;
    _proj4String = proj4String;
    _lonName = inNames[0];
    _latName = inNames[1];
    _make2DFlag = false;
    _uGridFlag = uGridFlag;
    _lonFlag = lonFlag;
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

bool DerivedCoordVar_PCSFromLatLon::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_PCSFromLatLon::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_PCSFromLatLon::GetDimLensAtLevel(int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    dims_at_level = _dimLens;

    // No blocking
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVar_PCSFromLatLon::OpenVariableRead(size_t ts, int, int)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, -1, -1);

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
int DerivedCoordVar_PCSFromLatLon::_readRegionHelperCylindrical(DC::FileTable::FileObject *f, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 1);
    VAssert(min.size() == max.size());

    size_t ts = f->GetTS();
    string varname = f->GetVarname();
    int    lod = f->GetLOD();

    size_t nElements = max[0] - min[0] + 1;
    float *buf = new float[nElements];
    for (int i = 0; i < nElements; i++) { buf[i] = 0.0; }

    string geoCoordVar;
    if (_lonFlag) {
        geoCoordVar = _lonName;
    } else {
        geoCoordVar = _latName;
    }

    int rc = _getVar(_dc, ts, geoCoordVar, -1, lod, min, max, region);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    if (_lonFlag) {
        rc = _proj4API.Transform(region, buf, nElements);
    } else {
        rc = _proj4API.Transform(buf, region, nElements);
    }

    delete[] buf;

    return (rc);
}

int DerivedCoordVar_PCSFromLatLon::_readRegionHelper1D(DC::FileTable::FileObject *f, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    size_t ts = f->GetTS();
    string varname = f->GetVarname();
    int    lod = f->GetLOD();

    // Need temporary buffer space for the X or Y coordinate
    // NOT being returned (we still need to calculate it)
    //
    size_t nElements = numElements(min, max);
    float *buf = new float[nElements];

    vector<size_t> roidims;
    for (int i = 0; i < min.size(); i++) { roidims.push_back(max[i] - min[i] + 1); }

    // Assign temporary buffer 'buf' as appropriate
    //
    float *lonBufPtr;
    float *latBufPtr;
    if (_lonFlag) {
        lonBufPtr = region;
        latBufPtr = buf;
    } else {
        lonBufPtr = buf;
        latBufPtr = region;
    }

    // Reading 1D data so no blocking
    //
    vector<size_t> lonMin = {min[0]};
    vector<size_t> lonMax = {max[0]};
    int            rc = _getVar(_dc, ts, _lonName, -1, lod, lonMin, lonMax, lonBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    vector<size_t> latMin = {min[1]};
    vector<size_t> latMax = {max[1]};
    rc = _getVar(_dc, ts, _latName, -1, lod, latMin, latMax, latBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    // Combine the 2 1D arrays into a 2D array
    //
    make2D(lonBufPtr, latBufPtr, roidims);

    rc = _proj4API.Transform(lonBufPtr, latBufPtr, vproduct(roidims));

    delete[] buf;

    return (rc);
}

int DerivedCoordVar_PCSFromLatLon::_readRegionHelper2D(DC::FileTable::FileObject *f, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    size_t ts = f->GetTS();
    string varname = f->GetVarname();
    int    lod = f->GetLOD();

    // Need temporary buffer space for the X or Y coordinate
    // NOT being returned (we still need to calculate it)
    //
    size_t nElements = numElements(min, max);
    float *buf = new float[nElements];

    // Assign temporary buffer 'buf' as appropriate
    //
    float *lonBufPtr;
    float *latBufPtr;
    if (_lonFlag) {
        lonBufPtr = region;
        latBufPtr = buf;
    } else {
        lonBufPtr = buf;
        latBufPtr = region;
    }

    int rc = _getVar(_dc, ts, _lonName, -1, lod, min, max, lonBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    rc = _getVar(_dc, ts, _latName, -1, lod, min, max, latBufPtr);
    if (rc < 0) {
        delete[] buf;
        return (rc);
    }

    rc = _proj4API.Transform(lonBufPtr, latBufPtr, nElements);

    delete[] buf;

    return (rc);
}

int DerivedCoordVar_PCSFromLatLon::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    if (min.size() == 1) {
        // Lat and Lon are 1D variables
        //
        return (_readRegionHelperCylindrical(f, min, max, region));
    } else {
        if (_make2DFlag) {
            // Lat and Lon are 1D variables but projections to PCS
            // result in X and Y coordinate variables that are 2D
            //
            return (_readRegionHelper1D(f, min, max, region));
        } else {
            return (_readRegionHelper2D(f, min, max, region));
        }
    }
}

bool DerivedCoordVar_PCSFromLatLon::VariableExists(size_t ts, int, int) const { return (_dc->VariableExists(ts, _lonName, -1, -1) && _dc->VariableExists(ts, _latName, -1, -1)); }

int DerivedCoordVar_PCSFromLatLon::_setupVar()
{
    DC::CoordVar lonVar;
    bool         ok = _dc->GetCoordVarInfo(_lonName, lonVar);
    if (!ok) return (-1);

    DC::CoordVar latVar;
    ok = _dc->GetCoordVarInfo(_latName, latVar);
    if (!ok) return (-1);

    vector<size_t> lonDims;
    ok = _dc->GetVarDimLens(_lonName, true, lonDims, -1);
    if (!ok) {
        SetErrMsg("GetVarDimLens(%s) failed", _lonName.c_str());
        return (-1);
    }

    vector<size_t> latDims;
    ok = _dc->GetVarDimLens(_latName, true, latDims, -1);
    if (!ok) {
        SetErrMsg("GetVarDimLens(%s) failed", _lonName.c_str());
        return (-1);
    }

    if (lonDims.size() != latDims.size()) {
        SetErrMsg("Incompatible block size");
        return (-1);
    }

    bool cylProj = _proj4API.IsCylindrical();

    vector<string> dimNames;
    if (lonVar.GetDimNames().size() == 1 && !_uGridFlag) {
        if (cylProj) {
            if (_lonFlag) {
                dimNames.push_back(lonVar.GetDimNames()[0]);
                _dimLens.push_back(lonDims[0]);
            } else {
                dimNames.push_back(latVar.GetDimNames()[0]);
                _dimLens.push_back(latDims[0]);
            }
        } else {
            dimNames.push_back(lonVar.GetDimNames()[0]);
            dimNames.push_back(latVar.GetDimNames()[0]);
            _dimLens.push_back(lonDims[0]);
            _dimLens.push_back(latDims[0]);
            _make2DFlag = true;
        }
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
        VAssert(lonVar.GetDimNames().size() == 1 && _uGridFlag);
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

    _coordVarInfo.SetName(_derivedVarName);
    _coordVarInfo.SetUnits("meters");
    _coordVarInfo.SetXType(xtype);
    _coordVarInfo.SetWName("");
    _coordVarInfo.SetCRatios(vector<size_t>());
    _coordVarInfo.SetPeriodic(periodic);
    _coordVarInfo.SetUniform(false);

    _coordVarInfo.SetDimNames(dimNames);
    _coordVarInfo.SetTimeDimName(timeDimName);

    _coordVarInfo.SetAxis(_lonFlag ? 0 : 1);

    return (0);
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_CF1D
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_CF1D::DerivedCoordVar_CF1D(string derivedVarName, DC *dc, string dimName, int axis, string units) : DerivedCoordVar(derivedVarName)
{
    _dc = dc;
    _dimName = dimName;

    _coordVarInfo = DC::CoordVar(_derivedVarName, units, DC::XType::FLOAT, vector<bool>(1, false), axis, true, vector<string>(1, dimName), "");
}

int DerivedCoordVar_CF1D::Initialize()
{
    return (0);
}

bool DerivedCoordVar_CF1D::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_CF1D::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_CF1D::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{ return GetDimLensAtLevel(level, dims_at_level, bs_at_level, -1); }

int DerivedCoordVar_CF1D::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level, long ts) const
{
    dims_at_level.clear();
    bs_at_level.clear();
    
    DC::Dimension dimension;
    int           rc = _dc->GetDimension(_dimName, dimension, ts);
    if (rc < 0) return (-1);

    dims_at_level.push_back(dimension.GetLength());

    // No blocking
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVar_CF1D::OpenVariableRead(size_t ts, int level, int lod)
{
//    if (level != 0) {
//        SetErrMsg("Invalid parameter");
//        return (-1);
//    }

//    if (lod != 0) {
//        SetErrMsg("Invalid parameter");
//        return (-1);
//    }

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
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

int DerivedCoordVar_CF1D::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 1);
    VAssert(max.size() == 1);

    float *regptr = region;
    for (size_t i = min[0]; i <= max[0]; i++) { *regptr++ = (float)i; }

    return (0);
}

bool DerivedCoordVar_CF1D::VariableExists(size_t ts, int reflevel, int lod) const
{
    if (reflevel != 0) return (false);
    if (lod != 0) return (false);

    return (true);
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_CF2D
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_CF2D::DerivedCoordVar_CF2D(string derivedVarName, vector<string> dimNames, vector<size_t> dimLens, int axis, string units, const vector<float> &data) : DerivedCoordVar(derivedVarName)
{
    VAssert(dimNames.size() == 2);
    VAssert(dimLens.size() == 2);
    VAssert(dimLens[0] * dimLens[1] <= data.size());

    _dimNames = dimNames;
    _dimLens = dimLens;
    _data = data;

    _coordVarInfo = DC::CoordVar(_derivedVarName, units, DC::XType::FLOAT, vector<bool>(2, false), axis, false, dimNames, "");
}

int DerivedCoordVar_CF2D::Initialize() { return (0); }

bool DerivedCoordVar_CF2D::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_CF2D::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_CF2D::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    dims_at_level = _dimLens;

    // No blocking
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVar_CF2D::OpenVariableRead(size_t ts, int level, int lod) { return (0); }

int DerivedCoordVar_CF2D::CloseVariable(int fd) { return (0); }

int DerivedCoordVar_CF2D::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 2);
    VAssert(max.size() == 2);

    float *regptr = region;
    for (size_t j = min[1]; j <= max[1]; j++) {
        for (size_t i = min[0]; i <= max[0]; i++) { *regptr++ = (float)_data[j * _dimLens[0] + i]; }
    }

    return (0);
}

bool DerivedCoordVar_CF2D::VariableExists(size_t ts, int reflevel, int lod) const
{
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
    _wrfTimeVar = wrfTimeVar;
    _p2si = p2si;
    _ovr_ts = 0;

    string units = "seconds";
    int    axis = 3;
    _coordVarInfo = DC::CoordVar(_derivedVarName, units, DC::XType::FLOAT, vector<bool>(), axis, false, vector<string>(), dimName);
}

int DerivedCoordVar_WRFTime::_encodeTime(UDUnits &udunits, const vector<string> &timeStrings, vector<double> &times) const
{
    times.clear();

    for (int i = 0; i < timeStrings.size(); i++) {
        const string &s = timeStrings[i];

        const char *format6 = "%4d-%2d-%2d_%2d:%2d:%2d";
        int         year, mon, mday, hour, min, sec;
        int         rc = sscanf(s.data(), format6, &year, &mon, &mday, &hour, &min, &sec);
        if (rc != 6) {
            // Alternate date format
            //
            const char *format5 = "%4d-%5d_%2d:%2d:%2d";
            rc = sscanf(s.data(), format5, &year, &mday, &hour, &min, &sec);
            if (rc != 5) {
                SetErrMsg("Unrecognized time stamp: %s", s.data());
                return (-1);
            }
            mon = 1;
        }

        times.push_back(udunits.EncodeTime(year, mon, mday, hour, min, sec) * _p2si);
    }
    return (0);
}

int DerivedCoordVar_WRFTime::Initialize()
{
    _times.clear();
    _timePerm.clear();

    // Use UDUnits for unit conversion
    //
    UDUnits udunits;
    int     rc = udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", udunits.GetErrMsg().c_str());
        return (-1);
    }

    size_t numTS = _ncdfc->GetNumTimeSteps();
    if (numTS < 1) return (0);

    vector<size_t> dims = _ncdfc->GetSpatialDims(_wrfTimeVar);
    if (dims.size() != 1) {
        SetErrMsg("Invalid WRF time variable : %s", _wrfTimeVar.c_str());
        return (-1);
    }

    // Read all of the formatted time strings up front - it's a 1D array
    // so we can simply store the results in memory - and convert from
    // a formatted time string to seconds since the EPOCH
    //
    vector<string> timeStrings;
    char *         buf = new char[dims[0] + 1];
    buf[dims[0]] = '\0';
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
            delete[] buf;
            return (-1);
        }
        _ncdfc->Close(fd);

        timeStrings.push_back(buf);
    }
    delete[] buf;

    // Encode time stamp string as double precision float
    //
    vector<double> timesD;
    rc = _encodeTime(udunits, timeStrings, timesD);
    if (rc < 0) return (rc);

    // Convert to single precision because that's what the API supports
    //
    for (int i = 0; i < timesD.size(); i++) { _times.push_back((float)timesD[i]); }

    // For high temporal resolution single precision may be insufficient:
    // converting from double to single may result in non-unique values.
    // Check to see that the number of unique values is the same for the
    // double and float vectors. If not, change the year to 2000 to reduce
    // the precision needed.
    //
    if (std::set<double>(timesD.begin(), timesD.end()).size() != std::set<double>(_times.begin(), _times.end()).size()) {
        _times.clear();

        for (int i = 0; i < timeStrings.size(); i++) { timeStrings[i].replace(0, 4, "2000"); }

        rc = _encodeTime(udunits, timeStrings, timesD);
        if (rc < 0) return (rc);

        for (int i = 0; i < timesD.size(); i++) { _times.push_back((float)timesD[i]); }
    }

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

bool DerivedCoordVar_WRFTime::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_WRFTime::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_WRFTime::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    return (0);
}

int DerivedCoordVar_WRFTime::OpenVariableRead(size_t ts, int level, int lod)
{
    ts = ts < _times.size() ? ts : _times.size() - 1;

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

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

int DerivedCoordVar_WRFTime::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 0);
    VAssert(max.size() == 0);

    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    size_t ts = f->GetTS();

    *region = _times[ts];

    return (0);
}

bool DerivedCoordVar_WRFTime::VariableExists(size_t ts, int reflevel, int lod) const { return (ts < _times.size()); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_TimeInSeconds
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_TimeInSeconds::DerivedCoordVar_TimeInSeconds(string derivedVarName, DC *dc, string nativeTimeVar, string dimName) : DerivedCoordVar(derivedVarName)
{
    _dc = dc;
    _times.clear();
    _nativeTimeVar = nativeTimeVar;

    string units = "seconds";
    int    axis = 3;
    _coordVarInfo = DC::CoordVar(_derivedVarName, units, DC::XType::FLOAT, vector<bool>(), axis, false, vector<string>(), dimName);
}

int DerivedCoordVar_TimeInSeconds::Initialize()
{
    // Use UDUnits for unit conversion
    //
    UDUnits udunits;
    int     rc = udunits.Initialize();
    if (rc < 0) {
        SetErrMsg("Failed to initialize udunits2 library : %s", udunits.GetErrMsg().c_str());
        return (-1);
    }

    DC::CoordVar cvar;
    bool         status = _dc->GetCoordVarInfo(_nativeTimeVar, cvar);
    if (!status) {
        SetErrMsg("Invalid coordinate variable %s", _nativeTimeVar.c_str());
        return (-1);
    }

    if (!udunits.IsTimeUnit(cvar.GetUnits())) {
        SetErrMsg("Invalid coordinate variable %s", _nativeTimeVar.c_str());
        return (-1);
    }

    size_t numTS = _dc->GetNumTimeSteps(_nativeTimeVar);

    // Need a single precision and double precision buffer. Single for GetVar,
    // double for udunits.Convert :-(
    //
    float * buf = new float[numTS];
    double *dbuf = new double[2 * numTS];
    double *dbufptr1 = dbuf;
    double *dbufptr2 = dbuf + numTS;

    rc = _dc->GetVar(_nativeTimeVar, -1, -1, buf);
    if (rc < 0) {
        SetErrMsg("Can't read time variable");
        return (-1);
    }
    for (int i = 0; i < numTS; i++) { dbufptr1[i] = (double)buf[i]; }

    status = udunits.Convert(cvar.GetUnits(), "seconds", dbufptr1, dbufptr2, numTS);
    if (!status) {
        SetErrMsg("Invalid coordinate variable %s", _nativeTimeVar.c_str());
        return (-1);
    }

    _times.clear();
    for (int i = 0; i < numTS; i++) { _times.push_back(dbufptr2[i]); }
    delete[] buf;
    delete[] dbuf;

    return (0);
}

bool DerivedCoordVar_TimeInSeconds::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_TimeInSeconds::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_TimeInSeconds::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    return (0);
}

int DerivedCoordVar_TimeInSeconds::OpenVariableRead(size_t ts, int level, int lod)
{
    ts = ts < _times.size() ? ts : _times.size() - 1;

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_TimeInSeconds::CloseVariable(int fd)
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

int DerivedCoordVar_TimeInSeconds::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 0);
    VAssert(max.size() == 0);

    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    size_t ts = f->GetTS();

    *region = _times[ts];

    return (0);
}

bool DerivedCoordVar_TimeInSeconds::VariableExists(size_t ts, int reflevel, int lod) const { return (ts < _times.size()); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_Time
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_Time::DerivedCoordVar_Time(string derivedVarName, string dimName, size_t n) : DerivedCoordVar(derivedVarName)
{
    _times.clear();
    for (size_t i = 0; i < n; i++) _times.push_back((float)i);

    string units = "seconds";
    int    axis = 3;
    _coordVarInfo = DC::CoordVar(_derivedVarName, units, DC::XType::FLOAT, vector<bool>(), axis, false, vector<string>(), dimName);
}

int DerivedCoordVar_Time::Initialize() { return (0); }

bool DerivedCoordVar_Time::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_Time::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_Time::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    return (0);
}

int DerivedCoordVar_Time::OpenVariableRead(size_t ts, int level, int lod)
{
    ts = ts < _times.size() ? ts : _times.size() - 1;

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_Time::CloseVariable(int fd)
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

int DerivedCoordVar_Time::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    VAssert(min.size() == 0);
    VAssert(max.size() == 0);

    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor: %d", fd);
        return (-1);
    }

    size_t ts = f->GetTS();

    *region = _times[ts];

    return (0);
}

bool DerivedCoordVar_Time::VariableExists(size_t ts, int reflevel, int lod) const { return (ts < _times.size()); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_Staggered
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_Staggered::DerivedCoordVar_Staggered(string derivedVarName, string stagDimName, DC *dc, string inName, string dimName) : DerivedCoordVar(derivedVarName)
{
    _stagDimName = stagDimName;
    _inName = inName;
    _dimName = dimName;
    _dc = dc;
}

int DerivedCoordVar_Staggered::Initialize()
{
    bool ok = _dc->GetCoordVarInfo(_inName, _coordVarInfo);
    if (!ok) return (-1);

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

bool DerivedCoordVar_Staggered::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_Staggered::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_Staggered::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    vector<size_t> dummy;
    int            rc = _dc->GetDimLensAtLevel(_inName, level, dims_at_level, dummy, -1);
    if (rc < 0) return (-1);

    dims_at_level[_stagDim] += 1;
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVar_Staggered::OpenVariableRead(size_t ts, int level, int lod)
{
    int fd = _dc->OpenVariableRead(ts, _inName, level, lod);
    if (fd < 0) return (fd);

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod, fd);

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

    _fileTable.RemoveEntry(fd);
    delete f;

    return (rc);
}

int DerivedCoordVar_Staggered::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    vector<size_t> dims, dummy;
    int            rc = GetDimLensAtLevel(f->GetLevel(), dims, dummy);
    if (rc < 0) return (-1);

    vector<size_t> inMin = min;
    vector<size_t> inMax = max;

    // adjust coords for native data so that we have what we
    // need for interpolation or extrapolation.
    //

    // Adjust min max boundaries to handle 4 case (below) where X's
    // are samples on the destination grid (staggered) and O's are samples
    // on the source grid (unstaggered) and the numbers represent
    // the address of the samples in their respective arrays
    //
    //	X O X O X O X
    //	0 0 1 1 2 2 3
    //
    //	  O X O X O X
    //	  0 1 1 2 2 3
    //
    //	X O X O X O
    //	0 0 1 1 2 2
    //
    //	  O X O X O
    //	  0 1 1 2 2

    // Adjust input min so we can interpolate interior.
    //
    if (min[_stagDim] > 0) { inMin[_stagDim] -= 1; }

    // input dimensions are one less then output
    //
    if (max[_stagDim] >= (dims[_stagDim] - 1)) { inMax[_stagDim] -= 1; }

    // Adjust min and max for edge cases
    //
    if (max[_stagDim] == 0 && (dims[_stagDim] - 1) > 1) { inMax[_stagDim] += 1; }
    if (min[_stagDim] == dims[_stagDim] - 1 && min[_stagDim] > 0) { inMin[_stagDim] -= 1; }

    vector<size_t> inDims, outDims;
    for (size_t i = 0; i < min.size(); i++) {
        inDims.push_back(inMax[i] - inMin[i] + 1);
        outDims.push_back(max[i] - min[i] + 1);
    }
    size_t sz = std::max(vproduct(outDims), vproduct(inDims));

    float *buf = new float[sz];

    // Read unstaggered data
    //
    rc = _dc->ReadRegion(f->GetAux(), inMin, inMax, buf);
    if (rc < 0) return (-1);

    resampleToStaggered(buf, inMin, inMax, region, min, max, _stagDim);

    delete[] buf;

    return (0);
}

bool DerivedCoordVar_Staggered::VariableExists(size_t ts, int reflevel, int lod) const { return (_dc->VariableExists(ts, _inName, reflevel, lod)); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVar_UnStaggered
//
//////////////////////////////////////////////////////////////////////////////

DerivedCoordVar_UnStaggered::DerivedCoordVar_UnStaggered(string derivedVarName, string unstagDimName, DC *dc, string inName, string dimName) : DerivedCoordVar(derivedVarName)
{
    _unstagDimName = unstagDimName;
    _inName = inName;
    _dimName = dimName;
    _dc = dc;
}

int DerivedCoordVar_UnStaggered::Initialize()
{
    bool ok = _dc->GetCoordVarInfo(_inName, _coordVarInfo);
    if (!ok) return (-1);

    vector<string> dimNames = _coordVarInfo.GetDimNames();
    _stagDim = -1;
    for (int i = 0; i < dimNames.size(); i++) {
        if (dimNames[i] == _dimName) {
            _stagDim = i;
            dimNames[i] = _unstagDimName;
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

bool DerivedCoordVar_UnStaggered::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVar_UnStaggered::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVar_UnStaggered::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    int rc = _dc->GetDimLensAtLevel(_inName, level, dims_at_level, bs_at_level, -1);
    if (rc < 0) return (-1);

    dims_at_level[_stagDim] -= 1;
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVar_UnStaggered::OpenVariableRead(size_t ts, int level, int lod)
{
    int fd = _dc->OpenVariableRead(ts, _inName, level, lod);
    if (fd < 0) return (fd);

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod, fd);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVar_UnStaggered::CloseVariable(int fd)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int rc = _dc->CloseVariable(f->GetAux());

    _fileTable.RemoveEntry(fd);
    delete f;

    return (rc);
}

int DerivedCoordVar_UnStaggered::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    vector<size_t> dims, dummy;
    int            rc = GetDimLensAtLevel(f->GetLevel(), dims, dummy);
    if (rc < 0) return (-1);

    vector<size_t> inMin = min;
    vector<size_t> inMax = max;

    // adjust coords for native data so that we have what we
    // need for interpolation .
    //
    inMax[_stagDim] += 1;

    vector<size_t> inDims, outDims;
    for (size_t i = 0; i < min.size(); i++) {
        inDims.push_back(inMax[i] - inMin[i] + 1);
        outDims.push_back(max[i] - min[i] + 1);
    }
    size_t sz = std::max(vproduct(outDims), vproduct(inDims));

    float *buf = new float[sz];

    // Read staggered data
    //
    rc = _dc->ReadRegion(f->GetAux(), inMin, inMax, buf);
    if (rc < 0) return (-1);

    resampleToUnStaggered(buf, inMin, inMax, region, min, max, _stagDim);

    delete[] buf;

    return (0);
}

bool DerivedCoordVar_UnStaggered::VariableExists(size_t ts, int reflevel, int lod) const { return (_dc->VariableExists(ts, _inName, reflevel, lod)); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCFVertCoordVar
//
//////////////////////////////////////////////////////////////////////////////

bool DerivedCFVertCoordVar::ParseFormula(string formula_terms, map<string, string> &parsed_terms)
{
    parsed_terms.clear();

    // Remove ":" to ease parsing. It's superflous
    //
    replace(formula_terms.begin(), formula_terms.end(), ':', ' ');

    string       buf;                  // Have a buffer string
    stringstream ss(formula_terms);    // Insert the string into a stream

    vector<string> tokens;    // Create vector to hold our words

    while (ss >> buf) { tokens.push_back(buf); }

    if (tokens.size() % 2) return (false);

    for (int i = 0; i < tokens.size(); i += 2) {
        parsed_terms[tokens[i]] = tokens[i + 1];
        if (parsed_terms[tokens[i]].empty()) return (false);
    }
    return (true);
}

bool DerivedCFVertCoordVar::ValidFormula(const vector<string> &required_terms, string formula)
{
    map<string, string> formulaMap;
    if (!ParseFormula(formula, formulaMap)) { return (false); }

    for (int i = 0; i < required_terms.size(); i++) {
        map<string, string>::const_iterator itr;
        itr = formulaMap.find(required_terms[i]);
        if (itr == formulaMap.end()) return (false);
    }
    return (true);
}

//////////////////////////////////////////////////////////////////////////
//
// DerivedCFVertCoordVarFactory Class
//
/////////////////////////////////////////////////////////////////////////

DerivedCFVertCoordVar *DerivedCFVertCoordVarFactory::CreateInstance(string standard_name, DC *dc, string mesh, string formula)
{
    DerivedCFVertCoordVar *instance = NULL;

    // find standard_name in the registry and call factory method.
    //
    auto it = _factoryFunctionRegistry.find(standard_name);
    if (it != _factoryFunctionRegistry.end()) { instance = it->second(dc, mesh, formula); }

    return instance;
}

vector<string> DerivedCFVertCoordVarFactory::GetFactoryNames() const
{
    vector<string>                                                                       names;
    map<string, function<DerivedCFVertCoordVar *(DC *, string, string)>>::const_iterator itr;

    for (const auto &itr : _factoryFunctionRegistry) { names.push_back(itr.first); }
    return (names);
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVarStandardWRF_Terrain
//
//////////////////////////////////////////////////////////////////////////////

static DerivedCFVertCoordVarFactoryRegistrar<DerivedCoordVarStandardWRF_Terrain> registrar_wrf_terrain("wrf_terrain");

DerivedCoordVarStandardWRF_Terrain::DerivedCoordVarStandardWRF_Terrain(DC *dc, string mesh, string formula) : DerivedCFVertCoordVar("", dc, mesh, formula)
{
    _PHVar.clear();
    _PHBVar.clear();
    _grav = 9.80665;
}

int DerivedCoordVarStandardWRF_Terrain::Initialize()
{
    map<string, string> formulaMap;
    if (!ParseFormula(_formula, formulaMap)) {
        SetErrMsg("Invalid conversion formula \"%s\"", _formula.c_str());
        return (-1);
    }

    map<string, string>::const_iterator itr;
    itr = formulaMap.find("PH");
    if (itr != formulaMap.end()) { _PHVar = itr->second; }

    itr = formulaMap.find("PHB");
    if (itr != formulaMap.end()) { _PHBVar = itr->second; }

    if (_PHVar.empty() || _PHBVar.empty()) {
        SetErrMsg("Invalid conversion formula \"%s\"", _formula.c_str());
        return (-1);
    }

    DC::DataVar dvarInfo;
    bool        status = _dc->GetDataVarInfo(_PHVar, dvarInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _PHVar.c_str());
        return (-1);
    }

    string timeCoordVar = dvarInfo.GetTimeCoordVar();
    string timeDimName;
    if (!timeCoordVar.empty()) {
        DC::CoordVar cvarInfo;
        bool         status = _dc->GetCoordVarInfo(timeCoordVar, cvarInfo);
        if (!status) {
            SetErrMsg("Invalid variable \"%s\"", timeCoordVar.c_str());
            return (-1);
        }
        timeDimName = cvarInfo.GetTimeDimName();
    }

    DC::Mesh m;
    status = _dc->GetMesh(_mesh, m);
    if (!status) {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }

    // Elevation variable
    //
    vector<string> dimnames = m.GetDimNames();
    VAssert(dimnames.size() == 3);
    if (dimnames[0] == "west_east" && dimnames[1] == "south_north" && dimnames[2] == "bottom_top") {
        _derivedVarName = "Elevation";
    } else if (dimnames[0] == "west_east_stag" && dimnames[1] == "south_north" && dimnames[2] == "bottom_top") {
        _derivedVarName = "ElevationU";
    } else if (dimnames[0] == "west_east" && dimnames[1] == "south_north_stag" && dimnames[2] == "bottom_top") {
        _derivedVarName = "ElevationV";
    } else if (dimnames[0] == "west_east" && dimnames[1] == "south_north" && dimnames[2] == "bottom_top_stag") {
        _derivedVarName = "ElevationW";
    } else {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }

    _coordVarInfo = DC::CoordVar(_derivedVarName, "m", DC::XType::FLOAT, dvarInfo.GetWName(), dvarInfo.GetCRatios(), vector<bool>(3, false), dimnames, timeDimName, 2, false);

    return (0);
}

bool DerivedCoordVarStandardWRF_Terrain::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVarStandardWRF_Terrain::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

int DerivedCoordVarStandardWRF_Terrain::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    vector<size_t> dims, bs;
    int            rc = _dc->GetDimLensAtLevel(_PHVar, -1, dims, bs, -1);
    if (rc < 0) return (-1);

    if (_derivedVarName == "Elevation") {
        if (dims[2] > 1) dims[2]--;
    } else if (_derivedVarName == "ElevationU") {
        dims[0]++;
        if (dims[2] > 1) dims[2]--;
    } else if (_derivedVarName == "ElevationV") {
        dims[1]++;
        if (dims[2] > 1) dims[2]--;
    } else if (_derivedVarName == "ElevationW") {
    } else {
        SetErrMsg("Invalid variable name: %s", _derivedVarName.c_str());
        return (-1);
    }

    int nlevels = _dc->GetNumRefLevels(_PHVar);
    if (level < 0) level = nlevels + level;

    WASP::InqDimsAtLevel(_coordVarInfo.GetWName(), level, dims, bs, dims_at_level, bs_at_level);

    // No blocking
    //
    //	bs_at_level = vector <size_t> (dims_at_level.size(), 1);

    return (0);
}

int DerivedCoordVarStandardWRF_Terrain::OpenVariableRead(size_t ts, int level, int lod)
{
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVarStandardWRF_Terrain::CloseVariable(int fd)
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

int DerivedCoordVarStandardWRF_Terrain::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    string varname = f->GetVarname();

    // Dimensions of "W" grid: PH and PHB variables are sampled on the
    // same grid as the W component of velocity
    //
    vector<size_t> wDims, dummy;
    int            rc = _dc->GetDimLensAtLevel(_PHVar, f->GetLevel(), wDims, dummy, -1);
    if (rc < 0) return (-1);

    vector<size_t> myDims;
    rc = DerivedCoordVarStandardWRF_Terrain::GetDimLensAtLevel(f->GetLevel(), myDims, dummy);
    if (rc < 0) return (-1);

    // coordinates of "W" grid.
    //
    vector<size_t> wMin = min;
    vector<size_t> wMax = max;

    // coordinates of base (Elevation) grid.
    //
    if (varname == "Elevation") {
        // In general myDims[2] != wDims[2]. However, for multiresolution
        // data the two can be equal
        //
        if (myDims[2] != wDims[2]) { wMax[2] += 1; }

    } else if (varname == "ElevationU") {
        if (myDims[2] != wDims[2]) { wMax[2] += 1; }

        if (min[0] > 0) { wMin[0] -= 1; }
        if (max[0] >= (wDims[0] - 1)) { wMax[0] -= 1; }
    } else if (varname == "ElevationV") {
        if (myDims[2] != wDims[2]) { wMax[2] += 1; }

        if (min[1] > 0) { wMin[1] -= 1; }
        if (max[1] >= (wDims[1] - 1)) { wMax[1] -= 1; }
    }

    // Base grid dimensions
    //
    vector<size_t> bMin = wMin;
    vector<size_t> bMax = wMax;
    if (myDims[2] != wDims[2]) { bMax[2] -= 1; }

    size_t nElements = std::max(numElements(wMin, wMax), numElements(min, max));

    vector<float> buf1(nElements);
    rc = _getVar(_dc, f->GetTS(), _PHVar, f->GetLevel(), f->GetLOD(), wMin, wMax, buf1.data());
    if (rc < 0) { return (rc); }

    vector<float> buf2(nElements);
    rc = _getVar(_dc, f->GetTS(), _PHBVar, f->GetLevel(), f->GetLOD(), wMin, wMax, buf2.data());
    if (rc < 0) { return (rc); }

    float *dst = region;
    if (varname != "ElevationW" && wDims[2] > 1) { dst = buf1.data(); }

    // Compute elevation on the W grid
    //
    for (size_t i = 0; i < nElements; i++) { dst[i] = (buf1.data()[i] + buf2.data()[i]) / _grav; }

    if (wDims[2] < 2) return (0);

    // Elevation is correct for W grid. If we want Elevation, ElevationU, or
    // Elevation V grid we need to interpolate
    //

    if (varname == "Elevation") {
        // Resample stagged W grid to base grid
        //
        resampleToUnStaggered(buf1.data(), wMin, wMax, region, min, max, 2);
    } else if (varname == "ElevationU") {
        // Resample stagged W grid to base grid
        //
        resampleToUnStaggered(buf1.data(), wMin, wMax, buf2.data(), bMin, bMax, 2);

        resampleToStaggered(buf2.data(), bMin, bMax, region, min, max, 0);
    } else if (varname == "ElevationV") {
        // Resample stagged W grid to base grid
        //
        resampleToUnStaggered(buf1.data(), wMin, wMax, buf2.data(), bMin, bMax, 2);

        resampleToStaggered(buf2.data(), bMin, bMax, region, min, max, 1);
    }

    return (0);
}

bool DerivedCoordVarStandardWRF_Terrain::VariableExists(size_t ts, int reflevel, int lod) const
{
    return (_dc->VariableExists(ts, _PHVar, reflevel, lod) && _dc->VariableExists(ts, _PHBVar, reflevel, lod));
}

bool DerivedCoordVarStandardWRF_Terrain::ValidFormula(string formula) { return (DerivedCFVertCoordVar::ValidFormula(vector<string>{"PH", "PHB"}, formula)); }

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVarStandardOceanSCoordinate
//
//////////////////////////////////////////////////////////////////////////////

//
// Register class with object factory!!!
//
static DerivedCFVertCoordVarFactoryRegistrar<DerivedCoordVarStandardOceanSCoordinate> registrar_ocean_s_coordinate_g1("ocean_s_coordinate_g1");

static DerivedCFVertCoordVarFactoryRegistrar<DerivedCoordVarStandardOceanSCoordinate> registrar_ocean_s_coordinate_g2("ocean_s_coordinate_g2");

DerivedCoordVarStandardOceanSCoordinate::DerivedCoordVarStandardOceanSCoordinate(DC *dc, string mesh, string formula) : DerivedCFVertCoordVar("", dc, mesh, formula)
{
    _standard_name = "ocean_s_coordinate_g1";
    _sVar.clear();
    _CVar.clear();
    _etaVar.clear();
    _depthVar.clear();
    _depth_cVar.clear();

    _CVarMV = std::numeric_limits<double>::infinity();
    _etaVarMV = std::numeric_limits<double>::infinity();
    _depthVarMV = std::numeric_limits<double>::infinity();

    _destaggerEtaXDim = false;
    _destaggerEtaYDim = false;
    _destaggerDepthXDim = false;
    _destaggerDepthYDim = false;
}

int DerivedCoordVarStandardOceanSCoordinate::initialize_missing_values()
{
    DC::DataVar dataInfo;
    bool        status = _dc->GetDataVarInfo(_CVar, dataInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _CVar.c_str());
        return (-1);
    }
    if (dataInfo.GetHasMissing()) _CVarMV = dataInfo.GetMissingValue();

    status = _dc->GetDataVarInfo(_etaVar, dataInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _etaVar.c_str());
        return (-1);
    }
    if (dataInfo.GetHasMissing()) _etaVarMV = dataInfo.GetMissingValue();

    status = _dc->GetDataVarInfo(_depthVar, dataInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _depthVar.c_str());
        return (-1);
    }
    if (dataInfo.GetHasMissing()) _depthVarMV = dataInfo.GetMissingValue();

    return (0);
}

int DerivedCoordVarStandardOceanSCoordinate::initialize_stagger_flags()
{
    vector<size_t> derivedDims;
    bool           status = _dc->GetMeshDimLens(_mesh, derivedDims);
    if (!status) {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }

    vector<size_t> nativeDims;
    int            rc = _dc->GetDimLens(_etaVar, nativeDims);
    if (rc < 0) return (-1);

    if (nativeDims[0] == derivedDims[0] - 1) _destaggerEtaXDim = true;
    if (nativeDims[1] == derivedDims[1] - 1) _destaggerEtaYDim = true;

    rc = _dc->GetDimLens(_depthVar, nativeDims);
    if (rc < 0) return (-1);

    if (nativeDims[0] == derivedDims[0] - 1) _destaggerDepthXDim = true;
    if (nativeDims[1] == derivedDims[1] - 1) _destaggerDepthYDim = true;

    return (0);
}

int DerivedCoordVarStandardOceanSCoordinate::Initialize()
{
    map<string, string> formulaMap;
    if (!ParseFormula(_formula, formulaMap)) {
        SetErrMsg("Invalid conversion formula \"%s\"", _formula.c_str());
        return (-1);
    }

    map<string, string>::const_iterator itr;
    VAssert((itr = formulaMap.find("s")) != formulaMap.end());
    _sVar = itr->second;

    VAssert((itr = formulaMap.find("C")) != formulaMap.end());
    _CVar = itr->second;

    VAssert((itr = formulaMap.find("eta")) != formulaMap.end());
    _etaVar = itr->second;

    VAssert((itr = formulaMap.find("depth")) != formulaMap.end());
    _depthVar = itr->second;

    VAssert((itr = formulaMap.find("depth_c")) != formulaMap.end());
    _depth_cVar = itr->second;

    if (initialize_missing_values() < 0) return (-1);

    if (initialize_stagger_flags() < 0) return (-1);

    // Figure out if this is a Ocean s-coordinate, generic form 1, or
    // Ocean s-coordinate, generic form 2
    //
    DC::CoordVar sInfo;
    bool         status = _dc->GetCoordVarInfo(_sVar, sInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _sVar.c_str());
        return (-1);
    }

    DC::Attribute attr_name;
    if (!sInfo.GetAttribute("standard_name", attr_name)) {
        // Default to generic form 1
        //
        _standard_name = "ocean_s_coordinate_g1";
    } else {
        attr_name.GetValues(_standard_name);
    }

    // Use the eta variable to set up metadata for the derived variable
    //
    DC::DataVar etaInfo;
    status = _dc->GetDataVarInfo(_etaVar, etaInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _etaVar.c_str());
        return (-1);
    }

    string timeCoordVar = etaInfo.GetTimeCoordVar();
    string timeDimName;
    if (!timeCoordVar.empty()) {
        DC::CoordVar cvarInfo;
        bool         status = _dc->GetCoordVarInfo(timeCoordVar, cvarInfo);
        if (!status) {
            SetErrMsg("Invalid variable \"%s\"", timeCoordVar.c_str());
            return (-1);
        }
        timeDimName = cvarInfo.GetTimeDimName();
    }

    _derivedVarName = "Z_" + _sVar;

    vector<string> dimnames;
    status = _dc->GetMeshDimNames(_mesh, dimnames);
    if (!status) {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }
    VAssert(dimnames.size() == 3);

    // We're deriving a 3D varible from 1D and 2D varibles. We arbitarily
    // use one of the 2D variables to configure metadata such as the
    // available compression ratios
    //
    _coordVarInfo = DC::CoordVar(_derivedVarName, "m", DC::XType::FLOAT, vector<bool>(3, false), 2, false, dimnames, timeDimName);

    return (0);
}

bool DerivedCoordVarStandardOceanSCoordinate::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVarStandardOceanSCoordinate::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

vector<string> DerivedCoordVarStandardOceanSCoordinate::GetInputs() const
{
    map<string, string> formulaMap;
    bool                ok = ParseFormula(_formula, formulaMap);
    VAssert(ok);

    vector<string> inputs;
    for (auto it = formulaMap.begin(); it != formulaMap.end(); ++it) { inputs.push_back(it->second); }
    return (inputs);
}

int DerivedCoordVarStandardOceanSCoordinate::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    vector<size_t> dims2d, bs2d;
    int            rc = _dc->GetDimLensAtLevel(_etaVar, -1, dims2d, bs2d);
    if (rc < 0) return (-1);

    vector<size_t> dims1d, bs1d;
    rc = _dc->GetDimLensAtLevel(_sVar, -1, dims1d, bs1d);
    if (rc < 0) return (-1);

    dims_at_level = {dims2d[0], dims2d[1], dims1d[0]};
    bs_at_level = {bs2d[0], bs2d[1], bs1d[0]};

    return (0);
}

int DerivedCoordVarStandardOceanSCoordinate::OpenVariableRead(size_t ts, int level, int lod)
{
    // Compression not supported
    //
    level = -1;
    lod = -1;
    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVarStandardOceanSCoordinate::CloseVariable(int fd)
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

int DerivedCoordVarStandardOceanSCoordinate::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    string varname = f->GetVarname();

    vector<size_t> dims, dummy;
    int            rc = GetDimLensAtLevel(f->GetLevel(), dims, dummy);
    if (rc < 0) return (-1);

    vector<float>  s(dims[2]);
    vector<size_t> myMin = {min[2]};
    vector<size_t> myMax = {max[2]};
    rc = _getVar(_dc, f->GetTS(), _sVar, f->GetLevel(), f->GetLOD(), myMin, myMax, s.data());
    if (rc < 0) return (rc);

    vector<float> C(dims[2]);
    myMin = {min[2]};
    myMax = {max[2]};
    rc = _getVar(_dc, f->GetTS(), _CVar, f->GetLevel(), f->GetLOD(), myMin, myMax, C.data());
    if (rc < 0) return (rc);

    vector<float> eta(dims[0] * dims[1]);
    myMin = {min[0], min[1]};
    myMax = {max[0], max[1]};
    if (_destaggerEtaXDim) {
        rc = _getVarDestagger(_dc, f->GetTS(), _etaVar, f->GetLevel(), f->GetLOD(), myMin, myMax, eta.data(), 0);
    } else if (_destaggerEtaYDim) {
        rc = _getVarDestagger(_dc, f->GetTS(), _etaVar, f->GetLevel(), f->GetLOD(), myMin, myMax, eta.data(), 1);
    } else {
        rc = _getVar(_dc, f->GetTS(), _etaVar, f->GetLevel(), f->GetLOD(), myMin, myMax, eta.data());
    }
    if (rc < 0) return (rc);

    vector<float> depth(dims[0] * dims[1]);
    myMin = {min[0], min[1]};
    myMax = {max[0], max[1]};
    if (_destaggerDepthXDim) {
        rc = _getVarDestagger(_dc, f->GetTS(), _depthVar, f->GetLevel(), f->GetLOD(), myMin, myMax, depth.data(), 0);
    } else if (_destaggerDepthYDim) {
        rc = _getVarDestagger(_dc, f->GetTS(), _depthVar, f->GetLevel(), f->GetLOD(), myMin, myMax, depth.data(), 1);
    } else {
        rc = _getVar(_dc, f->GetTS(), _depthVar, f->GetLevel(), f->GetLOD(), myMin, myMax, depth.data());
    }

    float depth_c;
    myMin = {};
    myMax = {};
    rc = _getVar(_dc, f->GetTS(), _depth_cVar, f->GetLevel(), f->GetLOD(), myMin, myMax, &depth_c);

    if (_standard_name == "ocean_s_coordinate_g1") {
        compute_g1(min, max, s.data(), C.data(), eta.data(), depth.data(), depth_c, region);
    } else {
        compute_g2(min, max, s.data(), C.data(), eta.data(), depth.data(), depth_c, region);
    }

    return (0);
}

bool DerivedCoordVarStandardOceanSCoordinate::VariableExists(size_t ts, int reflevel, int lod) const
{
    return (_dc->VariableExists(ts, _sVar, reflevel, lod) && _dc->VariableExists(ts, _CVar, reflevel, lod) && _dc->VariableExists(ts, _etaVar, reflevel, lod)
            && _dc->VariableExists(ts, _depthVar, reflevel, lod) && _dc->VariableExists(ts, _depth_cVar, reflevel, lod));
}

bool DerivedCoordVarStandardOceanSCoordinate::ValidFormula(string formula) { return (DerivedCFVertCoordVar::ValidFormula(vector<string>{"s", "C", "eta", "depth", "depth_c"}, formula)); }

void DerivedCoordVarStandardOceanSCoordinate::compute_g1(const vector<size_t> &min, const vector<size_t> &max, const float *s, const float *C, const float *eta, const float *depth, float depth_c,
                                                         float *region) const
{
    vector<size_t> rDims;
    for (int i = 0; i < 3; i++) { rDims.push_back(max[i] - min[i] + 1); }

    for (size_t k = 0; k < max[2] - min[2] + 1; k++) {
        for (size_t j = 0; j < max[1] - min[1] + 1; j++) {
            for (size_t i = 0; i < max[0] - min[0] + 1; i++) {
                if (depth[j * rDims[0]] == 0.0) {
                    region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = 0.0;
                    continue;
                }

                // We are deriving coordinate values from data values, so missing
                // values may be present
                //
                if (C[k] == _CVarMV || eta[j * rDims[0] + i] == _etaVarMV || depth[j * rDims[0] + i] == _depthVarMV) {
                    region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = 0.0;
                    continue;
                }

                float tmp = depth_c * s[k] + (depth[j * rDims[0] + i] - depth_c) * C[k];

                region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = tmp + eta[j * rDims[0] + i] * (1 + tmp / depth[j * rDims[0] + i]);
            }
        }
    }
}

void DerivedCoordVarStandardOceanSCoordinate::compute_g2(const vector<size_t> &min, const vector<size_t> &max, const float *s, const float *C, const float *eta, const float *depth, float depth_c,
                                                         float *region) const
{
    vector<size_t> rDims;
    for (int i = 0; i < 3; i++) { rDims.push_back(max[i] - min[i] + 1); }

    for (size_t k = 0; k < max[2] - min[2] + 1; k++) {
        for (size_t j = 0; j < max[1] - min[1] + 1; j++) {
            for (size_t i = 0; i < max[0] - min[0] + 1; i++) {
                if ((depth_c + depth[j * rDims[0]]) == 0.0) {
                    region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = 0.0;
                    continue;
                }

                // We are deriving coordinate values from data values, so missing
                // values may be present
                //
                if (C[k] == _CVarMV || eta[j * rDims[0] + i] == _etaVarMV || depth[j * rDims[0] + i] == _depthVarMV) {
                    region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = 0.0;
                    continue;
                }

                float tmp = (depth_c * s[k] + depth[j * rDims[0] + i] * C[k]) / (depth_c + depth[j * rDims[0] + i]);

                region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = eta[j * rDims[0] + i] + (eta[j * rDims[0] + i] + depth[j * rDims[0] + i]) * tmp;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
//	DerivedCoordVarStandardAHSPC
//
//	Atmosphere Hybrid Sigma Pressure Coordinate
//
//////////////////////////////////////////////////////////////////////////////

//
// Register class with object factory!!!
//

static DerivedCFVertCoordVarFactoryRegistrar<DerivedCoordVarStandardAHSPC> registrar_atmosphere_hybrid_sigma_pressure_coordinate("atmosphere_hybrid_sigma_pressure_coordinate");

DerivedCoordVarStandardAHSPC::DerivedCoordVarStandardAHSPC(DC *dc, string mesh, string formula) : DerivedCFVertCoordVar("", dc, mesh, formula)
{
    _standard_name = "atmosphere_hybrid_sigma_pressure_coordinate";
    _aVar.clear();
    _apVar.clear();
    _bVar.clear();
    _p0Var.clear();
    _psVar.clear();

    _psVarMV = std::numeric_limits<double>::infinity();
}

int DerivedCoordVarStandardAHSPC::initialize_missing_values()
{
    DC::DataVar dataInfo;
    bool        status = _dc->GetDataVarInfo(_psVar, dataInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _psVar.c_str());
        return (-1);
    }
    if (dataInfo.GetHasMissing()) _psVarMV = dataInfo.GetMissingValue();

    return (0);
}

int DerivedCoordVarStandardAHSPC::Initialize()
{
    map<string, string> formulaMap;
    if (!ParseFormula(_formula, formulaMap)) {
        SetErrMsg("Invalid conversion formula \"%s\"", _formula.c_str());
        return (-1);
    }

    // There are two possible formulations, one with 'ap', and one
    // with 'a' and 'p0'
    //
    map<string, string>::const_iterator itr;
    VAssert((itr = formulaMap.find("a")) != formulaMap.end());
    if (itr != formulaMap.end()) {
        _aVar = itr->second;
        VAssert((itr = formulaMap.find("p0")) != formulaMap.end());
        _p0Var = itr->second;
    } else {
        VAssert((itr = formulaMap.find("ap")) != formulaMap.end());
        _apVar = itr->second;
    }

    VAssert((itr = formulaMap.find("b")) != formulaMap.end());
    _bVar = itr->second;

    VAssert((itr = formulaMap.find("ps")) != formulaMap.end());
    _psVar = itr->second;

    if (initialize_missing_values() < 0) return (-1);

    // Use the 'b' and 'ps' variables to set up metadata for the derived
    // variable
    //
    DC::DataVar bInfo;
    bool        status = _dc->GetDataVarInfo(_bVar, bInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _bVar.c_str());
        return (-1);
    }

    DC::DataVar psInfo;
    status = _dc->GetDataVarInfo(_psVar, psInfo);
    if (!status) {
        SetErrMsg("Invalid variable \"%s\"", _psVar.c_str());
        return (-1);
    }

    // Construct spatial and temporal dimensions from the 1D 'b'
    // variable and 2D, time-varying 'ps' variable
    //
    DC::Mesh m;
    status = _dc->GetMesh(psInfo.GetMeshName(), m);
    if (!status) {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }
    vector<string> dimnames = m.GetDimNames();

    status = _dc->GetMesh(bInfo.GetMeshName(), m);
    if (!status) {
        SetErrMsg("Invalid mesh \"%s\"", _mesh.c_str());
        return (-1);
    }
    dimnames.push_back(m.GetDimNames()[0]);

    string timeCoordVar = psInfo.GetTimeCoordVar();
    string timeDimName;
    if (!timeCoordVar.empty()) {
        DC::CoordVar cvarInfo;
        bool         status = _dc->GetCoordVarInfo(timeCoordVar, cvarInfo);
        if (!status) {
            SetErrMsg("Invalid variable \"%s\"", timeCoordVar.c_str());
            return (-1);
        }
        timeDimName = cvarInfo.GetTimeDimName();
    }

    // We're deriving a 3D varible from 1D and 2D varibles. We arbitarily
    // use one of the 2D variables to configure metadata such as the
    // available compression ratios
    //
    _derivedVarName = "Z_" + _bVar;
    _coordVarInfo = DC::CoordVar(_derivedVarName, "m", DC::XType::FLOAT, vector<bool>(3, false), 2, false, dimnames, timeDimName);

    return (0);
}

bool DerivedCoordVarStandardAHSPC::GetBaseVarInfo(DC::BaseVar &var) const
{
    var = _coordVarInfo;
    return (true);
}

bool DerivedCoordVarStandardAHSPC::GetCoordVarInfo(DC::CoordVar &cvar) const
{
    cvar = _coordVarInfo;
    return (true);
}

vector<string> DerivedCoordVarStandardAHSPC::GetInputs() const
{
    map<string, string> formulaMap;
    bool                ok = ParseFormula(_formula, formulaMap);
    VAssert(ok);

    vector<string> inputs;
    for (auto it = formulaMap.begin(); it != formulaMap.end(); ++it) { inputs.push_back(it->second); }
    return (inputs);
}

int DerivedCoordVarStandardAHSPC::GetDimLensAtLevel(int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    vector<size_t> dims2d, bs2d;
    int            rc = _dc->GetDimLensAtLevel(_psVar, -1, dims2d, bs2d);
    if (rc < 0) return (-1);

    vector<size_t> dims1d, bs1d;
    rc = _dc->GetDimLensAtLevel(_bVar, -1, dims1d, bs1d);
    if (rc < 0) return (-1);

    dims_at_level = {dims2d[0], dims2d[1], dims1d[0]};
    bs_at_level = {bs2d[0], bs2d[1], bs1d[0]};

    return (0);
}

int DerivedCoordVarStandardAHSPC::OpenVariableRead(size_t ts, int level, int lod)
{
    // We don't support compressed data
    //
    level = -1;
    lod = -1;

    DC::FileTable::FileObject *f = new DC::FileTable::FileObject(ts, _derivedVarName, level, lod);

    return (_fileTable.AddEntry(f));
}

int DerivedCoordVarStandardAHSPC::CloseVariable(int fd)
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

int DerivedCoordVarStandardAHSPC::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region)
{
    DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    vector<size_t> dims, dummy;
    int            rc = GetDimLensAtLevel(f->GetLevel(), dims, dummy);
    if (rc < 0) return (-1);

    string         aVar = _aVar.empty() ? _apVar : _aVar;
    vector<float>  a(dims[2]);
    vector<size_t> myMin = {min[2]};
    vector<size_t> myMax = {max[2]};
    rc = _getVar(_dc, f->GetTS(), aVar, f->GetLevel(), f->GetLOD(), myMin, myMax, a.data());
    if (rc < 0) return (rc);

    vector<float> b(dims[2]);
    myMin = {min[2]};
    myMax = {max[2]};
    rc = _getVar(_dc, f->GetTS(), _bVar, f->GetLevel(), f->GetLOD(), myMin, myMax, b.data());
    if (rc < 0) return (rc);

    vector<float> ps(dims[0] * dims[1]);
    myMin = {min[0], min[1]};
    myMax = {max[0], max[1]};
    rc = _getVar(_dc, f->GetTS(), _psVar, f->GetLevel(), f->GetLOD(), myMin, myMax, ps.data());
    if (rc < 0) return (rc);

    float p0 = 1.0;
    if (!_aVar.empty()) {
        myMin = {};
        myMax = {};
        rc = _getVar(_dc, f->GetTS(), _p0Var, f->GetLevel(), f->GetLOD(), myMin, myMax, &p0);
    }

    compute_a(min, max, a.data(), b.data(), ps.data(), p0, region);

    return (0);
}

bool DerivedCoordVarStandardAHSPC::VariableExists(size_t ts, int reflevel, int lod) const
{
    if (!_aVar.empty()) {
        return (_dc->VariableExists(ts, _aVar, reflevel, lod) && _dc->VariableExists(ts, _bVar, reflevel, lod) && _dc->VariableExists(ts, _psVar, reflevel, lod));
    } else {
        return (_dc->VariableExists(ts, _apVar, reflevel, lod) && _dc->VariableExists(ts, _bVar, reflevel, lod) && _dc->VariableExists(ts, _psVar, reflevel, lod)
                && _dc->VariableExists(ts, _p0Var, reflevel, lod));
    }
}

bool DerivedCoordVarStandardAHSPC::ValidFormula(string formula)
{
    return (DerivedCFVertCoordVar::ValidFormula(vector<string>{"a", "b", "p0", "ps"}, formula) || DerivedCFVertCoordVar::ValidFormula(vector<string>{"ap", "b", "ps"}, formula));
}

void DerivedCoordVarStandardAHSPC::compute_a(const vector<size_t> &min, const vector<size_t> &max, const float *a, const float *b, const float *ps, float p0, float *region) const
{
    vector<size_t> rDims;
    for (int i = 0; i < 3; i++) { rDims.push_back(max[i] - min[i] + 1); }

    for (size_t k = 0; k < max[2] - min[2] + 1; k++) {
        for (size_t j = 0; j < max[1] - min[1] + 1; j++) {
            for (size_t i = 0; i < max[0] - min[0] + 1; i++) {
                // We are deriving coordinate values from data values, so missing
                // values may be present
                //
                float l_ps = ps[j * rDims[0] + i];
                if (l_ps == _psVarMV) l_ps = 0;

                float pressure = (a[k] * p0) + (b[k] * l_ps);

                // Convert from pressure to meters above the ground:
                // [1] "A Quick Derivation relating altitude to air pressure" from
                // Portland State Aerospace Society, Version 1.03, 12/22/2004.
                //
                region[k * rDims[0] * rDims[1] + j * rDims[0] + i] = 44331.5 - (4946.62 * std::pow(pressure, 0.190263));
            }
        }
    }
}
