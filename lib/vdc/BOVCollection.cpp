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

BOVCollection::BOVCollection() : 
    _time("0"),
    _dataFile(""),
    _dataFormat(DC::FLOAT), 
    _variable("brickVar"),
    _dataEndian("LITTLE"), 
    _centering("ZONAL"), 
    _byteOffset(0), 
    _divideBrick(false), 
    _dataComponents(1),
    _timeDimension("t")
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

            rc = _readMetadata("DATA_FILE", line, _dataFile);
            if (rc < 0) {
                SetErrMsg("Failure reading .bov data file");
                return (-1);
            }

            rc = _readMetadata("DATA_SIZE", line, _dataSize);
            if (rc < 0) {
                SetErrMsg("Failure reading .bov data size");
                return (-1);
            }

            _readMetadata("TIME", line, _time);
            _readMetadata("DATA_FORMAT", line, _dataFormat);
            _readMetadata("VARIABLE", line, _variable);
            _readMetadata("DATA_ENDIAN", line, _dataEndian);
            _readMetadata("CENTERING", line, _centering);
            _readMetadata("BRICK_ORIGIN", line, _brickOrigin);
            _readMetadata("BRICK_SIZE", line, _brickSize);
            _readMetadata("BYTE_OFFSET", line, _byteOffset);
            _readMetadata("DIVIDE_BRICK", line, _divideBrick);
            _readMetadata("DATA_BRICKLETS", line, _dataBricklets);
            _readMetadata("DATA_COMPONENTS", line, _dataComponents);
        }
    } else {
        // SetErrMsg("Failed to open %s", paths[0]);
        SetErrMsg("Failed to open bov file");
    }
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

template <typename T>
int BOVCollection::ReadRegion( std::string varname, T *region ) {
    return 0;
}

template<> int BOVCollection::_readMetadata<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return 0; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        std::string format = _findValue(line);
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
        stringstream ss(_findValue(line));
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
        std::stringstream lineStream = stringstream(_findValue(line));
        
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

std::string BOVCollection::_findValue(std::string &line) const
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
