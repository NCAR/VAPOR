#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <type_traits>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/BOVCollection.h>

using namespace VAPoR;

const std::string BOVCollection::_timeToken = "TIME";
const std::string BOVCollection::_dataFileToken = "DATA_FILE";
const std::string BOVCollection::_dataSizeToken = "DATA_SIZE";
const std::string BOVCollection::_formatToken = "DATA_FORMAT";
const std::string BOVCollection::_variableToken = "VARIABLE";
const std::string BOVCollection::_endianToken = "DATA_ENDIAN";
const std::string BOVCollection::_centeringToken = "CENTERING";
const std::string BOVCollection::_originToken = "BRICK_ORIGIN";
const std::string BOVCollection::_brickSizeToken = "BRICK_SIZE";
const std::string BOVCollection::_offsetToken = "BYTE_OFFSET";
const std::string BOVCollection::_divideBrickToken = "DIVIDE_BRICK";
const std::string BOVCollection::_dataBrickletsToken = "DATA_BRICKLETS";
const std::string BOVCollection::_dataComponentsToken = "DATA_COMPONENTS";

const std::string BOVCollection::_xDim = "x";
const std::string BOVCollection::_yDim = "y";
const std::string BOVCollection::_zDim = "z";
const std::string BOVCollection::_timeDim = "t";

const std::string BOVCollection::_byteFormatString = "BYTE";
const std::string BOVCollection::_shortFormatString = "SHORT";
const std::string BOVCollection::_intFormatString = "INT";
const std::string BOVCollection::_floatFormatString = "FLOAT";
const std::string BOVCollection::_doubleFormatString = "DOUBLE";

BOVCollection::BOVCollection()
: _time("0"), _dataFile(""), _dataFormat(DC::XType::INVALID), _variable("brickVar"), _dataEndian("LITTLE"), _centering("ZONAL"), _byteOffset(0), _divideBrick(false), _dataComponents(1),
  _timeDimension(_timeDim)
{
    _dataSize.clear();
    _brickOrigin.resize(3, 0.);
    _brickSize.resize(3, 1.);
    _dataBricklets.clear();
    _spatialDimensions = {_xDim, _yDim, _zDim};
}

int BOVCollection::Initialize(const std::vector<std::string> &paths)
{
    VAssert(paths.size() == 1);

    int           rc;
    std::string   line;
    std::ifstream header;
    header.open(paths[0]);
    if (header.is_open()) {
        while (getline(header, line)) {
            // The _dataFile, _dataSize, and _dataFormat variables are all required to process
            // the BOV.  Try to find them, and report errors if we can't.
            //
            rc = _findToken(_dataFileToken, line, _dataFile);
            if (rc == -1) {
                SetErrMsg(("Failure reading BOV data file " + _dataFileToken).c_str());
                return -1;
            }

            rc = _findToken(_dataSizeToken, line, _dataSize);
            if (rc == -1) {
                SetErrMsg(("Failure reading BOV data size token " + _dataSizeToken).c_str());
                return -1;
            } else if (rc == 1) {
                if (_dataSize[0] < 1 || _dataSize[1] < 1 || _dataSize[2] < 1) {
                    SetErrMsg((_dataSizeToken + " must have all dimensions > 1").c_str());
                    return -1;
                }
            }

            rc = _findToken(_formatToken, line, _dataFormat);
            if (rc == 1 && _dataFormat == DC::INVALID) {
                std::string message = _formatToken + " must be either BYTE, SHORT, INT, FLOAT, or DOUBLE.";
                SetErrMsg(message.c_str());
                return -1;
            }

            // Optional tokens.  If their values are invalid, SetErrMsg, and return -1.
            //
            rc = _findToken(_originToken, line, _brickOrigin);
            if (rc == -1) { SetErrMsg(("Invalid value for token: " + _originToken).c_str()); }
            rc = _findToken(_brickSizeToken, line, _brickSize);
            if (rc == -1) { SetErrMsg(("Invalid value for token: " + _brickSizeToken).c_str()); }
            rc = _findToken(_endianToken, line, _dataEndian);
            if (rc == -1) { SetErrMsg(("Invalid value for token: " + _endianToken).c_str()); }
            rc = _findToken(_timeToken, line, _time);
            if (rc == -1) { SetErrMsg(("Invalid value for token: " + _timeToken).c_str()); }
            rc = _findToken(_variableToken, line, _variable);
            if (rc == -1) { SetErrMsg(("Invalid value for token: " + _variableToken).c_str()); }

            // All other variables are currently unused.
            //
            _findToken(_centeringToken, line, _centering);
            _findToken(_offsetToken, line, _byteOffset);
            _findToken(_divideBrickToken, line, _divideBrick);
            _findToken(_dataBrickletsToken, line, _dataBricklets);
            _findToken(_dataComponentsToken, line, _dataComponents);
        }
    } else {
        SetErrMsg(("Failed to open BOV file " + paths[0]).c_str());
        return -1;
    }

    // Ensure we have the required tokens
    //
    if (_dataFile == "") {
        SetErrMsg(("BOV file missing token  " + _dataFileToken).c_str());
        return -1;
    }
    if (_dataFormat == DC::XType::INVALID) {
        SetErrMsg(("BOV file missing token  " + _formatToken).c_str());
        return -1;
    }
    if (_dataSize.empty()) {
        SetErrMsg(("BOV file missing token  " + _dataSizeToken).c_str());
        return -1;
    }

    return 0;
}

std::string BOVCollection::GetDataFile() const { return _dataFile; }

std::string BOVCollection::GetDataVariableName() const { return _variable; }

std::vector<std::string> BOVCollection::GetSpatialDimensions() const { return _spatialDimensions; }

std::string BOVCollection::GetTimeDimension() const { return _timeDimension; }

std::string BOVCollection::GetUserTime() const { return _time; }

std::vector<size_t> BOVCollection::GetDataSize() const { return _dataSize; }

DC::XType BOVCollection::GetDataFormat() const { return _dataFormat; }

std::vector<float> BOVCollection::GetBrickOrigin() const { return _brickOrigin; }

std::vector<float> BOVCollection::GetBrickSize() const { return _brickSize; }

std::string BOVCollection::GetDataEndian() const { return _dataEndian; }

// Template specialization for reading data of type DC::XType
template<> int BOVCollection::_findToken<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return NOT_FOUND; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        std::string format = line;
        _findTokenValue(format);
        if (format == _intFormatString)
            _dataFormat = DC::INT32;
        else if (format == _floatFormatString)
            _dataFormat = DC::FLOAT;
        else if (format == _doubleFormatString)
            _dataFormat = DC::DOUBLE;
        else
            _dataFormat = DC::INVALID;

        if (verbose) { std::cout << std::setw(20) << token << " " << _dataFormat << std::endl; }
        return FOUND;
    }
    return NOT_FOUND;
}

// Template specialization for reading data of types bool or string
template<typename T> int BOVCollection::_findToken(const std::string &token, std::string &line, T &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return NOT_FOUND; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        _findTokenValue(line);
        stringstream ss(line);
        if (std::is_same<T, bool>::value) {
            ss >> std::boolalpha >> value;
        } else {
            ss >> value;
        }

        if (verbose) { std::cout << std::setw(20) << token << " " << value << std::endl; }

        if (ss.eof() == 0) {
            std::string message = "The keyword " + token + " may only contain one value.";
            SetErrMsg(message.c_str());
            return ERROR;
        }

        if (ss.bad()) {
            std::string message = "Invalid value for " + token + " in BOV header file.";
            SetErrMsg(message.c_str());
            return ERROR;
        }
        return FOUND;
    }
    return NOT_FOUND;
}

// Template specialization for reading data of type std::vector<int> or std::vector<float>
// All std::vectors returned must have a size of 3
template<typename T> int BOVCollection::_findToken(const std::string &token, std::string &line, std::vector<T> &value, bool verbose)
{
    // Skip comments
    if (line[0] == '#') { return NOT_FOUND; }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        T lineValue;
        _findTokenValue(line);
        std::stringstream lineStream(line);

        value.clear();
        while (lineStream >> lineValue) { value.push_back(lineValue); }

        if (value.size() != 3) {
            value.clear();
            std::string message = token + " must be a set of three values.";
            SetErrMsg(message.c_str());
            return ERROR;
        }

        if (lineStream.bad()) {
            value.clear();
            std::cout << "FAIL v " << token << std::endl;
            std::string message = "Invalid value for " + token + " in BOV header file.";
            SetErrMsg(message.c_str());
            return ERROR;
        }

        if (verbose) {
            std::cout << std::setw(20) << token << " ";
            for (int i = 0; i < value.size(); i++) std::cout << value[i] << " ";
            std::cout << std::endl;
        }
        return FOUND;
    }
    return NOT_FOUND;
}

void BOVCollection::_findTokenValue(std::string &line) const
{
    std::string delimiter = ": ";

    size_t      pos = 0;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        line.erase(0, pos + delimiter.length());
    }
}

size_t BOVCollection::_sizeOfFormat(DC::XType type) const
{
    switch (type) {
    case DC::XType::INT32: return 4;
    case DC::XType::FLOAT: return 4;
    case DC::XType::DOUBLE: return 8;
    default: return -1;
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

template<class T> int BOVCollection::ReadRegion(const std::vector<size_t> &min, const std::vector<size_t> &max, T region)
{
    FILE *fp = fopen(_dataFile.c_str(), "rb");
    if (!fp) {
        SetErrMsg("Invalid file: %d", fp);
        return -1;
    }

    size_t formatSize = _sizeOfFormat(_dataFormat);
    if (formatSize < 0) {
        SetErrMsg("Unspecified data format");
        return (-1);
    }

    if (_dataSize.size() != 3) {
        SetErrMsg("Invalid grid size (must be 3D)");
        return (-1);
    }
    size_t numValues = _dataSize[0] * _dataSize[1] * _dataSize[2];

    int  n = 1;
    bool systemLittleEndian = *(char *)&n == 1 ? true : false;
    bool dataLittleEndian = _dataEndian == "LITTLE" ? true : false;
    bool needSwap = systemLittleEndian != dataLittleEndian ? true : false;

    // Read a "pencil" of data along the X axis, one row at a time
    size_t count = max[0] - min[0] + 1;
    for (int k = min[2]; k <= max[2]; k++) {
        int zOffset = _dataSize[0] * _dataSize[1] * k;
        for (int j = min[1]; j <= max[1]; j++) {
            int xOffset = min[0];
            int yOffset = _dataSize[0] * j;
            int offset = formatSize * (xOffset + yOffset + zOffset);

            static Wasp::SmartBuf smart_buf;
            unsigned char *       readBuffer = (unsigned char *)smart_buf.Alloc(count * formatSize);

            if (needSwap) { _swapBytes(readBuffer, formatSize, numValues); }

            fseek(fp, offset, SEEK_SET);
            size_t rc = fread(readBuffer, formatSize, count, fp);

            if (rc != count) {
                if (ferror(fp) != 0) {
                    MyBase::SetErrMsg("Error reading input file: %M");
                } else {
                    MyBase::SetErrMsg("Short read on input file: %M");
                }
                return -1;
            }

            if (_dataFormat == DC::XType::INT32) {
                int *castBuffer = (int *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            } else if (_dataFormat == DC::XType::FLOAT) {
                float *castBuffer = (float *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            } else if (_dataFormat == DC::XType::DOUBLE) {
                double *castBuffer = (double *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            }
        }
    }

    fclose(fp);

    return 0;
}

// ReadRegion can only be used with int* float* and double*
template int BOVCollection::ReadRegion<int *>(const std::vector<size_t> &, const std::vector<size_t> &, int *);
template int BOVCollection::ReadRegion<float *>(const std::vector<size_t> &, const std::vector<size_t> &, float *);
template int BOVCollection::ReadRegion<double *>(const std::vector<size_t> &, const std::vector<size_t> &, double *);
