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

    if (_bovCollection != nullptr) {
        delete _bovCollection;
    }
}

int DCBOV::initialize(const vector<string> &paths, const std::vector<string> &options)
{
    /*for (int i=0; i<paths.size(); i++)
        std::cout << paths[i] << std::endl;
    for (int i=0; i<options.size(); i++)
        std::cout << options[i] << std::endl;*/

    _bovCollection = new BOVCollection();
    int rc = _bovCollection->Initialize( paths );
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
    //std::vector<std::string> dimnames = {"t","z","y","x"};
    //std::vector<size_t>      dimlens  = {1,10,10,10};
    std::vector<std::string> dimnames = {"x","y","z","t"};
    std::vector<size_t>      dimlens  = {10,10,10,1};
    //std::vector<std::string> dimnames = {"x","y","z"};
    //std::vector<size_t>      dimlens  = {10,10,10};
    for (int i = 0; i < dimnames.size(); i++) {
        Dimension dim(dimnames[i], dimlens[i]);
        _dimsMap[dimnames[i]] = dim;
    }
}

int DCBOV::_InitCoordinates()
{
    //std::vector<std::string> cvars = {"x","y","z","t"};
    std::vector<std::string> spatialVars = {"x","y","z"};

    // See if the variable has uniform spacing. If so, set the uniform hint
    //
    bool uniformHint = true;

    // Finally, add the variable to _coordVarsMap.
    //
    //std::vector<string> dimnames = {"x", "y", "z", "t"};
    std::vector<string> dimnames = {"x", "y", "z"};
    /*for (std::map<string,DC::Dimension>::iterator it = _dimsMap.begin(); it!=_dimsMap.end(); ++it ) {
        std::cout << it->first << std::endl;
        dimnames.push_back(it->first);
    }*/
    vector<bool> periodic(false);

    std::string units = "m";

    DC::Attribute unitAttribute( "units", DC::TEXT, "m" );
   
    std::vector<DC::Attribute> axisAttributes = { 
        DC::Attribute( "axis", DC::TEXT, "X" ),
        DC::Attribute( "axis", DC::TEXT, "Y" ),
        DC::Attribute( "axis", DC::TEXT, "Z" ),
    };

    for (int i=0; i<spatialVars.size(); i++) {
        //_coordVarsMap[spatialVars[i]] = CoordVar(spatialVars[i], units, DC::FLOAT, periodic, i, uniformHint, {dimnames[i]}, "t");
        _coordVarsMap[spatialVars[i]] = CoordVar(spatialVars[i], units, DC::FLOAT, periodic, i, uniformHint, {dimnames[i]}, "");
        _coordVarsMap[spatialVars[i]].SetAttribute( axisAttributes[i] );
        _coordVarsMap[spatialVars[i]].SetAttribute( unitAttribute );
    }

    //_coordVarsMap["t"] = CoordVar("t", "seconds since 2000-1-1 0:0:0", DC::FLOAT, periodic, 3, uniformHint, dimnames, "t");
    _coordVarsMap["t"] = CoordVar("t", "seconds since 2000-1-1 0:0:0", DC::FLOAT, periodic, 3, true, {}, "t");
    _coordVarsMap["t"].SetAttribute( DC::Attribute( "units", DC::TEXT, "seconds since 2000-1-1 0:0:0" ) );
    _coordVarsMap["t"].SetAttribute( DC::Attribute( "axis",  DC::TEXT, "T" ) );
 
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

    vector<string> vars = {"myVar"};

    // For each variable add a member to _dataVarsMap
    //
    for (int i = 0; i < vars.size(); i++) {
        vector<string> sdimnames = {"x","y","z"};
        vector<string> scoordvars = {"x", "y", "z"};
        //vector<string> sdimnames;

        //string         time_dim_name = "t";
        //string         time_coordvar = "t";
        string         time_dim_name = "";
        string         time_coordvar = "";

        string units = "m";

        Mesh mesh("myMesh", sdimnames, scoordvars);

        // Create new mesh. We're being lazy here and probably should only
        // createone if it doesn't ready exist
        //
        _meshMap[mesh.GetName()] = mesh;

        _dataVarsMap[vars[i]] = DataVar(vars[i], units, DC::FLOAT, periodic, mesh.GetName(), time_coordvar, DC::Mesh::NODE);
        _dataVarsMap[vars[i]].SetAttribute( DC::Attribute( "standard_name", DC::TEXT, vars[i] ) );
        _dataVarsMap[vars[i]].SetAttribute( DC::Attribute( "units", DC::TEXT, units ) );
    }

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
    //return 0;
    //int aux = _ncdfc->OpenRead(ts, varname);

    //if (aux < 0) return (aux);

    FileTable::FileObject *f = new FileTable::FileObject(ts, varname, 0, 0, 0);//aux);
    return (_fileTable.AddEntry(f));
}

int DCBOV::closeVariable(int fd)
{
    /*DC::FileTable::FileObject *w = _fileTable.GetEntry(fd);

    if (!w) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }
    int aux = w->GetAux();

    int rc = _ncdfc->Close(aux);

    _fileTable.RemoveEntry(fd);

    return (rc);*/
    return 0;
}

template<class T> int DCBOV::_readRegionTemplate(int fd, const vector<size_t> &min, const vector<size_t> &max, T *region)
{
    if ( _varname == "x" || _varname == "y" || _varname == "z" ) {
        for (int i=0; i<10; i++) region[i] = float(i);
    }
    else if ( _varname == "t" ) {
        region[0] = 1.f;
    }
    else if ( _varname == "myVar" ) {
        for (int i=0; i<1000; i++)
            region[i] = float(i);
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
    //std::cout << ts << " " << varname << std::endl;
    return true;
    //return (_ncdfc->VariableExists(ts, varname)); 
}

BOVCollection::BOVCollection() : 
    _dataFormat(DC::FLOAT),
    _dataFile(""),
    _dataEndian(""),
    _centering(""),
    _byteOffset(0),
    _divideBrick(false),
    _dataComponents(1) 
{
    _files.clear();
    _dataSize.clear();
    _brickOrigin.clear();
    _brickSize.clear();
    _dataBricklets.clear();
}

int BOVCollection::Initialize( const std::vector<std::string> &paths ) {
    VAssert( paths.size() == 1 );

    size_t pos;
    std::string line;
    std::ifstream header;
    header.open( paths[0] );
    if( header.is_open() ) {
        while( getline( header, line ) ) {
            _readMetadataT("TIME", line, _time);
            _readMetadataT("DATA_FILE", line, _dataFile);
            _readMetadataT("DATA_SIZE", line, _dataSize);
            _readMetadataT("DATA_FORMAT", line, _dataFormat);
            _readMetadataT("VARIABLE", line, _variable);
            _readMetadataT("DATA_ENDIAN", line, _dataEndian);
            _readMetadataT("CENTERING", line, _centering);
            _readMetadataT("BRICK_ORIGIN", line, _brickSize);
            _readMetadataT("BRICK_SIZE", line, _brickSize);
            _readMetadataT("BYTE_OFFSET", line, _byteOffset);
            _readMetadataT("DIVIDE_BRICK", line, _divideBrick);
            _readMetadataT("DATA_BRICKLETS", line, _dataBricklets);
            _readMetadataT("DATA_COMPONENTS", line, _dataComponents);
        }
    }
    else {
        //SetErrMsg("Failed to open %s", paths[0]);
        SetErrMsg("Failed to open bov file");
    }
}

int BOVCollection::_readMetadata( const std::string &token, std::string &line, bool &value, bool verbose ) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find(token);
    if( pos != std::string::npos ) {  // Found the token
        value = _findValue(line) == "true" ? true : false;
        if (verbose) {
            std::cout << token << " " << _divideBrick << std::endl;
        }
    }
    return 0;
}

template<>
int BOVCollection::_readMetadataT<DC::XType>( const std::string &token, std::string &line, DC::XType &value, bool verbose ) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if( pos != std::string::npos ) {
        std::string format = _findValue( line );
        if      (format == "BYTE"  ) _dataFormat = DC::INT8;
        else if (format == "SHORT" ) _dataFormat = DC::INVALID;  // No XType for 16bit short
        else if (format == "INT"   ) _dataFormat = DC::INT32;
        else if (format == "FLOAT" ) _dataFormat = DC::FLOAT;
        else if (format == "DOUBLE") _dataFormat = DC::DOUBLE;
        else                         _dataFormat = DC::INVALID;

        if (verbose) {
            std::cout << token << " " << _dataFormat << std::endl;
        }

        if ( _dataFormat == DC::INVALID ) {
            SetErrMsg("Invalid BOV data format.  Must be either BYTE,SHORT,INT,FLOAT, or DOUBLE.");
            return -1;
        return 0;
        }
    }
}

int BOVCollection::_readMetadata( const std::string &token, std::string &line, std::string &value, bool verbose ) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if ( pos != std::string::npos ) { // We found the token
        value = _findValue( line );
        if (verbose ) {
            std::cout << token << " " << value << std::endl;
        }
        return 0;
    }
    return 0; // String assignment is no-fail
}

int BOVCollection::_readMetadata( const std::string &token, std::string &line, int &value, bool verbose ) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if ( pos != std::string::npos ) { // We found the token
        try {
            value = stoi( _findValue( line ) );
            if (verbose ) {
                std::cout << token << " " << value << std::endl;
            }
            return 0;
        }
        catch (const std::invalid_argument& ia) {
            std::string message = "Invalid integer value for " + token + " in BOV header file";
            SetErrMsg(message.c_str());
            return -1;
        }
    }
    return 0; // String assignment is no-fail
}

template<typename T>
int BOVCollection::_readMetadataT( const std::string &token, std::string &line, T &value, bool verbose ) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if ( pos != std::string::npos ) { // We found the token
        stringstream ss( _findValue( line ) );
        if ( std::is_same<T, bool>::value ) {
            ss >> std::boolalpha >> value;
        }
        else {
            ss >> value;
        }

        if (verbose ) {
            std::cout << token << " --- " << value << std::endl;
        }
        if (ss.bad()) {
            std::cout << "              FAIL" << std::endl;
            std::string message = "Invalid value for " + token + " in BOV header file";
            SetErrMsg(message.c_str());
            return -1;
        }
    }
    return 0; 
}

template<typename T>
int BOVCollection::_readMetadataT( const std::string & token, std::string &line, std::vector<T> &value, bool verbose) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if( pos != std::string::npos ) { // We found the token
        T lineValue;
        std::stringstream lineStream = stringstream( _findValue( line ) );
        std::cout << token << " " << lineStream.str() << std::endl;
        while( lineStream >> lineValue ) {
            value.push_back( lineValue );
        }
        
        if ( lineStream.bad() ) {
            value.clear();
            std::cout << "              FAIL v " << token << std::endl;
            std::string message = "Invalid value for " + token + " in BOV header file";
            SetErrMsg(message.c_str());
            return -1;
        }

        if (verbose) {
            std::cout << token << " ";
            for (int i=0; i<value.size();i++)
                std:: cout << value[i] << "v";
            std::cout << std::endl;
        }
    }
    return 0;
}

int BOVCollection::_readMetadata( const std::string & token, std::string &line, std::vector<int> &value, bool verbose) {
    // Skip comments
    if (line[0] == '#') {
        return 0;
    }

    size_t pos = line.find( token );
    if( pos != std::string::npos ) { // We found the token
        std::string stringValue;
        std::stringstream ss = stringstream( _findValue( line ) );
        while( std::getline( ss, stringValue, ' ' )) {  // Split the stringstream by ' '
            try {
                value.push_back( stoi(stringValue) );
            }
            catch (const std::invalid_argument& ia) {
                std::string message = "Invalid value for " + token + " in BOV header file";
                value.clear();
                return -1;
            }
        }

        if (verbose) {
        std::cout << token << " ";
        for (int i=0; i<value.size();i++)
            std:: cout << value[i] << " ";
        std::cout << std::endl;
        }
    }
    return 0;
}

std::string BOVCollection::_findValue( std::string &line ) const {
    std::string delimiter = ": ";

    size_t pos=0;
    std::string token;
    while(( pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        line.erase(0, pos+delimiter.length());
    } 
    return line; 
}
