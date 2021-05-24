#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <fstream>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
//#include <vapor/BOVCollection.h>
#include <vapor/DCBOV.h>
#include <vapor/DCUtils.h>

using namespace VAPoR;
using namespace std;

namespace {

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

DCBOV::DCBOV() : _bovCollection(nullptr)
{
    _dimsMap.clear();
    _coordVarsMap.clear();
    _dataVarsMap.clear();
    _meshMap.clear();
    _coordVarKeys.clear();
    _derivedVars.clear();
}

DCBOV::~DCBOV()
{
    for (int i = 0; i < _derivedVars.size(); i++) {
        if (_derivedVars[i]) delete _derivedVars[i];
    }
    _derivedVars.clear();

    if (_bovCollection != nullptr) { delete _bovCollection; }
}

int DCBOV::initialize(const vector<string> &paths, const std::vector<string> &options)
{
    _bovCollection = new BOVCollection();
    int rc = _bovCollection->Initialize(paths);
    if (rc < 0) {
        SetErrMsg("Failure reading .bov file");
        return (-1);
    }

    //
    //  Get the dimensions of the grid.
    //	Initializes members: _dimsMap
    //
    rc = _InitDimensions();
    if (rc < 0) {
        SetErrMsg("No valid dimensions");
        return (-1);
    }

    // Set up the coordinate variables
    //
    rc = _InitCoordinates();
    if (rc < 0) { return (-1); }

    // Identify data and coordinate variables. Sets up members:
    // Initializes members: _dataVarsMap, _meshMap
    //
    rc = _InitVars();
    if (rc < 0) return (-1);

    return (0);
}

int DCBOV::_InitDimensions()
{
    _dimsMap.clear();
    std::vector<std::string> dimnames = _bovCollection->GetSpatialDimensions();
    std::vector<size_t>      dimlens = _bovCollection->GetDataSize();
    VAssert(dimnames.size() == 3);
    VAssert(dimnames.size() == dimlens.size());

    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }

    string    timeDim = _bovCollection->GetTimeDimension();
    Dimension dim(timeDim, 1);
    _dimsMap[timeDim] = dim;

    return 0;
}

int DCBOV::_InitCoordinates()
{
    bool uniformHint = true;
    vector<bool> periodic(false);
    std::string units = "m";

    std::vector<std::string> dims = _bovCollection->GetSpatialDimensions();

    DC::Attribute unitAttribute("units", DC::TEXT, units);

    _coordVarsMap[dims[0]] = CoordVar(dims[0], units, DC::FLOAT, periodic, 0, uniformHint, {dims[0]}, "");
    _coordVarsMap[dims[0]].SetAttribute(DC::Attribute("axis", DC::TEXT, "X"));
    _coordVarsMap[dims[0]].SetAttribute(unitAttribute);

    _coordVarsMap[dims[1]] = CoordVar(dims[1], units, DC::FLOAT, periodic, 1, uniformHint, {dims[1]}, "");
    _coordVarsMap[dims[1]].SetAttribute(DC::Attribute("axis", DC::TEXT, "Y"));
    _coordVarsMap[dims[1]].SetAttribute(unitAttribute);

    _coordVarsMap[dims[2]] = CoordVar(dims[2], units, DC::FLOAT, periodic, 2, uniformHint, {dims[2]}, "");
    _coordVarsMap[dims[2]].SetAttribute(DC::Attribute("axis", DC::TEXT, "Z"));
    _coordVarsMap[dims[2]].SetAttribute(unitAttribute);

    // Is there something better to use than "seconds since 2000-1-1 0:0:0"?
    // Should it be stored in BOVCollection?
    std::string timeDim = _bovCollection->GetTimeDimension();
    _coordVarsMap[timeDim] = CoordVar(timeDim, "seconds since 2000-1-1 0:0:0", DC::FLOAT, periodic, 3, true, {}, timeDim);
    _coordVarsMap[timeDim].SetAttribute(DC::Attribute("units", DC::TEXT, "seconds since 2000-1-1 0:0:0"));
    _coordVarsMap[timeDim].SetAttribute(DC::Attribute("axis", DC::TEXT, "T"));

    return 0;
}

// Collect metadata for all data variables found in the CF data
// set. Initialize the _dataVarsMap member
//
int DCBOV::_InitVars()
{
    _dataVarsMap.clear();
    _meshMap.clear();

    vector<bool> periodic(3, false);

    std::vector<std::string> dimnames = _bovCollection->GetSpatialDimensions();
    std::string              var = _bovCollection->GetDataVariableName();
    DC::XType                format = _bovCollection->GetDataFormat();
    string                   time_dim_name = "";
    string                   time_coordvar = "";
    string                   units = "m";    // Should this be stored in BOVCollection?

    Mesh mesh(var, dimnames, dimnames);

    // Create new mesh. We're being lazy here and probably should only
    // createone if it doesn't ready exist
    //
    _meshMap[mesh.GetName()] = mesh;

    _dataVarsMap[var] = DataVar(var, units, format, periodic, mesh.GetName(), time_coordvar, DC::Mesh::NODE);
    _dataVarsMap[var].SetAttribute(DC::Attribute("standard_name", DC::TEXT, var));
    _dataVarsMap[var].SetAttribute(DC::Attribute("units", DC::TEXT, units));

    return (0);
}

bool DCBOV::getDimension(string dimname, DC::Dimension &dimension) const
{
    map<string, DC::Dimension>::const_iterator itr;

    itr = _dimsMap.find(dimname);
    if (itr == _dimsMap.end()) return (false);

    dimension = itr->second;
    return (true);
}

std::vector<string> DCBOV::getDimensionNames() const
{
    map<string, DC::Dimension>::const_iterator itr;

    vector<string> names;

    for (itr = _dimsMap.begin(); itr != _dimsMap.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

vector<string> DCBOV::getMeshNames() const
{
    vector<string>                         mesh_names;
    std::map<string, Mesh>::const_iterator itr = _meshMap.begin();
    for (; itr != _meshMap.end(); ++itr) { mesh_names.push_back(itr->first); }
    return (mesh_names);
}

bool DCBOV::getMesh(string mesh_name, DC::Mesh &mesh) const
{
    map<string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
    if (itr == _meshMap.end()) return (false);

    mesh = itr->second;
    return (true);
}

bool DCBOV::getCoordVarInfo(string varname, DC::CoordVar &cvar) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr == _coordVarsMap.end()) { return (false); }

    cvar = itr->second;
    return (true);
}

bool DCBOV::getDataVarInfo(string varname, DC::DataVar &datavar) const
{
    map<string, DC::DataVar>::const_iterator itr;

    itr = _dataVarsMap.find(varname);
    if (itr == _dataVarsMap.end()) { return (false); }

    datavar = itr->second;
    return (true);
}

bool DCBOV::getBaseVarInfo(string varname, DC::BaseVar &var) const
{
    map<string, DC::CoordVar>::const_iterator itr;

    itr = _coordVarsMap.find(varname);
    if (itr != _coordVarsMap.end()) {
        var = itr->second;
        return (true);
    }

    map<string, DC::DataVar>::const_iterator itr1 = _dataVarsMap.find(varname);
    if (itr1 != _dataVarsMap.end()) {
        var = itr1->second;
        return (true);
    }

    return (false);
}

std::vector<string> DCBOV::getDataVarNames() const
{
    map<string, DC::DataVar>::const_iterator itr;

    vector<string> names;
    for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

std::vector<string> DCBOV::getCoordVarNames() const
{
    map<string, DC::CoordVar>::const_iterator itr;

    vector<string> names;
    for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

template<class T> bool DCBOV::_getAttTemplate(string varname, string attname, T &values) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (status);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (status);

    att.GetValues(values);

    return (true);
}

bool DCBOV::getAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCBOV::getAtt(string varname, string attname, vector<long> &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

bool DCBOV::getAtt(string varname, string attname, string &values) const
{
    values.clear();

    return (_getAttTemplate(varname, attname, values));
}

std::vector<string> DCBOV::getAttNames(string varname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (vector<string>());

    vector<string> names;

    const std::map<string, Attribute> &         atts = var.GetAttributes();
    std::map<string, Attribute>::const_iterator itr;
    for (itr = atts.begin(); itr != atts.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}

DC::XType DCBOV::getAttType(string varname, string attname) const
{
    DC::BaseVar var;
    bool        status = getBaseVarInfo(varname, var);
    if (!status) return (DC::INVALID);

    DC::Attribute att;
    status = var.GetAttribute(attname, att);
    if (!status) return (DC::INVALID);

    return (att.GetXType());
}

int DCBOV::getDimLensAtLevel(string varname, int, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
{
    dims_at_level.clear();
    bs_at_level.clear();

    bool ok = GetVarDimLens(varname, true, dims_at_level);
    if (!ok) {
        SetErrMsg("Undefined variable name : %s", varname.c_str());
        return (-1);
    }

    // Never blocked
    //
    bs_at_level = vector<size_t>(dims_at_level.size(), 1);

    return (0);
}

int DCBOV::openVariableRead(size_t ts, string varname)
{
    _varname = varname;
    std::cout << "int DCBOV::openVariableRead(size_t ts, string varname) " << varname << std::endl;
    // return 0;
    // int aux = _ncdfc->OpenRead(ts, varname);

    // if (aux < 0) return (aux);

    FileTable::FileObject *f = new FileTable::FileObject(ts, varname, 0, 0, 0);    // aux);
    return (_fileTable.AddEntry(f));
}

int DCBOV::closeVariable(int fd)
{
    DC::FileTable::FileObject *w = _fileTable.GetEntry(fd);

    std::cout << "closeVariable( " << w->GetVarname() << std::endl;

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    // int aux = w->GetAux();

    // int rc = _ncdfc->Close(aux);

    _fileTable.RemoveEntry(fd);

    // return (rc);*/
    return 0;
}

size_t DCBOV::_sizeOfFormat( DC::XType type ) const {
    switch (type) {
        case DC::XType::INVALID: return -1;
        case DC::XType::INT8: return 1;
        case DC::XType::INT32: return 4;
        case DC::XType::FLOAT: return 4;
        case DC::XType::DOUBLE: return 8;
    }
}

void DCBOV::_swapBytes(void *vptr, size_t size, size_t n) const
{
    unsigned char *ucptr = (unsigned char *)vptr;
    unsigned char  uc;
    size_t         i, j;

    for (j = 0; j < n; j++) {
        for (i = 0; i < size / 2; i++) {
            uc = ucptr[i];
            ucptr[i] = ucptr[size - i - 1];
            ucptr[size - i - 1] = uc;
        }
        ucptr += size;
    }
}

template<class T> int DCBOV::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    for (int i=0; i<min.size(); i++) {
        std::cout << "_readRegionTemplate " << min[i] << " " << max[i] << std::endl;
    }

    FileTable::FileObject *w = (FileTable::FileObject *)_fileTable.GetEntry(fd);
    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    std::string varname = w->GetVarname();

    std::string              fileName    = _bovCollection->GetDataFile();
    std::vector<std::string> spatialDims = _bovCollection->GetSpatialDimensions();
    std::vector<size_t>      dataSize    = _bovCollection->GetDataSize();
    std::vector<float>       origin      = _bovCollection->GetBrickOrigin();
    std::vector<float>       brickSize   = _bovCollection->GetBrickSize();

    // Return spatial coordinate variable values
    for (int dim = 0; dim < spatialDims.size(); dim++) {
        if (varname == spatialDims[dim]) {
            float increment = brickSize[dim] / dataSize[dim];
            for (int i = 0; i < dataSize[dim]; i++) { region[i] = origin[dim] + i * increment; }
        }
    }

    if (_varname == "t") {
        region[0] = 1.f;
    } else if (_varname == _bovCollection->GetDataVariableName()) {
        //_bovCollection->ReadRegion( min, max, region );
        FILE* fp = fopen( fileName.c_str(), "rb" );
        if (!fp) {
            SetErrMsg("Invalid file: %d", fp);
            return (-1);
        }
       
        size_t formatSize = _sizeOfFormat( _bovCollection->GetDataFormat() );
        std::cout << "formatSize " << formatSize << std::endl;
        if ( formatSize < 0 ) {
            SetErrMsg("Invalid data format");
            return (-1);
        }

        std::vector<size_t> gridPts = _bovCollection->GetDataSize();
        if ( gridPts.size() != 3 ) {
            SetErrMsg("Invalid grid size (must be 3D)");
            return (-1);
        }
        size_t numValues = gridPts[0]*gridPts[1]*gridPts[2];
        if ( numValues < 1 ) {
            SetErrMsg("Invalid number of grid points (must be greater than 0)");
            return (-1);
        }

        std::cout << typeid(*region).name() << std::endl;

        size_t xStride = max[0]-min[0]+1; 
        for (int k=min[2]; k<=max[2]; k++) {
            int zOffset = dataSize[0]*dataSize[1]*k;
            for (int j=min[1]; j<=max[1]; j++) {
                int xOffset = min[0];
                int yOffset = dataSize[0]*j;
                int offset = formatSize*(xOffset + yOffset + zOffset);
                
                fseek( fp, offset, SEEK_SET );
                size_t rc = fread( region, formatSize, xStride, fp );

                if (rc != xStride) {
                    if (ferror(fp) != 0) {
                        MyBase::SetErrMsg("Error reading input file");
                    } else {
                        MyBase::SetErrMsg("Short read on input file");
                    }
                    return (-1);
                }

                region+=xStride;
            }
        }
        
        fclose(fp);

        int n = 1;
        bool systemLittleEndian = *(char *)&n == 1 ? true : false;
        bool dataLittleEndian = _bovCollection->GetDataEndian() == "LITTLE" ? true : false;
        if ( systemLittleEndian != dataLittleEndian ) {
            _swapBytes( region, formatSize, numValues );
        }
        //for (int i = 0; i < 1000; i++) region[i] = float(i);
    }
    


    /*FileTable::FileObject *w = (FileTable::FileObject *)_fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    vector<size_t> ncdf_start = min;
    reverse(ncdf_start.begin(), ncdf_start.end());

    vector<size_t> ncdf_max = max;
    reverse(ncdf_max.begin(), ncdf_max.end());

    vector<size_t> ncdf_count;
    for (int i = 0; i < ncdf_start.size(); i++) { ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1); }

    return (_ncdfc->Read(ncdf_start, ncdf_count, region, aux));*/
}

bool DCBOV::variableExists(size_t ts, string varname, int, int) const {
    // std::cout << ts << " " << varname << std::endl;
    return true;
    // return (_ncdfc->VariableExists(ts, varname));
}
