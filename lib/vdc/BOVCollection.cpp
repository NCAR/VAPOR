#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/BOVCollection.h>

using namespace VAPoR;

const std::string BOVCollection::_timeToken           = "TIME";
const std::string BOVCollection::_dataFileToken       = "DATA_FILE";
const std::string BOVCollection::_dataSizeToken       = "DATA_SIZE";
const std::string BOVCollection::_formatToken         = "DATA_FORMAT";
const std::string BOVCollection::_variableToken       = "VARIABLE";
const std::string BOVCollection::_endianToken         = "DATA_ENDIAN";
const std::string BOVCollection::_centeringToken      = "CENTERING";
const std::string BOVCollection::_originToken         = "BRICK_ORIGIN";
const std::string BOVCollection::_sizeToken           = "BRICK_SIZE";
const std::string BOVCollection::_offsetToken         = "BYTE_OFFSET";
const std::string BOVCollection::_divideBrickToken    = "DIVIDE_BRICK";
const std::string BOVCollection::_dataBrickletsToken  = "DATA_BRICKLETS";
const std::string BOVCollection::_dataComponentsToken = "DATA_COMPONENTS";

BOVCollection::BOVCollection()
: _time("0"), _dataFile(""), _dataFormat(DC::FLOAT), _variable("brickVar"), _dataEndian("LITTLE"), _centering("ZONAL"), _byteOffset(0), _divideBrick(false), _dataComponents(1), _timeDimension("t")
{
    _dataSize.clear();
    _brickOrigin.resize(3, 0.);
    _brickSize.resize(3, 1.);
    _dataBricklets.clear();
    _spatialDimensions = {"x", "y", "z"};
}

int BOVCollection::Initialize(const std::vector<std::string> &paths)
{
    VAssert(paths.size() == 1);

    int           rc;
    size_t        pos;
    std::string   line;
    std::ifstream header;
    header.open(paths[0]);
    if (header.is_open()) {
        while (getline(header, line)) {
            rc = _readMetadata(_dataFileToken, line, _dataFile);
            if (rc < 0) {
                SetErrMsg(("Failure reading .bov data file " + _dataFileToken).c_str());
                return (-1);
            }

            rc = _readMetadata(_dataSizeToken, line, _dataSize);
            if (rc < 0) {
                SetErrMsg(("Failure reading .bov data size token " + _dataSizeToken).c_str());
                return (-1);
            }

            rc = _readMetadata(_formatToken, line, _dataFormat);
            if (rc < 0) {
                SetErrMsg(("Failure reading .bov data format token " + _formatToken).c_str());
                return (-1);
            }

            _readMetadata(_timeToken, line, _time);
            _readMetadata(_variableToken, line, _variable);
            _readMetadata(_endianToken, line, _dataEndian);
            _readMetadata(_centeringToken, line, _centering);
            _readMetadata(_originToken, line, _brickOrigin);
            _readMetadata(_sizeToken, line, _brickSize);
            _readMetadata(_offsetToken, line, _byteOffset);
            _readMetadata(_divideBrickToken, line, _divideBrick);
            _readMetadata(_dataBrickletsToken, line, _dataBricklets);
            _readMetadata(_dataComponentsToken, line, _dataComponents);
        }
    } else {
        SetErrMsg(("Failed to open bov file " + paths[0]).c_str());
    }
}

std::string BOVCollection::GetDataFile() const { 
    return _dataFile; 
}

std::string BOVCollection::GetDataVariableName() const { 
    return _variable; 
}

std::vector<std::string> BOVCollection::GetSpatialDimensions() const { 
    return _spatialDimensions; 
}

std::string BOVCollection::GetTimeDimension() const { 
    return _timeDimension; 
}

std::vector<size_t> BOVCollection::GetDataSize() const { 
    return _dataSize; 
}

DC::XType BOVCollection::GetDataFormat() const { 
    return _dataFormat; 
}

std::vector<float> BOVCollection::GetBrickOrigin() const { 
    return _brickOrigin; 
}

std::vector<float> BOVCollection::GetBrickSize() const { 
    return _brickSize; 
}

std::string BOVCollection::GetDataEndian() const { 
    return _dataEndian; 
}

/*template<typename T> int BOVCollection::ReadRegion(std::string varname, T *region) { 
    return 0; 
}*/

template<> int BOVCollection::_readMetadata<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return 0; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        std::string format = _findTokenValue(line);
        if (format == "BYTE")
            _dataFormat = DC::INT8;
        else if (format == "SHORT")
            _dataFormat = DC::INVALID;    // No XType for 16bit short
        else if (format == "INT")
            _dataFormat = DC::INT32;
        else if (format == "FLOAT")
            _dataFormat = DC::FLOAT;
        else if (format == "DOUBLE")
            _dataFormat = DC::DOUBLE;
        else
            _dataFormat = DC::INVALID;

        if (verbose) { std::cout << std::setw(20) << token << " " << _dataFormat << std::endl; }

        if (_dataFormat == DC::INVALID) {
            SetErrMsg("Invalid BOV data format.  Must be either BYTE,SHORT,INT,FLOAT, or DOUBLE.");
            return -1;
        }
        return 0;
    }
}

template<typename T> int BOVCollection::_readMetadata(const std::string &token, std::string &line, T &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return 0; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        stringstream ss(_findTokenValue(line));
        if (std::is_same<T, bool>::value) {
            ss >> std::boolalpha >> value;
        } else {
            ss >> value;
        }

        if (verbose) { std::cout << std::setw(20) << token << " " << value << std::endl; }
        if (ss.bad()) {
            std::string message = "Invalid value for " + token + " in BOV header file.";
            SetErrMsg(message.c_str());
            return -1;
        }
    }
    return 0;
}

template<typename T> int BOVCollection::_readMetadata(const std::string &token, std::string &line, std::vector<T> &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return 0; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        T                 lineValue;
        std::stringstream lineStream = stringstream(_findTokenValue(line));

        value.clear();
        while (lineStream >> lineValue) { value.push_back(lineValue); }

        if (lineStream.bad() || value.size() != 3) {
            value.clear();
            std::cout << "FAIL v " << token << std::endl;
            std::string message = "Invalid value for " + token + " in BOV header file.";
            SetErrMsg(message.c_str());
            return -1;
        }

        if (verbose) {
            std::cout << std::setw(20) << token << " ";
            for (int i = 0; i < value.size(); i++) std::cout << value[i] << " ";
            std::cout << std::endl;
        }
    }
    return 0;
}

std::string BOVCollection::_findTokenValue(std::string &line) const
{
    std::string delimiter = ": ";

    size_t      pos = 0;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        line.erase(0, pos + delimiter.length());
    }
    return line;
}

/*size_t BOVCollection::_sizeOfFormat( DC::XType type ) const {
    switch (type) {
        case DC::XType::INVALID: return -1;
        case DC::XType::INT8: return 1;
        case DC::XType::INT32: return 4;
        case DC::XType::FLOAT: return 4;
        case DC::XType::DOUBLE: return 8;
    }
}

void BOVCollection::_swapBytes(void *vptr, size_t size, size_t n) const
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

template<class T>
int BOVCollection::ReadRegion( const std::vector<size_t> &min, const std::vector<size_t> &max, T *region ) {
    FILE* fp = fopen( _dataFile.c_str(), "rb" );
    if (!fp) {
        SetErrMsg("Invalid file: %d", fp);
        return (-1);
    }

    size_t dataSize = _sizeOfFormat( _dataFormat );
    std::cout << "dataSize " << dataSize << std::endl;
    if ( dataSize < 0 ) {
        SetErrMsg("Invalid data format");
        return (-1);
    }

    if ( _dataSize.size() != 3 ) {
        SetErrMsg("Invalid grid size (must be 3D)");
        return (-1);
    }
    size_t numValues = _dataSize[0]*_dataSize[1]*_dataSize[2];
    if ( numValues < 1 ) {
        SetErrMsg("Invalid number of grid points");
        return (-1);
    }

    std::cout << typeid(*region).name() << std::endl;

    size_t rc = fread( region, dataSize, numValues, fp );
    if (rc != numValues) {
        if (ferror(fp) != 0) {
            MyBase::SetErrMsg("Error reading input file");
        } else {
            MyBase::SetErrMsg("Short read on input file");
        }
        return (-1);
    }

    fclose(fp);

    int n = 1;
    bool systemLittleEndian = *(char *)&n == 1 ? true : false;
    bool dataLittleEndian = _dataEndian == "LITTLE" ? true : false;
    if ( systemLittleEndian != dataLittleEndian ) {
        _swapBytes( region, dataSize, numValues );
    }
    //for (int i = 0; i < 1000; i++) region[i] = float(i);

    return 0;
}*/
