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

template<typename T> int BOVCollection::ReadRegion(std::string varname, T *region) { 
    return 0; 
}

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
            std::cout << "FAIL T" << std::endl;
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
