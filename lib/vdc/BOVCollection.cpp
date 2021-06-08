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
const std::string BOVCollection::_gridSizeToken = "DATA_SIZE";
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

const std::vector<double> BOVCollection::_defaultOrigin = {0., 0., 0.};
const std::vector<double> BOVCollection::_defaultBrickSize = {1., 1., 1.};
const std::vector<size_t> BOVCollection::_defaultBricklets = {0, 0, 0};
const std::vector<size_t> BOVCollection::_defaultGridSize = {0, 0, 0};
const DC::XType           BOVCollection::_defaultFormat = DC::XType::INVALID;
const std::string         BOVCollection::_defaultFile = "";
const std::string         BOVCollection::_defaultVar = "brickVar";
const std::string         BOVCollection::_defaultEndian = "LITTLE";
const std::string         BOVCollection::_defaultCentering = "ZONAL";
const double              BOVCollection::_defaultTime = 0.;
const size_t              BOVCollection::_defaultOffset = 0;
const size_t              BOVCollection::_defaultComponents = 1;
const bool                BOVCollection::_defaultDivBrick = false;

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
: _time(_defaultTime), _dataFile(_defaultFile), _dataFormat(_defaultFormat), _variable(_defaultVar), _dataEndian(_defaultEndian), _centering(_defaultCentering), _byteOffset(_defaultOffset),
  _divideBrick(_defaultDivBrick), _dataComponents(_defaultComponents), _timeDimension(_timeDim), _gridSizeAssigned(false), _formatAssigned(false), _brickOriginAssigned(false),
  _brickSizeAssigned(false), _dataEndianAssigned(false)
{
    _dataFiles.clear();
    _times.clear();
    _gridSize = _defaultGridSize;
    _brickOrigin = _defaultOrigin;
    _brickSize = _defaultBrickSize;
    _dataBricklets = _defaultBricklets;
    _spatialDimensions = {_xDim, _yDim, _zDim};
}

int BOVCollection::Initialize(const std::vector<std::string> &paths)
{
    VAssert(paths.size() > 0);

    int           rc;
    std::ifstream header;
    for (int i = 0; i < paths.size(); i++) {
        header.open(paths[i]);
        if (header.is_open()) {
            rc = _parseHeader(header);
            if (rc < 0) {
                SetErrMsg(("Error parsing BOV file " + paths[0]).c_str());
                return -1;
            }

            // Ensure we have the required tokens in the header
            //
            if (_dataFile == _defaultFile) { return _missingValueError(_dataFileToken); }
            if (_dataFormat == _defaultFormat) { return _missingValueError(_formatToken); }
            if (_gridSize == _defaultGridSize) { return _missingValueError(_gridSizeToken); }

            _variables.push_back(_variable);
            _times.push_back(_time);
            size_t ts = _times.size() - 1;
            _dataFileMap[_variable][ts] = _dataFile;
        } else {
            SetErrMsg(("Failed to open BOV file " + paths[0]).c_str());
            return -1;
        }
        header.close();
    }


    std::sort(_times.begin(), _times.end());


    return 0;
}

int BOVCollection::_parseHeader(std::ifstream &header)
{
    int         rc;
    std::string line;
    std::string dataFile;
    while (getline(header, line)) {
        // The _dataFile, _gridSize, and _dataFormat variables are all required to process
        // the BOV.  Try to find them, and report errors if we can't.
        //
        rc = _findToken(_dataFileToken, line, dataFile);
        if (rc == ERROR)
            return _failureToReadError(_dataFileToken);
        else if (rc == FOUND)
            _dataFile = dataFile;
        //_dataFiles.push_back( dataFile );

        double time;
        rc = _findToken(_timeToken, line, time);
        if (rc == ERROR)
            return _failureToReadError(_timeToken);
        else if (rc == FOUND)
            _time = time;
        //_times.push_back( time );

        std::string variable;
        rc = _findToken(_variableToken, line, variable);
        if (rc == ERROR)
            return _invalidValueError(_variableToken);
        else if (rc == FOUND)
            _variable = variable;
        //_variables.push_back(variable);

        std::vector<size_t> gridSize;
        rc = _findToken(_gridSizeToken, line, gridSize);
        if (rc == ERROR) {
            return _failureToReadError(_gridSizeToken);
        } else if (rc == FOUND) {
            if (gridSize[0] < 1 || gridSize[1] < 1 || gridSize[2] < 1)
                return _invalidDimensionError(_gridSizeToken);
            else if (gridSize != _gridSize && _gridSizeAssigned == true)    // Make sure gridSize does not vary across files
                return _inconsistentValueError(_gridSizeToken);
            else {
                _gridSize = gridSize;
                _gridSizeAssigned = true;
            }
        }

        DC::XType dataFormat;
        rc = _findToken(_formatToken, line, dataFormat);
        if (rc == ERROR)
            return _failureToReadError(_formatToken);
        else if (rc == FOUND) {
            if (dataFormat == DC::INVALID)
                return _invalidFormatError(_formatToken);
            else if (dataFormat != _dataFormat && _formatAssigned == true) {    // Make sure dataFormat does not vary across files
                return _inconsistentValueError(_formatToken);
            } else {
                _dataFormat = dataFormat;
                _formatAssigned = true;
            }
        }

        // Optional tokens.  If their values are invalid, SetErrMsg, and return -1.
        //
        std::vector<double> brickOrigin;
        rc = _findToken(_originToken, line, brickOrigin);
        if (rc == ERROR)
            return _invalidValueError(_originToken);
        else if (rc == FOUND) {
            if (brickOrigin != _brickOrigin && _brickOriginAssigned == true)
                return _inconsistentValueError(_originToken);
            else {
                _brickOrigin = brickOrigin;
                _brickOriginAssigned = true;
            }
        }

        std::vector<double> brickSize;
        rc = _findToken(_brickSizeToken, line, brickSize);
        if (rc == ERROR)
            return _invalidValueError(_brickSizeToken);
        else if (rc == FOUND) {
            if (brickSize != _brickSize && _brickSizeAssigned == true)    // Make sure _brickSize does not vary across files
                return _inconsistentValueError(_brickSizeToken);
            else {
                _brickSize = brickSize;
                _brickSizeAssigned = true;
            }
        }

        std::string dataEndian;
        rc = _findToken(_endianToken, line, dataEndian);
        if (rc == ERROR)
            return _invalidValueError(_endianToken);
        else if (rc == FOUND) {
            if (dataEndian != _dataEndian && _dataEndianAssigned == true)    // Make sure _dataEndian does not vary across files
                return _inconsistentValueError(_endianToken);
            else {
                _dataEndian = dataEndian;
                _dataEndianAssigned = true;
            }
        }


        // All other variables are currently unused.
        //
        _findToken(_centeringToken, line, _centering);
        _findToken(_offsetToken, line, _byteOffset);
        _findToken(_divideBrickToken, line, _divideBrick);
        _findToken(_dataBrickletsToken, line, _dataBricklets);
        _findToken(_dataComponentsToken, line, _dataComponents);
    }
    return 0;
}

int BOVCollection::_missingValueError(std::string token) const
{
    SetErrMsg(("BOV file must contain token: " + token).c_str());
    return -1;
}

int BOVCollection::_invalidDimensionError(std::string token) const
{
    SetErrMsg((token + " must have all dimensions > 1").c_str());
    return -1;
}

int BOVCollection::_invalidFormatError(std::string token) const
{
    std::string message = token + " must be either INT, FLOAT, or DOUBLE.";
    SetErrMsg(message.c_str());
    return -1;
}

int BOVCollection::_failureToReadError(std::string token) const
{
    SetErrMsg(("Failure reading BOV time token: " + token).c_str());
    return -1;
}

int BOVCollection::_inconsistentValueError(std::string token) const
{
    SetErrMsg((token + " must be consistent in all BOV files").c_str());
    return -1;
}

int BOVCollection::_invalidValueError(std::string token) const
{
    SetErrMsg(("Invalid value for token: " + token).c_str());
    return -1;
}

std::string BOVCollection::GetDataFile() const { return _dataFile; }

std::string BOVCollection::GetDataVariableName() const { return _variable; }

std::vector<std::string> BOVCollection::GetSpatialDimensions() const { return _spatialDimensions; }

std::string BOVCollection::GetTimeDimension() const { return _timeDimension; }

double BOVCollection::GetUserTime() const { return _time; }

std::vector<double> BOVCollection::GetUserTimes() const { return _times; }

std::vector<size_t> BOVCollection::GetDataSize() const { return _gridSize; }

DC::XType BOVCollection::GetDataFormat() const { return _dataFormat; }

std::vector<double> BOVCollection::GetBrickOrigin() const { return _brickOrigin; }

std::vector<double> BOVCollection::GetBrickSize() const { return _brickSize; }

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
            value = DC::INT32;
        else if (format == _floatFormatString)
            value = DC::FLOAT;
        else if (format == _doubleFormatString)
            value = DC::DOUBLE;
        else
            value = DC::INVALID;

        if (verbose) { std::cout << std::setw(20) << token << " " << value << std::endl; }
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
            // if (value !=
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

// Template specialization for reading data of type std::vector<size_t> or std::vector<float>
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

template<class T> int BOVCollection::ReadRegion(std::string varname, size_t ts, const std::vector<size_t> &min, const std::vector<size_t> &max, T region)
{
    std::string dataFile = _dataFileMap[varname][ts];
    std::cout << ts << " " << varname << std::endl;
    for (int i = 0; i < _variables.size(); i++) {
        std::string var = _variables[i];
        for (int j = 0; j < _times.size(); j++) { std::cout << "    " << var << " " << _times[j] << " " << _dataFileMap[varname][j] << std::endl; }
    }
    std::cout << "        " << dataFile << std::endl;

    FILE *fp = fopen(dataFile.c_str(), "rb");
    if (!fp) {
        SetErrMsg("Invalid file: %d", fp);
        fclose(fp);
        return -1;
    }

    size_t formatSize = _sizeOfFormat(_dataFormat);
    if (formatSize < 0) {
        SetErrMsg("Unspecified data format");
        fclose(fp);
        return (-1);
    }

    if (_gridSize.size() != 3) {
        SetErrMsg("Invalid grid size (must be 3D)");
        fclose(fp);
        return (-1);
    }
    size_t numValues = _gridSize[0] * _gridSize[1] * _gridSize[2];

    int  n = 1;
    bool systemLittleEndian = *(char *)&n == 1 ? true : false;
    bool dataLittleEndian = _dataEndian == "LITTLE" ? true : false;
    bool needSwap = systemLittleEndian != dataLittleEndian ? true : false;

    // Read a "pencil" of data along the X axis, one row at a time
    size_t count = max[0] - min[0] + 1;
    for (int k = min[2]; k <= max[2]; k++) {
        int zOffset = _gridSize[0] * _gridSize[1] * k;
        for (int j = min[1]; j <= max[1]; j++) {
            int xOffset = min[0];
            int yOffset = _gridSize[0] * j;
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
                fclose(fp);
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
template int BOVCollection::ReadRegion<int *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, int *);
template int BOVCollection::ReadRegion<float *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, float *);
template int BOVCollection::ReadRegion<double *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, double *);
